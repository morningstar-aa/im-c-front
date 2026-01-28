#include "contact_select_sendmsg.h"
#include "module/login/login_manager.h"
#include "../../../../../nim_win_demo/module/db/public_db.h"
#include "gui/video/multi_video_form.h"
#include "shared/modal_wnd/file_dialog_ex.h"
#include "gui/link/link_form.h"
#include "util/user.h"
#include "gui/session/control/bubbles/bubble_file.h"

using namespace ui;
using namespace std;

namespace nim_comp
{
const LPCSTR ContactSelectSendForm::kCreateGroup = "CreateGroup";
const LPCSTR ContactSelectSendForm::kCreateTeam = "CreateTeam";
const LPCSTR ContactSelectSendForm::kRetweetMessage = "RetweetMessage";

const LPCTSTR ContactSelectSendForm::kClassName = _T("ContactSelectSendForm");

ContactSelectSendForm::ContactSelectSendForm(const UTF8String& uid_or_tid,
	const std::list<UTF8String>& exclude_ids,
	const SelectedCompletedCallback& completedCallback,
	bool need_select_group/* = false*/, bool is_multi_vchat/*=false*/)
	: uid_or_tid_(uid_or_tid)
	, exclude_ids_(exclude_ids)
	, completedCallback_(completedCallback)
	, need_select_group_(need_select_group)
	, is_multi_vchat_(is_multi_vchat)
{
	auto friend_list_change_cb = nbase::Bind(&ContactSelectSendForm::OnFriendListChange, this, std::placeholders::_1, std::placeholders::_2);
	unregister_cb.Add(UserService::GetInstance()->RegFriendListChange(friend_list_change_cb));
	auto black_change_cb = nbase::Bind(&ContactSelectSendForm::OnSetBlackCallback, this, std::placeholders::_1, std::placeholders::_2);
	unregister_cb.Add(MuteBlackService::GetInstance()->RegSyncSetBlackCallback(black_change_cb));
	auto user_info_change_cb = nbase::Bind(&ContactSelectSendForm::OnUserInfoChange, this, std::placeholders::_1);
	unregister_cb.Add(UserService::GetInstance()->RegUserInfoChange(user_info_change_cb));
	auto user_photo_ready_cb = nbase::Bind(&ContactSelectSendForm::OnUserPhotoReady, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	unregister_cb.Add(PhotoService::GetInstance()->RegPhotoReady(user_photo_ready_cb));

	auto add_team_cb = nbase::Bind(&ContactSelectSendForm::OnAddTeam, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	unregister_cb.Add(TeamService::GetInstance()->RegAddTeam(add_team_cb));
	auto remove_team_cb = nbase::Bind(&ContactSelectSendForm::OnRemoveTeam, this, std::placeholders::_1);
	unregister_cb.Add(TeamService::GetInstance()->RegRemoveTeam(remove_team_cb));
	auto change_team_name_cb = nbase::Bind(&ContactSelectSendForm::OnTeamNameChanged, this, std::placeholders::_1);
	unregister_cb.Add(TeamService::GetInstance()->RegChangeTeamName(change_team_name_cb));
}

ContactSelectSendForm::~ContactSelectSendForm()
{
}

std::wstring ContactSelectSendForm::GetSkinFolder()
{
	return L"contact_select_send";
}

std::wstring ContactSelectSendForm::GetSkinFile()
{
	return L"contact_select_sendmsg.xml";
}

std::wstring ContactSelectSendForm::GetWindowClassName() const
{
	return kClassName;
}

std::wstring ContactSelectSendForm::GetWindowId() const
{
	std::wstring uid_or_tid = nbase::UTF8ToUTF16(uid_or_tid_);
	return uid_or_tid;
}

UINT ContactSelectSendForm::GetClassStyle() const
{
	return (UI_CLASSSTYLE_FRAME | CS_DBLCLKS);
}

void ContactSelectSendForm::InitWindow()
{
	std::wstring title_id;
	/*if (uid_or_tid_ == kCreateGroup)
		title_id = L"STRING_INVITEUSERFORM_CREATE_GROUP";
	else if (uid_or_tid_ == kCreateTeam)
		title_id = L"STRING_INVITEUSERFORM_CREATE_TEAM";
	else if (uid_or_tid_ == kRetweetMessage)
		title_id = L"STRING_INVITEUSERFORM_RETWEET_MSG";
	else
		title_id = L"STRING_INVOKECHATFORM_CAPTION";*/
	title_id = L"STRID_SESSION_GROUP_SEND";

	std::wstring title = ui::MutiLanSupport::GetInstance()->GetStringViaID(title_id);
	SetTaskbarTitle(title);
	((Label*)FindControl(L"title"))->SetText(title);
	 
	contract_list_ = (ui::TreeView*)FindControl(L"user_list");
	contract_list_->AttachBubbledEvent(kEventClick, nbase::Bind(&ContactSelectSendForm::OnListItemClick, this, std::placeholders::_1));
	search_result_list_ = static_cast<ui::ListBox*>(FindControl(L"search_result"));
	search_result_list_->AttachBubbledEvent(kEventClick, nbase::Bind(&ContactSelectSendForm::OnSearchResultListItemClick, this, std::placeholders::_1));
	selected_user_list_ = (ui::ListBox*)FindControl(L"selected_user_list");

	TreeNode* tree_node = ListItemUtil::CreateFirstLetterListItem(L"*");
	AddGroup(tree_node);

	tree_node = ListItemUtil::CreateFirstLetterListItem(L"群组");
	AddGroup(tree_node);

	tree_node = ListItemUtil::CreateFirstLetterListItem(L"#");
	AddGroup(tree_node);

	for (int i = 0; i < 26; i++)
	{
		wchar_t letter = L'A' + i;
		wstring letter_str;
		letter_str += letter;
		tree_node = ListItemUtil::CreateFirstLetterListItem(letter_str);
		AddGroup(tree_node);
	}

	tool_tip_content_ = static_cast<ui::Label*>(FindControl(L"tool_tip"));
	search_edit_ = (RichEdit*)FindControl(L"search");
	search_edit_->AttachTextChange(nbase::Bind(&ContactSelectSendForm::OnSearchEditChange, this, std::placeholders::_1));
	search_edit_->SetLimitText(30);

	checkall_ = (CheckBox*)FindControl(L"check_all");
	checkall_->AttachSelect(nbase::Bind(&ContactSelectSendForm::OnSelAllClick, this, std::placeholders::_1));
	checkall_->AttachUnSelect(nbase::Bind(&ContactSelectSendForm::OnSelAllClick, this, std::placeholders::_1));

	btn_clear_input_ = (Button*)FindControl(L"clear_input");
	btn_clear_input_->SetNoFocus();
	btn_clear_input_->AttachClick(nbase::Bind(&ContactSelectSendForm::OnClearBtnClick, this, std::placeholders::_1));

	ui::Button* btn_confirm = (ui::Button*)FindControl(L"confirm");
	btn_confirm->AttachClick(nbase::Bind(&ContactSelectSendForm::OnBtnConfirmClick, this, std::placeholders::_1));
	ui::Button* btn_cancel = (ui::Button*)FindControl(L"cancel");
	btn_cancel->AttachClick(nbase::Bind(&ContactSelectSendForm::OnBtnCancelClick, this, std::placeholders::_1));

	if (is_multi_vchat_)
	{
		tool_tip_content_->SetText(ui::MutiLanSupport::GetInstance()->GetStringViaID(L"STRING_MULTIVIDEOCHATFORM_SEARCH_ORG_EMPTY").c_str());

		std::wstring title_id;
		title_id = L"STRING_INVITEUSERFORM_MULTI_VCHAT";
		std::wstring title = ui::MutiLanSupport::GetInstance()->GetStringViaID(title_id);
		SetTaskbarTitle(title);
		((Label*)FindControl(L"title"))->SetText(title);
		title_id = L"STRING_INVITEUSERFORM_TEAM_MEMBERS";
		title = ui::MutiLanSupport::GetInstance()->GetStringViaID(title_id);
// 		((Label*)FindControl(L"label_contact"))->SetText(title);
		title_id = L"STRING_INVITEUSERFORM_START_VCHAT";
		title = ui::MutiLanSupport::GetInstance()->GetStringViaID(title_id);
		btn_confirm->SetText(title);
		TeamService* team_service = TeamService::GetInstance();
		nim::Team::QueryTeamMembersAsync(uid_or_tid_, nbase::Bind(&ContactSelectSendForm::OnGetTeamMembers, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	}
	else
	{
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

		// 添加讨论组和高级群
		if (need_select_group_)
		{
			std::list<nim::TeamInfo> teams = TeamService::GetInstance()->GetCachedTeamsInfo();
			for (auto &it : teams)
			{
				AddTeamListItem(it.GetTeamID(), true);
			}
		}
	}

	btn_face_ = (CheckBox*)FindControl(L"btn_face");
	btn_face_->AttachSelect(nbase::Bind(&ContactSelectSendForm::OnClickBtnFaceCallBack, this, std::placeholders::_1));
	
	input_edit_ = (RichEdit*)FindControl(L"input_edit");
	input_edit_->SetLimitText(5000);
	input_edit_->SetNoCaretReadonly();
	//input_edit_->AttachReturn(nbase::Bind(&SessionBox::OnInputEditEnter, this, std::placeholders::_1));
	//input_edit_->AttachBubbledEvent(ui::kEventAll, nbase::Bind(&SessionBox::EditNotify, this, std::placeholders::_1));

	btn_image_ = (ui::Button*)FindControl(L"btn_image");
	btn_image_->AttachClick(nbase::Bind(&ContactSelectSendForm::OnClickedImage, this, std::placeholders::_1));

	//btn_miniv_ = (ui::Button*)FindControl(L"btn_miniv");
	//btn_miniv_->AttachClick(nbase::Bind(&ContactSelectSendForm::OnClickedMiniv, this, std::placeholders::_1));

	btn_file_ = (ui::Button*)FindControl(L"btn_file");
	btn_file_->AttachClick(nbase::Bind(&ContactSelectSendForm::OnClickedFile, this, std::placeholders::_1));

	btn_send_ = (Button*)FindControl(L"btn_send");
	btn_send_->AttachClick(nbase::Bind(&ContactSelectSendForm::OnClickedSendBnt, this, std::placeholders::_1));
}

/*bool ContactSelectSendForm::OnInputEditEnter(ui::EventArgs* param)
{
	//OnBtnSend();
	return true;
}*/

bool ContactSelectSendForm::OnClickBtnFaceCallBack(ui::EventArgs* msg)
{
	if (msg->Type == kEventSelect)
		OnBtnEmoji();
	return true;
}

void ContactSelectSendForm::OnBtnEmoji()
{
	RECT rc = btn_face_->GetPos(true);
	POINT pt_offset = { 150, 350 };
	DpiManager::GetInstance()->ScalePoint(pt_offset);
	POINT pt = { rc.left - pt_offset.x, rc.top - pt_offset.y };
	::ClientToScreen(this->GetHWND(), &pt);

	OnSelectEmotion sel = nbase::Bind(&ContactSelectSendForm::OnEmotionSelected, this, std::placeholders::_1);
	OnSelectSticker sel2 = nbase::Bind(&ContactSelectSendForm::OnEmotionSelectedSticker, this, std::placeholders::_1, std::placeholders::_2);
	OnSelectSticker sel3 = nbase::Bind(&ContactSelectSendForm::OnEmotionSelectedCustom, this, std::placeholders::_1, std::placeholders::_2);
	OnSelectSticker sel4 = nbase::Bind(&ContactSelectSendForm::OnEmotionDeleteCustom, this, std::placeholders::_1, std::placeholders::_2);
	OnEmotionClose  cls = nbase::Bind(&ContactSelectSendForm::OnEmotionClosed, this);

	EmojiForm* emoji_form = new EmojiForm;
	emoji_form->ShowEmoj(pt, sel, sel2, cls);
}

void ContactSelectSendForm::OnEmotionSelected(std::wstring emo)
{
	std::wstring file;
	if (emoji::GetEmojiFileByTag(emo, file))
	{
		InsertFaceToEdit(input_edit_, file, emo);
	}
}


void ContactSelectSendForm::OnEmotionSelectedSticker(const std::wstring &catalog, const std::wstring &name)
{
	//std::list<UTF8String> friend_list;
	//std::list<UTF8String> team_list;
	for (int i = 0; i < selected_user_list_->GetCount(); i++)
	{
		SelContactItemSendUI* select_item = dynamic_cast<SelContactItemSendUI*>(selected_user_list_->GetItemAt(i));
		UTF8String id = select_item->GetAccountID();
		if (select_item->IsTeam())
		{
			//team_list.push_back(id);
		}
		else
		{
			SendSticker(nbase::UTF16ToUTF8(catalog), nbase::UTF16ToUTF8(name), id);
		}
			
	}
}

void ContactSelectSendForm::OnEmotionClosed()
{
	btn_face_->Selected(false, false);

	input_edit_->SetFocus();

}

void ContactSelectSendForm::OnEmotionSelectedCustom(const std::wstring &catalog, const std::wstring &name)
{
	std::string userId = LoginManager::GetInstance()->GetAccount();
	std::wstring file = QPath::GetUserAppDataDir(userId) + L"customImg\\custom_o\\" + name;
	if (nbase::FilePathIsExist(file, false))
	{
		OnImageSelected(false, true, file);
	}
}

void ContactSelectSendForm::OnEmotionDeleteCustom(const std::wstring &catalog, const std::wstring &name)
{
	//DeleteCustomEmotion(name);
}

void ContactSelectSendForm::SendSticker(const std::string &catalog, const std::string &name, const std::string id)
{
	nim::IMMessage msg;
	PackageMsg(msg, id);
	msg.type_ = nim::kNIMMessageTypeCustom;

	Json::Value json;
	Json::FastWriter writer;
	json["type"] = CustomMsgType_Sticker;
	json["data"]["catalog"] = catalog;
	json["data"]["chartlet"] = name;

	if (catalog == "like")
	{
		std::string userId = LoginManager::GetInstance()->GetAccount();
		std::string imgId = name.substr(0, name.length() - 4);
		std::string originalPath = PublicDB::GetInstance()->GetImgPathFromImgId(userId, imgId);
		if (!originalPath.empty())
		{
			json["data"]["chartlet"] = originalPath;
			//json["data"]["imgId"] = imgId;
		}
	}

	msg.content_ = nbase::UTF16ToUTF8(MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_SESSION_CHARLET"));
	msg.attach_ = writer.write(json);

	//AddSendingMsg(msg);

	nim::Talk::SendMsg(msg.ToJsonString(true));
}

void ContactSelectSendForm::PackageMsg(nim::IMMessage &msg, const std::string id)
{
	msg.session_type_ = nim::kNIMSessionTypeP2P;
	msg.receiver_accid_ = id;
	msg.sender_accid_ = LoginManager::GetInstance()->GetAccount();
	msg.client_msg_id_ = QString::GetGUID();
	msg.msg_setting_.resend_flag_ = BS_FALSE;

	//base获取的时间单位是s，服务器的时间单位是ms
	msg.timetag_ = 1000 * nbase::Time::Now().ToTimeT();

	msg.status_ = nim::kNIMMsgLogStatusSending;
}

bool ContactSelectSendForm::OnClickedImage(ui::EventArgs* param)
{
	OnBtnImage(false);
	return true;
}

bool ContactSelectSendForm::OnClickedMiniv(ui::EventArgs* param)
{
	OnBtnMiniv();
	return true;
}

bool ContactSelectSendForm::OnClickedFile(ui::EventArgs* param)
{
	OnBtnFile();
	return true;
}

void ContactSelectSendForm::OnBtnImage(bool is_snapchat)
{
	std::wstring file_type = ui::MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_SESSION_PIC_FILE");
	LPCTSTR filter = L"*.jpg;*.jpeg;*.png;*.bmp";
	std::wstring text = nbase::StringPrintf(L"%s(%s)", file_type.c_str(), filter);
	std::map<LPCTSTR, LPCTSTR> filters;
	filters[text.c_str()] = filter;

	CFileDialogEx::FileDialogCallback2 cb = nbase::Bind(&ContactSelectSendForm::OnImageSelected, this, is_snapchat, std::placeholders::_1, std::placeholders::_2);

	CFileDialogEx* file_dlg = new CFileDialogEx();
	file_dlg->SetFilter(filters);
	file_dlg->SetMultiSel(TRUE);
	file_dlg->SetParentWnd(this->GetHWND());
	file_dlg->AyncShowOpenFileDlg(cb);
}

void ContactSelectSendForm::OnBtnMiniv()
{
	std::wstring file_type = ui::MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_SESSION_MINIV_FILE");
	LPCTSTR filter = L"*.avi;*.wmv;*.mpg;*.mpeg;*.mov;*.rm;*.mp4";
	std::wstring text = nbase::StringPrintf(L"%s(%s)", file_type.c_str(), filter);
	std::map<LPCTSTR, LPCTSTR> filters;
	filters[text.c_str()] = filter;

	CFileDialogEx::FileDialogCallback2 cb = nbase::Bind(&ContactSelectSendForm::OnMinivSelected, this, std::placeholders::_1, std::placeholders::_2);

	CFileDialogEx* file_dlg = new CFileDialogEx();
	file_dlg->SetFilter(filters);
	file_dlg->SetMultiSel(TRUE);
	file_dlg->SetParentWnd(this->GetHWND());
	file_dlg->AyncShowOpenFileDlg(cb);
}

void ContactSelectSendForm::OnImageSelected(bool is_snapchat, BOOL ret, std::wstring file_path)
{
	if (ret)
	{
		if (!nbase::FilePathIsExist(file_path, FALSE) || 0 == nbase::GetFileSize(file_path))
			return;

		std::wstring file_ext;
		nbase::FilePathExtension(file_path, file_ext);
		nbase::LowerString(file_ext);
		if (file_ext != L".jpg" && file_ext != L".jpeg" && file_ext != L".png" && file_ext != L".bmp")
			return;

		if (!is_snapchat)
		{
			InsertImageToEdit(input_edit_, file_path, false);
		}
		else
		{
			//SendSnapChat(file_path);
		}
	}
}

void ContactSelectSendForm::OnMinivSelected(BOOL ret, std::wstring file_path)
{
	if (ret)
	{
		if (!nbase::FilePathIsExist(file_path, FALSE) || 0 == nbase::GetFileSize(file_path))
			return;

		std::wstring file_ext;
		nbase::FilePathExtension(file_path, file_ext);
		nbase::LowerString(file_ext);
		if (file_ext != L".avi"
			&& file_ext != L".wmv"
			&& file_ext != L".mpg"
			&& file_ext != L".mpeg"
			&& file_ext != L".mov"
			&& file_ext != L".rm"
			&& file_ext != L".ram"
			&& file_ext != L".swf"
			&& file_ext != L".flv"
			&& file_ext != L".mp4")
		{
			return;
		}

		//SendMiniv(file_path, file_ext);
	}
}

void ContactSelectSendForm::OnBtnFile()
{
	std::wstring file_type = MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_SESSION_FILE_FORMAT") + L"(*.*)";
	LPCTSTR filter = L"*.*";
	std::map<LPCTSTR, LPCTSTR> filters;
	filters[file_type.c_str()] = filter;

	CFileDialogEx::FileDialogCallback2 cb = nbase::Bind(&ContactSelectSendForm::OnFileSelected, this, std::placeholders::_1, std::placeholders::_2);

	CFileDialogEx* file_dlg = new CFileDialogEx();
	file_dlg->SetFilter(filters);
	file_dlg->SetMultiSel(TRUE);
	file_dlg->SetParentWnd(this->GetHWND());
	file_dlg->AyncShowOpenFileDlg(cb);
}

void ContactSelectSendForm::OnFileSelected(BOOL ret, std::wstring file_path)
{
	if (ret)
	{
		if (CheckFileSize(file_path))
		{
			InsertFileToEdit(input_edit_, file_path);
		}
		else
		{
			ShowMsgBox(this->GetHWND(), MsgboxCallback(), L"STRID_SESSION_SUPPORT_15MB");
		}
	}
}

bool ContactSelectSendForm::CheckFileSize(const std::wstring &src)
{
	int64_t sz = nbase::GetFileSize(src);
	if (sz > LoginManager::GetInstance()->GetFileSizeLimit() * 1024 * 1024 || sz <= 0)
	{
		return false;
	}
	return true;
}

bool ContactSelectSendForm::OnClickedSendBnt(ui::EventArgs* param)
{
	OnBtnSend();
	return true;
}
//cjy 群发
void ContactSelectSendForm::OnBtnSend()
{
	if (!LoginManager::GetInstance()->IsLinkActive())
	{
		//AddTip(STT_LINK);

		//ShowLinkForm((nim::NIMResCode)LoginManager::GetInstance()->GetErrorCode(), true);
		return;
	}

	ITextServices *text_service = input_edit_->GetTextServices();
	if (!text_service)
	{
		QLOG_ERR(L"DoBtnSend occur error. text service null.");
		return;
	}

	std::vector<RE_OLE_ITEM_CONTENT> content_list;
	Re_GetTextEx(text_service, content_list);
	text_service->Release();

	bool empty_msg = content_list.empty();
	for (UINT i = 0; i < content_list.size(); ++i)
	{
		RE_OLE_ITEM_CONTENT info = content_list.at(i);
		std::string msg_body = nbase::UTF16ToUTF8(info.content_);
		switch (info.type_)
		{
		case RE_OLE_TYPE_TEXT:
		{
			if (CheckMsgContent(info.content_).empty())
			{
				if (content_list.size() == 1)
				{
					empty_msg = true;
				}
			}
			else
			{
				SendText(msg_body);
			}
		}
		break;
		case RE_OLE_TYPE_IMAGE:
		{
			if (nbase::FilePathIsExist(info.image_src_, FALSE))
			{
				SendImage(info.image_src_);
			}
		}
		break;
		case RE_OLE_TYPE_FILE:
		{
			std::wstring file_path = info.content_;
			if (CheckFileSize(file_path))
			{
				SendFile(file_path);
			}
			else
			{
				ShowMsgBox(this->GetHWND(), MsgboxCallback(), L"STRID_SESSION_SUPPORT_15MB");
			}
		}
		break;
		}
	}

	if (empty_msg)
	{
		input_edit_->SetText(L"");
		//ShowMsgBox(this->GetHWND(), MsgboxCallback(), L"STRID_SESSION_EDIT_EMPTY");
		return;
	}

	input_edit_->SetText(L"");
	input_edit_->SetFocus();
}

void ContactSelectSendForm::SendText(const std::string &text, bool team_msg_need_ack/* = false*/)
{
	 
	nim::IMMessage msg;
	PackageMsg(msg,"");
	std::string robot_accid;

	if (team_msg_need_ack)
		msg.msg_setting_.team_msg_need_ack_ = BS_TRUE;

	std::string json_msg="[";
	msg.content_ = text;
	if (msg.type_ != nim::kNIMMessageTypeRobot)
	{
		msg.type_ = nim::kNIMMessageTypeText;
		
		for (int i = 0; i < selected_user_list_->GetCount(); i++)
		{
			SelContactItemSendUI* select_item = dynamic_cast<SelContactItemSendUI*>(selected_user_list_->GetItemAt(i));
			UTF8String id = select_item->GetAccountID();
			if (i == 0){
				json_msg = json_msg + "'" + id + "'";
			}
			else
			{
				json_msg = json_msg + ",'" + id + "'";
			}
			/*if (!select_item->IsTeam())
			{
				json_msg = nim::Talk::CreateTextMessage(id, msg.session_type_, msg.client_msg_id_, msg.content_, msg.msg_setting_, msg.timetag_);
				nim::Talk::SendMsg(json_msg);
			}*/
		}
	}
	json_msg = json_msg + "]"; 
	 
	std::string body;
	body += "fromAccid=" + LoginManager::GetInstance()->GetAccount();
	body += "&uid=" + LoginManager::GetInstance()->GetAccount() + "&token=" + LoginManager::GetInstance()->GetAccountToken();
	body += "&msg=" + text + "&type=0";
	body += "&toAccids=" + json_msg;
	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(app_sdk::AppSDKInterface::GetInstance()->GetAppHost() + "/api/other/sendBatchMsg", body.c_str(), body.size(),nullptr);// , ToWeakCallback([this](bool ret, int response_code, const std::string& reply) {
		/*ShowMsgBox(this->GetHWND(), MsgboxCallback(), L"发送成功", false);
	}));*/
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
	ShowMsgBox(this->GetHWND(), MsgboxCallback(), L"发送成功", false);
	
}

void ContactSelectSendForm::SendImage(const std::wstring &src)
{
	nim::IMMessage msg;
	PackageMsg(msg);
	msg.type_ = nim::kNIMMessageTypeImage;

	//先以消息id为图片名，生成用于上传的图片
	std::wstring image_dir = GetUserImagePath();
	if (!nbase::FilePathIsExist(image_dir, true))
		nbase::CreateDirectory(image_dir);
	std::wstring zoom_path = image_dir + nbase::UTF8ToUTF16(msg.client_msg_id_);
	if (!nbase::FilePathIsExist(zoom_path, false))
	{
		GenerateUploadImage(src, zoom_path);
		if (!nbase::FilePathIsExist(zoom_path, false))
		{
			QLOG_ERR(L"Zoomed image does not exist.");
			return;
		}
	}

	//再计算用于上传的图片的md5，以md5重命名之
	std::string image_md5 = GetFileMD5(zoom_path);
	std::wstring dest = image_dir + nbase::UTF8ToUTF16(image_md5);
	if (!nbase::FilePathIsExist(dest, false))
	{
		if (!::MoveFileEx(zoom_path.c_str(), dest.c_str(), 0))
		{
			QLOG_ERR(L"Rename image error: {0}.") << ::GetLastError();
			return;
		}
	}
	else
		nbase::DeleteFile(zoom_path);

	msg.local_res_path_ = nbase::UTF16ToUTF8(dest);

	nim::IMImage img;
	img.md5_ = image_md5;
	img.size_ = (long)nbase::GetFileSize(dest);

	Gdiplus::Image image(dest.c_str());
	if (image.GetLastStatus() != Gdiplus::Ok)
	{
		assert(0);
	}
	else
	{
		img.width_ = image.GetWidth();
		img.height_ = image.GetHeight();
	}

	msg.attach_ = img.ToJsonString();

	for (int i = 0; i < selected_user_list_->GetCount(); i++)
	{
		SelContactItemSendUI* select_item = dynamic_cast<SelContactItemSendUI*>(selected_user_list_->GetItemAt(i));
		UTF8String id = select_item->GetAccountID();
		if (!select_item->IsTeam())
		{
			msg.timetag_ = 1000 * nbase::Time::Now().ToTimeT();
			msg.client_msg_id_ = QString::GetGUID();
			std::string json_msg = nim::Talk::CreateImageMessage(id, msg.session_type_, msg.client_msg_id_, img, msg.local_res_path_, nim::MessageSetting(), msg.timetag_);
			nim::Talk::SendMsg(json_msg);
			QLOG_APP(L"nim::Talk::SendMsg(json_msg) i: {0} and uid : {1} and msg.session_type_ is :{2} and msg.client_msg_id_ is:{3}.") << i << id << msg.session_type_ << msg.client_msg_id_;
			QLOG_APP(L"nim::Talk::SendMsg(json_msg) msg.local_res_path_  is: {0} and msg.timetag_ : {1}.") << msg.local_res_path_ << msg.timetag_;
		}
	}
	//AddSendingMsg(msg);
	//std::string json_msg = nim::Talk::CreateImageMessage(msg.receiver_accid_, msg.session_type_, msg.client_msg_id_, img, msg.local_res_path_, nim::MessageSetting(), msg.timetag_);
	//nim::Talk::SendMsg(json_msg);
}

void ContactSelectSendForm::SendFile(const std::wstring &src)
{
	nim::IMMessage msg;
	PackageMsg(msg);
	msg.type_ = nim::kNIMMessageTypeFile;
	msg.local_res_path_ = nbase::UTF16ToUTF8(src);

	nim::IMFile file;
	file.md5_ = GetFileMD5(src);
	file.size_ = (long)nbase::GetFileSize(src);

	nbase::PathString file_name;
	nbase::FilePathApartFileName(src, file_name);
	std::wstring file_exten;
	nbase::FilePathExtension(file_name, file_exten);
	if (!file_exten.empty())
		file_exten = file_exten.substr(1);
	file.display_name_ = nbase::UTF16ToUTF8(file_name);
	file.file_extension_ = nbase::UTF16ToUTF8(file_exten);
	msg.attach_ = file.ToJsonString();

	//AddSendingMsg(msg);

	nim::Talk::FileUpPrgCallback* cb_pointer = nullptr;
	/*MsgBubbleFile* bubble = dynamic_cast<MsgBubbleFile*>(msg_list_->FindSubControl(nbase::UTF8ToUTF16(msg.client_msg_id_)));
	if (!msg.local_res_path_.empty() && nbase::FilePathIsExist(nbase::UTF8ToUTF16(msg.local_res_path_), false) && bubble)
	{
		cb_pointer = new nim::Talk::FileUpPrgCallback(bubble->GetFileUpPrgCallback());
	}*/
	//std::string json_msg = nim::Talk::CreateFileMessage(msg.receiver_accid_, msg.session_type_, msg.client_msg_id_, file, msg.local_res_path_, nim::MessageSetting(), msg.timetag_);
	//nim::Talk::SendMsg(json_msg, msg.client_msg_id_, cb_pointer);
	for (int i = 0; i < selected_user_list_->GetCount(); i++)
	{
		SelContactItemSendUI* select_item = dynamic_cast<SelContactItemSendUI*>(selected_user_list_->GetItemAt(i));
		UTF8String id = select_item->GetAccountID();
		if (!select_item->IsTeam())
		{
			std::string json_msg = nim::Talk::CreateFileMessage(id, msg.session_type_, msg.client_msg_id_, file, msg.local_res_path_, nim::MessageSetting(), msg.timetag_);
			nim::Talk::SendMsg(json_msg, msg.client_msg_id_, cb_pointer);
		}
	}
}

/*void ContactSelectSendForm::AddSendingMsg(const nim::IMMessage &msg)
{
	writing_time_ = 0;
	RemoveTip(STT_WRITING);

	bool show_time = false;
	if (last_msg_time_ == 0)
	{
		show_time = true;
		farst_msg_time_ = msg.timetag_;
	}
	else
	{
		show_time = CheckIfShowTime(last_msg_time_, msg.timetag_);
	}

	if (!IsNoticeMsg(msg))
		last_msg_time_ = msg.timetag_;

	MsgBubbleItem* item = ShowMsg(msg, false, show_time);
	msg_list_->EndDown(true, false);
}*/

void ContactSelectSendForm::OnGetTeamMembers(const std::string& team_id, int count, const std::list<nim::TeamMemberProperty>& team_member_list)
{
	auto cb = [=]()
	{
		UTF8String current_user_id = LoginManager::GetInstance()->GetAccount();
		for (const auto& member : team_member_list)
		{
			std::string member_id = member.GetAccountID();
			if (current_user_id != member_id)
			{
				MultiVideoChatForm *multi_window = (MultiVideoChatForm*)(WindowsManager::GetInstance()->GetWindow(MultiVideoChatForm::kClassName, MultiVideoChatForm::kClassName));
				if (multi_window)
				{
					std::set<std::string> talking_members = multi_window->GetTalkingMember();
					bool is_enable = true;
					std::set<std::string>::iterator it = talking_members.find(member.GetAccountID());
					if (it == talking_members.end())
					{
						AddFriendListItem(member.GetAccountID(), is_enable);
					}
				}
				else
					AddFriendListItem(member.GetAccountID(), true);;
			}
		}
	};
	Post2UI(cb);
}


bool ContactSelectSendForm::OnBtnDeleteClick(const UTF8String& user_id, ui::EventArgs* param)
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
		for (auto& it : tree_node_ver_)
		{
			for (int i = 0; i < it->GetCount(); i++)
			{
				ContactListItemSendUI* item = (ContactListItemSendUI*)it->GetItemAt(i);
				if (item->GetUTF8DataID() == user_id)
				{
					EventArgs args;
					args.pSender = item;
					OnListItemClick(&args);
				}
			}
		}
	}
	return true;
}

bool ContactSelectSendForm::OnBtnConfirmClick(ui::EventArgs* param)
{
	/*
	std::list<UTF8String> friend_list;
	std::list<UTF8String> team_list;
	for (int i = 0; i < selected_user_list_->GetCount(); i++)
	{
		SelContactItemSendUI* select_item = dynamic_cast<SelContactItemSendUI*>(selected_user_list_->GetItemAt(i));
		UTF8String id = select_item->GetAccountID();
		if (select_item->IsTeam())
			team_list.push_back(id);
		else
			friend_list.push_back(id);
	}

	completedCallback_(friend_list, team_list);
	*/
	Close();
	return true;
}

bool ContactSelectSendForm::OnBtnCancelClick(ui::EventArgs* param)
{
	Close();
	return true;
}

bool ContactSelectSendForm::OnSearchEditChange(ui::EventArgs* param)
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

		for (auto& it : tree_node_ver_)
		{
			for (int i = 0; i < it->GetCount(); i++)
			{
				ContactListItemSendUI* user_listitem = (ContactListItemSendUI*)it->GetItemAt(i);
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

bool ContactSelectSendForm::OnClearBtnClick(ui::EventArgs* param)
{
	btn_clear_input_->SetVisible(false);
	tool_tip_content_->SetVisible(false);

	search_edit_->SetText(L"");

	return false;
}
bool ContactSelectSendForm::OnSelAllClick(ui::EventArgs* param)
{ 
	bool b=checkall_->IsSelected();
	for (auto& it : tree_node_ver_)
	{
		for (int i = 0; i < it->GetCount(); i++)
		{
			auto listitem = (ContactListItemSendUI*)(it->GetItemAt(i));
			if (listitem)
			{
				UTF8String id = listitem->GetUTF8DataID();
				ui::CheckBox* checkbox = (ui::CheckBox*)listitem->FindSubControl(L"checkbox");
				checkbox->Selected(b, false);
				if (!checkbox->IsSelected()) {
					listitem->Selected(false, true);
				}
				OnCheckBox(id, listitem->IsTeam(),b);
			}

		 
		}
	}
	return false;
}

bool ContactSelectSendForm::OnListItemClick(ui::EventArgs* param)
{
	ContactListItemSendUI* listitem = dynamic_cast<ContactListItemSendUI*>(param->pSender);
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

bool ContactSelectSendForm::OnSearchResultListItemClick(ui::EventArgs* param)
{
	OnListItemClick(param);

	for (auto& it : tree_node_ver_)
	{
		for (int i = 0; i < it->GetCount(); i++)
		{
			auto listitem = (ContactListItemSendUI*)(it->GetItemAt(i));
			if (listitem->GetUTF8DataID() == param->pSender->GetUTF8DataID())
			{
				ui::CheckBox* checkbox = (ui::CheckBox*)listitem->FindSubControl(L"checkbox");
				checkbox->Selected(!checkbox->IsSelected(), false);
				if (!checkbox->IsSelected()) {
					listitem->Selected(false, true);
				}
			}
		}
	}

	return true;
}

void ContactSelectSendForm::OnCheckBox(UTF8String id, bool is_team, bool check)
{
	if (check)
	{
		SelContactItemSendUI* selected_listitem = CreateSelectedListItem(id, is_team);
		selected_user_list_->Add(selected_listitem);
		selected_user_list_->EndDown();
	}
	else
	{
		for (int i = 0; i < selected_user_list_->GetCount(); i++)
		{
			SelContactItemSendUI* listitem = (SelContactItemSendUI*)selected_user_list_->GetItemAt(i);
			if (listitem->GetUTF8DataID() == id)
			{
				selected_user_list_->RemoveAt(i);
				break;
			}
		}
	}
}

}