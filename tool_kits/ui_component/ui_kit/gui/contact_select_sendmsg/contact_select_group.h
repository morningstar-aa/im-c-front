#ifndef INVOKE_CHAT_FORM_H_
#define INVOKE_CHAT_FORM_H_

#include "util/window_ex.h"
#include "module/service/user_service.h"
#include "module/service/photo_service.h"
#include "shared/pin_yin_helper.h"
#include "shared/list_item_util.h"

namespace nim_comp
{
/** @class SelectedContactItemUI
  * @brief 被选中的联系人列表项控件
  * @copyright (c) 2016, NetEase Inc. All rights reserved
  * @author Redrain
  * @date 2016/09/19
  */
class SelectedContactItemUI : public ui::HBox
{
public:
	SelectedContactItemUI()
	{
		ui::GlobalManager::FillBoxWithCache(this, L"contact_select/user_photo.xml");
	}
	~SelectedContactItemUI(){};

	/**
	* 初始化控件
	* @param[in] accid 好友用户id或者群组id
	* @param[in] is_team 是否为群组
	* @return void	无返回值
	*/
	void Init(const std::string& accid, bool is_team)
	{
		is_team_ = is_team;
		accid_ = accid;

		SetUTF8Name(accid);
		SetUTF8DataID(accid);
		ui::Label* show_name_label = (ui::Label*)FindSubControl(L"show_name");
		show_name_label->SetText(UserService::GetInstance()->GetUserName(accid));
	}

	/**
	* 是否为群组项
	* @return bool true 是，false 否
	*/
	bool IsTeam() const { return is_team_; }

	/**
	* 获取好友用户id或者群组id
	* @return string 好友用户id或者群组id
	*/
	std::string GetAccountID() const { return accid_; }

private:
	std::string accid_;
	bool is_team_;
};

/** @class ContactListItemUI
  * @brief 联系人列表项控件
  * @copyright (c) 2016, NetEase Inc. All rights reserved
  * @date 2016/09/19
  */
class ContactListItemUI : public ui::ListContainerElement, public ListItemUserData
{
public:
	ContactListItemUI()
	{
		ui::GlobalManager::FillBoxWithCache(this, L"contact_select/start_chat_friend.xml");
	}

	/**
	* 初始化控件
	* @param[in] accid 好友用户id或者群组id
	* @param[in] is_team 是否为群组
	* @return void	无返回值
	*/
	void Init(const std::string& accid, bool is_team)
	{
		is_team_ = is_team;
		accid_ = accid;

		SetUTF8Name(accid);
		SetUTF8DataID(accid);

		ui::Button* head_image_button = (ui::Button*)FindSubControl(L"head_image");
		/*ui::Control* head_image_mask = (ui::Control*)FindSubControl(L"headmask");
		head_image_mask->SetClass(L"checkbox_headimage_mask_40x40");*/
		if (is_team)
			head_image_button->SetBkImage(PhotoService::GetInstance()->GetTeamPhoto(accid, false));
		else
			head_image_button->SetBkImage(PhotoService::GetInstance()->GetUserPhoto(accid));

		ui::Label* show_name_label = (ui::Label*)FindSubControl(L"show_name");
		if (is_team)
			nick_name = TeamService::GetInstance()->GetTeamName(accid);
		else
			nick_name = UserService::GetInstance()->GetUserName(accid);
		show_name_label->SetText(nick_name);

		nick_name = nbase::MakeLowerString(nick_name);
		nick_name_full_spell = nbase::MakeLowerString(PinYinHelper::GetInstance()->ConvertToFullSpell(nick_name));
		nick_name_simple_spell = nbase::MakeLowerString(PinYinHelper::GetInstance()->ConvertToSimpleSpell(nick_name));
	}

	/**
	* 是否为群组项
	* @return bool true 是，false 否
	*/
	bool IsTeam() const { return is_team_; }

	/**
	* 获取好友用户id或者群组id
	* @return string 好友用户id或者群组id
	*/
	std::string GetAccountID() const { return accid_; }

	/**
	* 用联系人昵称、用户名等信息匹配搜索关键字
	* @param[in] search_key 关键字
	* @return bool true 匹配成功，false 匹配失败
	*/
	bool Match(const UTF8String& search_key)
	{
		std::wstring ws_search_key = nbase::UTF8ToUTF16(search_key);
		ws_search_key = nbase::MakeLowerString(ws_search_key);
		if (nick_name.find(ws_search_key) != std::wstring::npos
			|| nick_name_full_spell.find(search_key) != UTF8String::npos
			|| nick_name_simple_spell.find(search_key) != UTF8String::npos)
		{
			return true;
		}
		return false;
	}

private:
	std::string accid_;
	bool is_team_;
};

/** @class ContactTileListUI
  * @brief 联系人列表控件
  * @copyright (c) 2016, NetEase Inc. All rights reserved
  * @date 2016/09/19
  */
class ContactTileListUI : public ui::ListBox
{
public:
	ContactTileListUI() : ListBox(new ui::TileLayout){}
};

/** @class ContactSelectGroup
  * @brief 联系人选择器窗口，提供给多个地方公共使用
  * @copyright (c) 2016, NetEase Inc. All rights reserved
  * @date 2016/09/19
  */
class ContactSelectGroup : public WindowEx
{
public:
	typedef std::function<void(const std::list<UTF8String>& selected_friends, const std::string groupname)> SelectedCompletedCallback;
	static const LPCSTR kCreateGroup;	//创建讨论组
	static const LPCSTR kCreateTeam;	//创建高级群
	static const LPCSTR kRetweetMessage;//转发消息
	static const LPCSTR kSendPersonCard;//转发消息

	/**
	* 构造函数
	* @param[in] uid_or_tid 指定调用联系人选择器的用户id或者群组id，也可以指定kCreateGroup等常量表明本窗口用于创建讨论组等
	* @param[in] exclude_ids 被排除的id列表
	* @param[in] completedCallback 选择完成后的回调函数
	* @param[in] need_select_group 是否需要选择群组
	*/
	ContactSelectGroup(const UTF8String& uid_or_tid, const std::list<UTF8String>& exclude_ids, const SelectedCompletedCallback& completedCallback, std::string group_name, bool is_multi_vchat = false);
	virtual ~ContactSelectGroup();
	
	//覆盖虚函数
	virtual std::wstring GetSkinFolder() override;
	virtual std::wstring GetSkinFile() override;	
	virtual std::wstring GetWindowClassName() const override;
	virtual std::wstring GetWindowId() const override;
	virtual UINT GetClassStyle() const override;
	
	/**
	* 窗口初始化函数
	* @return void	无返回值
	*/
	virtual void InitWindow() override;

private:
	/**
	* 添加一个联系人分类到联系人列表、默认设置为隐藏状态
	* @param[in] tree_node 分类控件指针
	* @return void	无返回值
	*/
	void AddGroup(std::string skey, ui::TreeNode* tree_node);

	/**
	* 添加一个联系人到某个分类中
	* @param[in] accid 用户id
	* @param[in] is_enable 是否处于可选状态
	* @return void	无返回值
	*/
	void AddFriendListItem(const std::string& accid, bool is_enable);

	/**
	* 从某个分类中移除一个联系人
	* @param[in] accid 用户id
	* @return void	无返回值
	*/
	void RemoveFriendListItem(const std::string& accid);
	
	
	/**
	* 添加一个群组到某个分类中
	* @param[in] accid 用户id或者群组id
	* @param[in] is_team 是否为群组
	* @param[in] tile_layout 被添加的分类控件指针
	* @return ui::Box* 被添加的联系人列表项控件的指针
	*/
	ui::Box* AddListItemInGroup(const std::string& accid, bool is_team, ContactTileListUI* tile_layout,int k);

	/**
	* 从某个分类中移除一个群组
	* @param[in] accid 用户id或者群组id
	* @param[in] tile_layout 被移除的分类控件指针
	* @return bool true 成功，false 失败
	*/
	bool RemoveListItemInGroup(const std::string& accid, ContactTileListUI* tile_layout);

	/**
	* 创建一个联系人列表项控件
	* @param[in] accid 用户id或者群组id
	* @param[in] is_team 是否为群组
	* @return ContactListItemUI*联系人列表项控件的指针
	*/
	ContactListItemUI* CreateListItem(const std::string& accid, bool is_team);

	/**
	* 创建一个被选中的联系人列表项控件
	* @param[in] accid 用户id或者群组id
	* @param[in] is_team 是否为群组
	* @return SelectedContactItemUI* 被选中的联系人列表项控件的指针
	*/
	SelectedContactItemUI* CreateSelectedListItem(const std::string& accid, bool is_team);
	
	/**
	* 根据分类类型和标签找到对应的分类控件
	* @param[in] groupType 分类类型
	* @param[in] letter 标签
	* @return ContactTileListUI* 联系人列表控件的指针
	*/
	ContactTileListUI* GetGroup(std::string i);

private:
	/**
	* 处理被选中的联系人列表项控件中删除按钮的单击消息
	* @param[in] param 消息的相关信息
	* @return bool true 继续传递控件消息，false 停止传递控件消息
	*/
	bool OnBtnDeleteClick(const UTF8String& user_id, ui::EventArgs* param);

	/**
	* 处理确认按钮的单击消息
	* @param[in] param 消息的相关信息
	* @return bool true 继续传递控件消息，false 停止传递控件消息
	*/
	bool OnBtnConfirmClick(ui::EventArgs* param);
	//全选
	bool OnSelAllClick(ui::EventArgs* param);
	/**
	* 处理取消按钮的单击消息
	* @param[in] param 消息的相关信息
	* @return bool true 继续传递控件消息，false 停止传递控件消息
	*/
	bool OnBtnCancelClick(ui::EventArgs* param);

	/**
	* 处理搜索输入框内容改变的消息
	* @param[in] param 消息的相关信息
	* @return bool true 继续传递控件消息，false 停止传递控件消息
	*/
	bool OnSearchEditChange(ui::EventArgs* param);

	/**
	* 处理清理输入框按钮的单击消息
	* @param[in] param 消息的相关信息
	* @return bool true 继续传递控件消息，false 停止传递控件消息
	*/
	bool OnClearBtnClick(ui::EventArgs* param);

	/**
	* 处理联系人列表项控件的单击消息
	* @param[in] param 消息的相关信息
	* @return bool true 继续传递控件消息，false 停止传递控件消息
	*/
	bool OnListItemClick(ui::EventArgs* param);

	/**
	* 处理搜索结果列表中列表项控件的单击消息
	* @param[in] param 消息的相关信息
	* @return bool true 继续传递控件消息，false 停止传递控件消息
	*/
	bool OnSearchResultListItemClick(ui::EventArgs* param);

	/**
	* 选中或取消选中某个联系人
	* @param[in] id 用户id或者群组id
	* @param[in] is_team 是否为群组
	* @param[in] check 是否选中
	* @return void	无返回值
	*/
	void OnCheckBox(UTF8String id, bool is_team, bool check);


  
public:
	static const LPCTSTR kClassName;
	list<std::string> _sessions;
	list<std::string> _sessionsQun;
	std::list<std::pair<std::string, std::string>> group_data_;
private:
	UTF8String	uid_or_tid_;
	bool		need_select_group_;
	bool		is_multi_vchat_;
	std::list<UTF8String>		exclude_ids_;
	SelectedCompletedCallback	completedCallback_;
	std::string group_name_;
	ui::CheckBox	*checkall_;
	ui::Label		*tool_tip_content_;
	ui::Button		*btn_clear_input_;	
	ui::TreeView	*contract_list_;
	//vector<ContactTileListUI*>  tree_node_ver_;
	map<std::string, ContactTileListUI*> tree_node_ver_;
	ui::RichEdit	*search_edit_;
	ui::ListBox		*selected_user_list_;
	ui::ListBox		*search_result_list_;
	
	AutoUnregister	unregister_cb;
};
}
#endif // INVOKE_CHAT_FORM_H_
