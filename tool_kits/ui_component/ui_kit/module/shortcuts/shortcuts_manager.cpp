
#include "shortcuts_manager.h"
#include "gui/video/multi_video_form.h"
#include "gui/video/video_form.h"
#include "gui/setting/Shortcuts_setting.h"
#include "util/windows_manager.h"
#include "callback/vchat/vchat_callback.h"
#include "../rts/rts_manager.h"
#include "gui/contact_select_form/contact_select_form.h"
#include "export/nim_ui_window_manager.h"
#include "module/session/session_manager.h"
#include "gui/video/multi_video_invite.h"
#include "gui/video/multi_video_form.h"
//#include "../../../nim_win_demo/gui/main/main_form.h"
#include "../../guiex/main/main_form_ex.h"
#include "export/nim_ui_login_manager.h"
//#include <iostream>
//#include <fstream>
#include <fstream> 

namespace nim_comp
{

	ShortcutsManager::ShortcutsManager()
	{
		/*for (int i = nim::kNIMDeviceTypeAudioIn; i <= nim::kNIMDeviceTypeVideo; i++)
		{
			device_session_type_[i] = kDeviceSessionTypeNone;
		}
		//webrtc_setting_ = false;
		setting_audio_in_ = true;
		setting_audio_out_ = true;
		setting_video_in_ = true;*/
		//vchat_status_ = kMultiVChatEnd;
		setting_path_ = QPath::GetUserAppDataDir(nim_ui::LoginManager::GetInstance()->GetAccount());
		setting_path_ += L"shortcuts.json";

		if (!nbase::FilePathIsExist(setting_path_, false))
		{
			std::wstring theme_dir = QPath::GetAppPath();
			std::wstring app_setting_dir = theme_dir + L"resources\\themes\\default\\setting\\shortcuts.json";
			nbase::CopyFile(app_setting_dir, setting_path_);
		}
	}

	ShortcutsManager::~ShortcutsManager()
	{

	}

	bool ShortcutsManager::ShowShortcutsSetting(bool pageTag)
	{
		ShortcutsSettingForm *window = (ShortcutsSettingForm*)(WindowsManager::GetInstance()->GetWindow(ShortcutsSettingForm::kClassName, ShortcutsSettingForm::kClassName));
		if (window)
		{
			window->ShowWindow(true, true);
			window->ShowPage();
			SetForegroundWindow(window->GetHWND());
			return false;
		}
		else
		{
			window = new ShortcutsSettingForm();
			window->Create(NULL, L"", WS_OVERLAPPEDWINDOW, 0);
			window->CenterWindow();
			window->ShowWindow();
			window->ShowPage();
		}
		return true;
	}

	std::list<MEDIA_SHORTCUTS_EDIT_INFO> ShortcutsManager::GetEditInfo()
	{
		//读取配置文件
		//std::wstring theme_dir = QPath::GetAppPath();

		std::wstring path = setting_path_; //theme_dir + L"themes\\default\\setting\\shortcuts.json";
		std::string str = nbase::UTF16ToUTF8(path);
		ifstream infile;
		infile.open(str.data());   //将文件流对象与文件连接起来 
		if (!infile.is_open())
		{
			return edit_info_list_;
		}
		std::stringstream ss;
		ss << infile.rdbuf();
		std::string strJsonData = ss.str();
		infile.close();

		Json::Value root;
		Json::Reader reader;
		
		if (reader.parse(strJsonData, root))
		{
			MEDIA_SHORTCUTS_EDIT_INFO sendInfo;
			sendInfo.type = "send";
			sendInfo.value = root["send"].asString();
			edit_info_list_.push_back(sendInfo);

			MEDIA_SHORTCUTS_EDIT_INFO screenshotsInfo;
			screenshotsInfo.type = "Screenshots";
			screenshotsInfo.value = root["Screenshots"].asString();
			edit_info_list_.push_back(screenshotsInfo);

			MEDIA_SHORTCUTS_EDIT_INFO popMsgInfo;
			popMsgInfo.type = "PopMsg";
			popMsgInfo.value = root["PopMsg"].asString();
			edit_info_list_.push_back(popMsgInfo);
			/*Json::Value dataValue = root["data"];
			for (auto i = dataValue.begin(); i != dataValue.end(); i++)
			{
				Json::Value value = *i;
				MEDIA_SHORTCUTS_EDIT_INFO info;
				info.type = value["key"].asString();
				info.value = value["value"].asString();
				edit_info_list_.push_back(info);
			}*/
		}
		return edit_info_list_;
	}
	bool ShortcutsManager::GetSendShortcutsStatus()
	{
		//std::wstring theme_dir = QPath::GetAppPath();

		std::wstring path = setting_path_; //theme_dir + L"themes\\default\\setting\\shortcuts.json";
		std::string str = nbase::UTF16ToUTF8(path);
		ifstream infile;
		infile.open(str.data());   //将文件流对象与文件连接起来 
		if (!infile.is_open())
		{
			return false;
		}
		std::stringstream ss;
		ss << infile.rdbuf();
		std::string strJsonData = ss.str();
		infile.close();

		Json::Value root;
		Json::Reader reader;

		if (reader.parse(strJsonData, root))
		{
			if (root["send"].asString() == "Enter")
			{
				return false;
			}
			else
			{
				return true;
			}
		}
		return false;
	}


	void ShortcutsManager::SaveShortcutsSetting(std::string screenshots, std::string popMsg)
	{
		//std::wstring theme_dir = QPath::GetAppPath();
		std::wstring path = setting_path_; //theme_dir + L"themes\\default\\setting\\shortcuts.json";
		std::string str = nbase::UTF16ToUTF8(path);
		ifstream infile;
		infile.open(str.data());   //将文件流对象与文件连接起来 
		if (!infile.is_open())
		{
			return;
		}
		std::stringstream ss;
		ss << infile.rdbuf();
		std::string strJsonData = ss.str();
		infile.close();

		Json::Value root;
		Json::Reader reader;
		if (reader.parse(strJsonData, root))
		{
			root["Screenshots"] = screenshots;
			root["PopMsg"] = popMsg;
		}
		Json::FastWriter writer;
		std::string shortcuts_config = writer.write(root);

		ofstream out(str);
		if (out.is_open())
		{
			out << shortcuts_config;
			out.close();
		}
		ResShortcutsSetting(screenshots, popMsg);
	}

	void ShortcutsManager::GetShortcutsSetting(std::string &screenshots, std::string &popMsg)
	{
		//读取配置文件
		//std::wstring theme_dir = QPath::GetUserAppDataDir();

		std::wstring path = setting_path_;// theme_dir + L"themes\\default\\setting\\shortcuts.json";
		std::string str = nbase::UTF16ToUTF8(path);
		ifstream infile;
		infile.open(str.data());   //将文件流对象与文件连接起来 
		if (infile.is_open())
		{
			std::stringstream ss;
			ss << infile.rdbuf();
			std::string strJsonData = ss.str();
			infile.close();

			Json::Value root;
			Json::Reader reader;

			if (reader.parse(strJsonData, root))
			{
				screenshots = root["Screenshots"].asString();
				popMsg = root["PopMsg"].asString();
				//ResShortcutsSetting(screenshots, popMsg);
			}
			else
			{
				screenshots = "Ctrl+Alt+A";
				popMsg = "Ctrl+Alt+Z";
			}
		}
		else
		{
			screenshots = "Ctrl+Alt+A";
			popMsg = "Ctrl+Alt+Z";
		}
	}

	void ShortcutsManager::ResShortcutsSetting(std::string screenshots, std::string popMsg)
	{
		std::wstring mainClassName = L"MainFormEx";
		((MainFormEx *)(nim_ui::WindowsManager::GetInstance()->GetWindow(mainClassName, mainClassName)))->RegShortcuts();// OnlyRegShortcuts(screenshots, popMsg);
	}

	void ShortcutsManager::StartSendShortcuts(std::string key)
	{
		string tag = "Enter";
		if ("Enter" == key)
		{
			SessionManager::GetInstance()->SetSendButStatus(false);
		}
		else
		{
			SessionManager::GetInstance()->SetSendButStatus(true);
			tag = "Ctrl+Enter";
		}

		//std::wstring theme_dir = QPath::GetAppPath();
		std::wstring path = setting_path_; //theme_dir + L"themes\\default\\setting\\shortcuts.json";
		std::string str = nbase::UTF16ToUTF8(path);
		ifstream infile;
		infile.open(str.data());   //将文件流对象与文件连接起来 
		if (!infile.is_open())
		{
			return;
		}
		std::stringstream ss;
		ss << infile.rdbuf();
		std::string strJsonData = ss.str();
		infile.close();

		Json::Value root;
		Json::Reader reader;
		if (reader.parse(strJsonData, root))
		{
			root["send"] = tag;
		}
		Json::FastWriter writer;
		std::string shortcuts_config = writer.write(root);

		ofstream out(str);
		if (out.is_open())
		{
			out << shortcuts_config;
			out.close();
		}
	}

}