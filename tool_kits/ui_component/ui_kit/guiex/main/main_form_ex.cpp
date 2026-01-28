#include "stdafx.h"
#include "main_form_ex.h"
#include "shared\ui\ui_menu.h"
#include "export\nim_ui_contacts_list_manager.h"
#include "export\nim_ui_session_list_manager.h"
#include "callback\login\login_callback.h"
#include "util\user.h"
#include "nim_service\module\service\session_service.h"
#include "module\plugins\main_plugins_manager.h"
#include "module\session\session_manager.h"
#include "gui/profile_form/profile_form.h"
//#include "gui/profile_mine/profile_mine.h"
#include "gui/plugins/session/session_plugin.h"
#include "nim_service\module\subscribe_event\subscribe_event_manager.h"
#include "main_form_menu.h"
//#include "OleIdl.h"
#include "ShObjIdl.h"
#include <shlobj.h>
#include "../../../../../nim_win_demo/gui/main/shortcutMsgBoxEx.h"
#include "../../module/shortcuts/shortcuts_manager.h"
#include "capture_image/src/capture_manager.h"
#include "../../export/nim_ui_login_manager.h"
#include "ui_kit\export\nim_ui_user_manager.h"
#include "nim_win_demo/module/db/public_db.h"
#include "export\nim_ui_window_manager.h"
#include "gui/session/dragdrop/drag_form.h"
#include "module/dragdrop/drag_drop.h"

namespace
{
	const int kDragImageWidth = 300;
	const int kDragImageHeight = 300;
}

namespace
{
	const int kSplitFormXOffset = 20;	//自动拆分会话窗口后新窗口的x偏移坐标
	const int kSplitFormYOffset = 20;	//自动拆分会话窗口后新窗口的y偏移坐标
	const int kDragFormXOffset = -100;	//拖拽出新会话窗口后的相对鼠标的x偏移坐标
	const int kDragFormYOffset = -20;	//拖拽出新会话窗口后的相对鼠标的y偏移坐标
}

namespace nim_comp
{
	const LPCTSTR MainFormEx::kClassName = L"MainFormEx";
	MainFormEx::MainFormEx(IMainFormMenuHandler* main_menu_handler) : 
		is_trayicon_left_double_clicked_(false), is_busy_(false),
		main_menu_handler_(main_menu_handler),
		btn_max_restore_(nullptr)
	{
		OnUserInfoChangeCallback cb1 = nbase::Bind(&MainFormEx::OnUserInfoChange, this, std::placeholders::_1);
		unregister_cb.Add(nim_comp::UserService::GetInstance()->RegUserInfoChange(cb1));

		OnPhotoReadyCallback cb2 = nbase::Bind(&MainFormEx::OnUserPhotoReady, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		unregister_cb.Add(nim_comp::PhotoService::GetInstance()->RegPhotoReady(cb2));
		main_menu_handler_->SetHostWindow(this);
		drop_helper_ = nullptr;

		old_drag_point_.x = old_drag_point_.y = 0;

		
	}

	MainFormEx::~MainFormEx()
	{

	}

	std::wstring MainFormEx::GetSkinFolder()
	{
		return L"mainex";
	}

	std::wstring MainFormEx::GetSkinFile()
	{
		return L"main.xml";
	}

	std::wstring MainFormEx::GetWindowClassName() const
	{
		return kClassName;
	}

	std::wstring MainFormEx::GetWindowId() const
	{
		return kClassName;
	}
	void MainFormEx::starClip()
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
		StdClosure callback = nbase::Bind(&MainFormEx::DoClip, this);
		nbase::ThreadManager::PostDelayedTask(kThreadUI, callback, nbase::TimeDelta::FromMilliseconds(500));
	}

	void MainFormEx::DoClip()
	{
		std::wstring send_info;
		CaptureManager::CaptureCallback callback = nbase::Bind(&MainFormEx::OnClipCallback, this, std::placeholders::_1, std::placeholders::_2);
		std::string acc = nim_ui::LoginManager::GetInstance()->GetAccount();
		assert(!acc.empty());
		std::wstring app_data_audio_path = QPath::GetUserAppDataDir(acc);
		if (CaptureManager::GetInstance()->StartCapture(callback, app_data_audio_path, send_info) == false)
		{
			OnClipCallback(FALSE, L"");
		}
	}

	void MainFormEx::OnClipCallback(bool ret, const std::wstring& file_path)
	{
		if (ret)
		{
			//InsertImageToEdit(input_edit_, file_path, false);
		}
	};

	/*void RegisterShortcuts(LPVOID Para)
	{
		std::string screenshots = "";
		std::string popMsg = "";
		nim_comp::ShortcutsManager::GetInstance()->GetShortcutsSetting(screenshots, popMsg);


		shortcutMsgBoxEx *registerMsgBox = shortcutMsgBoxEx::getInstence();
		ShortcutCallBack callbackshortcut = nbase::Bind(&MainFormEx::starClip, this);
		ShortcutCallBack callbackpopmsgbox = nbase::Bind(&MainFormEx::showSessionBox, this);
		registerMsgBox->setCallbackFuntion(callbackshortcut, callbackpopmsgbox);

		registerMsgBox->registerShortcuts(screenshots, popMsg);
		registerMsgBox->setReceivedMsg();
	}*/
	void ReceivedMsg(LPVOID Para)
	{
		std::string screenshots = "";
		std::string popMsg = "";
		nim_comp::ShortcutsManager::GetInstance()->GetShortcutsSetting(screenshots, popMsg);
		shortcutMsgBoxEx *registerMsgBox = shortcutMsgBoxEx::getInstence();
		registerMsgBox->registerShortcuts(screenshots, popMsg);
		registerMsgBox->setReceivedMsg();
	}
	void MainFormEx::OnlyRegShortcuts(std::string screenshots, std::string popMsg)
	{
		shortcutMsgBoxEx *registerMsgBox = shortcutMsgBoxEx::getInstence();
		registerMsgBox->reRegisterShortcuts(screenshots, popMsg);
	}
	void MainFormEx::RegShortcuts()
	{
		shortcutMsgBoxEx *registerMsgBox = shortcutMsgBoxEx::getInstence();
		ShortcutCallBack callbackshortcut = nbase::Bind(&MainFormEx::starClip, this);
		ShortcutCallBack callbackpopmsgbox = nbase::Bind(&MainFormEx::showSessionBox, this);
		registerMsgBox->setCallbackFuntion(callbackshortcut, callbackpopmsgbox);

		//registerMsgBox->setReceivedMsg();
		_beginthread(ReceivedMsg, 0, this);

		//_beginthread(RegisterShortcuts, 0, this);
		//return;
		//shortcutMsgBoxEx::getInstence()->unRegisterShortcuts();
	}

	void MainFormEx::showSessionBox()
	{
		ShowWindow();
		WindowEx* box = (WindowEx *)(nim_comp::SessionManager::GetInstance()->OpenCurrentSessionBox());
		if (NULL == box || true)
		{
			ActiveWindow();
		
			/*if (::IsWindow(m_hWnd))
			{
				if (::IsIconic(m_hWnd))
				{
					::ShowWindow(m_hWnd, SW_RESTORE);
				}
				else
				{
					//if (!::IsWindowVisible(m_hWnd))
					if (!::GetWindowLong(m_hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST)
					{
						::ShowWindow(m_hWnd, SW_SHOW);
						::SetForegroundWindow(m_hWnd);
					}
					else
					{
						::ShowWindow(m_hWnd, SW_SHOWMINIMIZED);
					}
				}
			}*/
			//ActiveWindow();
		}
		else
		{
			if (::IsWindow(box->GetHWND()))
			{
				if (::IsIconic(box->GetHWND()))
				{
					::ShowWindow(box->GetHWND(), SW_RESTORE);
				}
				else
				{
					if (!::IsWindowVisible(box->GetHWND()))
					{
						::ShowWindow(box->GetHWND(), SW_SHOW);
						::SetForegroundWindow(box->GetHWND());
					}
					else
					{
						::ShowWindow(box->GetHWND(), SW_SHOWMINIMIZED);
						::ShowWindow(m_hWnd, SW_SHOWMINIMIZED);
					}
				}
			}
			//box->ActiveWindow();
		}
	}

	void BeginAsynSynDataEx()
	{
		//MainFormEx * main_form = (MainFormEx *)(nim_ui::WindowsManager::GetInstance()->GetWindow(MainFormEx::kClassName, MainFormEx::kClassName));
		MainFormEx * main_form = (MainFormEx *)(nim_ui::WindowsManager::GetInstance()->GetWindow(MainFormEx::kClassName, MainFormEx::kClassName));
		//main_form->GetCustomImgAsyn();
		main_form->GetCustomEmoticonAsyn();
		main_form->GeClientDataAsyn();
	}

	void MainFormEx::InitWindow()
	{
		//SessionManager::GetInstance()->SetEnableMerge(true);
		enable_merge_ = true;
		InitDragDrop();
		main_bar_ = dynamic_cast<ui::VBox*>(FindControl(L"main_bar"));
		main_plugins_bar_ = dynamic_cast<ui::VBox*>(FindControl(L"main_plugin_bar"));
		simple_plugin_bar_ = dynamic_cast<ui::VBox*>(FindControl(L"simple_plugin_bar"));
		main_plugins_showpage_ = dynamic_cast<ui::TabBox*>(FindControl(L"main_plugins_showpage"));
		btn_max_restore_ = static_cast<ui::Button*>(FindControl(L"btn_max_restore"));
		btn_header_ = dynamic_cast<ui::Button*>(FindControl(L"btn_header"));		
		InitHeader();
		btn_online_state_ = dynamic_cast<ui::Button*>(FindControl(L"btn_online_state"));
		btn_online_state_->SetVisible(nim_comp::SubscribeEventManager::GetInstance()->IsEnabled());		
		btn_header_->AttachClick([this](ui::EventArgs* param){
			//nim_comp::WindowsManager::GetInstance()->SingletonShow<nim_comp::ProfileMine>(nim_comp::ProfileMine::kClassName);
			nim_comp::ProfileForm::ShowProfileForm(nim_comp::LoginManager::GetInstance()->GetAccount());
			return true;
		});
		btn_online_state_->AttachClick(nbase::Bind(&MainFormEx::OnlineStateMenuButtonClick, this, std::placeholders::_1));
		TrayIconManager::GetInstance()->AddTrayIconEventHandler(this);
		
		LoadPlugins();
		nim_ui::ContactsListManager::GetInstance()->InvokeGetAllUserInfo();
		nim_ui::SessionListManager::GetInstance()->QueryUnreadSysMsgCount();
		nim_comp::SessionService::GetInstance()->InvokeLoadSessionList();
		

		ui::ButtonBox* main_menu_button = (ui::ButtonBox*)FindControl(L"main_menu_button");
		main_menu_button->AttachClick(nbase::Bind(&MainFormEx::MainMenuButtonClick, this, std::placeholders::_1));

		InitSearchBar();
		m_pRoot->AttachBubbledEvent(ui::kEventAll, nbase::Bind(&MainFormEx::Notify, this, std::placeholders::_1));
		m_pRoot->AttachBubbledEvent(ui::kEventClick, nbase::Bind(&MainFormEx::OnClicked, this, std::placeholders::_1));

		RegShortcuts();
		 
		nbase::ThreadManager::PostTask(kThreadGlobalMisc, nbase::Bind(BeginAsynSynDataEx));
		
		/*if (nim_ui::LoginManager::GetInstance()->GetIsFristRun() == true)
		{
			nim_ui::SessionManager::GetInstance()->OpenSessionBox(nim_ui::LoginManager::GetInstance()->GetAccount(), nim::kNIMSessionTypeP2P);
		}*/
		//auto name_task = [](){
		//	static std::wstring xing = L"赵钱孙李周吴郑王冯陈褚卫蒋沈韩杨朱秦尤许何吕施张孔曹严华金魏陶姜戚谢邹喻柏水窦章云苏潘葛奚范彭郎鲁韦昌马苗凤花方俞任袁柳酆鲍史唐";
		//	static std::wstring ming = L"凤佛否夫敷扶拂幅符伏服浮福弗甫辅俯斧府腐赴副覆赋复傅付父腹负富附妇该改概盖溉干甘柑竿肝赶感敢刚纲皋高膏稿告歌鸽格阁隔个各给根耕更庚耿工攻功恭供公宫弓拱贡共钩勾沟苟狗垢沽孤姑鼓古骨谷故顾固刮瓜寡挂怪棺关官冠观管馆罐灌贯光广规归龟闺轨鬼癸桂";
		//	std::wstring ret;
		//	ret.append(1,xing[rand() % xing.length()]);
		//	auto ming_length = rand() % 2 + 1;
		//	for (auto i = 0; i < ming_length; i++)
		//		ret.append(1, ming[rand() % ming.length()]);
		//	return ret;
		//};
		//auto mobile_task = [](){
		//	static std::vector<std::wstring> head = { L"160", L"161", L"162", L"163", L"164", L"165", L"166", L"167" };
		//	static std::vector<std::wstring> body = { L"11111111", L"22222222", L"33333333", L"55555555", L"66666666", L"77777777", L"88888888" };
		//	std::wstring ret;
		//	ret.append(head[rand() % head.size()]);
		//	ret.append(body[rand() % body.size()]);
		//	return ret;
		//};
		//auto department_task = [](int index){
		//	std::wstring ret;
		//	static std::vector<std::wstring> dep_index = { L"一", L"二", L"三", L"四", L"五", L"六", L"七", L"八", L"九", L"十", L"十一" };
		//	ret = nbase::StringPrintf(L"第%s事业部", dep_index[index].c_str());
		//	return ret;
		//};
		//auto employee_sa_task = [](){
		//	static int employee_sa_begin = 200;
		//	return nbase::StringPrintf(L"%06d", employee_sa_begin++);
		//};
		//auto email_task = [](const std::wstring& employee_sa){
		//	std::wstring ret(L"addresbook_");
		//	if (employee_sa.empty())
		//		ret.append(L"No").append(L"@demo.com");
		//	else
		//		ret.append(employee_sa).append(L"@demo.com");
		//	return ret;
		//};
		//TiXmlDocument* document = new TiXmlDocument("d:/addresbook.xml");
		//TiXmlElement* root = new TiXmlElement("Item");	
		//root->SetAttribute("id", QString::GetGUID().c_str());
		//root->SetAttribute("type", 0);
		//root->SetAttribute("name", nbase::UTF16ToUTF8(L"nim_demo科技"));
		//document->LinkEndChild(root);
		//TiXmlElement* elment = new TiXmlElement("Item");
		//root->LinkEndChild(elment);
		//elment->SetAttribute("id", QString::GetGUID().c_str());
		//elment->SetAttribute("type", 0);
		//elment->SetAttribute("name", nbase::UTF16ToUTF8(L"企业直属人员"));
		//auto count = rand() % 100 + 1;
		//if (count < 30)
		//	count = 30;
		//for (auto i = 0; i < count; i++)
		//{
		//	auto sub_elment = new TiXmlElement("Item");
		//	elment->LinkEndChild(sub_elment);
		//	sub_elment->SetAttribute("id", QString::GetGUID().c_str());
		//	sub_elment->SetAttribute("type", 1);
		//	sub_elment->SetAttribute("name", nbase::UTF16ToUTF8(name_task()));
		//	sub_elment->SetAttribute("mobile", nbase::UTF16ToUTF8(mobile_task()));
		//	sub_elment->SetAttribute("mail", nbase::UTF16ToUTF8(email_task(L"")));
		//}

		//auto task_create_sub = [&](TiXmlElement* dep_elment){
		//	for (int j = 0; j < 10; j++)
		//	{
		//		auto sub_dep_elment = new TiXmlElement("Item");
		//		dep_elment->LinkEndChild(sub_dep_elment);
		//		sub_dep_elment->SetAttribute("id", QString::GetGUID().c_str());
		//		sub_dep_elment->SetAttribute("type", 0);
		//		sub_dep_elment->SetAttribute("name", nbase::UTF16ToUTF8(department_task(j)));
		//		int count = rand() % 80;
		//		if (count < 30)
		//			count = 30;
		//		if (count > 0)
		//		{
		//			for (int k = 0; k < count; k++)
		//			{
		//				auto  employee_elment = new TiXmlElement("Item");
		//				sub_dep_elment->LinkEndChild(employee_elment);
		//				employee_elment->SetAttribute("id", QString::GetGUID().c_str());
		//				employee_elment->SetAttribute("type", 1);
		//				auto sa = employee_sa_task();
		//				employee_elment->SetAttribute("employeesa", nbase::UTF16ToUTF8(sa));
		//				employee_elment->SetAttribute("name", nbase::UTF16ToUTF8(name_task()));
		//				employee_elment->SetAttribute("mobile", nbase::UTF16ToUTF8(mobile_task()));
		//				employee_elment->SetAttribute("mail", nbase::UTF16ToUTF8(email_task(sa)));
		//			}
		//		}

		//		{
		//			for (int j = 0; j < 10; j++)
		//			{
		//				auto sub_sub_dep_elment = new TiXmlElement("Item");
		//				sub_dep_elment->LinkEndChild(sub_sub_dep_elment);
		//				sub_sub_dep_elment->SetAttribute("id", QString::GetGUID().c_str());
		//				sub_sub_dep_elment->SetAttribute("type", 0);
		//				sub_sub_dep_elment->SetAttribute("name", nbase::UTF16ToUTF8(department_task(j)));
		//				int count = rand() % 60;
		//				if (count < 5)
		//					count = 5;
		//				if (count > 0)
		//				{
		//					for (int k = 0; k < count; k++)
		//					{
		//						auto  employee_elment = new TiXmlElement("Item");
		//						sub_sub_dep_elment->LinkEndChild(employee_elment);
		//						employee_elment->SetAttribute("id", QString::GetGUID().c_str());
		//						employee_elment->SetAttribute("type", 1);
		//						auto sa = employee_sa_task();
		//						employee_elment->SetAttribute("employeesa", nbase::UTF16ToUTF8(sa));
		//						employee_elment->SetAttribute("name", nbase::UTF16ToUTF8(name_task()));
		//						employee_elment->SetAttribute("mobile", nbase::UTF16ToUTF8(mobile_task()));
		//						employee_elment->SetAttribute("mail", nbase::UTF16ToUTF8(email_task(sa)));
		//					}
		//				}
		//			}
		//		}
		//	}
		//};

		//for (int i = 1; i < 10; i++)
		//{
		//	auto dep_elment = new TiXmlElement("Item");
		//	root->LinkEndChild(dep_elment);
		//	dep_elment->SetAttribute("id", QString::GetGUID().c_str());
		//	dep_elment->SetAttribute("type", 0);
		//	dep_elment->SetAttribute("name", nbase::UTF16ToUTF8(department_task(i)));
		//	task_create_sub(dep_elment);
		//}
		//document->SaveFile();
	}

	void MainFormEx::GeClientDataAsyn()
	{
		//向服务器发送请求获取客户端数据
		nim_ui::UserManager::GetInstance()->InvokeGetClientSetting(ToWeakCallback([this](int code, const std::string& err_msg, const std::map<std::string, std::string>&map_) {
			if (code == 0)
			{
				//读取数据库中对应用户id的图片id
				//GetCustomEmoticonFromDB(imgInfo);
				nim_ui::LoginManager::GetInstance()->SetClientSetting(map_);
				//cjy
				UTF16String sloging = nbase::UTF8ToUTF16(LoginManager::GetInstance()->GetLoginName());
				nim::UserNameCard info;
				std::string uid = LoginManager::GetInstance()->GetAccount();
				UserService::GetInstance()->GetUserInfo(uid, info);
				UTF16String sname = nbase::UTF8ToUTF16(info.GetName());

				auto hicon = ::ExtractIcon(nbase::win32::GetCurrentModuleHandle(), nbase::win32::GetCurrentModuleName().c_str(), 0);
				TrayIconManager::GetInstance()->SetTrayIcon(hicon, ui::MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_MIANWINDOW_TITLE") + L"\n用户名:" + sloging + L"\n昵称:" + sname);
				((MainFormEx *)(nim_ui::WindowsManager::GetInstance()->GetWindow(MainFormEx::kClassName, MainFormEx::kClassName)))->SetTaskbarTitle(sname);
			}
			else
			{
				//ShowLoginTip(MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_LOGIN_FORM_TIP_PASSWORD_ERROR"));
				//弹框展示失败
			}
		}));
	}

	void MainFormEx::GetCustomEmoticonAsyn()
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

	long URLDownloadToImgFileEx(std::string url, std::wstring name)
	{
		size_t len = url.length();//获取字符串长度
		int nmlen = MultiByteToWideChar(CP_ACP, 0, url.c_str(), len + 1, NULL, 0);//如果函数运行成功，并且cchWideChar为零，

		//返回值是接收到待转换字符串的缓冲区所需求的宽字符数大小。
		wchar_t* buffer = new wchar_t[nmlen];
		MultiByteToWideChar(CP_ACP, 0, url.c_str(), len + 1, buffer, nmlen);

		return URLDownloadToFile(0, buffer, name.data(), 0, 0);
	}

	void MainFormEx::GetForcePushFromDB()
	{
		/*std::string userId = nim_ui::LoginManager::GetInstance()->GetAccount();
		std::map<std::string, std::string> countMap;
		PublicDB::GetInstance()->ReadForcePushData(userId, countMap);*/

		//nim_ui::LoginManager::GetInstance()->GetAccount()
	}

	void MainFormEx::GetCustomEmoticonFromDB(const std::list<CustomImgInfo>& imgInfoList)
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
				//更新图列表，同步云端图片
				/*string url_o = info.OriginalPath;
				std::wstring dir_o = QPath::GetUserAppDataDir(userId) + L"customImg\\custom_o\\";
				std::wstring name_o = dir_o + nbase::UTF8ToUTF16(data.img_id_) + L".png";*/

				string url = info.thumbnailPath;
				std::wstring dir = QPath::GetUserAppDataDir(userId) + L"like\\";
				std::wstring name = dir + nbase::UTF8ToUTF16(data.img_id_) + L".png";

				if (!nbase::FilePathIsExist(dir, true) || nbase::FilePathIsExist(name, true))
					//nbase::win32::CreateDirectoryRecursively(photo_dir.c_str());
					return;
				//URLDownloadToImgFile(url_o, name_o);
				URLDownloadToImgFileEx(url, name);
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
				/*std::wstring dir_o = QPath::GetUserAppDataDir(userId) + L"customImg\\custom_o\\";
				std::wstring wName_o = dir_o + nbase::UTF8ToUTF16(data.img_id_) + L".png";
				std::string name_o = nbase::UTF16ToUTF8(wName_o);*/

				std::wstring dir = QPath::GetUserAppDataDir(userId) + L"like\\";
				std::wstring wName = dir + nbase::UTF8ToUTF16(data.img_id_) + L".png";
				std::string name = nbase::UTF16ToUTF8(wName);

				//std::remove(name_o.data());
				std::remove(name.data());
				//if (!nbase::FilePathIsExist(name_o, false) && !nbase::FilePathIsExist(name, false))
				//return;

			}
		}
	}

	LRESULT MainFormEx::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LRESULT result = WindowEx::OnNcHitTest(uMsg, wParam, lParam, bHandled);
		if (result == HTCLIENT)
		{
			POINT point;
			GetCursorPos(&point);
			::ScreenToClient(m_hWnd, &point);
			if (main_bar_)
			{
				RECT rect = main_bar_->GetPos();
				if (::PtInRect(&rect, point))
				{
					ui::Control* pControl = FindControl(point);
					if (pControl)
					{
						if (dynamic_cast<ui::Button*>(pControl) || dynamic_cast<ui::ButtonBox*>(pControl) || dynamic_cast<ui::RichEdit*>(pControl))
							return HTCLIENT;						
						else
							return HTCAPTION;
					}
				}
			}
		}
		return result;
	}
	void MainFormEx::LoadPlugins()
	{
		MainPluginsManager::GetInstance()->LoadPlugins();
		auto&& plugins = MainPluginsManager::GetInstance()->GetPluginList();
		ui::VBox* plugin_bar = nullptr;
		for (auto plugin : plugins)
		{
			if (plugin->GetPluginType() == IMainPlugin::PluginType::PluginType_Main)
				plugin_bar = main_plugins_bar_;
			else if(plugin->GetPluginType() == IMainPlugin::PluginType::PluginType_Simple)
				plugin_bar = simple_plugin_bar_;
			else
				continue;
			auto icon = plugin->GetPluginIcon();
			auto page = plugin->GetPluginPage();
			if (icon != nullptr)	plugin_bar->Add(icon);
			if (page != nullptr)	main_plugins_showpage_->Add(page);
			plugin->DoInit();
			if (plugin->GetPluginType() == IMainPlugin::PluginType::PluginType_Main)
			{
				plugin->SetGroup(L"Main_Plugin_Group");
				plugin->AttachSelect(plugin->ToWeakCallback([this, plugin](ui::EventArgs* param){
					search_bar_->SetVisible(plugin->ShowSearchBar());
					search_edit_->SetText(L"");
					if (plugin->GetPluginPage() != nullptr)
						main_plugins_showpage_->SelectItem(plugin->GetPluginPage());

					return true;
				}));
			}			
		}
	}
	ui::TabBox * MainFormEx::getActiveSessionBoxTab()
	{
		ISessionDock* ret = nullptr;
		auto&& plugin = MainPluginsManager::GetInstance()->GetPlugin(SessionPlugin::kPLUGIN_NAME);
		if (plugin != nullptr)
		{
			ret = std::dynamic_pointer_cast<SessionPlugin>(plugin)->GetSessionDock();
		}
		SessionPluginPage *pluginPage = (SessionPluginPage*)ret;
		if (pluginPage == nullptr)
		{
			return NULL;
		}
		ui::TabBox *session_box_tab = pluginPage->getActiveSessionBoxTab();
		return session_box_tab;
	}

	SessionBox * MainFormEx::getActiveSessionBox()
	{
		ISessionDock* ret = nullptr;
		auto&& plugin = MainPluginsManager::GetInstance()->GetPlugin(SessionPlugin::kPLUGIN_NAME);
		if (plugin != nullptr)
		{
			ret = std::dynamic_pointer_cast<SessionPlugin>(plugin)->GetSessionDock();
		}
		SessionPluginPage *pluginPage = (SessionPluginPage*)ret;
		if (pluginPage == nullptr)
		{
			return NULL;
		}
		SessionBox	*active_session_box = pluginPage->getActiveSessionBox();
		return active_session_box;
	}

	//bool MainFormEx::OnProcessSessionBoxHeaderDrag(ui::EventArgs* param)
	//{
	//	if (!SessionManager::GetInstance()->IsEnableMerge())
	//		return true;
	//	ui::TabBox * session_box_tab = getActiveSessionBoxTab();
	//	switch (param->Type)
	//	{
	//	case ui::EventType::kEventMouseMove:
	//	{
	//		if (::GetKeyState(VK_LBUTTON) >= 0)
	//			break;

	//		LONG cx = abs(param->ptMouse.x - old_drag_point_.x);
	//		LONG cy = abs(param->ptMouse.y - old_drag_point_.y);

	//		if (!is_drag_state_ && (cx > 5 || cy > 5))
	//		{
	//			SessionBox *active_session_box = getActiveSessionBox();
	//			if (NULL == active_session_box)
	//				break;

	//			is_drag_state_ = true;

	//			// 把被拖拽的会话盒子生成一个宽度300的位图
	//			HBITMAP bitmap = GenerateSessionBoxBitmap(session_box_tab->GetPos(true));

	//			// pt应该指定相对bitmap位图的左上角(0,0)的坐标,这里设置为bitmap的中上点
	//			POINT pt = { kDragImageWidth / 2, 0 };

	//			StdClosure cb = [=]{
	//				SessionManager::GetInstance()->DoDragSessionBox(active_session_box, bitmap, pt);
	//			};
	//			nbase::ThreadManager::PostTask(ThreadId::kThreadUI, cb);
	//		}
	//	}
	//	break;
	//	case ui::EventType::kEventMouseButtonDown:
	//	{
	//		// 如果当前会话窗口里只有一个会话盒子，则可以通过拖拽头像来拖拽会话盒子
	//		// 否则只能拖拽merge_list的merge_item来拖拽
	//		
	//		if (1 == session_box_tab->GetCount())
	//		{
	//			old_drag_point_ = param->ptMouse;
	//			is_drag_state_ = false;
	//		}
	//	}
	//	break;
	//	}
	//	return true;
	//}

	bool MainFormEx::OnProcessMergeItemDrag(ui::EventArgs* param)
	{
		if (!enable_merge_)
			return true;

		switch (param->Type)
		{
		case ui::EventType::kEventMouseMove:
		{
			item_param_ = param;
			if (::GetKeyState(VK_LBUTTON) >= 0)
				break;

			LONG cx = abs(param->ptMouse.x - old_drag_point_.x);
			LONG cy = abs(param->ptMouse.y - old_drag_point_.y);

			if (!is_drag_state_ && (cx > 5 || cy > 5))
			{
				SessionBox *active_session_box = getActiveSessionBox();
				if (NULL == active_session_box)
					break;

				ui::TabBox * session_box_tab = getActiveSessionBoxTab();
				is_drag_state_ = true;

				// 把被拖拽的会话盒子生成一个宽度300的位图
				HBITMAP bitmap = GenerateSessionBoxBitmap(session_box_tab->GetPos(true));

				// pt应该指定相对bitmap位图的左上角(0,0)的坐标,这里设置为bitmap的中上点
				POINT pt = { kDragImageWidth / 2, 0 };

				StdClosure cb = [=]{
					//SessionManager::GetInstance()->DoDragSessionBox(active_session_box, bitmap, pt);
					this->DoDragSessionBox(active_session_box, bitmap, pt);
				};
				nbase::ThreadManager::PostTask(ThreadId::kThreadUI, cb);
			}
		}
		break;
		case ui::EventType::kEventMouseButtonDown:
		{
			old_drag_point_ = param->ptMouse;
			is_drag_state_ = false;
		}
		break;
		}
		return true;
	}

	HBITMAP MainFormEx::GenerateSessionBoxBitmap(const ui::UiRect &src_rect)
	{
	ASSERT(!src_rect.IsRectEmpty());
	int src_width = src_rect.right - src_rect.left;
	int src_height = src_rect.bottom - src_rect.top;

	auto render = ui::GlobalManager::CreateRenderContext();
	if (render->Resize(kDragImageWidth, kDragImageHeight))
	{
	int dest_width = 0;
	int dest_height = 0;
	float scale = (float)src_width / (float)src_height;
	if (scale >= 1.0)
	{
	dest_width = kDragImageWidth;
	dest_height = (int)(kDragImageWidth * (float)src_height / (float)src_width);
	}
	else
	{
	dest_height = kDragImageHeight;
	dest_width = (int)(kDragImageHeight * (float)src_width / (float)src_height);
	}

	render->AlphaBlend((kDragImageWidth - dest_width) / 2, 0, dest_width, dest_height, this->GetRenderContext()->GetDC(),
	src_rect.left, src_rect.top, src_rect.right - src_rect.left, src_rect.bottom - src_rect.top);
	}

	return render->DetachBitmap();
	}

	SdkDataObject* MainFormEx::CreateDragDataObject(HBITMAP bitmap, POINT pt_offset)
	{
		SdkDataObject* data_object = new SdkDataObject;
		if (data_object == NULL)
			return NULL;

		if (use_custom_drag_image_)
		{
			FORMATETC fmtetc = { 0 };
			fmtetc.dwAspect = DVASPECT_CONTENT;
			fmtetc.lindex = -1;
			fmtetc.cfFormat = CF_HDROP;
			fmtetc.tymed = TYMED_NULL;

			STGMEDIUM medium = { 0 };
			medium.tymed = TYMED_NULL;
			data_object->SetData(&fmtetc, &medium, FALSE);
		}
		else
		{
			FORMATETC fmtetc = { 0 };
			fmtetc.dwAspect = DVASPECT_CONTENT;
			fmtetc.lindex = -1;
			fmtetc.cfFormat = CF_BITMAP;
			fmtetc.tymed = TYMED_GDI;

			STGMEDIUM medium = { 0 };
			medium.tymed = TYMED_GDI;
			HBITMAP hBitmap = (HBITMAP)OleDuplicateData(bitmap, fmtetc.cfFormat, NULL);
			medium.hBitmap = hBitmap;
			data_object->SetData(&fmtetc, &medium, FALSE);

			BITMAP bitmap_info;
			GetObject(hBitmap, sizeof(BITMAP), &bitmap_info);
			SIZE bitmap_size = { bitmap_info.bmWidth, bitmap_info.bmHeight };
			SdkDragSourceHelper dragSrcHelper;
			dragSrcHelper.InitializeFromBitmap(hBitmap, pt_offset, bitmap_size, data_object, RGB(255, 0, 255));
		}

		return data_object;
	}

	bool MainFormEx::DoDragSessionBox(SessionBox *session_box, HBITMAP bitmap, POINT pt_offset)
	{
		if (!enable_merge_)
			return false;

		SdkDropSource* drop_src = new SdkDropSource;
		if (drop_src == NULL)
			return false;

		SdkDataObject* data_object = CreateDragDataObject(bitmap, pt_offset);
		if (data_object == NULL)
			return false;

		// 无论什么时候都让拖拽时光标显示为箭头
		drop_src->SetFeedbackCursor(::LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));

		OnBeforeDragSessionBox(session_box, bitmap, pt_offset);

		// 此函数会阻塞直到拖拽完成
		DWORD dwEffect;
		HRESULT hr = ::DoDragDrop(data_object, drop_src, DROPEFFECT_COPY | DROPEFFECT_MOVE, &dwEffect);

		OnAfterDragSessionBox();

		// 销毁位图
		DeleteObject(bitmap);
		drop_src->Release();
		data_object->Release();
		return true;
	}

	void MainFormEx::OnBeforeDragSessionBox(SessionBox *session_box, HBITMAP bitmap, POINT pt_offset)
	{
		// 获取当前被拖拽的会话盒子所属的会话窗口
		draging_session_box_ = session_box;
		ISessionDock *drag_session_form = draging_session_box_->GetSessionForm();
		ASSERT(NULL != drag_session_form);

		// 获取被拖拽会话窗口中会话盒子的数量
		int box_count = drag_session_form->GetSessionBoxCount();
		ASSERT(box_count > 0);
		drop_session_form_ = NULL;

		drag_session_form->OnBeforeDragSessionBoxCallback(nbase::UTF8ToUTF16(draging_session_box_->GetSessionId()));

		if (use_custom_drag_image_)
			DragForm::CreateCustomDragImage(bitmap, pt_offset);
	}

	void MainFormEx::OnAfterDragSessionBox()
	{
		if (use_custom_drag_image_)
			DragForm::CloseCustomDragImage();

		if (NULL == draging_session_box_)
			return;

		// 获取当前被拖拽的会话盒子所属的会话窗口
		ISessionDock *drag_session_form = draging_session_box_->GetSessionForm();
		ASSERT(NULL != drag_session_form);

		// 获取被拖拽会话窗口中会话盒子的数量
		int box_count = drag_session_form->GetSessionBoxCount();
		ASSERT(box_count > 0);

		// 如果被拖拽的会话盒子放入到一个会话窗口里
		if (NULL != drop_session_form_)
		{
			if (drag_session_form == drop_session_form_)
			{
				drag_session_form->OnAfterDragSessionBoxCallback(false);
			}
			else
			{
				drag_session_form->OnAfterDragSessionBoxCallback(true);
				if (drag_session_form->DetachSessionBox(draging_session_box_))
				{
					drop_session_form_->AttachSessionBox(draging_session_box_);
				}
			}

			// 如果被拖拽的会话窗口包含多个会话盒子，就投递一个WM_LBUTTONUP消息给窗口
			// (因为窗口被拖拽时触发了ButtonDown和ButtonMove消息，但是最终的ButtonUp消息会被忽略，这里补上)
			// 如果只有一个会话盒子，则会话盒子脱离会话窗口时，会话窗体就会关闭，不需要投递
			if (box_count > 1)
				drag_session_form->PostMessage(WM_LBUTTONUP, 0, 0);
		}
		// 如果没有被拖拽到另一个会话窗口里
		else
		{
			// 如果被拖拽的会话窗口里只有一个会话盒子,则拖拽失败
			if (1 == box_count)
			{
				drag_session_form->OnAfterDragSessionBoxCallback(false);
			}
			// 如果有多个会话盒子, 就把会话盒子脱离原会话窗口，附加到新的会话窗口，拖拽成功
			else
			{
				ISessionDock* ret = nullptr;
				auto&& plugin = MainPluginsManager::GetInstance()->GetPlugin(SessionPlugin::kPLUGIN_NAME);
				if (plugin != nullptr)
				{
					ret = std::dynamic_pointer_cast<SessionPlugin>(plugin)->GetSessionDock();
				}
				SessionPluginPage *pluginPage = (SessionPluginPage*)ret;
				pluginPage->RemoveItemAt(draging_session_box_, item_param_);

				/*nim::Session::DeleteRecentSession(msg_.type_, msg_.id_, nbase::Bind(&SessionItem::DeleteRecentSessionCb, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
				SubscribeEventManager::GetInstance()->UnSubscribeSessionEvent(msg_);
				m_pWindow->SendNotify(this, ui::kEventNotify, SET_DELETE, 0);*/

				//drag_session_form->OnAfterDragSessionBoxCallback(true);

				//if (drag_session_form->DetachSessionBox(draging_session_box_))
				//{
				//	ISessionDock *session_form = ISessionDock::InstantDock();
				//	HWND hwnd = session_form->Create();
				//	if (hwnd != NULL)
				//	{
				//		if (session_form->AttachSessionBox(draging_session_box_))
				//		{
				//			// 这里设置新会话窗口的位置，设置到偏移鼠标坐标100,20的位置
				//			POINT pt_mouse;
				//			::GetCursorPos(&pt_mouse);
				//			ui::UiRect rect(pt_mouse.x + kDragFormXOffset, pt_mouse.y + kDragFormYOffset, 0, 0);
				//			session_form->SetPos(rect, false, SWP_NOSIZE);
				//		}
				//	}
				//}
			}

			// 如果没有被拖拽到另一个会话窗口里，这时不会有会话窗口被关闭，所以直接投递ButtonUp消息
			drag_session_form->PostMessage(WM_LBUTTONUP, 0, 0);
		}

		draging_session_box_ = NULL;
		drop_session_form_ = NULL;
	}

	void MainFormEx::NewSessionForm(nim::SessionData item_msg)
	{
		RunTimeDataManager::GetInstance()->SetUIStyle(UIStyle::conventional);
		SessionManager::GetInstance()->OpenSessionBoxEx(item_msg.id_, item_msg.type_ == kFriendItemTypeTeam ? nim::kNIMSessionTypeTeam : nim::kNIMSessionTypeP2P, true);
		//ISessionDock *session_form = ISessionDock::InstantDock();
		//HWND hwnd = session_form->Create();
		//if (hwnd != NULL)
		//{
		//	SessionBox* cursessionbox = session_form->CreateSessionBox(item_msg.id_, item_msg.type_);
		//	if (cursessionbox != nullptr && session_form->AttachSessionBox(@))
		//	{
		//		// 这里设置新会话窗口的位置，设置到偏移鼠标坐标100,20的位置
		//		POINT pt_mouse;
		//		::GetCursorPos(&pt_mouse);
		//		ui::UiRect rect(pt_mouse.x + kDragFormXOffset, pt_mouse.y + kDragFormYOffset, 0, 0);
		//		session_form->SetPos(rect, false, SWP_NOSIZE);
		//	}
		//}
		RunTimeDataManager::GetInstance()->SetUIStyle(UIStyle::join);
	}

	void MainFormEx::OnUserInfoChange(const std::list<nim::UserNameCard> &uinfos)
	{
		for (auto iter = uinfos.cbegin(); iter != uinfos.cend(); iter++)
		{
			if (nim_comp::LoginManager::GetInstance()->IsEqual(iter->GetAccId()))
			{
				InitHeader();
				break;
			}
		}
	}

	void MainFormEx::OnUserPhotoReady(PhotoType type, const std::string& account, const std::wstring& photo_path)
	{
		if (type == kUser && nim_comp::LoginManager::GetInstance()->GetAccount() == account)
		{
			btn_header_->SetBkImage(photo_path);
			//更新数据库头像地址
			std::string path = nbase::UTF16ToUTF8(photo_path);
			PublicDB::GetInstance()->UpdateLoginData(account, "user_header", path);
		}
			
	}
	void MainFormEx::InitHeader()
	{
		std::string my_id = nim_comp::LoginManager::GetInstance()->GetAccount();
		std::wstring head_image = nim_comp::PhotoService::GetInstance()->GetUserPhoto(my_id);
		btn_header_->SetBkImage(head_image);
		std::string path = nbase::UTF16ToUTF8(head_image);
		PublicDB::GetInstance()->UpdateLoginData(my_id, "user_header", path);
	}
	bool MainFormEx::OnLeftClick()
	{
		this->ActiveWindow();
		::ShowWindow(m_hWnd, SW_SHOW);
		if (is_trayicon_left_double_clicked_)
		{
			is_trayicon_left_double_clicked_ = false;
			return true;
		}
		::SetForegroundWindow(m_hWnd);
		::BringWindowToTop(m_hWnd);
		return true;
	}

	bool MainFormEx::OnLeftDoubleClick()
	{
		is_trayicon_left_double_clicked_ = true;
		return true;
	}

	bool MainFormEx::OnRightClick()
	{
		POINT point;
		::GetCursorPos(&point);
		PopupTrayMenu(point);
		return true;
	}

	void MainFormEx::PopupTrayMenu(POINT point)
	{
		//创建菜单窗口
		ui::CMenuWnd* pMenu = new ui::CMenuWnd(NULL);
		ui::STRINGorID xml(L"tray_menu.xml");
		pMenu->Init(xml, _T("xml"), point, ui::CMenuWnd::RIGHT_TOP);
		//注册回调
		ui::CMenuElementUI* display_session_list = (ui::CMenuElementUI*)pMenu->FindControl(L"display_session_list");
		display_session_list->AttachSelect(ToWeakCallback([this](ui::EventArgs* param){
			auto session_plugin = MainPluginsManager::GetInstance()->GetPlugin(SessionPlugin::kPLUGIN_NAME);
			session_plugin->Selected(true, true);			
			LeftClick();
			return true;
		}));

		ui::CMenuElementUI* logoff = (ui::CMenuElementUI*)pMenu->FindControl(L"logoff");
		logoff->AttachSelect(ToWeakCallback([this](ui::EventArgs* param){
			QCommand::Set(kCmdRestart, L"true");
			std::wstring wacc = nbase::UTF8ToUTF16(LoginManager::GetInstance()->GetAccount());
			QCommand::Set(kCmdAccount, wacc);
			auto task = [](){
				nim_comp::LoginCallback::DoLogout(false, nim::kNIMLogoutChangeAccout);			
			};
			nbase::ThreadManager::PostTask(task);
			return true;
		}));

		ui::CMenuElementUI* quit = (ui::CMenuElementUI*)pMenu->FindControl(L"quit");
		quit->AttachSelect(ToWeakCallback([this](ui::EventArgs* param){
			auto task = [](){
				nim_comp::LoginCallback::DoLogout(false);
			};
			nbase::ThreadManager::PostTask(task);
			return true;
		}));
		//显示
		pMenu->Show();
	}
	bool MainFormEx::OnlineStateMenuButtonClick(ui::EventArgs* param)
	{
		RECT rect = param->pSender->GetPos();
		ui::CPoint point;
		point.x = rect.left;
		point.y = rect.bottom;
		ClientToScreen(m_hWnd, &point);

		//创建菜单窗口
		ui::CMenuWnd* pMenu = new ui::CMenuWnd(NULL);
		ui::STRINGorID xml(L"online_state_menu.xml");
		pMenu->Init(xml, _T("xml"), point);
		//注册回调
		ui::CMenuElementUI* look_log = (ui::CMenuElementUI*)pMenu->FindControl(L"online");
		look_log->AttachSelect(nbase::Bind(&MainFormEx::OnlineStateMenuItemClick, this, std::placeholders::_1));

		ui::CMenuElementUI* file_trans = (ui::CMenuElementUI*)pMenu->FindControl(L"busy");
		file_trans->AttachSelect(nbase::Bind(&MainFormEx::OnlineStateMenuItemClick, this, std::placeholders::_1));

		//显示
		pMenu->Show();
		return true;
	}
	bool MainFormEx::OnlineStateMenuItemClick(ui::EventArgs* param)
	{
		std::wstring name = param->pSender->GetName();
		if (name == L"online")
		{
			if (!is_busy_)
				return true;

			is_busy_ = false;
		}
		else if (name == L"busy")
		{
			if (is_busy_)
				return true;

			is_busy_ = true;
		}

		SetOnlineState();
		return true;
	}
	void MainFormEx::SetOnlineState()
	{
		if (!nim_comp::SubscribeEventManager::GetInstance()->IsEnabled())
			return;

		nim::EventData event_data = nim_comp::OnlineStateEventHelper::CreateBusyEvent(is_busy_);
		nim::SubscribeEvent::Publish(event_data,
			this->ToWeakCallback([this](nim::NIMResCode res_code, int event_type, const nim::EventData& event_data){
			if (res_code == nim::kNIMResSuccess)
			{
				if (is_busy_)
					btn_online_state_->SetBkImage(L"..\\menu\\icon_busy.png");
				else
					btn_online_state_->SetBkImage(L"..\\menu\\icon_online.png");
			}
			else
			{
				QLOG_ERR(L"OnlineStateMenuItemClick publish busy event error, code:{0}, event_type:{1}") << res_code << event_type;
			}
		}));
	}
	bool MainFormEx::MainMenuButtonClick(ui::EventArgs* param)
	{
		RECT rect = param->pSender->GetPos();
		ui::CPoint point;
		point.x = rect.left ;
		point.y = rect.top - 2;
		ClientToScreen(m_hWnd, &point);
		main_menu_handler_->PopupMainMenu(point);		
		return true;
	}
	void MainFormEx::InitSearchBar()
	{
		search_bar_ = dynamic_cast<ui::ButtonBox*>(FindControl(L"search_bar"));
		search_edit_ = static_cast<ui::RichEdit*>(FindControl(_T("search_edit")));
		search_edit_->AttachTextChange(nbase::Bind(&MainFormEx::SearchEditChange, this, std::placeholders::_1));
		search_edit_->SetLimitText(30);
		search_edit_->AttachBubbledEvent(ui::kEventKeyDown, nbase::Bind(&MainFormEx::Notify, this, std::placeholders::_1));
		btn_clear_input_ = static_cast<ui::Button*>(FindControl(L"clear_input"));
		btn_clear_input_->AttachClick(nbase::Bind(&MainFormEx::OnClearInputBtnClicked, this, std::placeholders::_1));
		btn_clear_input_->SetNoFocus();

		search_result_list_ = static_cast<ui::ListBox*>(FindControl(_T("search_result_list")));
		search_result_list_->AttachBubbledEvent(ui::kEventReturn, nbase::Bind(&MainFormEx::OnReturnEventsClick, this, std::placeholders::_1));
		search_result_list_->AttachBubbledEvent(ui::kEventMouseDoubleClick, [this](ui::EventArgs* param){
			search_result_list_->SetVisible(false);
			return true;
		});
	}
	bool MainFormEx::SearchEditChange(ui::EventArgs* param)
	{
		UTF8String search_key = search_edit_->GetUTF8Text();
		bool has_serch_key = !search_key.empty();
		RunTimeDataManager::GetInstance()->SetSearchingFriendState(has_serch_key);
		btn_clear_input_->SetVisible(has_serch_key);
		search_result_list_->SetVisible(has_serch_key);
		//option_panel_->SetVisible(!has_serch_key);
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

	bool MainFormEx::OnClearInputBtnClicked(ui::EventArgs* param)
	{
		search_edit_->SetText(L"");
		return true;
	}
	bool MainFormEx::OnReturnEventsClick(ui::EventArgs* param)
	{
		if (param->Type == ui::kEventReturn)
		{
			nim_comp::FriendItem* item = dynamic_cast<nim_comp::FriendItem*>(param->pSender);
			assert(item);
			if (item)
			{
				search_edit_->SetText(L"");
				nim_comp::SessionManager::GetInstance()->OpenSessionBox(item->GetUTF8DataID(), item->GetFriendItemType() == kFriendItemTypeTeam ?  nim::kNIMSessionTypeTeam:nim::kNIMSessionTypeP2P );
			}
		}

		return true;
	}
	bool MainFormEx::OnClicked(ui::EventArgs* msg)
	{
		std::wstring name = msg->pSender->GetName();
		if (name == L"btn_wnd_close")
		{
			SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
			::ShowWindow(m_hWnd, SW_HIDE);
		}	
		else if (name == L"btn_max_restore")
		{
			DWORD style = GetWindowLong(m_hWnd, GWL_STYLE);
			if (style & WS_MAXIMIZE)
				::ShowWindow(m_hWnd, SW_RESTORE);
			else
				::ShowWindow(m_hWnd, SW_MAXIMIZE);
		}
		return true;
	}
	bool MainFormEx::Notify(ui::EventArgs* msg)
	{
		std::wstring name = msg->pSender->GetName();
		if (msg->Type == ui::kEventReturn)
		{
			if (search_edit_->IsFocused() && !search_edit_->GetText().empty())
			{
				if (search_result_list_->GetCount() != 0)
				{
					nim_comp::FriendItem* item = dynamic_cast<nim_comp::FriendItem*>(search_result_list_->GetItemAt(search_result_list_->GetCurSel()));
					assert(item);
					if (item)
					{
						search_edit_->SetText(L"");
						nim_comp::SessionManager::GetInstance()->OpenSessionBox(item->GetUTF8DataID(), item->GetFriendItemType() == kFriendItemTypeTeam ?  nim::kNIMSessionTypeTeam:nim::kNIMSessionTypeP2P);
					}
				}
				return true;
			}
		}
		if (msg->Type == ui::kEventKeyDown && search_result_list_->IsVisible())
		{
			switch (msg->chKey)
			{
			case VK_UP:
			case VK_DOWN:
				ui::EventArgs msg_temp = *msg;
				msg_temp.pSender = search_result_list_;
				search_result_list_->HandleMessage(msg_temp);
				return true;
			}			
		}
		return true;
	}
	LRESULT MainFormEx::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (uMsg == WM_KEYDOWN)
		{
			if (wParam == VK_DOWN)
			{
				nim_ui::SessionListManager::GetInstance()->UpDownSessionItem(true);
				return 0;
			}
			else if (wParam == VK_UP)
			{
				nim_ui::SessionListManager::GetInstance()->UpDownSessionItem(false);
				return 0;
			}
		}
		if (uMsg == WM_SIZE)
		{
			if (wParam == SIZE_RESTORED)
				OnWndSizeMax(false);
			else if (wParam == SIZE_MAXIMIZED)
				OnWndSizeMax(true);
		}
		return __super::HandleMessage(uMsg, wParam, lParam);
	}
	void MainFormEx::OnWndSizeMax(bool max)
	{
		if (btn_max_restore_)
			btn_max_restore_->SetClass(max ? L"btn_wnd_restore" : L"btn_wnd_max");
	}
	void MainFormEx::OnEsc(BOOL &bHandled)
	{
		bHandled = TRUE;
	}
	LRESULT MainFormEx::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
		::ShowWindow(m_hWnd, SW_HIDE);
		bHandled = true;
		return 0;
	}
	void MainFormEx::OnFinalMessage(HWND hWnd)
	{
		UnInitDragDrop();
		TrayIconManager::GetInstance()->Destroy();
		__super::OnFinalMessage(hWnd);
	}
	//drop

	bool MainFormEx::InitDragDrop()
	{
		if (NULL != drop_helper_)
			return false;

		if (FAILED(CoCreateInstance(CLSID_DragDropHelper, NULL,
			CLSCTX_INPROC_SERVER,
			IID_IDropTargetHelper,
			(void**)&drop_helper_)))
		{
			QLOG_ERR(L"MainFormEx::InitDragDrop Create CLSID_DragDropHelper faild");
			return false;
		}

		if (FAILED(RegisterDragDrop(this->GetHWND(), this)))
		{
			QLOG_ERR(L"MainFormEx::InitDragDrop RegisterDragDrop faild");
			return false;
		}

		QLOG_APP(L"MainFormEx::InitDragDrop succeed");
		return true;
	}

	void MainFormEx::UnInitDragDrop()
	{
		if (NULL != drop_helper_)
			drop_helper_->Release();

		RevokeDragDrop(this->GetHWND());
	}

	HRESULT MainFormEx::QueryInterface(REFIID iid, void ** ppvObject)
	{
		if (NULL == drop_helper_)
			return E_NOINTERFACE;

		return drop_helper_->QueryInterface(iid, ppvObject);
	}

	ULONG MainFormEx::AddRef(void)
	{
		if (NULL == drop_helper_)
			return 0;

		return drop_helper_->AddRef();
	}

	ULONG MainFormEx::Release(void)
	{
		if (NULL == drop_helper_)
			return 0;

		return drop_helper_->Release();
	}

	HRESULT MainFormEx::DragEnter(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
	{
		if (NULL == drop_helper_)
			return S_OK;

		// 如果不是拖拽会话盒子
		auto active_session_box = SessionManager::GetInstance()->GetFirstActiveSession();
		if (NULL != active_session_box)
		{
			active_session_box->DragEnter(pDataObject, grfKeyState, pt, pdwEffect);
			ActiveWindow();
		}
		else
		{
			*pdwEffect = DROPEFFECT_MOVE;
		}

		drop_helper_->DragEnter(this->GetHWND(), pDataObject, (LPPOINT)&pt, *pdwEffect);
		return S_OK;
	}

	HRESULT MainFormEx::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
	{
		if (NULL == drop_helper_)
			return S_OK;

		// 如果不是拖拽会话盒子
		auto active_session_box = SessionManager::GetInstance()->GetFirstActiveSession();
		if (NULL != active_session_box)
		{
			active_session_box->DragOver(grfKeyState, pt, pdwEffect);
		}
		else
		{
			*pdwEffect = DROPEFFECT_MOVE;
		}

		drop_helper_->DragOver((LPPOINT)&pt, *pdwEffect);
		return S_OK;
	}

	HRESULT MainFormEx::DragLeave(void)
	{
		if (NULL == drop_helper_)
			return S_OK;

		// 如果不是拖拽会话盒子
		auto active_session_box = SessionManager::GetInstance()->GetFirstActiveSession();
		if (NULL != active_session_box)
		{
			active_session_box->DragLeave();
		}

		drop_helper_->DragLeave();
		return S_OK;
	}

	HRESULT MainFormEx::Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD __RPC_FAR *pdwEffect)
	{
		// 如果不是拖拽会话盒子
		auto active_session_box = SessionManager::GetInstance()->GetFirstActiveSession();
		if (NULL != active_session_box)
		{
			active_session_box->Drop(pDataObj, grfKeyState, pt, pdwEffect);
		}
		else
		{
			*pdwEffect = DROPEFFECT_MOVE;
		}

		drop_helper_->Drop(pDataObj, (LPPOINT)&pt, *pdwEffect);
		return S_OK;
	}
	/*void MainFormEx::SaveCreateTeamInfo(const nim::TeamEvent& result)
	{
		std::string userId = nim_ui::LoginManager::GetInstance()->GetAccount();
		nim_ui::UserManager::GetInstance()->InvokeCreateTeam(result.team_id_, result.team_info_.GetName(), userId, ToWeakCallback([this](int code, const std::string &msg) {
			if (code == 0)
			{
			}
			else
			{
			}
		}));
	}*/

	bool MainFormEx::IsActiveMainForm()
	{
		return (::GetForegroundWindow() == GetHWND() && !::IsIconic(GetHWND()) && IsWindowVisible(GetHWND()));
	}

	/*VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
	{
		MainFormEx * main_form = (MainFormEx *)(nim_ui::WindowsManager::GetInstance()->GetWindow(MainFormEx::kClassName, MainFormEx::kClassName));
		//nim_comp::SessionForm * session_form = (nim_comp::SessionForm *)(nim_ui::WindowsManager::GetInstance()->GetWindow(nim_comp::SessionForm::kClassName, nim_comp::SessionForm::kClassName));
		//if (main_form->IsActiveMainForm() || session_form->IsActiveSessionForm())
		{
			TrayIcon::GetInstance()->SetTrayIcon(::LoadIconW(nbase::win32::GetCurrentModuleHandle(), MAKEINTRESOURCE(IDI_ICON)), MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_MIANWINDOW_TITLE"));
			main_form->CloseTrayTimer();
			return;
		}

		if (idEvent == 1)
		{
			//main_form->GetCustomImgAsyn();
			main_form->TrayIconTimerCallBack();
		}
	}

	void MainFormEx::OpenTrayTimer()
	{
		SetTimer(m_hWnd, 1, 1000, TimerProc);
	}

	void MainFormEx::CloseTrayTimer()
	{
		//SetTimer(m_hWnd, 1, 1000, TimerProc);
		KillTimer(m_hWnd, 1);
	}

	void MainFormEx::TrayIconTimerCallBack()
	{
		static bool msg_tag = true;
		if (msg_tag)
		{
			TrayIcon::GetInstance()->SetTrayIcon(::LoadIconW(nbase::win32::GetCurrentModuleHandle(), MAKEINTRESOURCE(IDI_ICON_MSG)), MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_MIANWINDOW_TITLE"));
		}
		else
		{
			TrayIcon::GetInstance()->SetTrayIcon(::LoadIconW(nbase::win32::GetCurrentModuleHandle(), MAKEINTRESOURCE(IDI_ICON)), MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_MIANWINDOW_TITLE"));
		}
		msg_tag = !msg_tag;
	}*/

}
