#include "team_item.h"
#include "shared/ui/ui_menu.h"
#include "callback/team/team_callback.h"
#include "module/session/session_manager.h"
#include "gui/profile_form/profile_form.h"
#include "gui/team_info/member_manager.h"
#include "export/nim_ui_user_manager.h"
#include "export/nim_ui_window_manager.h" 
#include "shared/pin_yin_helper.h"

using namespace ui;

namespace nim_comp
{
	void TeamItem::InitControl()
	{
		this->AttachMenu(nbase::Bind(&TeamItem::OnItemMenu, this, std::placeholders::_1));
		this->AttachDoubleClick(nbase::Bind(&TeamItem::OnDbClicked, this, std::placeholders::_1));

		member_icon_ = (Button*) this->FindSubControl(L"member_icon");
		member_name_ = (Label*) this->FindSubControl(L"member_name");

		icon_admin_ = this->FindSubControl(L"icon_admin");
		icon_mute_ = this->FindSubControl(L"icon_mute");
	}

	void TeamItem::InitInfo(const nim::TeamMemberProperty &info)
	{
		member_info_ = info;
		

		this->SetUTF8Name(info.GetAccountID());
		SetMemberName(member_info_.GetNick());
		member_icon_->SetBkImage(PhotoService::GetInstance()->GetUserPhoto(info.GetAccountID()));

		nim::NIMTeamUserType team_user_type = member_info_.GetUserType();
		if (team_user_type == nim::kNIMTeamUserTypeCreator || team_user_type == nim::kNIMTeamUserTypeManager)
		{
			icon_admin_->SetBkImage(team_user_type == nim::kNIMTeamUserTypeCreator ? L"..\\public\\icon\\team_creator.png" : L"..\\public\\icon\\team_manager.png");
			icon_admin_->SetVisible(true);
		}
		else
			icon_admin_->SetVisible(false);

		is_mute_ = info.IsMute();
		if (info.IsMute())
		{
			icon_mute_->SetBkImage(L"..\\public\\icon\\mute.png");
			icon_mute_->SetVisible(true);
		}

		member_icon_->AttachClick(nbase::Bind(&TeamItem::OnHeadImageClick, this, member_info_.GetAccountID(), std::placeholders::_1));
	}

	bool TeamItem::OnHeadImageClick(const std::string& uid, ui::EventArgs*)
	{
		//判断是否是管理员类型
		nim::NIMTeamUserType my_team_user_type = nim::kNIMTeamUserTypeNomal;
		SessionBox* session_box = dynamic_cast<SessionBox*>(SessionManager::GetInstance()->FindSessionBox(member_info_.GetTeamID()));
		if (session_box)
		{
			auto my_pro = session_box->GetTeamMemberInfo(LoginManager::GetInstance()->GetAccount());
			my_team_user_type = my_pro.GetUserType();
		}
		//nim::NIMTeamUserType team_user_type = member_info_.GetUserType();

		if (my_team_user_type == nim::kNIMTeamUserTypeCreator
			|| my_team_user_type == nim::kNIMTeamUserTypeManager)
		{
			ProfileForm::ShowProfileForm(uid);
			return true;
		}
		curClickId = uid;
		//ProfileForm::ShowProfileForm(uid);
		//从服务器获取能否添加好友
		nim_ui::UserManager::GetInstance()->InvokeGetMemberAddFriend(member_info_.GetTeamID(), ToWeakCallback([this](int code, const std::string &msg, const std::string &canAddFriend) {
			if (code == 0)
			{
				if (canAddFriend == "1")
				{
					ProfileForm::ShowProfileForm(curClickId);
				}
				else
				{
					//提示用户群管理员未分配权限
					std::wstring toast = L"管理员未开通查看权限";
					nim_ui::ShowToast(toast, 5000);
				}
			}
			else
			{
				//提示用户错误
				std::wstring toast = L"查看群权限失败";
				nim_ui::ShowToast(toast, 5000);
			}
		}));
		return true;
	}

	void TeamItem::SetAdmin(bool admin)
	{
		icon_admin_->SetBkImage(L"..\\public\\icon\\team_manager.png");
		icon_admin_->SetVisible(admin);
		if (admin)
		{
			member_info_.SetUserType(nim::kNIMTeamUserTypeManager);
		}
		else
		{
			member_info_.SetUserType(nim::kNIMTeamUserTypeNomal);
		}
	}

	void TeamItem::SetMute(bool mute)
	{
		is_mute_ = mute;
		icon_mute_->SetBkImage(L"..\\public\\icon\\mute.png");
		icon_mute_->SetVisible(mute);
	}

	void TeamItem::SetOwner(bool is_owner)
	{
		icon_admin_->SetBkImage(L"..\\public\\icon\\team_creator.png");
		icon_admin_->SetVisible(is_owner);
		if (is_owner)
		{
			member_info_.SetUserType(nim::kNIMTeamUserTypeCreator);
		}
		else
		{
			member_info_.SetUserType(nim::kNIMTeamUserTypeNomal);
		}
	}

	nim::NIMTeamUserType TeamItem::GetTeamUserType()
	{
		return member_info_.GetUserType();
	}

	void TeamItem::SetMemberName(const std::string& team_card)
	{
		if (NULL == member_name_)
			return;

		if (!team_card.empty())
		{
			member_name_->SetUTF8Text(team_card);
			member_info_.SetNick(team_card);
		}
		else
			member_name_->SetText(UserService::GetInstance()->GetUserName(GetUTF8Name()));

		nick_name_ = member_name_->GetText();
		nick_name_full_spell_ = nbase::UTF8ToUTF16(nbase::MakeLowerString(PinYinHelper::GetInstance()->ConvertToFullSpell(nick_name_)));
		nick_name_simple_spell_ = nbase::UTF8ToUTF16(nbase::MakeLowerString(PinYinHelper::GetInstance()->ConvertToSimpleSpell(nick_name_)));
	}
	
    bool TeamItem::Match(const UTF8String& search_key)
	{
		std::wstring ws_search_key = nbase::UTF8ToUTF16(search_key);
		ws_search_key = nbase::MakeLowerString(ws_search_key);
		//std::wstring member_name = member_name_->GetText();//member_info_.GetNick();
		std::string member_id = member_info_.GetAccountID();
		
		if (nick_name_.find(ws_search_key) != std::wstring::npos
			|| nick_name_simple_spell_.find(ws_search_key) != std::wstring::npos
			|| nick_name_full_spell_.find(ws_search_key) != std::wstring::npos
			|| member_id.find(search_key) != UTF8String::npos)
		{
			return true;
		}
		return false;
	}

	nim::TeamMemberProperty TeamItem::GetMemberInfo()
	{
		return member_info_;
	}
	
	std::string TeamItem::GetTeamCard() const
	{
		return member_info_.GetNick();
	}

	bool TeamItem::OnItemMenu(ui::EventArgs* arg)
	{
		POINT point;
		::GetCursorPos(&point);
		PopupItemMenu(point);
		return true;
	}

	void TeamItem::PopupItemMenu(POINT point)
	{
		CMenuWnd* pMenu = new CMenuWnd(NULL);
		STRINGorID xml(L"team_item_menu.xml");
		pMenu->Init(xml, _T("xml"), point);

		nim::NIMTeamUserType my_team_user_type = nim::kNIMTeamUserTypeNomal;
		SessionBox* session_box = dynamic_cast<SessionBox*>(SessionManager::GetInstance()->FindSessionBox(member_info_.GetTeamID()));
		if (session_box)
		{
			auto my_pro = session_box->GetTeamMemberInfo(LoginManager::GetInstance()->GetAccount());
			my_team_user_type = my_pro.GetUserType();
		}
		nim::NIMTeamUserType team_user_type = member_info_.GetUserType();

		CMenuElementUI* mute_item = (CMenuElementUI*)pMenu->FindControl(L"mute");
		if (my_team_user_type == nim::kNIMTeamUserTypeNomal
			|| my_team_user_type == team_user_type
			|| team_user_type == nim::kNIMTeamUserTypeCreator
			|| team_user_type == nim::kNIMTeamUserTypeApply
			|| team_user_type == nim::kNIMTeamUserTypeLocalWaitAccept)
			mute_item->SetEnabled(false);
		else
			mute_item->SetEnabled(true);

		if (is_mute_)
		{
			((Label*)(mute_item->FindSubControl(L"mute_text")))->SetText(MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_SESSION_TEAM_ITEM_MENU_UNMUTE"));
		}
		else
		{
			((Label*)(mute_item->FindSubControl(L"mute_text")))->SetText(MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_SESSION_TEAM_ITEM_MENU_MUTE"));
		}
		mute_item->AttachSelect(nbase::Bind(&TeamItem::TeamItemMenuItemClick, this, std::placeholders::_1));
		((CMenuElementUI*)pMenu->FindControl(L"info"))->AttachSelect(nbase::Bind(&TeamItem::TeamItemMenuItemClick, this, std::placeholders::_1));
		((CMenuElementUI*)pMenu->FindControl(L"talk"))->AttachSelect(nbase::Bind(&TeamItem::TeamItemMenuItemClick, this, std::placeholders::_1));
		((CMenuElementUI*)pMenu->FindControl(L"call_he"))->AttachSelect(nbase::Bind(&TeamItem::TeamItemMenuItemClick, this, std::placeholders::_1));
		((CMenuElementUI*)pMenu->FindControl(L"name_card"))->AttachSelect(nbase::Bind(&TeamItem::TeamItemMenuItemClick, this, std::placeholders::_1));
		((CMenuElementUI*)pMenu->FindControl(L"set_manager"))->AttachSelect(nbase::Bind(&TeamItem::TeamItemMenuItemClick, this, std::placeholders::_1));
		((CMenuElementUI*)pMenu->FindControl(L"out_group"))->AttachSelect(nbase::Bind(&TeamItem::TeamItemMenuItemClick, this, std::placeholders::_1));
		if (LoginManager::GetInstance()->GetAccount() == member_info_.GetAccountID())
		{
			mute_item = (CMenuElementUI*)pMenu->FindControl(L"call_he");
			mute_item->SetVisible(false);
		}
		if (my_team_user_type != nim::kNIMTeamUserTypeCreator && my_team_user_type != nim::kNIMTeamUserTypeManager)// && LoginManager::GetInstance()->GetAccount() != member_info_.GetAccountID())
		{
			mute_item = (CMenuElementUI*)pMenu->FindControl(L"name_card");
			mute_item->SetVisible(false);
		}
		
		if (my_team_user_type != nim::kNIMTeamUserTypeCreator && my_team_user_type != nim::kNIMTeamUserTypeManager)
		{
			//mute_item = (CMenuElementUI*)pMenu->FindControl(L"info");
			//mute_item->SetVisible(false);
			mute_item = (CMenuElementUI*)pMenu->FindControl(L"talk");
			mute_item->SetVisible(false);
		}
		
		if (my_team_user_type != nim::kNIMTeamUserTypeCreator || LoginManager::GetInstance()->GetAccount() == member_info_.GetAccountID() || member_info_.GetUserType() == nim::kNIMTeamUserTypeManager)
		{
			mute_item = (CMenuElementUI*)pMenu->FindControl(L"set_manager");
			mute_item->SetVisible(false);
		}

		mute_item = (CMenuElementUI*)pMenu->FindControl(L"out_group");
		mute_item->SetVisible(false);

		if (my_team_user_type == nim::kNIMTeamUserTypeCreator && LoginManager::GetInstance()->GetAccount() != member_info_.GetAccountID())// && my_team_user_type != nim::kNIMTeamUserTypeManager || LoginManager::GetInstance()->GetAccount() != member_info_.GetAccountID() || member_info_.GetUserType() == nim::kNIMTeamUserTypeCreator)
		{
			mute_item = (CMenuElementUI*)pMenu->FindControl(L"out_group");
			mute_item->SetVisible(true);
		}
		else if (my_team_user_type == nim::kNIMTeamUserTypeManager && LoginManager::GetInstance()->GetAccount() != member_info_.GetAccountID() && member_info_.GetUserType() != nim::kNIMTeamUserTypeCreator && member_info_.GetUserType() != nim::kNIMTeamUserTypeManager)
		{
			mute_item = (CMenuElementUI*)pMenu->FindControl(L"out_group");
			mute_item->SetVisible(true);
		}
		pMenu->Show();
	}

	bool TeamItem::TeamItemMenuItemClick(ui::EventArgs* param)
	{
		std::wstring name = param->pSender->GetName();
		if (name == L"mute")
		{
			nim::Team::MuteMemberAsync(member_info_.GetTeamID(), member_info_.GetAccountID(), !is_mute_, nbase::Bind(&TeamCallback::OnTeamEventCallback, std::placeholders::_1));
		}
		else if (name == L"info")
		{
			/*nim::NIMTeamUserType my_team_user_type = nim::kNIMTeamUserTypeNomal;
			SessionBox* session_box = dynamic_cast<SessionBox*>(SessionManager::GetInstance()->FindSessionBox(member_info_.GetTeamID()));
			if (session_box)
			{
				auto my_pro = session_box->GetTeamMemberInfo(LoginManager::GetInstance()->GetAccount());
				my_team_user_type = my_pro.GetUserType();
			}
			bool tpye = false;
			if (my_team_user_type == nim::kNIMTeamUserTypeCreator || my_team_user_type == nim::kNIMTeamUserTypeManager)
			{
				tpye = true;
			}
			ProfileForm::ShowProfileForm(member_info_.GetAccountID());*/
			//判断是否是管理员类型
			nim::NIMTeamUserType my_team_user_type = nim::kNIMTeamUserTypeNomal;
			SessionBox* session_box = dynamic_cast<SessionBox*>(SessionManager::GetInstance()->FindSessionBox(member_info_.GetTeamID()));
			if (session_box)
			{
				auto my_pro = session_box->GetTeamMemberInfo(LoginManager::GetInstance()->GetAccount());
				my_team_user_type = my_pro.GetUserType();
			}
			//nim::NIMTeamUserType team_user_type = member_info_.GetUserType();

			if (my_team_user_type == nim::kNIMTeamUserTypeCreator
				|| my_team_user_type == nim::kNIMTeamUserTypeManager)
			{
				ProfileForm::ShowProfileForm(member_info_.GetAccountID());
				return true;
			}
			curClickId = member_info_.GetAccountID();
			//ProfileForm::ShowProfileForm(uid);
			//从服务器获取能否添加好友
			nim_ui::UserManager::GetInstance()->InvokeGetMemberAddFriend(member_info_.GetTeamID(), ToWeakCallback([this](int code, const std::string &msg, const std::string &canAddFriend) {
				if (code == 0)
				{
					if (canAddFriend == "1")
					{
						ProfileForm::ShowProfileForm(curClickId);
					}
					else
					{
						//提示用户群管理员未分配权限
						std::wstring toast = L"管理员未开通查看权限";
						nim_ui::ShowToast(toast, 5000);
					}
				}
				else
				{
					//提示用户错误
					std::wstring toast = L"查看群权限失败";
					nim_ui::ShowToast(toast, 5000);
				}
			}));
		}
		else if (name == L"talk")
		{
			std::string id = member_info_.GetAccountID();

			nim_comp::SessionManager::GetInstance()->OpenSessionBox(id, nim::kNIMSessionTypeP2P);
		}
		else if (name == L"call_he")
		{
			//nim_comp::SessionManager::GetInstance()
			std::string id = member_info_.GetAccountID();
			std::string teamId = member_info_.GetTeamID();
			SessionBox *box = nim_comp::SessionManager::GetInstance()->OpenSessionBox(teamId, nim::kNIMSessionTypeTeam);
			box->SelectAtUser(id, false);
		}
		else if (name == L"name_card")
		{
			std::string userId = member_info_.GetAccountID();
			std::wstring ws_user_id = nbase::UTF8ToUTF16(userId);
			MemberManagerForm* member_manager_form = (MemberManagerForm*)WindowsManager::GetInstance()->GetWindow\
				(MemberManagerForm::kClassName, ws_user_id);
			if (member_manager_form == NULL)
			{
				bool show_privilege_panel = (member_info_.GetUserType() == nim::kNIMTeamUserTypeCreator && userId != LoginManager::GetInstance()->GetAccount());
				member_manager_form = new MemberManagerForm(member_info_.GetTeamID(), member_info_, show_privilege_panel);
				member_manager_form->Create(NULL, L"", WS_OVERLAPPEDWINDOW& ~WS_MAXIMIZEBOX, 0L);
				member_manager_form->CenterWindow();
				member_manager_form->ShowWindow(true);
			}
			else
			{
				member_manager_form->ActiveWindow();
			}
		}
		else if (name == L"set_manager")
		{
			std::list<std::string> uids_list;
			uids_list.push_back(member_info_.GetAccountID());
			nim::Team::AddManagersAsync(member_info_.GetTeamID(), uids_list, nbase::Bind(&TeamCallback::OnTeamEventCallback, std::placeholders::_1));
		}
		else if (name == L"out_group")
		{
			std::list<std::string> uids_list;
			uids_list.push_back(member_info_.GetAccountID());
			nim::Team::KickAsync(member_info_.GetTeamID(), uids_list, nbase::Bind(&TeamCallback::OnTeamEventCallback, std::placeholders::_1));
			MemberManagerForm* member_manager_form = (MemberManagerForm*)WindowsManager::GetInstance()->GetWindow\
				(MemberManagerForm::kClassName, nbase::UTF8ToUTF16(member_info_.GetAccountID()));
			if (member_manager_form)
				member_manager_form->Close();
		}
		return true;
	}

	bool TeamItem::OnDbClicked(ui::EventArgs* arg)
	{
		//std::string id = member_info_.GetAccountID();

		//nim_comp::SessionManager::GetInstance()->OpenSessionBox(id, nim::kNIMSessionTypeP2P);

		return true;
	}
}