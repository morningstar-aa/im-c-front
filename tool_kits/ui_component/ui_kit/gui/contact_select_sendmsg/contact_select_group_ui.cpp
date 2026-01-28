#include "contact_select_group.h"
#include "module/login/login_manager.h"
#include "av_kit/module/video/video_manager.h"
#include "nim_service/module/service/session_service.h"
#include "tool_kits/ui_component/ui_kit/export/nim_ui_session_list_manager.h"
#include "../../../../../nim_win_demo/module/db/public_db.h"

using namespace ui;
using namespace std;

namespace nim_comp
{
const LPCSTR ContactSelectGroup::kCreateGroup = "CreateGroup"; 

const LPCTSTR ContactSelectGroup::kClassName = _T("ContactSelectGroup");

ContactSelectGroup::ContactSelectGroup(const UTF8String& uid_or_tid,
	const std::list<UTF8String>& exclude_ids,
	const SelectedCompletedCallback& completedCallback,
	std::string group_name/* = false*/, bool is_multi_vchat)
	: uid_or_tid_(uid_or_tid)
	, exclude_ids_(exclude_ids)
	, completedCallback_(completedCallback)
	, group_name_(group_name)
	, is_multi_vchat_(is_multi_vchat) 
{
	need_select_group_ = false;
	  
	
}

ContactSelectGroup::~ContactSelectGroup()
{
}

std::wstring ContactSelectGroup::GetSkinFolder()
{
	return L"contact_select";
}

std::wstring ContactSelectGroup::GetSkinFile()
{
	return L"contact_select_form.xml";
}

std::wstring ContactSelectGroup::GetWindowClassName() const
{
	return kClassName;
}

std::wstring ContactSelectGroup::GetWindowId() const
{
	std::wstring uid_or_tid = nbase::UTF8ToUTF16(uid_or_tid_);
	return uid_or_tid;
}

UINT ContactSelectGroup::GetClassStyle() const
{
	return (UI_CLASSSTYLE_FRAME | CS_DBLCLKS);
}
 
void ContactSelectGroup::InitWindow()
{
	std::wstring title = L"创建分组";
	SetTaskbarTitle(title);
	((Label*)FindControl(L"title"))->SetText(title);

	contract_list_ = (ui::TreeView*)FindControl(L"user_list");
	contract_list_->AttachBubbledEvent(kEventClick, nbase::Bind(&ContactSelectGroup::OnListItemClick, this, std::placeholders::_1));
	search_result_list_ = static_cast<ui::ListBox*>(FindControl(L"search_result"));
	search_result_list_->AttachBubbledEvent(kEventClick, nbase::Bind(&ContactSelectGroup::OnSearchResultListItemClick, this, std::placeholders::_1));
	selected_user_list_ = (ui::ListBox*)FindControl(L"selected_user_list");

	TreeNode* tree_node = ListItemUtil::CreateFirstLetterListItem(L"我的好友");
	AddGroup("我的好友",tree_node);

	std::string userId = LoginManager::GetInstance()->GetAccount();
	group_data_ = PublicDB::GetInstance()->GetGroupFriend(userId);
	for (auto it = group_data_.begin(); it != group_data_.end(); it++)
	{
		if (it->second == ""){
			wchar_t letter = L'';
			wstring letter_str = nbase::UTF8ToUTF16(it->first);
			letter_str += letter;
			tree_node = ListItemUtil::CreateFirstLetterListItem(letter_str);
			AddGroup(it->first,tree_node);
		}
	}

	tool_tip_content_ = static_cast<ui::Label*>(FindControl(L"tool_tip"));
	search_edit_ = (RichEdit*)FindControl(L"search");
	search_edit_->AttachTextChange(nbase::Bind(&ContactSelectGroup::OnSearchEditChange, this, std::placeholders::_1));
	search_edit_->SetLimitText(30);
	btn_clear_input_ = (Button*)FindControl(L"clear_input");
	btn_clear_input_->SetNoFocus();
	btn_clear_input_->AttachClick(nbase::Bind(&ContactSelectGroup::OnClearBtnClick, this, std::placeholders::_1));

	checkall_ = (CheckBox*)FindControl(L"check_all");
	checkall_->AttachSelect(nbase::Bind(&ContactSelectGroup::OnSelAllClick, this, std::placeholders::_1));
	checkall_->AttachUnSelect(nbase::Bind(&ContactSelectGroup::OnSelAllClick, this, std::placeholders::_1));

	ui::Button* btn_confirm = (ui::Button*)FindControl(L"confirm");
	btn_confirm->AttachClick(nbase::Bind(&ContactSelectGroup::OnBtnConfirmClick, this, std::placeholders::_1));
	ui::Button* btn_cancel = (ui::Button*)FindControl(L"cancel");
	btn_cancel->AttachClick(nbase::Bind(&ContactSelectGroup::OnBtnCancelClick, this, std::placeholders::_1));
	
	if (is_multi_vchat_)
	{
	//	tool_tip_content_->SetText(ui::MutiLanSupport::GetInstance()->GetStringViaID(L"STRING_MULTIVIDEOCHATFORM_SEARCH_ORG_EMPTY").c_str());

	//	std::wstring title_id;
	//	title_id = L"STRING_INVITEUSERFORM_MULTI_VCHAT";
	//	std::wstring title = ui::MutiLanSupport::GetInstance()->GetStringViaID(title_id);
	//	SetTaskbarTitle(title);
	//	((Label*)FindControl(L"title"))->SetText(title);
	//	title_id = L"STRING_INVITEUSERFORM_TEAM_MEMBERS";
	//	title = ui::MutiLanSupport::GetInstance()->GetStringViaID(title_id);
	//	// 		((Label*)FindControl(L"label_contact"))->SetText(title);
	//	title_id = L"STRING_INVITEUSERFORM_START_VCHAT";
	//	title = ui::MutiLanSupport::GetInstance()->GetStringViaID(title_id);
	//	btn_confirm->SetText(title);
	//	//TeamService* team_service = TeamService::GetInstance();
	//	nim::Team::QueryTeamMembersAsync(uid_or_tid_, nbase::Bind(&ContactSelectGroup::OnGetTeamMembers, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	//
	}
	else//cjy3
	{
		/*_sessionsQun = nim_ui::SessionListManager::GetInstance()->GetAllSession(true);
		list<string>::iterator testiterator;
		int k = -1;
		ContactTileListUI* tile_layout = GetGroup(GT_TEAM);
		for (testiterator = _sessionsQun.begin(); testiterator != _sessionsQun.end(); ++testiterator)
		{
			k = k + 1;
			ListContainerElement* contain = (ListContainerElement*)AddListItemInGroup(*testiterator, true, tile_layout, k);
		}
		_sessions = nim_ui::SessionListManager::GetInstance()->GetAllSession(false);
		k = -1;

		ContactTileListUI* tile_layout2 = GetGroup(GT_COMMON_NUMBER);
		for (testiterator = _sessions.begin(); testiterator != _sessions.end(); ++testiterator)
		{
			k = k + 1;
			ListContainerElement* contain = (ListContainerElement*)AddListItemInGroup(*testiterator, false, tile_layout2, k);
		}*/
		tool_tip_content_->SetText(ui::MutiLanSupport::GetInstance()->GetStringViaID(L"STRING_INVOKECHATFORM_SEARCH_TOOLTIP").c_str());
		// 添加好友
		UTF8String current_user_id = LoginManager::GetInstance()->GetAccount();
		UserService* user_service = UserService::GetInstance();
		const std::map<std::string, nim::UserNameCard>& users = user_service->GetAllUserInfos();
		MuteBlackService* mb_service = MuteBlackService::GetInstance();
		for (auto &it : users)
		{
			if (it.first != current_user_id &&
				user_service->GetUserType(it.second.GetAccId()) == nim::kNIMFriendFlagNormal &&
				!mb_service->IsInBlackList(it.first))
			{
				AddFriendListItem(it.second.GetAccId(), true);
			}
		}
 
	}
}  

bool ContactSelectGroup::OnSelAllClick(ui::EventArgs* param)
{
	bool b = checkall_->IsSelected();
	for (auto it = tree_node_ver_.begin(); it != tree_node_ver_.end(); it++)
	{
		ContactListItemUI* listitem = (ContactListItemUI*)it->second;
		
		if (listitem)
		{
			UTF8String id = "";
			for (int i = 0; i < listitem->GetCount(); i++){
				ContactListItemUI* temp = (ContactListItemUI*)listitem->GetItemAt(i);
				id = temp->GetUTF8DataID();
				if (id != ""){
					ui::CheckBox* checkbox = (ui::CheckBox*)temp->FindSubControl(L"checkbox");
					checkbox->Selected(b);
					OnCheckBox(id, listitem->IsTeam(), b);
				}
			}
		}
	}
	return true;
}

bool ContactSelectGroup::OnBtnDeleteClick(const UTF8String& user_id, ui::EventArgs* param)
{
	for (int i = 0; i < selected_user_list_->GetCount(); i++)
	{
		Control* item = selected_user_list_->GetItemAt(i);
		if (item->GetUTF8DataID() == user_id)
		{
			selected_user_list_->RemoveAt(i);
		}
	}

	if (search_result_list_->IsVisible())
	{
		for (int i = 0; i < search_result_list_->GetCount(); i++)
		{
			Control* item = search_result_list_->GetItemAt(i);
			if (item->GetUTF8DataID() == user_id)
			{
				EventArgs args;
				args.pSender = item;
				OnSearchResultListItemClick(&args);
			}
		}
	}
	else{
		for (auto it = tree_node_ver_.begin(); it != tree_node_ver_.end(); it++)
		{
			ContactListItemUI* item = (ContactListItemUI*)it->second;
			if (item->GetUTF8DataID() == user_id)
			{
				EventArgs args;
				args.pSender = item;
				OnListItemClick(&args);
			}
		}
	}
	return true;
}

bool ContactSelectGroup::OnBtnConfirmClick(ui::EventArgs* param)
{
	std::list<UTF8String> friend_list;
	for (int i = 0; i < selected_user_list_->GetCount(); i++)
	{
		SelectedContactItemUI* select_item = dynamic_cast<SelectedContactItemUI*>(selected_user_list_->GetItemAt(i));
		UTF8String id = select_item->GetAccountID();
		if (!select_item->IsTeam())
			friend_list.push_back(id);
	}

	completedCallback_(friend_list, group_name_);
	Close();
	return true;
}

bool ContactSelectGroup::OnBtnCancelClick(ui::EventArgs* param)
{
	Close();
	return true;
}

bool ContactSelectGroup::OnSearchEditChange(ui::EventArgs* param)
{
	UTF8String search_key = search_edit_->GetUTF8Text();
	if(search_key.empty())
	{
		search_result_list_->RemoveAll();
		search_result_list_->SetVisible(false);
		btn_clear_input_->SetVisible(false);
		tool_tip_content_->SetVisible(false);
	}
	else
	{
		search_result_list_->RemoveAll();
		for (auto it = tree_node_ver_.begin(); it != tree_node_ver_.end(); it++)
		{
			ContactListItemUI* listitemx = (ContactListItemUI*)it->second;

			if (listitemx)
			{
				UTF8String id = "";
				for (int i = 0; i < listitemx->GetCount(); i++){
					ContactListItemUI* user_listitem = (ContactListItemUI*)listitemx->GetItemAt(i);
					if (user_listitem->Match(search_key))
					{
						ui::CheckBox* checkbox = (ui::CheckBox*)user_listitem->FindSubControl(L"checkbox");
						auto search_listitem = CreateListItem(user_listitem->GetAccountID(), user_listitem->IsTeam());
						ui::CheckBox* search_checkbox = (ui::CheckBox*)search_listitem->FindSubControl(L"checkbox");
						search_checkbox->Selected(checkbox->IsSelected());
						search_result_list_->Add(search_listitem);
					}
				}
			}
		}

		search_result_list_->SetVisible(true);
		int count = search_result_list_->GetCount();
		if(count > 0)
			tool_tip_content_->SetVisible(false);
		else
			tool_tip_content_->SetVisible(true);

		btn_clear_input_->SetVisible(true);
	}

	return true;
}

bool ContactSelectGroup::OnClearBtnClick(ui::EventArgs* param)
{
	btn_clear_input_->SetVisible(false);
	tool_tip_content_->SetVisible(false);

	search_edit_->SetText(L"");

	return false;
}

bool ContactSelectGroup::OnListItemClick(ui::EventArgs* param)
{
	ContactListItemUI* listitem = dynamic_cast<ContactListItemUI*>(param->pSender);
	if (listitem)
	{
		UTF8String id = listitem->GetUTF8DataID();
		ui::CheckBox* checkbox = (ui::CheckBox*)listitem->FindSubControl(L"checkbox");
		checkbox->Selected(!checkbox->IsSelected(), false);
		if (!checkbox->IsSelected()) {
			listitem->Selected(false, true);
		}
		OnCheckBox(id, listitem->IsTeam(), checkbox->IsSelected());
	}

	return true;
}

bool ContactSelectGroup::OnSearchResultListItemClick(ui::EventArgs* param)
{
	OnListItemClick(param);

	for (auto it = tree_node_ver_.begin(); it != tree_node_ver_.end(); it++)
	{
		ContactListItemUI* listitem = (ContactListItemUI*)it->second;
			//auto listitem = (ContactListItemUI*)(it->GetItemAt(i));
			if (listitem->GetUTF8DataID() == param->pSender->GetUTF8DataID())
			{
				ui::CheckBox* checkbox = (ui::CheckBox*)listitem->FindSubControl(L"checkbox");
				checkbox->Selected(!checkbox->IsSelected(), false);
				if (!checkbox->IsSelected()) {
					listitem->Selected(false, true);
				}
			} 
	}

	return true;
}

void ContactSelectGroup::OnCheckBox(UTF8String id, bool is_team, bool check)
{
	if (check)
	{
		SelectedContactItemUI* selected_listitem = CreateSelectedListItem(id, is_team);
		selected_user_list_->Add(selected_listitem);
		selected_user_list_->EndDown();
	}
	else
	{
		for (int i = 0; i < selected_user_list_->GetCount(); i++)
		{
			SelectedContactItemUI* listitem = (SelectedContactItemUI*)selected_user_list_->GetItemAt(i);
			if (listitem->GetUTF8DataID() == id)
			{
				selected_user_list_->RemoveAt(i);
				break;
			}
		}
	}
}

}