#include "stdafx.h"
#include "nim_app.h"

#include "resource.h"
#include "app_dump.h"
#include "base/util/at_exit.h"
#include "base/util/string_number_conversions.h"
#include "gui/extern_ctrl/extern_ctrl_manager.h"
#include "shared/xml_util.h"
#include "gui/login/login_form.h"
#include "gui/main/main_form.h"
//#include "callback/chatroom_callback.h"
#include "module/config/config_helper.h"
//#include "cef/cef_module/manager/cef_manager.h"
#include "duilib/Utils/MultiLangSupport.h"
#include "nim_service\module\local\local_helper.h"
#include "tool_kits\ui_component\ui_kit\export\nim_ui_runtime_manager.h"
#include "app_sdk\app_config\app_sdk_config.h"
#include "module/winhttp/HttpUploadFiles.h"

void MainThread::Init()
{
	nbase::ThreadManager::RegisterThread(kThreadUI);
	PreMessageLoop();

	std::wstring theme_dir = QPath::GetAppPath();
	bool adapt_api = ConfigHelper::GetInstance()->IsAdaptDpi();
	std::wstring language = nbase::UTF8ToUTF16(ConfigHelper::GetInstance()->GetLanguage());
	ui::GlobalManager::Startup(theme_dir + L"resources\\", ExternCtrlManager::CreateExternCtrl, adapt_api, 
		L"themes\\default", language);
	nim_ui::UserConfig::GetInstance()->SetDefaultIcon(IDI_ICON);
	nim_ui::WindowsManager::SingletonShow<LoginForm>(LoginForm::kClassName);

	std::wstring app_crash = QCommand::Get(kCmdAppCrash);
	if (app_crash.empty()) {
		nim_ui::WindowsManager::SingletonShow<LoginForm>(LoginForm::kClassName);
	}
	else {
		std::wstring content =
			nbase::StringPrintf(ui::MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_APP_DUMP_DUMP_TIP").c_str(), app_crash.c_str());
		MsgboxCallback cb = nbase::Bind(&MainThread::OnMsgBoxCallback, this, std::placeholders::_1, nbase::UTF16ToUTF8(app_crash));
		ShowMsgBox(NULL, cb, content, false, L"STRING_TIPS", true, L"STRID_APP_DUMP_OPEN", true, L"STRING_NO", true);
	}

	//std::wstring app_crash = QCommand::Get(kCmdAppCrash);
	//if (false && app_crash.empty())
	//{
	//	nim_ui::WindowsManager::SingletonShow<LoginForm>(LoginForm::kClassName);
	//}
	//else
	//{
	//	/*std::wstring content = nbase::StringPrintf(ui::MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_APP_DUMP_DUMP_TIP").c_str(), app_crash.c_str());
	//	MsgboxCallback cb = nbase::Bind(&MainThread::OnMsgBoxCallback, this, std::placeholders::_1,nbase::UTF16ToUTF8(app_crash));
	//	ShowMsgBox(NULL, cb, content, false, L"STRING_TIPS", true, L"STRID_APP_DUMP_OPEN", true, L"STRING_NO", true);*/
	//	std::wstring content = nbase::StringPrintf(ui::MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_APP_DUMP_DUMP_TIP").c_str(), app_crash.c_str());
	//	MsgboxCallback cb = nbase::Bind(&MainThread::OnMsgBoxCallbackEx, this, std::placeholders::_1, nbase::UTF16ToUTF8(app_crash));
	//	ShowMsgBox(NULL, cb, content, false, L"STRING_TIPS", true, L"STRID_APP_DUMP_UPLOAD", true, L"STRING_NO", true);
	//}
}

void MainThread::Cleanup()
{
	ui::GlobalManager::Shutdown();

	PostMessageLoop();
	SetThreadWasQuitProperly(true);
	nbase::ThreadManager::UnregisterThread();	
}

void MainThread::PreMessageLoop()
{
	misc_thread_.reset(new MiscThread(kThreadGlobalMisc, "Global Misc Thread"));
	misc_thread_->Start();

	screen_capture_thread_.reset(new MiscThread(kThreadScreenCapture, "screen capture"));
	screen_capture_thread_->Start();

	db_thread_.reset(new DBThread(kThreadDatabase, "Database Thread"));
	db_thread_->Start();

	app_sdk_thread_.reset(new MiscThread(kThreadApp, "App SDK Thread"));
	app_sdk_thread_->Start();
}

void MainThread::PostMessageLoop()
{
	app_sdk_thread_->Stop();
	app_sdk_thread_.reset(NULL);

	misc_thread_->Stop();
	misc_thread_.reset(NULL);

	screen_capture_thread_->Stop();
	screen_capture_thread_.reset(NULL);

	db_thread_->Stop();
	db_thread_.reset(NULL);


}


void UploadDumpFile(const std::wstring wstrUrl, VecStParam& VecExtInfo,
	const std::wstring& wstrFilePath, const std::wstring& wstrFileKey, const OnUploadImgCallback& cb)
{
	CHttpUploadFiles request;
	request.TransDataToServer(wstrUrl, VecExtInfo, wstrFilePath, wstrFileKey);
	std::string strReceiveData;
	bool tag = false;
	//do
	{
		tag = request.getReceiveData(strReceiveData);
	} //while (!tag);

	if (strReceiveData.empty())
	{
		nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 1, "", ""));
		return;
	}

	Json::Value json;
	Json::Reader reader;
	bool res = reader.parse(strReceiveData, json);
	if (!res)
	{
		nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 1, "", ""));
		return;
	}

	int code = json["code"].asInt();
	if (0 != code) {
		std::string msg = json["msg"].asString();
		nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, "", ""));
		return;
	}
	Json::Value data = json["data"];
	std::string imagePath = data["image"].asString();
	std::string compressImage = data["imgCompPath"].asString();
	nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, imagePath, compressImage));
}

void MainThread::OnMsgBoxCallbackEx(MsgBoxRet ret, const std::string& dmp_path)
{
	if (ret == MB_YES)
	{
		//std::string directory;
		//shared::FilePathApartDirectory(dmp_path, directory);
		//QCommand::AppStartWidthCommand(nbase::UTF8ToUTF16(directory), L"");

		//上传dump文件
		/*nim_ui::UserManager::GetInstance()->InvokeUploadFile(directory, ToWeakCallback([this](int code, const std::string filePath, const std::string thumbnailPath) {
			if (code == 0)
			{
			}
		}));*/
		std::wstring wstrFilePath = nbase::UTF8ToUTF16(dmp_path);
		QLOG_APP(wstrFilePath);
		std::wstring wstrUrl = nbase::UTF8ToUTF16(app_sdk::AppSDKInterface::GetInstance()->GetAppHost() + "/api/other/uploadfile");
		std::wstring wstrFileKey = L"file";
		VecStParam VecExtInfo;
		auto cb = ToWeakCallback([this](int code, const std::string filePath, const std::string thumbnailPath) {
			if (code == 0)
			{
			}
		});
		nbase::ThreadManager::PostTask(kThreadGlobalMisc, nbase::Bind(UploadDumpFile, wstrUrl, VecExtInfo, wstrFilePath, wstrFileKey, cb));
	}
	nim_ui::WindowsManager::SingletonShow<LoginForm>(LoginForm::kClassName);
}

void MainThread::OnMsgBoxCallback(MsgBoxRet ret, const std::string& dmp_path)
{
	if (ret == MB_YES)
	{
		std::string directory;
		shared::FilePathApartDirectory(dmp_path, directory);
		QCommand::AppStartWidthCommand(nbase::UTF8ToUTF16(directory), L"");

	}
	nim_ui::WindowsManager::SingletonShow<LoginForm>(LoginForm::kClassName);
}

/**
* 全局函数，初始化云信。包括读取应用服务器地址，载入云信sdk，初始化安装目录和用户目录，注册收到推送时执行的回调函数。
* @return bool是否初始化成功
*/
bool NimAPP::InitNim(const std::string& server_conf_file_path)
{
	std::wstring server_conf_path = QPath::GetAppPath();
	server_conf_path.append(L"global_conf.txt");
	nim::SDKConfig config;
	TiXmlDocument document;
	if (shared::LoadXmlFromFile(document, server_conf_path))
	{
		TiXmlElement* root = document.RootElement();
		if (root)
		{
			if (auto pchar = root->Attribute("kNIMSDKLogLevel")) {
				int log_level = 5;
				nbase::StringToInt((std::string)pchar, &log_level);
				config.sdk_log_level_ = (nim::NIMSDKLogLevel)log_level;
			}
			if (auto pchar = root->Attribute("kNIMPreloadImageQuality")) {
				int quality = -1;
				nbase::StringToInt((std::string)pchar, &quality);
				config.preload_image_quality_ = quality;
			}
			if (auto pchar = root->Attribute("kNIMPreloadImageResize")) {
				config.preload_image_resize_ = (std::string)pchar;
			}
			if (auto pchar = root->Attribute("kNIMPreloadAttachImageNameTemplate")) {
				config.preload_image_name_template_ = (std::string)pchar;
			}
			if (auto pchar = root->Attribute("kNIMTeamNotificationUnreadCount")) {
				int need = -1;
				nbase::StringToInt((std::string)pchar, &need);
				config.team_notification_unread_count_ = need > 0;
			}
			if (auto pchar = root->Attribute("kNIMResetUnreadCountWhenRecall")) {
				int need = -1;
				nbase::StringToInt((std::string)pchar, &need);
				config.reset_unread_count_when_recall_ = need > 0;
			}
			if (auto pchar = root->Attribute("kNIMAnimatedImageThumbnailEnabled")) {
				int need = -1;
				nbase::StringToInt((std::string)pchar, &need);
				config.animated_image_thumbnail_enabled_ = need > 0;
			}
			if (auto pchar = root->Attribute("kNIMNosUseHttps")) {
				int need = 0;
				nbase::StringToInt((std::string)pchar, &need);
				config.use_https_ = (need != 0);
			}
			if (auto pchar = root->Attribute("kNIMServerConfFilePath")) {
				config.server_conf_file_path_ = (std::string)pchar;
			}
			if (auto pchar = root->Attribute("kNIMTeamMessageAckEnabled")) {
				int need = 0;
				nbase::StringToInt((std::string)pchar, &need);
				config.team_msg_ack_ = (need != 0);
			}
			if (auto pchar = root->Attribute("kNIMCachingMarkreadEnabled")) {
				int enable = 0;
				nbase::StringToInt((std::string)pchar, &enable);
				config.caching_markread_ = (enable != 0);
			}
			if (auto pchar = root->Attribute("kNIMCachingMarkreadTime")) {
				int time = 1000;
				nbase::StringToInt((std::string)pchar, &time);
				config.caching_markread_time_ = time;
			}
			if (auto pchar = root->Attribute("kNIMCachingMarkreadCount")) {
				int count = 10;
				nbase::StringToInt((std::string)pchar, &count);
				config.caching_markread_count_ = count;
			}
			if (auto pchar = root->Attribute("kNIMClientAntispam")) {
				int enable = 10;
				nbase::StringToInt((std::string)pchar, &enable);
				config.client_antispam_ = (enable != 0);
			}
			if (!server_conf_file_path.empty())
				config.server_conf_file_path_ = server_conf_file_path;
		}
	}

	//string（db key必填，目前只支持最多32个字符的加密密钥！建议使用32个字符）
#ifdef _DEBUG
	config.database_encrypt_key_ = "FeiGeIMNetease";
#else
	config.database_encrypt_key_ = "FeiGeIMNetease";
#endif

	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	std::wstring app_install;// = QPath::GetAppPath() + L"\\x64_dlls\\";
	bool ret = nim::Client::Init(app_key, "FeiGeIMNetease", nbase::UTF16ToUTF8(app_install), config); // 载入云信sdk，初始化安装目录和用户目录
	assert(ret);
#ifdef CPPWRAPPER_DLL
	nim::Client::SetCallbackFunction([](const StdClosure & task) {
		nbase::ThreadManager::PostTask(ThreadId::kThreadUI, task);
	});
#endif // #ifdef CPPWRAPPER_DLL

	//初始化聊天室
//	{
//		Json::Value extension;
//		nim_chatroom::ChatRoomPlatformConfig chatroom_platform_config;
//		chatroom_platform_config.AddNTServerAddress(config.ntserver_address_);
//		chatroom_platform_config.EnableUploadStatisticsData(config.upload_statistics_data_);
//		chatroom_platform_config.ToJsonObject(extension);
//		ret = nim_chatroom::ChatRoom::Init(nbase::UTF16ToUTF8(app_install), nim::GetJsonStringWithNoStyled(extension));
//#ifdef CPPWRAPPER_DLL
//		nim_chatroom::ChatRoom::SetCallbackFunction([](const StdClosure & task) {
//			nbase::ThreadManager::PostTask(ThreadId::kThreadUI, task);
//		});
//#endif//CPPWRAPPER_DLL		
//		assert(ret);
//	}
	// 初始化云信音视频
	ret = nim::VChat::Init(server_conf_file_path);
	assert(ret);
	
	if (ret)
	{
		nim_ui::RunTimeDataManager::GetInstance()->SetSDKInited();
		app_sdk::AppSDKConfig::GetInstance()->SetAppConfigPath(server_conf_file_path);
		//nim_chatroom::ChatroomCallback::InitChatroomCallback();
		//nim_chatroom::ChatRoom::SetMsgsBatchReport(true);
	}	
	else
	{
		CleanupSDKBeforLogin();
	}
	return ret;
}
void NimAPP::CleanupSDKBeforLogin()
{
	nim::VChat::Cleanup();
	//nim_chatroom::ChatRoom::Cleanup();
	nim::Client::Cleanup2();	
}
int NimAPP::InitInstance(HINSTANCE hInst, HINSTANCE hPrevInst, LPWSTR lpszCmdLine, int nCmdShow)
{
	

	// 初始化云信http
	nim_http::Init();
	g_need_restart_after_dump = true;
	return 0;
}
int NimAPP::ExitInstance()
{
	

	// 程序结束之前，清理云信sdk和UI组件
	nim_ui::InitManager::GetInstance()->CleanupUiKit();
	//if (nim_ui::RunTimeDataManager::GetInstance()->IsSDKInited())
	//	nim_chatroom::ChatRoom::Cleanup();
	
	return 0;
}
int NimAPP::InitEnvironment(HINSTANCE hInst, HINSTANCE hPrevInst, LPWSTR lpszCmdLine, int nCmdShow)
{
	// 把cef dll文件的位置添加到程序的"path"环境变量中,这样可以把dll文件放到bin以外的目录，并且不需要手动频繁切换dll文件，这行代码必须写到main的开头
	//nim_cef::CefManager::GetInstance()->AddCefDllToPath();
	_wsetlocale(LC_ALL, L"chs");
	srand((unsigned int)time(NULL));
	::SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);
	com_module_ = std::make_unique<CComModule>();
	com_module_->Init(NULL, hInst);	
	if (FAILED(::OleInitialize(NULL)))
		return 0;
	QCommand::ParseCommand(lpszCmdLine);
	//初始化cef
	//CefSettings settings;
	//if (!nim_cef::CefManager::GetInstance()->Initialize(QPath::GetNimAppDataDir(APPDATA_DIR), settings, atoi(GetConfigValue("kNIMCefOsrEnabled").c_str()) > 0))
	//	return 0;
	return 1;
}
int NimAPP::UninitEnvironment()
{
	com_module_->Term();
	com_module_.reset();
	::OleUninitialize();
	//清理cef
	//nim_cef::CefManager::GetInstance()->UnInitialize();	
	return 0;
}
int NimAPP::Exec(HINSTANCE hInst, HINSTANCE hPrevInst, LPWSTR lpszCmdLine, int nCmdShow)
{
	if (InitEnvironment(hInst, hPrevInst, lpszCmdLine, nCmdShow) == 0)
		return 0;
	nbase::AtExitManager at_manager;	
	InitInstance(hInst, hPrevInst, lpszCmdLine, nCmdShow);
	// 创建主线程
	QLOG_APP(L"begin ui loop");
	MainThread().RunOnCurrentThreadWithLoop(nbase::MessageLoop::kUIMessageLoop); // 执行主线程循环
	QLOG_APP(L"exit ui loop");	
	UninitEnvironment();
	ExitInstance();
	QLOG_APP(L"app exit");
	// 是否重新运行程序
	CheckRestartApp();
	return 0;
}
void NimAPP::CheckRestartApp()
{
	std::wstring restart = QCommand::Get(kCmdRestart);
	if (!restart.empty())
	{
		std::wstring cmd;
		std::wstring acc = QCommand::Get(kCmdAccount);
		if (!acc.empty())
			cmd.append(nbase::StringPrintf(L" /%s %s ", kCmdAccount.c_str(), acc.c_str()));
		std::wstring exit_why = QCommand::Get(kCmdExitWhy);
		if (!exit_why.empty())
			cmd.append(nbase::StringPrintf(L" /%s %s ", kCmdExitWhy.c_str(), exit_why.c_str()));
		QCommand::RestartApp(cmd);
	}
}
