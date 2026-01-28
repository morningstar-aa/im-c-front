#include "session_form.h"
#include "gui/session/atlist/at_list_form.h"
#include "module/session/session_manager.h"
#include "gui/profile_form/profile_form.h"
#include "gui/contact_select_form/contact_select_form.h"
#include "callback/team/team_callback.h"

using namespace ui;

namespace nim_comp
{

void SessionBox::InvokeGetTeamInfo(bool sync_block/* = false*/)
{
	if (sync_block)
	{
		//TODO(oleg) 测试用，不建议使用同步堵塞接口
		nim::TeamInfo info = nim::Team::QueryTeamInfoBlock(session_id_);
		OnGetTeamInfoCallback(session_id_, info);
	}
	else
		nim::Team::QueryTeamInfoAsync(session_id_, nbase::Bind(&SessionBox::OnGetTeamInfoCallback, this, std::placeholders::_1, std::placeholders::_2));

	nim::Team::QueryTeamInfoAsync(session_id_, nbase::Bind(&TeamCallback::QueryTeamInfoCallback, std::placeholders::_1, std::placeholders::_2));
}

void SessionBox::OnGetTeamInfoCallback(const std::string& tid, const nim::TeamInfo& result)
{
	team_info_ = result;
	
	QLOG_WAR(L"begin OnGetTeamInfoCallback mute_all_ ={0}") << team_info_.GetMuteType();

	std::wstring wname = nbase::UTF8ToUTF16(team_info_.GetName());
	if (!wname.empty())
	{
		SetTitleName(wname);
	}

	if (!team_info_.IsMemberValid())
	{
		HandleLeaveTeamEvent();
	}
	else if (!team_info_.IsValid())
	{
		HandleDismissTeamEvent();
	}
	else
	{
		HandleEnterTeamEvent();
	}

	if (team_info_.GetType() == nim::kNIMTeamTypeAdvanced)
	{
		if (session_form_)
		{
			session_form_->AdjustFormSize();
		}
		
		Json::Value json;
		if (StringToJson(team_info_.GetAnnouncement(), json) && json.isArray())
		{
			UpdateBroad(json);
		}
		else
		{
			edit_broad_->SetUTF8Text(team_info_.GetAnnouncement());
		}
		CheckTeamType(nim::kNIMTeamTypeAdvanced);
		
		if (isBoardVisible_)
		{
			std::string strBroad = edit_broad_->GetUTF8Text();//nbase::UTF8ToUTF16(msg.content_);
			BoardView::BoardInfo info;
			info.uuid = nbase::UTF8ToUTF16(tid);
			info.msg_body = strBroad;
			info.sender_name = nbase::UTF8ToUTF16(tid);
			board_view_->AddMessage(info, false);
			isBoardVisible_ = false;
		}		
	}

	if (team_info_.ExistValue(nim::kNIMTeamInfoKeyMuteAll)
		|| team_info_.ExistValue(nim::kNIMTeamInfoKeyMuteType))
	{
		QLOG_WAR(L"mute_all_ ={0}") << mute_all_;
		bool new_mute_all_status = team_info_.GetMuteType() != nim::kNIMTeamMuteTypeNone;
		mute_all_ = new_mute_all_status;
		QLOG_WAR(L"maute ={0}") << mute_all_;
	}

	is_header_enable_ = true;

	ShowEditControls();
	ResetNewBroadButtonVisible();
}

void SessionBox::InvokeGetTeamMember()
{
	btn_refresh_member_->SetEnabled(false);
	nim::Team::QueryTeamMembersAsync(session_id_, nbase::Bind(&SessionBox::OnGetTeamMemberCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void SessionBox::OnAddMember()
{
	std::string wnd_id = session_id_;

	if (wnd_id.empty())
		return;

	ContactSelectForm *contact_select_form = (ContactSelectForm *)WindowsManager::GetInstance()->GetWindow\
		(ContactSelectForm::kClassName, nbase::UTF8ToUTF16(wnd_id));

	if (!contact_select_form)
	{
		std::list<UTF8String> exnclude_ids;
		for (auto it : team_member_info_list_)
		{
			exnclude_ids.push_back(it.first);
		}
		contact_select_form = new ContactSelectForm(wnd_id, exnclude_ids, nbase::Bind(&SessionBox::SelectedCompleted, this, std::placeholders::_1, std::placeholders::_2));
		contact_select_form->Create(NULL, L"", UI_WNDSTYLE_FRAME& ~WS_MAXIMIZEBOX, 0L);
		contact_select_form->CenterWindow();
	}

	contact_select_form->ActiveWindow();

	return;
}

void SessionBox::SelectedCompleted(const std::list<UTF8String>& friend_list, const std::list<UTF8String>& team_list)
{
	nim::Team::InviteAsync(session_id_, friend_list, "", nbase::Bind(&TeamCallback::OnTeamEventCallback, std::placeholders::_1));
}

bool SessionBox::MuteAllSession(bool isMute)
{
	return nim::Team::MuteAsync(session_id_, isMute, nbase::Bind(&SessionBox::OnTeamMuteEventCallback, std::placeholders::_1), "");
}

void SessionBox::OnTeamMuteEventCallback(const nim::TeamEvent& result)
{
	nim::Team::QueryTeamInfoAsync(result.team_id_, nbase::Bind(&TeamCallback::QueryTeamInfoCallback, std::placeholders::_1, std::placeholders::_2));
}

void SessionBox::setSessionMuteTag(bool tag)
{
	if (is_manager_)
	{
		btn_mute_all_->SetEnabled(!tag);
		btn_mute_all_->SetVisible(!tag);
		btn_clear_mute_all_->SetEnabled(tag);
		btn_clear_mute_all_->SetVisible(tag);
	}
	else
	{
		//FindSubControl(L"input_edit_mute_tip")->SetVisible(tag);
		SetTeamMuteUI(tag);
	}
}

void SessionBox::OnGetTeamMemberCallback(const std::string& tid, int count, const std::list<nim::TeamMemberProperty>& team_member_info_list)
{
	team_member_info_list_.clear();
	std::set<std::string> account_set;
	for (auto& it : team_member_info_list)
	{
		if (IsTeamMemberType(it.GetUserType()))
		{
			team_member_info_list_[it.GetAccountID()] = it;
			account_set.insert(it.GetAccountID());
		}
	}

	// 设置群信息
	btn_refresh_member_->SetEnabled(true);
	member_list_->RemoveAll();
	label_member_->SetText(nbase::StringPrintf(MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_SESSION_MEMBER_NUM_EX").c_str(), team_member_info_list_.size()));
	//btn_new_broad_->SetVisible(set_new_broad_visible);

	// 提前批量查询群用户信息,优化用户查询
	UserService::GetInstance()->DoQueryUserInfos(account_set); 

	// 初始化@列表群成员
	AtlistForm* at_list_form = (AtlistForm*)WindowsManager::GetInstance()->GetWindow(AtlistForm::kClassName, nbase::UTF8ToUTF16(session_id_));
	if (at_list_form)
		at_list_form->InitTeamMembers(team_member_info_list_);

	// 初始化群成员控件
	for (const auto &tm_info : team_member_info_list_)
	{
		std::string uid = tm_info.second.GetAccountID();
		if (member_list_->FindSubControl(nbase::UTF8ToUTF16(uid)) == NULL)
		{
			TeamItem* item = new TeamItem;
			GlobalManager::FillBoxWithCache(item, L"session/team_item.xml");
			auto user_type = tm_info.second.GetUserType();
			if (user_type == nim::kNIMTeamUserTypeCreator)
				member_list_->AddAt(item, 0);
			else
				member_list_->Add(item);

			item->InitControl();
			item->InitInfo(tm_info.second);
			//SetTeamMemberMute(tid, uid, tm_info.second.IsMute());
			if (mute_all_ && user_type != nim::kNIMTeamUserTypeCreator && user_type != nim::kNIMTeamUserTypeManager && false)
			{
				SetTeamMemberMute(tid, uid, tm_info.second.IsMute(), mute_all_);
				QLOG_WAR(L"maute1111 ={0}") << mute_all_;
			}
			else
			{
				SetTeamMemberMute(tid, uid, tm_info.second.IsMute(), false);
				QLOG_WAR(L"maute222222 ={0}") << mute_all_;
			}			
		}
		else
		{
			QLOG_WAR(L"OnGetTeamMemberCb found the duplicate id, id={0}") << nbase::UTF8ToUTF16(uid).c_str();
		}

		RefreshMsglistShowname(uid); //刷新消息列表中显示的名字

		bool set_new_broad_visible = team_info_.GetUpdateInfoMode() == nim::kNIMTeamUpdateCustomModeEveryone;
		if (!set_new_broad_visible)
		{
			if (LoginManager::GetInstance()->IsEqual(tm_info.second.GetAccountID()))
			{
				if (tm_info.second.GetUserType() == nim::kNIMTeamUserTypeCreator || tm_info.second.GetUserType() == nim::kNIMTeamUserTypeManager)
				{
					btn_new_broad_->SetVisible(true);
					btn_new_broad_->SetEnabled(true);

					btn_add_member_->SetVisible(true);
					is_manager_ = true;
				}			
				set_new_broad_visible = true;
			}
		}

		if (LoginManager::GetInstance()->IsEqual(tm_info.second.GetAccountID()))
		{
			auto bits = tm_info.second.GetBits();
			InvokeSetTeamNotificationMode(bits);
		}
	}
}

void SessionBox::InvokeSetTeamNotificationMode(const int64_t bits)
{
	FindSubControl(L"not_disturb")->SetVisible(SessionManager::GetInstance()->IsTeamMsgMuteShown(session_id_, bits));
}

void SessionBox::OnTeamNotificationModeChangeCallback(const std::string& id, int64_t bits)
{
	if (session_id_ == id)
		InvokeSetTeamNotificationMode(bits);
}

void SessionBox::ResetNewBroadButtonVisible()
{
	if (team_info_.GetUpdateInfoMode() != nim::kNIMTeamUpdateCustomModeEveryone)
	{
		for (const auto &tm_info : team_member_info_list_)
		{
			if (LoginManager::GetInstance()->IsEqual(tm_info.second.GetAccountID()))
			{
				if (tm_info.second.GetUserType() == nim::kNIMTeamUserTypeCreator || tm_info.second.GetUserType() == nim::kNIMTeamUserTypeManager)
				{
					btn_new_broad_->SetVisible(true);
					btn_new_broad_->SetEnabled(true);
				}					
				else
				{
					btn_new_broad_->SetVisible(false);
				}					
				break;
			}
		}
	}
	else
	{
		btn_new_broad_->SetVisible(true);
		btn_new_broad_->SetEnabled(true);
	}		
}

nim::TeamMemberProperty SessionBox::GetTeamMemberInfo(const std::string& uid)
{
	auto iter = team_member_info_list_.find(uid);
	if (iter != team_member_info_list_.cend())
		return iter->second;
	else
		return nim::TeamMemberProperty();
}

void SessionBox::OnTeamMemberAdd(const std::string& tid, const nim::TeamMemberProperty& team_member_info)
{
	if (!IsTeamMemberType(team_member_info.GetUserType()))
		return;

	if(tid == session_id_)
	{
		team_member_info_list_[team_member_info.GetAccountID()] = team_member_info;
		std::wstring wid = nbase::UTF8ToUTF16(team_member_info.GetAccountID());

		Control* ctrl = member_list_->FindSubControl(wid);
		if (ctrl)
		{
			QLOG_WAR(L"OnTeamMemberAdd found the duplicate id, id={0}") << wid.c_str();
			member_list_->Remove(ctrl);
		}

		TeamItem* item = new TeamItem;
		GlobalManager::FillBoxWithCache(item, L"session/team_item.xml");
		auto user_type = team_member_info.GetUserType();
		if (user_type == nim::kNIMTeamUserTypeCreator)
			member_list_->AddAt(item, 0);
		else
			member_list_->Add(item);

		item->InitControl();
		item->InitInfo(team_member_info);
		std::string uid = team_member_info.GetAccountID();
		if (mute_all_ && user_type != nim::kNIMTeamUserTypeCreator && user_type != nim::kNIMTeamUserTypeManager)
			SetTeamMemberMute(tid, uid, team_member_info.IsMute(), mute_all_);
		else
			SetTeamMemberMute(tid, uid, team_member_info.IsMute(), mute_all_);

		std::wstring str = nbase::StringPrintf(MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_SESSION_MEMBER_NUM_EX").c_str(), member_list_->GetCount());
		label_member_->SetText(str);

		if (LoginManager::GetInstance()->IsEqual(team_member_info.GetAccountID()))
		{
			btn_header_->SetEnabled(true);
		}
	}
}

void SessionBox::OnTeamMemberRemove(const std::string& tid, const std::list<std::string>& uid_list)
{
	if(tid == session_id_)
	{
		for (auto uid : uid_list)
		{
			std::wstring wid = nbase::UTF8ToUTF16(uid);

			Control* ctrl = member_list_->FindSubControl(wid);
			if (ctrl)
			{
				member_list_->Remove(ctrl);
			}
			
			//同步搜索列表
			//TeamItem* ctrl_search = (TeamItem*)(search_result_list_->FindSubControl(wid));
			//if (ctrl_search)
			//{
			//	search_result_list_->Remove(ctrl_search);
			//}

			std::wstring str = nbase::StringPrintf(MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_SESSION_MEMBER_NUM_EX").c_str(), member_list_->GetCount());
			label_member_->SetText(str);

			team_member_info_list_.erase(uid);

			RefreshMsglistShowname(uid);
		}		
	}
}

void SessionBox::OnTeamMemberChange(const std::string& tid_uid, const std::string& team_card)
{
	size_t splitter = tid_uid.find_first_of('#');
	std::string tid = tid_uid.substr(0, splitter), uid = tid_uid.substr(splitter + 1);
	if(tid == session_id_)
	{
		auto it = team_member_info_list_.find(uid);
		if (it != team_member_info_list_.end())
		{
			it->second.SetNick(team_card);
		}
		std::wstring wid = nbase::UTF8ToUTF16(uid);

		TeamItem* ctrl = (TeamItem*)(member_list_->FindSubControl(wid));
		if(ctrl)
		{
			ctrl->SetMemberName(team_card);
		}
		
		//同步搜索列表
		TeamItem* ctrl_search = (TeamItem*)(search_result_list_->FindSubControl(wid));
		if (ctrl_search)
		{
			ctrl_search->SetMemberName(team_card);
		}
	}
}

void SessionBox::OnTeamAdminSet(const std::string& tid, const std::string& uid, bool admin)
{
	if(tid == session_id_)
	{
		//更新数据
		if (team_member_info_list_.find(uid) != team_member_info_list_.end())
			team_member_info_list_[uid].SetUserType(admin ? nim::kNIMTeamUserTypeManager : nim::kNIMTeamUserTypeNomal);
		std::wstring wid = nbase::UTF8ToUTF16(uid);		
		TeamItem* item = dynamic_cast<TeamItem*>(member_list_->FindSubControl(wid));
		if(item)
		{
			item->SetAdmin(admin);
		}

		//同步搜索列表
		TeamItem* item_search = (TeamItem*)(search_result_list_->FindSubControl(wid));
		if (item_search)
		{
			item_search->SetAdmin(admin);
		}
		
		if (team_info_.GetUpdateInfoMode() != nim::kNIMTeamUpdateCustomModeEveryone && LoginManager::GetInstance()->IsEqual(uid))
		{
			btn_new_broad_->SetVisible(admin);
			btn_new_broad_->SetEnabled(admin);
		}

		ShowEditControls();
	}
}

void SessionBox::OnTeamMuteMember(const std::string& tid, const std::string& uid, bool set_mute)
{
	if (tid == session_id_)
	{
		for (auto& member : team_member_info_list_)
		{
			if (member.second.GetAccountID() == uid)
			{
				member.second.SetMute(set_mute);
			}
		}

		SetTeamMemberMute(tid, uid, set_mute, false);
		ShowEditControls();
	}
}

void SessionBox::OnTeamOwnerChange(const std::string& tid, const std::string& uid)
{
	if (tid == session_id_)
	{
		for (auto& it_member : team_member_info_list_)
		{
			if (it_member.second.GetUserType() == nim::kNIMTeamUserTypeCreator && it_member.second.GetAccountID() != uid)
				it_member.second.SetUserType(nim::kNIMTeamUserTypeNomal);
		}
		auto team_member = team_member_info_list_.find(uid);
		if (team_member != team_member_info_list_.end())
		{
			team_member->second.SetUserType(nim::kNIMTeamUserTypeCreator);
		}

		std::wstring wid = nbase::UTF8ToUTF16(uid);
		for (int i = 0; i < member_list_->GetCount(); i++)
		{
			TeamItem* item = dynamic_cast<TeamItem*>(member_list_->GetItemAt(i));
			if (item->GetTeamUserType() == nim::kNIMTeamUserTypeCreator)
			{
				item->SetOwner(false);

				if (team_info_.GetUpdateInfoMode() == nim::kNIMTeamUpdateCustomModeEveryone)
					break;

				if(LoginManager::GetInstance()->IsEqual(item->GetUTF8Name()))
					btn_new_broad_->SetVisible(false);

				break;
			}
		}

		TeamItem* item = dynamic_cast<TeamItem*>(member_list_->FindSubControl(wid));
		if (item)
			item->SetOwner(true);
		
		//同步搜索列表
		TeamItem* item_search = dynamic_cast<TeamItem*>(search_result_list_->FindSubControl(wid));
		if (item_search)
			item_search->SetOwner(true);
		
		if (LoginManager::GetInstance()->IsEqual(nbase::UTF16ToUTF8(wid)))
		{
			btn_new_broad_->SetVisible(true);
			btn_new_broad_->SetEnabled(true);
		}

		ShowEditControls();
	}
}

void SessionBox::OnTeamNameChange(const nim::TeamInfo& team_info)
{
	if (session_id_ == team_info.GetTeamID())
	{
		std::wstring name = nbase::UTF8ToUTF16(team_info.GetName());

		SetTitleName(name);
	}
}

void SessionBox::OnTeamRemove(const std::string& tid)
{
	if (tid == session_id_)
	{
		HandleLeaveTeamEvent();
	}
}

void SessionBox::OnTeamAdd(const std::string& tid, const std::string& tname, nim::NIMTeamType ttype)
{
	//只处理这种场景 lty 20170807
	if (!is_team_valid_ && tid == session_id_)
	{
		InvokeGetTeamMember();
		InvokeGetTeamInfo();
	}
}

bool SessionBox::IsTeamMemberType(const nim::NIMTeamUserType user_type)
{
	if (user_type == nim::kNIMTeamUserTypeNomal || 
		user_type == nim::kNIMTeamUserTypeManager || 
		user_type == nim::kNIMTeamUserTypeCreator)
		return true;

	return false;
}

void SessionBox::SendReceiptIfNeeded(bool auto_detect/* = false*/)
{
	//发送已读回执目前只支持P2P会话
	if (session_type_ != nim::kNIMSessionTypeP2P)
		return;

	if (auto_detect)
	{
		if (!session_form_->IsActiveSessionBox(this) || msg_list_ == nullptr || !msg_list_->IsAtEnd())
		{
			receipt_need_send_ = true;
			return;
		}
	}

	nim::IMMessage msg;
	if (GetLastNeedSendReceiptMsg(msg))
	{
		receipt_need_send_ = false;
		nim::MsgLog::SendReceiptAsync(msg.ToJsonString(false), [](const nim::MessageStatusChangedResult& res) {
			auto iter = res.results_.begin();
			QLOG_APP(L"mark receipt id:{0} time:{1} rescode:{2}") << iter->talk_id_ << iter->msg_timetag_ << res.rescode_;
		});
	}
}

void SessionBox::RefreshMsglistShowname(const std::string& uid)
{
	if (session_type_ != nim::kNIMSessionTypeTeam)
		return;

	nim::TeamMemberProperty tm_info = GetTeamMemberInfo(uid);
	std::string show_name = tm_info.GetNick();
	if (show_name.empty())
		show_name = nbase::UTF16ToUTF8(UserService::GetInstance()->GetUserName(uid));

	int msg_count = msg_list_->GetCount();
	for (int i = 0; i < msg_count; i++)
	{
		MsgBubbleItem* bubble_item = dynamic_cast<MsgBubbleItem*>(msg_list_->GetItemAt(i));
		if (bubble_item && bubble_item->sender_name_->IsVisible() && bubble_item->msg_.sender_accid_ == uid)
			bubble_item->SetShowName(true, show_name);

		if (!bubble_item)
		{
			MsgBubbleNotice* notice_item = dynamic_cast<MsgBubbleNotice*>(msg_list_->GetItemAt(i));
			if (notice_item)
				notice_item->RefreshNotice();
		}
	}
}

void SessionBox::SetTeamMemberMute(const std::string& tid, const std::string& uid, bool set_mute, bool team_mute)
{
	std::wstring wid = nbase::UTF8ToUTF16(uid);

	TeamItem* item = dynamic_cast<TeamItem*>(member_list_->FindSubControl(wid));
	if (item)
		item->SetMute(set_mute);

	//同步搜索列表
	//TeamItem* item_search = dynamic_cast<TeamItem*>(search_result_list_->FindSubControl(wid));
	//if (item_search)
	//{
	//	item_search->SetMute(set_mute);
	//}
}

void SessionBox::SetTeamMuteUI(bool mute)
{
	ControlStateType type = mute ? kControlStateDisabled : kControlStateNormal;
	input_edit_->SetEnabled(!mute);
	input_edit_->SetReadOnly(mute);
	btn_capture_audio_->SetEnabled(!mute);
	//btn_send_->SetEnabled(!mute);
	FindSubControl(L"btn_face")->SetEnabled(!mute);
	FindSubControl(L"btn_image")->SetEnabled(!mute);
	FindSubControl(L"btn_miniv")->SetEnabled(!mute);
	FindSubControl(L"btn_file")->SetEnabled(!mute);
	FindSubControl(L"btn_transfer_file")->SetEnabled(!mute);
	FindSubControl(L"btn_jsb")->SetEnabled(!mute);
	//FindSubControl(L"btn_tip")->SetEnabled(!mute);
	FindSubControl(L"btn_clip")->SetEnabled(!mute);
	FindSubControl(L"btn_custom_msg")->SetEnabled(!mute);
	FindSubControl(L"input_edit_mute_tip")->SetVisible(mute);
	FindSubControl(L"hbox_send")->SetEnabled(!mute);
	FindSubControl(L"btn_personcard")->SetEnabled(!mute);
}

void SessionBox::UpdateBroad(const Json::Value &broad)
{
	std::string output;
	for (int i = 0; i < (int)broad.size(); i++)
	{
		std::string title = broad[i]["title"].asString();
		std::string content = broad[i]["content"].asString();
		if (!output.empty())
			output.append("\r\n");
		output.append(title + " : " + content);
	}
	edit_broad_->SetUTF8Text(output);
}

void SessionBox::CheckTeamType(nim::NIMTeamType type)
{

	bool show = (type == nim::kNIMTeamTypeAdvanced);

	//Control* split = FindSubControl(L"frame_mid_split");
	//split->SetVisible(show);
	//Control* frame_right = FindSubControl(L"frame_right");
	//frame_right->SetVisible(show);
	//FindSubControl(L"btn_team_ack_msg")->SetVisible(LoginManager::GetInstance()->IsTeamMsgAckUIEnabled() && show);
	Control* frame_right = FindSubControl(L"frame_right");
	auto hied_right = dynamic_cast<ui::Button*>(FindSubControl(L"hide_right"));
	auto show_right = dynamic_cast<ui::Button*>(FindSubControl(L"show_right"));
	hied_right->SetVisible(frame_right->IsVisible());
	show_right->SetVisible(!frame_right->IsVisible());
}

bool SessionBox::IsAdvancedTeam()
{
	return (session_type_ == nim::kNIMSessionTypeTeam && team_info_.GetType() == nim::kNIMTeamTypeAdvanced);
}

void SessionBox::HandleEnterTeamEvent()
{
	RemoveTip(STT_LEAVE);
	is_team_valid_ = true;
	is_header_enable_ = true;
	ResetNewBroadButtonVisible();

	input_edit_->SetEnabled(true);
	input_edit_->SetReadOnly(false);
	bottom_panel_->SetEnabled(true);
}

void SessionBox::HandleLeaveTeamEvent()
{
	AddTip(STT_LEAVE);
	is_team_valid_ = false;
	is_header_enable_ = false;
	btn_invite_->SetEnabled(false);
	btn_new_broad_->SetEnabled(false);
	btn_refresh_member_->SetEnabled(false);

	input_edit_->SetEnabled(false);
	input_edit_->SetReadOnly(true);
	bottom_panel_->SetEnabled(false);
}

void SessionBox::HandleDismissTeamEvent()
{
	AddTip(STT_DISMISS);
	is_team_valid_ = false;
	is_header_enable_ = false;
	btn_invite_->SetEnabled(false);
	btn_new_broad_->SetEnabled(false);
	btn_refresh_member_->SetEnabled(false);
	btn_mute_all_->SetEnabled(false);
	btn_clear_mute_all_->SetEnabled(false);

	input_edit_->SetEnabled(false);
	input_edit_->SetReadOnly(true);
	bottom_panel_->SetEnabled(false);
}

void SessionBox::UpdateUnreadCount(const std::string &msg_id, const int unread)
{
	auto iter = cached_msgs_bubbles_.rbegin();
	for (; iter != cached_msgs_bubbles_.rend(); ++iter)
	{
		MsgBubbleItem* item = (MsgBubbleItem*)(*iter);
		if (item)
		{
			nim::IMMessage msg = item->GetMsg();
			if (msg.client_msg_id_ == msg_id)
			{
				item->SetUnreadCount(unread);
				break;
			}
		}
	}
}

void SessionBox::InvokeSetRead(const std::list<nim::IMMessage> &msgs)
{
	//nim::Team::TeamMsgAckRead(session_id_, msgs, nim::Team::TeamEventCallback());
	nim::Team::TeamMsgAckRead(session_id_, msgs, [](const nim::TeamEvent& team_event) {
		auto tt = team_event.attach_;
		tt.empty();
	});
	new_msgs_need_to_send_mq_.clear();
}


void SessionBox::SendTeamReceiptIfNeeded(bool auto_detect/* = false*/)
{
	//发送已读回执目前只支持Tea,会话
	if (session_type_ != nim::kNIMSessionTypeTeam)
		return;

	if (auto_detect)
	{
		if (!session_form_->IsActiveSessionBox(this) || msg_list_ == nullptr || !msg_list_->IsAtEnd())
		{
			return;
		}
	}

	if (!new_msgs_need_to_send_mq_.empty())
		InvokeSetRead(new_msgs_need_to_send_mq_);
}

void SessionBox::ShowEditControls()
{
	nim::NIMTeamUserType member_type = nim::kNIMTeamUserTypeNomal;
	bool member_is_mute = false;
	for (const auto& member : team_member_info_list_)
	{
		if (member.second.GetAccountID() == LoginManager::GetInstance()->GetAccount())
		{
			member_type = member.second.GetUserType();
			member_is_mute = member.second.IsMute();
			break;
		}
	}

	bool disable_input = mute_all_ && member_type != nim::kNIMTeamUserTypeCreator && member_type != nim::kNIMTeamUserTypeManager;
	input_edit_->SetEnabled(!(disable_input || member_is_mute));
	input_edit_->SetReadOnly((disable_input || member_is_mute));
	bottom_panel_->SetEnabled(!(disable_input || member_is_mute));
	FindSubControl(L"mute_tips")->SetVisible(mute_all_);
}


bool SessionBox::SearchEditChange(ui::EventArgs* param)
{
	UTF8String search_key = search_edit_->GetUTF8Text();
	bool has_serch_key = !search_key.empty();
	btn_clear_input_->SetVisible(has_serch_key);
	search_result_list_->SetVisible(has_serch_key);
	member_list_->SetVisible(!has_serch_key);
	//option_panel_->SetVisible(!has_serch_key);
	FindSubControl(L"no_search_result_tip")->SetVisible(has_serch_key);
	if (has_serch_key)
	{
		FillSearchResultList(search_result_list_, search_key);
		FindSubControl(L"no_search_result_tip")->SetVisible(search_result_list_->GetCount() == 0);
		if (search_result_list_->GetCount() > 0)
			search_result_list_->SelectItem(0);
	}
	return true;
}

bool SessionBox::OnClearInputBtnClicked(ui::EventArgs* param)
{
	search_edit_->SetText(L"");
	return true;
}

void SessionBox::FillSearchResultList(ui::ListBox* search_result_list, const UTF8String& search_key)
{
	if (NULL == search_result_list)
		return;

	search_result_list->RemoveAll();

	if (NULL != member_list_)
	{
		for (int i = 0; i < member_list_->GetCount(); i++)
		{
			TeamItem* friend_item = dynamic_cast<TeamItem*>(member_list_->GetItemAt(i));
			if (friend_item && friend_item->Match(search_key))
			{
				nim::TeamMemberProperty team_member_info = friend_item->GetMemberInfo();
				
				nim_comp::TeamItem* search_res_friend_item = new nim_comp::TeamItem;
				ui::GlobalManager::FillBoxWithCache(search_res_friend_item, L"session/team_item.xml");

				auto user_type = team_member_info.GetUserType();
				if (user_type == nim::kNIMTeamUserTypeCreator)
					search_result_list->AddAt(search_res_friend_item, 0);
				else
					search_result_list->Add(search_res_friend_item);

				search_res_friend_item->InitControl();
				search_res_friend_item->InitInfo(team_member_info);
				
				search_res_friend_item->SetMute(team_member_info.IsMute());

			}
		}
	}
}
}