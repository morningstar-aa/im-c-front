#include "nim_ui_contacts_list_manager.h"
#include "module/subscribe_event/subscribe_event_manager.h"
#include "gui/main/friend_list.h"
#include "gui/main/group_list.h"
#include "gui/main/control/friend_item.h"
#include "export/nim_ui_user_manager.h"
#include "export/nim_ui_login_manager.h"

namespace nim_ui
{
void ContactsListManager::AttachFriendListBox(ui::TreeView *list_box)
{
	if (NULL == list_box)
	{
		friend_list_ = nullptr;
	}
	else
	{
		friend_list_.reset(new nim_comp::FriendList(list_box));
	}
}

void ContactsListManager::AttachGroupListBox(ui::TreeView *list_box)
{
	if (NULL == list_box)
	{
		group_list_ = nullptr;
	}
	else
	{
		group_list_.reset(new nim_comp::GroupList(list_box));
	}
}

void ContactsListManager::SaveCreateTeamInfo(const nim::TeamEvent& result)
{
	std::string userId = nim_ui::LoginManager::GetInstance()->GetAccount();
	nim::NIMTeamMemberAddFriendMode addFriend = result.team_info_.GetMemberAddFriendMode();
	nim_ui::UserManager::GetInstance()->InvokeCreateTeam(result.team_id_, result.team_info_.GetName(), userId, addFriend, ToWeakCallback([this](int code, const std::string &msg) {
		if (code == 0)
		{
		}
		else
		{
			//nim_ui::ShowToast(nbase::UTF8ToUTF16(msg), 2000);
		}
	}));
}

void ContactsListManager::UpdateTeamInfo(const nim::TeamEvent& result)
{
	std::string userId = nim_ui::LoginManager::GetInstance()->GetAccount();
	nim::NIMTeamMemberAddFriendMode addFriend = result.team_info_.GetMemberAddFriendMode();
	nim_ui::UserManager::GetInstance()->InvokeUpdateTeamInfo(result.team_id_, result.team_info_.GetName(), userId, addFriend, ToWeakCallback([this](int code, const std::string &msg) {
		if (code == 0)
		{
		}
		else
		{
		}
	}));
}

void ContactsListManager::DissmissTeam(const nim::TeamEvent& result)
{
	nim_ui::UserManager::GetInstance()->InvokeDissmissTeam(result.team_id_, ToWeakCallback([this](int code, const std::string &msg) {
		if (code == 0)
		{
		}
		else
		{
		}
	}));
}

void ContactsListManager::InvokeGetAllUserInfo()
{
	ui::TreeNode* friend_list_root = friend_list_->GetFriendList()->GetRootNode();
	for (std::size_t i = 2; i < friend_list_root->GetChildNodeCount(); i++)
		friend_list_root->GetChildNode(i)->RemoveAllChildNode();

	nim_comp::UserService::GetInstance()->InvokeGetAllUserInfo(nbase::Bind(&ContactsListManager::OnGetAllFriendInfo, this, std::placeholders::_1));
}

void ContactsListManager::OnGetAllFriendInfo(const std::list<nim::UserNameCard> &uinfos)
{
	nim_comp::SubscribeEventManager::GetInstance()->SubscribeAllFriendEvent();

	if (NULL != friend_list_)
	{
		friend_list_->OnGetFriendList(uinfos);
	}
}

void ContactsListManager::FillSearchResultList(ui::ListBox* search_result_list, const UTF8String& search_key)
{
	if (NULL == search_result_list)
		return;

	std::list<UTF8String> searched_ids;
	search_result_list->RemoveAll();

	if (NULL != friend_list_)
	{
		for (int i = 0; i < friend_list_->GetFriendList()->GetCount(); i++)
		{
			nim_comp::FriendItem* friend_item = dynamic_cast<nim_comp::FriendItem*>(friend_list_->GetFriendList()->GetItemAt(i));
			if (friend_item && friend_item->Match(search_key))
			{
				nim_comp::FriendItem* search_res_friend_item = new nim_comp::FriendItem;
				ui::GlobalManager::FillBoxWithCache(search_res_friend_item, L"main/friend_item.xml");
				search_res_friend_item->Init(friend_item->GetFriendItemType(), friend_item->GetId());
				if (std::find(searched_ids.begin(), searched_ids.end(), friend_item->GetId()) == searched_ids.end())
				{
					search_result_list->Add(search_res_friend_item);
				}
			}
		}
	}

	if (NULL != group_list_)
	{
		for (int i = 0; i < group_list_->GetGroupList()->GetCount(); i++)
		{
			nim_comp::FriendItem* friend_item = dynamic_cast<nim_comp::FriendItem*>(group_list_->GetGroupList()->GetItemAt(i));
			if (friend_item && friend_item->Match(search_key))
			{
				nim_comp::FriendItem* search_res_friend_item = new nim_comp::FriendItem;
				ui::GlobalManager::FillBoxWithCache(search_res_friend_item, L"main/friend_item.xml");
				search_res_friend_item->Init(friend_item->GetFriendItemType(), friend_item->GetId());
				if (std::find(searched_ids.begin(), searched_ids.end(), friend_item->GetId()) == searched_ids.end())
				{
					search_result_list->Add(search_res_friend_item);
				}
			}
		}
	}

}

}

