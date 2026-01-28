#include "bookmarks_form.h"

using namespace ui;

/*void BookmarksForm::starClip()
{
	nim_comp::SessionBox* box = nim_comp::SessionManager::GetInstance()->GetCurrentSessionBox();
	if (NULL != box)
	{
		box->starClip();
		return;
	}
	if (CaptureManager::GetInstance()->IsCaptureTracking())
	{
		return;
	}
	StdClosure callback = nbase::Bind(&BookmarksForm::DoClip, this);
	nbase::ThreadManager::PostDelayedTask(kThreadUI, callback, nbase::TimeDelta::FromMilliseconds(500));
}

void BookmarksForm::DoClip()
{
	std::wstring send_info;
	CaptureManager::CaptureCallback callback = nbase::Bind(&BookmarksForm::OnClipCallback, this, std::placeholders::_1, std::placeholders::_2);
	std::string acc = nim_ui::LoginManager::GetInstance()->GetAccount();
	assert(!acc.empty());
	std::wstring app_data_audio_path = QPath::GetUserAppDataDir(acc);
	if (CaptureManager::GetInstance()->StartCapture(callback, app_data_audio_path, send_info) == false)
	{
		OnClipCallback(FALSE, L"");
	}
}

void BookmarksForm::OnClipCallback(bool ret, const std::wstring& file_path)
{
	if (ret)
	{
		//InsertImageToEdit(input_edit_, file_path, false);
	}
};

void BookmarksForm::showSessionBox()
{
	WindowEx* box = (WindowEx *)(nim_comp::SessionManager::GetInstance()->OpenCurrentSessionBox());
	if (NULL == box)
	{
		ActiveWindow();
	}
	else
	{
		box->ActiveWindow();
	}
}

bool BookmarksForm::SearchEditChange(ui::EventArgs* param)
{
	UTF8String search_key = search_edit_->GetUTF8Text();
	bool has_serch_key = !search_key.empty();
	btn_clear_input_->SetVisible(has_serch_key);
	search_result_list_->SetVisible(has_serch_key);
	option_panel_->SetVisible(!has_serch_key);
	FindControl(L"no_search_result_tip")->SetVisible(has_serch_key);
	if (has_serch_key)
	{
		nim_ui::ContactsListManager::GetInstance()->FillSearchResultList(search_result_list_, search_key);
		FindControl(L"no_search_result_tip")->SetVisible(search_result_list_->GetCount() == 0);
		if (search_result_list_->GetCount() > 0)
			search_result_list_->SelectItem(0);
	}
	return true;
}

bool BookmarksForm::OnClearInputBtnClicked(ui::EventArgs* param)
{
	search_edit_->SetText(L"");
	return true;
}

void BookmarksForm::OnUserInfoChange(const std::list<nim::UserNameCard> &uinfos)
{
	for (auto iter = uinfos.cbegin(); iter != uinfos.cend(); iter++)
	{
		if (nim_ui::LoginManager::GetInstance()->IsEqual(iter->GetAccId()))
		{
			InitHeader();
			break;
		}
	}
}

void BookmarksForm::OnUserPhotoReady(PhotoType type, const std::string& account, const std::wstring& photo_path)
{
	if (type == kUser && nim_ui::LoginManager::GetInstance()->GetAccount() == account)
		btn_header_->SetBkImage(photo_path);
}

void BookmarksForm::OnUnreadCountChange(int unread_count)
{
	if (unread_count > 0)
	{
		label_unread_->SetText(nbase::StringPrintf(L"%d", unread_count));
		box_unread_->SetVisible(true);
	}
	else
	{
		box_unread_->SetVisible(false);
	}
}

void BookmarksForm::InitHeader()
{
	std::string my_id = nim_ui::LoginManager::GetInstance()->GetAccount();
	label_name_->SetText(nim_ui::UserManager::GetInstance()->GetUserName(my_id, false));
	btn_header_->SetBkImage(nim_ui::PhotoManager::GetInstance()->GetUserPhoto(my_id));
}

void BookmarksForm::LeftClick()
{
	this->ActiveWindow();
	if (is_trayicon_left_double_clicked_)
	{
		is_trayicon_left_double_clicked_ = false;
		return;
	}
	::SetForegroundWindow(m_hWnd);
	::BringWindowToTop(m_hWnd);
}

void BookmarksForm::LeftDoubleClick()
{
	is_trayicon_left_double_clicked_ = true;
}

void BookmarksForm::RightClick()
{
	POINT point;
	::GetCursorPos(&point);
	PopupTrayMenu(point);
}

void BookmarksForm::PopupTrayMenu(POINT point)
{
	//创建菜单窗口
	CMenuWnd* pMenu = new CMenuWnd(NULL);
	STRINGorID xml(L"tray_menu.xml");
	pMenu->Init(xml, _T("xml"), point, CMenuWnd::RIGHT_TOP);
	//注册回调
	CMenuElementUI* display_session_list = (CMenuElementUI*)pMenu->FindControl(L"display_session_list");
	display_session_list->AttachSelect(nbase::Bind(&BookmarksForm::SessionListMenuItemClick, this, std::placeholders::_1));

	CMenuElementUI* logoff = (CMenuElementUI*)pMenu->FindControl(L"logoff");
	logoff->AttachSelect(nbase::Bind(&BookmarksForm::LogoffMenuItemClick, this, std::placeholders::_1));

	CMenuElementUI* quit = (CMenuElementUI*)pMenu->FindControl(L"quit");
	quit->AttachSelect(nbase::Bind(&BookmarksForm::QuitMenuItemClick, this, std::placeholders::_1));
	//显示
	pMenu->Show();
}

void BookmarksForm::GetCustomImgAsyn()
{
	//判断是否存在收藏图像路径，如果不存在则新建
	std::string userId = nim_ui::LoginManager::GetInstance()->GetAccount();
	std::wstring dir_o = QPath::GetUserAppDataDir(userId) + L"customImg\\custom_o\\";
	std::wstring dir = QPath::GetUserAppDataDir(userId) + L"customImg\\custom\\";
	std::wstring parnetDir = QPath::GetUserAppDataDir(userId) + L"customImg\\";

	if (!nbase::FilePathIsExist(parnetDir, true))
	{
		nbase::win32::CreateDirectoryRecursively(parnetDir.c_str());
	}

	if (!nbase::FilePathIsExist(dir_o, true))
	{
		nbase::win32::CreateDirectoryRecursively(dir_o.c_str());
	}

	if (!nbase::FilePathIsExist(dir, true))
	{
		nbase::win32::CreateDirectoryRecursively(dir.c_str());
	}

	//向服务器发送请求获取
	nim_ui::UserManager::GetInstance()->QueryImgInfo(userId, ToWeakCallback([this](int code, const std::list<CustomImgInfo>& imgInfo) {
		if (code == 0)
		{
				//读取数据库中对应用户id的图片id
			GetCustomImgFromDB(imgInfo);
		}
		else
		{
			//ShowLoginTip(MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_LOGIN_FORM_TIP_PASSWORD_ERROR"));
			//弹框展示失败
		}
	}));
	//比较下载图片
}

long URLDownloadToImgFile(std::string url, std::wstring name)
{
	size_t len = url.length();//获取字符串长度
	int nmlen = MultiByteToWideChar(CP_ACP, 0, url.c_str(), len + 1, NULL, 0);//如果函数运行成功，并且cchWideChar为零，

	//返回值是接收到待转换字符串的缓冲区所需求的宽字符数大小。
	wchar_t* buffer = new wchar_t[nmlen];
	MultiByteToWideChar(CP_ACP, 0, url.c_str(), len + 1, buffer, nmlen);

	return URLDownloadToFile(0, buffer, name.data(), 0, 0);
}

long DeleteCustomImgFile(std::string url, std::wstring name)
{
	size_t len = url.length();//获取字符串长度
	int nmlen = MultiByteToWideChar(CP_ACP, 0, url.c_str(), len + 1, NULL, 0);//如果函数运行成功，并且cchWideChar为零，

	//返回值是接收到待转换字符串的缓冲区所需求的宽字符数大小。
	wchar_t* buffer = new wchar_t[nmlen];
	MultiByteToWideChar(CP_ACP, 0, url.c_str(), len + 1, buffer, nmlen);

	return URLDownloadToFile(0, buffer, name.data(), 0, 0);
}

void BookmarksForm::GetCustomImgFromDB(const std::list<CustomImgInfo>& imgInfoList)
{
	std::string userId = nim_ui::LoginManager::GetInstance()->GetAccount();
	std::list<CustomImgInfo> localImgInfoList;
	PublicDB::GetInstance()->ReadCustomImgData(userId,localImgInfoList);

	for (auto info : imgInfoList)
	{
		bool tag = false;
		for (auto localInfo : localImgInfoList)
		{
			if (info.imgId == localInfo.imgId )
			{
				tag = true;
				break;
			}
		}
		if (!tag)
		{
			//云端图片写入本地数据库
			CustomData data;
			data.user_id_ = userId;
			data.img_id_ = info.imgId;
			data.OriginalPath = info.OriginalPath;
			data.thumbnailPath = info.thumbnailPath;
			PublicDB::GetInstance()->WriteCustomData(data);
			//更新图列表，同步云端图片
			string url_o = info.OriginalPath;
			std::wstring dir_o = QPath::GetUserAppDataDir(userId) + L"customImg\\custom_o\\";
			std::wstring name_o = dir_o + nbase::UTF8ToUTF16(data.img_id_) + L".png";

			string url = info.thumbnailPath;
			std::wstring dir = QPath::GetUserAppDataDir(userId) + L"customImg\\custom\\";
			std::wstring name = dir + nbase::UTF8ToUTF16(data.img_id_) + L".png";

			if (!nbase::FilePathIsExist(dir_o, true) || nbase::FilePathIsExist(name_o, true)
				|| !nbase::FilePathIsExist(dir, true) || nbase::FilePathIsExist(name, true))
				//nbase::win32::CreateDirectoryRecursively(photo_dir.c_str());
				return;
			URLDownloadToImgFile(url_o, name_o);
			URLDownloadToImgFile(url, name);

		}
	}
	for (auto localInfo : localImgInfoList)
	{
		bool tag = false;
		for (auto info : imgInfoList)
		{
			if (info.imgId == localInfo.imgId)
			{
				tag = true;
				break;
			}
		}
		if (!tag)
		{
			//删除本地数据库对应图片数据
			CustomData data;
			data.user_id_ = userId;
			data.img_id_ = localInfo.imgId;
			data.OriginalPath = localInfo.OriginalPath;
			data.thumbnailPath = localInfo.thumbnailPath;
			PublicDB::GetInstance()->DeleteCustomData(data);
			//删除本地图片
			std::wstring dir_o = QPath::GetUserAppDataDir(userId) + L"customImg\\custom_o\\";
			std::wstring wName_o = dir_o + nbase::UTF8ToUTF16(data.img_id_) + L".png";
			std::string name_o = nbase::UTF16ToUTF8(wName_o);

			std::wstring dir = QPath::GetUserAppDataDir(userId) + L"customImg\\custom\\";
			std::wstring wName = dir + nbase::UTF8ToUTF16(data.img_id_) + L".png";
			std::string name = nbase::UTF16ToUTF8(wName);

			std::remove(name_o.data());
			std::remove(name.data());
			//if (!nbase::FilePathIsExist(name_o, false) && !nbase::FilePathIsExist(name, false))
				//return;

		}
	}
}

void BookmarksForm::GetCustomEmoticonAsyn()
{
	//判断是否存在添加表情路径，如果不存在则新建
	std::string userId = nim_ui::LoginManager::GetInstance()->GetAccount();
	std::wstring likeDir = QPath::GetUserAppDataDir(userId) + L"like\\";

	if (!nbase::FilePathIsExist(likeDir, true))
	{
		nbase::win32::CreateDirectoryRecursively(likeDir.c_str());
	}


	//向服务器发送请求获取
	nim_ui::UserManager::GetInstance()->QueryImgInfo(userId, ToWeakCallback([this](int code, const std::list<CustomImgInfo>& imgInfo) {
		if (code == 0)
		{
			//读取数据库中对应用户id的图片id
			GetCustomEmoticonFromDB(imgInfo);
		}
		else
		{
			//ShowLoginTip(MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_LOGIN_FORM_TIP_PASSWORD_ERROR"));
			//弹框展示失败
		}
	}));
	//比较下载图片
}

void BookmarksForm::GetCustomEmoticonFromDB(const std::list<CustomImgInfo>& imgInfoList)
{
	std::string userId = nim_ui::LoginManager::GetInstance()->GetAccount();
	std::list<CustomImgInfo> localImgInfoList;
	PublicDB::GetInstance()->ReadCustomImgData(userId, localImgInfoList);

	for (auto info : imgInfoList)
	{
		bool tag = false;
		for (auto localInfo : localImgInfoList)
		{
			if (info.imgId == localInfo.imgId)
			{
				tag = true;
				break;
			}
		}
		if (!tag)
		{
			//云端图片写入本地数据库
			CustomData data;
			data.user_id_ = userId;
			data.img_id_ = info.imgId;
			data.OriginalPath = info.OriginalPath;
			data.thumbnailPath = info.thumbnailPath;
			PublicDB::GetInstance()->WriteCustomData(data);


			string url = info.thumbnailPath;
			std::wstring dir = QPath::GetUserAppDataDir(userId) + L"like\\";
			std::wstring name = dir + nbase::UTF8ToUTF16(data.img_id_) + L".png";

			if (!nbase::FilePathIsExist(dir, true) || nbase::FilePathIsExist(name, true))
				//nbase::win32::CreateDirectoryRecursively(photo_dir.c_str());
				return;
			//URLDownloadToImgFile(url_o, name_o);
			URLDownloadToImgFile(url, name);
		}
	}
	for (auto localInfo : localImgInfoList)
	{
		bool tag = false;
		for (auto info : imgInfoList)
		{
			if (info.imgId == localInfo.imgId)
			{
				tag = true;
				break;
			}
		}
		if (!tag)
		{
			//删除本地数据库对应图片数据
			CustomData data;
			data.user_id_ = userId;
			data.img_id_ = localInfo.imgId;
			data.OriginalPath = localInfo.OriginalPath;
			data.thumbnailPath = localInfo.thumbnailPath;
			PublicDB::GetInstance()->DeleteCustomData(data);


			std::wstring dir = QPath::GetUserAppDataDir(userId) + L"like\\";
			std::wstring wName = dir + nbase::UTF8ToUTF16(data.img_id_) + L".png";
			std::string name = nbase::UTF16ToUTF8(wName);

			//std::remove(name_o.data());
			std::remove(name.data());
			//if (!nbase::FilePathIsExist(name_o, false) && !nbase::FilePathIsExist(name, false))
			//return;

		}
	}
}*/