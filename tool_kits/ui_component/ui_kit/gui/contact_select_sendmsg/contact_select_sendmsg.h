#ifndef INVOKE_CHAT_FORM_H_
#define INVOKE_CHAT_FORM_H_

#include "util/window_ex.h"
#include "module/service/user_service.h"
#include "module/service/photo_service.h"
#include "shared/pin_yin_helper.h"
#include "shared/list_item_util.h"
#include "gui/emoji/emoji_form.h"

namespace nim_comp
{
/** @class SelContactItemSendUI
  * @brief 被选中的联系人列表项控件
  * @copyright (c) 2016, NetEase Inc. All rights reserved
  * @author Redrain
  * @date 2016/09/19
  */
class SelContactItemSendUI : public ui::HBox
{
public:
	SelContactItemSendUI()
	{
		ui::GlobalManager::FillBoxWithCache(this, L"contact_select/user_photo.xml");
	}
	~SelContactItemSendUI(){};

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
		if (is_team)
			show_name_label->SetText(TeamService::GetInstance()->GetTeamName(accid));
		else
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

/** @class ContactListItemSendUI
  * @brief 联系人列表项控件
  * @copyright (c) 2016, NetEase Inc. All rights reserved
  * @date 2016/09/19
  */
class ContactListItemSendUI : public ui::ListContainerElement, public ListItemUserData
{
public:
	ContactListItemSendUI()
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

/** @class ContactTileListSendUI
  * @brief 联系人列表控件
  * @copyright (c) 2016, NetEase Inc. All rights reserved
  * @date 2016/09/19
  */
class ContactTileListSendUI : public ui::ListBox
{
public:
	ContactTileListSendUI() : ListBox(new ui::TileLayout){}
};

/** @class ContactSelectSendForm
  * @brief 联系人选择器窗口，提供给多个地方公共使用
  * @copyright (c) 2016, NetEase Inc. All rights reserved
  * @date 2016/09/19
  */
class ContactSelectSendForm : public WindowEx
{
public:
	typedef std::function<void(const std::list<UTF8String>& selected_friends, const std::list<UTF8String>& selected_teams)> SelectedCompletedCallback;
	static const LPCSTR kCreateGroup;	//创建讨论组
	static const LPCSTR kCreateTeam;	//创建高级群
	static const LPCSTR kRetweetMessage;//转发消息

	/**
	* 构造函数
	* @param[in] uid_or_tid 指定调用联系人选择器的用户id或者群组id，也可以指定kCreateGroup等常量表明本窗口用于创建讨论组等
	* @param[in] exclude_ids 被排除的id列表
	* @param[in] completedCallback 选择完成后的回调函数
	* @param[in] need_select_group 是否需要选择群组
	*/
	ContactSelectSendForm(const UTF8String& uid_or_tid, const std::list<UTF8String>& exclude_ids, const SelectedCompletedCallback& completedCallback, bool need_select_group = false, bool is_multi_vchat = false);
	virtual ~ContactSelectSendForm();
	
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
	void AddGroup(ui::TreeNode* tree_node);

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
	* @param[in] teamid 群组id
	* @param[in] is_enable 是否处于可选状态
	* @return void	无返回值
	*/
	void AddTeamListItem(const std::string& teamid, bool is_enable);

	/**
	* 从某个分类中移除一个群组
	* @param[in] teamid 群组id
	* @return void	无返回值
	*/
	void RemoveTeamListItem(const std::string& teamid);

	/**
	* 添加一个群组到某个分类中
	* @param[in] accid 用户id或者群组id
	* @param[in] is_team 是否为群组
	* @param[in] tile_layout 被添加的分类控件指针
	* @return ui::Box* 被添加的联系人列表项控件的指针
	*/
	ui::Box* AddListItemInGroup(const std::string& accid, bool is_team, ContactTileListSendUI* tile_layout);

	/**
	* 从某个分类中移除一个群组
	* @param[in] accid 用户id或者群组id
	* @param[in] tile_layout 被移除的分类控件指针
	* @return bool true 成功，false 失败
	*/
	bool RemoveListItemInGroup(const std::string& accid, ContactTileListSendUI* tile_layout);

	/**
	* 创建一个联系人列表项控件
	* @param[in] accid 用户id或者群组id
	* @param[in] is_team 是否为群组
	* @return ContactListItemSendUI*联系人列表项控件的指针
	*/
	ContactListItemSendUI* CreateListItem(const std::string& accid, bool is_team);

	/**
	* 创建一个被选中的联系人列表项控件
	* @param[in] accid 用户id或者群组id
	* @param[in] is_team 是否为群组
	* @return SelContactItemSendUI* 被选中的联系人列表项控件的指针
	*/
	SelContactItemSendUI* CreateSelectedListItem(const std::string& accid, bool is_team);
	
	/**
	* 根据分类类型和标签找到对应的分类控件
	* @param[in] groupType 分类类型
	* @param[in] letter 标签
	* @return ContactTileListSendUI* 联系人列表控件的指针
	*/
	ContactTileListSendUI* GetGroup(GroupType groupType, wchar_t letter = L'\0');

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

	//全选
	bool OnSelAllClick(ui::EventArgs* param);
	
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

private:
	/**
	* 响应好友名单改变的回调函数
	* @param[in] change_type 好友变化类型
	* @param[in] accid 好友用户id
	* @return void 无返回值
	*/
	void OnFriendListChange(FriendChangeType change_type, const std::string& accid);

	/**
	* 响应黑名单改变的回调函数
	* @param[in] id 用户id
	* @param[in] black 是否加入黑名单
	* @return void 无返回值
	*/
	void OnSetBlackCallback(const std::string& id, bool black);

	/**
	* 响应用户信息改变的回调函数
	* @param[in] uinfos 用户信息列表
	* @return void 无返回值
	*/
	void OnUserInfoChange(const std::list<nim::UserNameCard> &uinfos);

	/**
	* 响应用户头像改变的回调函数
	* @param[in] type 头像类型
	* @param[in] account 用户id
	* @param[in] photo_path 头像路径
	* @return void 无返回值
	*/
	void OnUserPhotoReady(PhotoType type, const std::string& account, const std::wstring& photo_path);

	/**
	* 响应新增群或者讨论组回调函数
	* @param[in] teamid 群组id
	* @param[in] tname 群组名字
	* @param[in] type 群组类型
	* @return void 无返回值
	*/
	void OnAddTeam(const std::string& teamid, const std::string& tname, nim::NIMTeamType type);

	/**
	* 响应移除群或者讨论组回调函数
	* @param[in] teamid 群组id
	* @return void 无返回值
	*/
	void OnRemoveTeam(const std::string& teamid);

	/**
	* 响应群或者讨论组名称改变回调函数
	* @param[in] team_info 群组信息
	* @return void 无返回值
	*/
	void OnTeamNameChanged(const nim::TeamInfo& team_info);

	void OnGetTeamMembers(const std::string& team_id, int count, const std::list<nim::TeamMemberProperty>& team_member_list);

	/**
	* 单击表情回调
	*/
	bool OnClickBtnFaceCallBack(ui::EventArgs* param);
	//void OnClickBtnFaceCallBack(ui::EventArgs* param);

	/**
	* 响应表情按钮消息
	* @return void 无返回值
	*/
	void OnBtnEmoji();

	/**
	* 某个表情被选择后的回调函数
	* @param[in] emo		 选择的表情
	* @return void 无返回值
	*/
	void OnEmotionSelected(std::wstring emo);

	/**
	* 某个贴图表情被选择后的回调函数
	* @param[in] catalog	贴图所在目录
	* @param[in] name		贴图名字
	* @return void 无返回值
	*/
	void OnEmotionSelectedSticker(const std::wstring &catalog, const std::wstring &name);

	/**
	* 表情选择窗体关闭后的回调函数
	* @return void 无返回值
	*/
	void OnEmotionClosed();

	/**
	* 某个收藏表情被选择后的回调函数
	* @param[in] catalog	贴图所在目录
	* @param[in] name		贴图名字
	* @return void 无返回值
	*/
	void OnEmotionSelectedCustom(const std::wstring &catalog, const std::wstring &name);

	/**
	* 删除自定义图片回调函数
	* @param[in] catalog	贴图所在目录
	* @param[in] name		贴图名字
	* @return void 无返回值
	*/
	void OnEmotionDeleteCustom(const std::wstring &catalog, const std::wstring &name);

	/**
	* 处理消息编辑控件的所有消息
	* @param[in] param 消息的相关信息
	* @return bool true 继续传递控件消息，false 停止传递控件消息
	*/
	//bool EditNotify(ui::EventArgs* param);

	/**
	* 发送一条贴图消息
	* @param[in] catalog 贴图所属的分类
	* @param[in] name 贴图名字
	* @return void	无返回值
	*/
	void SendSticker(const std::string &catalog, const std::string &name, const std::string id);

	/**
	* 自动填充nim::IMMessage结构体一些基本的字段，方便发送消息时使用
	* @param[out] msg 会话消息
	* @return void	无返回值
	*/
	void PackageMsg(nim::IMMessage &msg, const std::string id = "");

	/**
	* 在界面上添加一个当前正在被发送的消息
	* @param[in] msg 会话消息
	* @return void	无返回值
	*/
	//void AddSendingMsg(const nim::IMMessage &msg);

	/**
	* 图片被选择后的回调函数
	* @param[in] is_snapchat 是否阅后即焚
	* @param[in] ret		 是否选择了图片
	* @param[in] file_path	 选择的图片的路径
	* @return void 无返回值
	*/
	//void OnImageSelected(bool is_snapchat, BOOL ret, std::wstring file_path);

	/**
	* 处理图片按钮单击消息
	* @param[in] param 消息的相关信息
	* @return bool true 继续传递控件消息，false 停止传递控件消息
	*/
	bool OnClickedImage(ui::EventArgs* param);

	/**
	* 处理小视频按钮单击消息
	* @param[in] param 消息的相关信息
	* @return bool true 继续传递控件消息，false 停止传递控件消息
	*/
	bool OnClickedMiniv(ui::EventArgs* param);

	/**
	* 处理文件按钮单击消息
	* @param[in] param 消息的相关信息
	* @return bool true 继续传递控件消息，false 停止传递控件消息
	*/
	bool OnClickedFile(ui::EventArgs* param);

	/**
	* 处理发送按钮单击消息
	* @param[in] param 消息的相关信息
	* @return bool true 继续传递控件消息，false 停止传递控件消息
	*/
	bool OnClickedSendBnt(ui::EventArgs* param);

	/**
	* 响应发送文件按钮消息
	* @return void 无返回值
	*/
	void OnBtnFile();

	/**
	* 文件被选择后的回调函数
	* @param[in] ret		 是否选择了文件
	* @param[in] file_path	 选择的文件的路径
	* @return void 无返回值
	*/
	void OnFileSelected(BOOL ret, std::wstring file_path);

	/**
	* 检测文件大小是否符合发送要求
	* @param[in] src 文件路径
	* @return bool true 符合，false 不符合
	*/
	bool CheckFileSize(const std::wstring &src);

	/**
	* 响应发送图片按钮消息
	* @param[in] is_snapchat 是否阅后即焚
	* @return void 无返回值
	*/
	void OnBtnImage(bool is_snapchat);

	/**
	* 响应发送小视频按钮消息
	* @return void 无返回值
	*/
	void OnBtnMiniv();

	/**
	* 图片被选择后的回调函数
	* @param[in] is_snapchat 是否阅后即焚
	* @param[in] ret		 是否选择了图片
	* @param[in] file_path	 选择的图片的路径
	* @return void 无返回值
	*/
	void OnImageSelected(bool is_snapchat, BOOL ret, std::wstring file_path);

	/**
	* 小视频被选择后的回调函数
	* @param[in] is_snapchat 是否阅后即焚
	* @param[in] ret		 是否选择了图片
	* @param[in] file_path	 选择的图片的路径
	* @return void 无返回值
	*/
	void OnMinivSelected(BOOL ret, std::wstring file_path);

	/**
	* 响应发送文本的消息
	* @return void 无返回值
	*/
	void OnBtnSend();

	/**
	* 发送一条文本消息
	* @param[in] text 文本内容
	* @return void	无返回值
	*/
	void SendText(const std::string &text, bool team_msg_need_ack = false); 
	/**
	* 发送一条图片消息
	* @param[in] src 图片路径
	* @return void	无返回值
	*/
	void SendImage(const std::wstring &src);

	/**
	* 发送一条文件消息
	* @param[in] src 文件路径
	* @return void	无返回值
	*/
	void SendFile(const std::wstring &src);

public:
	static const LPCTSTR kClassName;
private:
	UTF8String	uid_or_tid_;
	bool		need_select_group_;
	bool		is_multi_vchat_;
	std::list<UTF8String>		exclude_ids_;
	SelectedCompletedCallback	completedCallback_;
	
	ui::Label		*tool_tip_content_;
	ui::Button		*btn_clear_input_;	
	ui::TreeView	*contract_list_;
	vector<ContactTileListSendUI*>  tree_node_ver_;
	ui::RichEdit	*search_edit_;
	ui::ListBox		*selected_user_list_;
	ui::ListBox		*search_result_list_;
	ui::CheckBox	*checkall_;
	AutoUnregister	unregister_cb;

	//发送相关空间
	ui::CheckBox*	btn_face_;

	ui::Button*		btn_send_;
	ui::RichEdit*	input_edit_;
	ui::Button*     btn_image_;
	//ui::Button*		btn_miniv_;
	ui::Button*     btn_file_;
	//ui::Button*		btn_send_;
};
}
#endif // INVOKE_CHAT_FORM_H_
