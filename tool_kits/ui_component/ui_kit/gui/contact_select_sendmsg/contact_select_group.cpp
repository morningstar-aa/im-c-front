#include "contact_select_group.h"
#include "util/windows_manager.h"
#include "shared/pin_yin_helper.h"
#include "shared/tool.h"
#include "module/login/login_manager.h"
#include "module/service/mute_black_service.h"
#include "module/db/user_db.h"
#include "../../../../../nim_win_demo/module/db/public_db.h"

using namespace ui;
using namespace std;

namespace nim_comp
{
	void ContactSelectGroup::AddGroup(std::string skey,ui::TreeNode* tree_node)
{
	tree_node->SetEnabled(false);
	contract_list_->GetRootNode()->AddChildNode(tree_node);

	TreeNode* tree_node2 = new TreeNode();
	ContactTileListUI* tile_layout = new ContactTileListUI;
	ui::GlobalManager::FillBox(tile_layout, L"contact_select/contact_select_tile_list.xml");

	RECT rect = { 5, 5, 5, 5 };
	tree_node2->GetLayout()->SetPadding(rect);
	tree_node2->Add(tile_layout);
	tree_node2->SetFixedWidth(DUI_LENGTH_AUTO);
	tree_node2->SetFixedHeight(DUI_LENGTH_AUTO);

	tree_node->AddChildNode(tree_node2);
	tree_node_ver_.insert(make_pair(skey, tile_layout)); 
}

//cjy3
void ContactSelectGroup::AddFriendListItem(const std::string& accid, bool is_enable)
{
	std::string skey = "我的好友";
	for (auto it = group_data_.begin(); it != group_data_.end(); it++)
	{ 
		if (it->second == accid)
		{
			skey = it->first;
			break;
		}
	}
	ContactTileListUI* tile_layout = GetGroup(skey);
	list<string>::iterator testiterator;
	for (testiterator = _sessions.begin(); testiterator != _sessions.end(); ++testiterator)
	{
		if (*testiterator == accid){
			return;
		}
	}
	 

	ListContainerElement* contain = (ListContainerElement*)AddListItemInGroup(accid, false, tile_layout,-1);
	if (contain)
	{
		//contain->AttachSelect(notify);
		CheckBox* ckb = (CheckBox*)contain->FindSubControl(L"checkbox");
		if (!is_enable)
		{
			ckb->Selected(true);
			contain->SetEnabled(false);
		}
	}
}

void ContactSelectGroup::RemoveFriendListItem(const std::string& accid)
{
	wstring ws_show_name = UserService::GetInstance()->GetUserName(accid);
	string spell = PinYinHelper::GetInstance()->ConvertToFullSpell(ws_show_name);
	wstring ws_spell = nbase::UTF8ToUTF16(spell);
	ContactTileListUI* tile_layout;
	if (!ws_spell.empty())
	{
		tile_layout = GetGroup(0);
	}
	else
	{
		tile_layout = GetGroup(0);
	}
	RemoveListItemInGroup(accid, tile_layout);

	//从已选列表和搜索列表中删除
	auto selected_item = selected_user_list_->FindSubControl(nbase::UTF8ToUTF16(accid));
	if (selected_item != NULL)
		selected_user_list_->Remove(selected_item);
	auto search_item = search_result_list_->FindSubControl(nbase::UTF8ToUTF16(accid));
	if (search_item != NULL)
		search_result_list_->Remove(search_item);
} 
ui::Box* ContactSelectGroup::AddListItemInGroup(const std::string& accid, bool is_team, ContactTileListUI* tile_layout,int k)
{
	if (tile_layout->GetCount() == 0)
	{
		((TreeNode*)tile_layout->GetParent())->GetParentNode()->SetVisible(true);
	}

	ContactListItemUI* container_element = CreateListItem(accid, is_team);
	tile_layout->Add(container_element);
	return container_element;
}

bool ContactSelectGroup::RemoveListItemInGroup(const std::string& accid, ContactTileListUI* tile_layout)
{
	bool ret = false;
	int index = 0;
	for (index = 0; index < tile_layout->GetCount(); index++)
	{
		ContactListItemUI* temp = (ContactListItemUI*)tile_layout->GetItemAt(index);
		if (accid == temp->GetUTF8DataID())
		{
			OnCheckBox(accid, temp->IsTeam(), false); //同时从已选择列表中删除这个帐号
			tile_layout->RemoveAt(index);
			ret = true;
			break;
		}
	}

	if (tile_layout->GetCount() == 0)
	{
		((TreeNode*)tile_layout->GetParent())->GetParentNode()->SetVisible(false);
	}

	return ret;
}

ContactListItemUI* ContactSelectGroup::CreateListItem(const std::string& accid, bool is_team)
{
	ContactListItemUI* container_element = new ContactListItemUI();
	container_element->Init(accid, is_team);
	
	if (std::find(exclude_ids_.begin(), exclude_ids_.end(), accid) != exclude_ids_.end())
	{
		((ui::CheckBox*)container_element->FindSubControl(L"checkbox"))->Selected(true);
		container_element->SetEnabled(false);
	}

	return container_element;
}

SelectedContactItemUI* ContactSelectGroup::CreateSelectedListItem(const std::string& accid, bool is_team)
{
	SelectedContactItemUI* selected_item = new SelectedContactItemUI();
	selected_item->Init(accid, is_team);

	Button* btn_delete = (Button*)selected_item->FindSubControl(L"delete");
	btn_delete->AttachClick(nbase::Bind(&ContactSelectGroup::OnBtnDeleteClick, this, accid, std::placeholders::_1));

	return selected_item;
}

ContactTileListUI* ContactSelectGroup::GetGroup(std::string i)
{
	return tree_node_ver_[i];
}
 
 

}