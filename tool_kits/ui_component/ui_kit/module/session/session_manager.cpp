#include "session_manager.h"
#include "export/nim_ui_session_list_manager.h"
#include "module/session/force_push_manager.h"
#include "module/service/mute_black_service.h"
#include "module/runtime_data/runtime_data_manager.h"
#include "gui/session/session_form.h"
#include "gui/session/session_box.h"
#include "gui/session/session_dock_def.h"
namespace nim_comp
{
	SessionManager::SessionManager()
	{
		enable_merge_ = false;
		use_custom_drag_image_ = true;
		ring_.Init(NULL);
	}

	SessionManager::~SessionManager()
	{

	}

	SessionBox* SessionManager::GetCurrentSessionBox()
	{
		if (session_box_map_.size() <= 0)
			return NULL;

		for (auto it_box : session_box_map_)
		{
			ASSERT(NULL != it_box.second);
			if (NULL != it_box.second)
			{
				return it_box.second;
			}
		}
		return NULL;
	}

	void SessionManager::SetSpeakerStatus(bool isSpeaker)
	{
		is_speaker_ = isSpeaker;
		ring_.SetSpeakerStatus(isSpeaker);
	}

	SessionBox* SessionManager::OpenCurrentSessionBox()
	{
		if (session_box_map_.size() <= 0)
			return NULL;

		for (auto it_box : session_box_map_)
		{
			ASSERT(NULL != it_box.second);
			return it_box.second;
		}
		return NULL;
	}

	SessionBox* SessionManager::OpenSessionBox(std::string session_id, nim::NIMSessionType type, bool reopen, bool independent)
	{
		if (session_id.empty())
		{
			ASSERT(0);
			return NULL;
		}

		if (!independent) {
			// 在当前列表中查找是否有要打开的会话
			auto it_box = session_box_map_.find(session_id);
			if (it_box != session_box_map_.end())
			{
				// 如果需要重新打开则删除原来的 session box 再重新创建一个，否则就激活当前查找到的 session box
				ISessionDock *parent_form = it_box->second->GetSessionForm();
				if (!reopen)
				{
					parent_form->SetActiveSessionBox(session_id);
					it_box->second->OnMsgDown(NULL);
					// check the position of scroll near bottom, if not, go to end of scroll
					it_box->second->ShowLatestMsg();
					return it_box->second;
				}
				else
				{
					parent_form->CloseSessionBox(session_id);
					RemoveSessionBox(session_id);
				}
			}
		}
		std::vector<ForcePushManager::ForcePushInfo> infos;
		ForcePushManager::GetInstance()->GetAtMeMsgs(session_id, infos);

		std::vector<ForcePushManager::ForcePushInfo> board_infos;
		ForcePushManager::GetInstance()->GetBoardMsgs(session_id, board_infos);

		SessionBox *session_box = CreateSessionBox(session_id, type, independent);
		if (NULL == session_box)
			return NULL;

		session_box->SetVisible(true);
		session_box->InvokeShowMsgs(true);
		session_box->InitAtMeView(infos);
		session_box->InitBoardView(board_infos);
		if (session_box->GetSessionForm() != nullptr) {
			session_box->GetSessionForm()->ActiveWindow();
		}
		return session_box;
	}

	SessionBox* SessionManager::OpenSessionBoxEx(std::string session_id, nim::NIMSessionType type, bool reopen)
	{
		if (session_id.empty())
		{
			ASSERT(0);
			return NULL;
		}

		// 在当前列表中查找是否有要打开的会话
		auto it_box = session_box_map_.find(session_id);
		if (it_box != session_box_map_.end())
		{
			// 如果需要重新打开则删除原来的 session box 再重新创建一个，否则就激活当前查找到的 session box
			ISessionDock *parent_form = it_box->second->GetSessionForm();
			if (reopen)
			{
				parent_form->CloseSessionBox(session_id);
				RemoveSessionBox(session_id);
			}
		}
		std::vector<ForcePushManager::ForcePushInfo> infos;
		ForcePushManager::GetInstance()->GetAtMeMsgs(session_id, infos);

		std::vector<ForcePushManager::ForcePushInfo> board_infos;
		ForcePushManager::GetInstance()->GetBoardMsgs(session_id, board_infos);

		SessionBox *session_box = CreateSessionBoxEx(session_id, type);
		if (NULL == session_box)
			return NULL;

		session_box->InvokeShowMsgs(true);
		session_box->InitAtMeView(infos);
		session_box->InitBoardView(board_infos);
		if (session_box->GetSessionForm() != nullptr) {
			session_box->GetSessionForm()->ActiveWindow();
		}
		return session_box;
	}

	void SessionManager::AddNewMsg(const nim::IMMessage &msg)
	{
		//自定义消息
		if ((msg.type_ == nim::kNIMMessageTypeCustom && (msg.attach_.find("\"type\":101") != std::string::npos)) || (msg.type_ == nim::kNIMMessageTypeTips && msg.content_ == "")){
			return;
		}
		std::string id = GetSessionId(msg);
		SessionBox *session_box = FindSessionBox(id);

		if (session_box && !session_box->IsTeamValid())
		{
			ISessionDock *parent_form = session_box->GetSessionForm();
			parent_form->CloseSessionBox(id);
			session_box = NULL;
		}

		bool create = false;
		bool msg_notify = true;

		// octodo: disable auto open session box
		if (false && NULL == session_box)
		{
			if (msg_notify)
			{
				create = true;
				// 如果使用了合并会话功能并且已经有了会话窗体，则取消create标志
				if (enable_merge_ && !session_box_map_.empty()){
					create = false;
				}
				if (create){
					session_box = CreateSessionBox(id, msg.session_type_, false);
				}
				if (NULL == session_box){
					return;
				}
			}
		}

		// 不播放 P2P 传送文件时自定义协商消息
		if (msg.type_ == nim::kNIMMessageTypeCustom)
		{
			Json::Reader reader;
			Json::Value values;

			if (reader.parse(msg.attach_, values) && values.isMember(kJsonKeyCommand))
			{
				msg_notify = values[kJsonKeyCommand].asString() == kJsonKeyTransferFileRequest;
			}
		}

		if (session_box)
		{
			session_box->AddNewMsg(msg, create);
			/*if (msg.session_type_ == nim::kNIMSessionTypeTeam)
			{
				if (!IsTeamMsgNotify(id, msg.sender_accid_))
					msg_notify = false;
			}
			else
				msg_notify = !MuteBlackService::GetInstance()->IsInMuteList(id);
			if (msg_notify)
				ring_.Play(RING_NEW_MESSAGE);*/
		}
		
		// octodo: sync independent data
		session_box = FindSessionBoxEx(id);
		if (session_box){
			session_box->AddNewMsg(msg, create);
		}

		bool flash = true;
		if (msg.feature_ == nim::kNIMMessageFeatureSyncMsg || msg.type_ == nim::kNIMMessageTypeNotification)
		{
			flash = false;
		}
	}


	void SessionManager::AddTeamMsg(const nim::IMMessage &msg)
	{
		if (msg.type_ == nim::kNIMMessageTypeCustom&&msg.content_ == ""){
			return;
		}
		std::string id = GetSessionId(msg);
		SessionBox *session_box = FindSessionBox(id);

		if (session_box && !session_box->IsTeamValid())
		{
			session_box->UninitSessionBox();
			session_box = NULL;
		}

		bool create = false;
		bool msg_notify = true;
		if (msg.session_type_ == nim::kNIMSessionTypeTeam)
		{
			if (!IsTeamMsgNotify(id, msg.sender_accid_))
				msg_notify = false;
		}
		else
			msg_notify = !MuteBlackService::GetInstance()->IsInMuteList(id);

		if (NULL == session_box)
		{
			if (msg_notify)
			{
				create = true;
				// 如果使用了合并会话功能并且已经有了会话窗体，则取消create标志
				if (enable_merge_ && !session_box_map_.empty())
					create = false;

				session_box = CreateSessionBox(id, msg.session_type_, false);
				if (NULL == session_box)
					return;
			}
		}
		//cjy2 群免打扰
		if (session_box)
		{
			session_box->AddTeamMsg(msg, create);

			if (msg_notify)
				ring_.Play(RING_NEW_MESSAGE);
		}
	}

	bool SessionManager::IsSessionBoxActive(const std::string& id)
	{
		SessionBox *session_box = FindSessionBox(id);
		if (NULL != session_box)
		{
			ISessionDock *parent_form = session_box->GetSessionForm();
			return parent_form->IsActiveSessionBox(session_box);
		}

		return false;
	}
	SessionBox* SessionManager::GetFirstActiveSession()
	{
		for (auto &it_box : session_box_map_)
		{
			ISessionDock *parent_form = it_box.second->GetSessionForm();
			if (parent_form && parent_form->IsActiveSessionBox(it_box.second))
			{
				return it_box.second;
			}
		}
		return nullptr;
	}

	SessionBox* SessionManager::FindSessionBox(const std::string &id)
	{
		std::map<std::string, SessionBox*>::const_iterator i = session_box_map_.find(id);
		if (i == session_box_map_.end())
			return NULL;
		else
			return i->second;
	}

	SessionBox* SessionManager::FindSessionBoxEx(const std::string &id)
	{
		std::map<std::string, SessionBox*>::const_iterator i = session_box_map_ind_.find(id);
		if (i == session_box_map_ind_.end())
			return NULL;
		else
			return i->second;
	}

	void SessionManager::SwapSessionBox(const std::string &id)
	{
		std::map<std::string, SessionBox*>::const_iterator i = session_box_map_ind_.find(id);
		if (i == session_box_map_ind_.end())
		{
			return;
		}
		else
		{
			//SessionBox* tmp =
			session_box_map_[id] = i->second;
		}
	}

	void SessionManager::RemoveSessionBox(std::string id, const SessionBox* box /*=NULL*/)
	{
		auto it_box = session_box_map_.find(id);
		if (it_box != session_box_map_.end())
		{
			if (NULL == box || box == it_box->second)
			{
				session_box_map_.erase(it_box);
			}
			else
			{
				assert(0);
			}
		}

		// octodo: remove cache from ind
		auto it_box_ind = session_box_map_ind_.find(id);
		if (it_box_ind != session_box_map_ind_.end())
		{
			if (NULL == box || box == it_box_ind->second)
			{
				session_box_map_ind_.erase(it_box_ind);
			}
			else
			{
				assert(0);
			}
		}
	}

	nim_comp::SessionBox* SessionManager::CreateSessionBox(const std::string &session_id, nim::NIMSessionType type, bool independent)
	{
		SessionBox *session_box = NULL;
		ISessionDock *session_form = NULL;

		// 如果启用了窗口合并功能，就把新会话盒子都集中创建到某一个会话窗体里
		// 否则每个会话盒子都创建一个会话窗体
		if (enable_merge_)
		{
			if (session_box_map_.size() >= 1)
			{
				session_form = session_box_map_.begin()->second->GetSessionForm();
			}
			else
			{
				session_form = ISessionDock::InstantDock(independent);
				if (session_form == NULL)
				{
					return NULL;
				}
				HWND hwnd = session_form->Create();
				if (hwnd == NULL)
				{
					session_form = NULL;
					return NULL;
				}
				session_form->CenterWindow();
			}

			session_box = session_form->CreateSessionBox(session_id, type);
			if (NULL == session_box)
				return NULL;
		}
		else
		{
			session_form = ISessionDock::InstantDock(independent);
			HWND hwnd = session_form->Create();
			if (hwnd == NULL)
				return NULL;

			session_box = session_form->CreateSessionBox(session_id, type);
			if (NULL == session_box)
				return NULL;

			session_form->CenterWindow();
		}

		// octodo: check simple session box is exists
		SessionBox* simpleSession = FindSessionBox(session_id);
		if (simpleSession) {
			session_box_map_ind_[session_id] = simpleSession;
		}

		session_box_map_[session_id] = session_box;

		return session_box;
	}

	nim_comp::SessionBox* SessionManager::CreateSessionBoxEx(const std::string &session_id, nim::NIMSessionType type)
	{
		SessionBox *session_box = NULL;
		ISessionDock *session_form = NULL;

		session_form = ISessionDock::InstantDock(false);
		HWND hwnd = session_form->Create();
		if (hwnd == NULL)
		{
			session_form = NULL;
			return NULL;
		}
		session_form->CenterWindow();
		session_box = session_form->CreateSessionBox(session_id, type);
		session_box->GetSessionForm(session_form);
		if (NULL == session_box)
			return NULL;
		session_box_map_[session_id] = session_box;
		return session_box;
	}
	void SessionManager::ResetUnread(const std::string &id)
	{
		// 重置对应会话中的@me消息为已读
		ForcePushManager::GetInstance()->ResetAtMeMsg(id);

		// 重置对应回话中的群公告消息为已读
		ForcePushManager::GetInstance()->ResetBoardMsg(id);

		// 重置会话列表未读消息数
		nim_ui::SessionListManager::GetInstance()->InvokeResetSessionUnread(id);
	}

	void SessionManager::QueryMyTeamMemberInfo(const std::string& tid)
	{
		nim::Team::QueryTeamMemberAsync(tid, LoginManager::GetInstance()->GetAccount(), nbase::Bind(&SessionManager::OnQueryMyTeamMemberInfo, this, tid, std::placeholders::_1));
	}

	void SessionManager::QueryMyAllTeamMemberInfos()
	{
		nim::Team::QueryMyAllMemberInfosAsync(nbase::Bind(&SessionManager::OnQueryMyAllTeamMemberInfos, this, std::placeholders::_1, std::placeholders::_2));
	}

	void SessionManager::OnQueryMyTeamMemberInfo(const std::string& tid, const nim::TeamMemberProperty& team_member_info)
	{
		if (team_list_bits_[tid] != team_member_info.GetBits())
		{
			TeamService::GetInstance()->InvokeChangeNotificationMode(tid, team_member_info.GetBits());
		}
		team_list_bits_[tid] = team_member_info.GetBits();
	}

	void SessionManager::OnQueryMyAllTeamMemberInfos(int count, const std::list<nim::TeamMemberProperty>& all_my_member_info_list)
	{
		for (auto it : all_my_member_info_list)
		{
			if (team_list_bits_[it.GetTeamID()] != it.GetBits())
			{
				TeamService::GetInstance()->InvokeChangeNotificationMode(it.GetTeamID(), it.GetBits());
			}
			team_list_bits_[it.GetTeamID()] = it.GetBits();
		}
	}

	bool SessionManager::IsTeamMsgNotify(const std::string& tid, const std::string& sender_id)
	{
		auto it = team_list_bits_.find(tid);
		if (it != team_list_bits_.end())
		{
			if ((it->second & nim::kNIMTeamBitsConfigMaskMuteNotify) == nim::kNIMTeamBitsConfigMaskMuteNotify)
			{
				return false;
			}
			else if ((it->second & nim::kNIMTeamBitsConfigMaskOnlyAdmin) == nim::kNIMTeamBitsConfigMaskOnlyAdmin)
			{
				if (!sender_id.empty())
				{
					auto prop = nim::Team::QueryTeamMemberBlock(tid, sender_id);
					return prop.GetUserType() > nim::kNIMTeamUserTypeNomal;
				}
				return false;
			}
		}
		return true;
	}

	bool SessionManager::IsTeamMsgMuteShown(const std::string& tid, int64_t bits)
	{
		if (bits < 0)
		{
			auto it = team_list_bits_.find(tid);
			if (it == team_list_bits_.end())
				return false;
			bits = it->second;
		}
		if ((bits & nim::kNIMTeamBitsConfigMaskMuteNotify) == nim::kNIMTeamBitsConfigMaskMuteNotify
			|| (bits & nim::kNIMTeamBitsConfigMaskOnlyAdmin) == nim::kNIMTeamBitsConfigMaskOnlyAdmin)
		{
			return true;
		}
		return false;
	}
}