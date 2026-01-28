#include "stdafx.h"
#include "app_sdk_interface.h"
#include "shared/xml_util.h"
#include "protocol/sdk_pro.h"
#include "base/http/sdk_http_manager.h"
#include "app_sdk_config_helper.h"
namespace app_sdk
{
	const std::string AppSDKInterface::kAppKey = "73d074339715c1339640f92f96148de1";//bc4bc49b41487bc1160a0fa9ec296c6b
	const std::string AppSDKInterface::kAppHost = "https://bnbx.me";//http://13.248.245.109:30001/index.php
	const std::string AppSDKInterface::kAppVersion = "2.1.2.9";

const std::map<std::string,std::tuple< std::string, NimServerConfType>> key_use_nim_server_conf = {
	{ "kAppKey", std::make_tuple("appkey", NimServerConfType::NimServerConfType_String) },
	{ "rts_record", std::make_tuple("kNIMRtsRecord", NimServerConfType::NimServerConfType_String) },
	{ "record_type", std::make_tuple("kNIMRecordType", NimServerConfType::NimServerConfType_String) },
	{ "multi_video", std::make_tuple("kNIMMultiVideo", NimServerConfType::NimServerConfType_String) },
	{ "video_scale", std::make_tuple("kNIMVideoScale", NimServerConfType::NimServerConfType_String) },
	{ "keep_calling", std::make_tuple("kNIMKeepCalling", NimServerConfType::NimServerConfType_String) },
	{ "audio_record", std::make_tuple("kNIMAudioRecord", NimServerConfType::NimServerConfType_String) },
	{ "password_use_md5", std::make_tuple("kNIMPasswordMD5", NimServerConfType::NimServerConfType_String) },
	{ "video_quality", std::make_tuple("kNIMVideoQuality", NimServerConfType::NimServerConfType_String) },
	{ "cef_osr_enabled", std::make_tuple("kNIMCefOsrEnabled", NimServerConfType::NimServerConfType_String) },
	{ "limit_file_size", std::make_tuple("kNIMLimitFileSize", NimServerConfType::NimServerConfType_String) },
	{ "check_singleton", std::make_tuple("kNIMCheckSingleton", NimServerConfType::NimServerConfType_String) },
	{ "chatroomDemoListUrl", std::make_tuple("kNIMChatRoomAddress", NimServerConfType::NimServerConfType_String) },
	{ "record_host_speaker", std::make_tuple("kNIMRecordHostSpeaker", NimServerConfType::NimServerConfType_String) }
};

std::string AppSDKInterface::GetConfigValue(const std::string& key)
{
	if (GetConfigFileVersion() >= 1 && key_use_nim_server_conf.find(key) != key_use_nim_server_conf.end())
	{
		auto it = key_use_nim_server_conf.find(key);
		return GetConfigStringValueFromNimServerConf(std::get<0>(it->second), std::get<1>(it->second));
	}			
	return GetConfigValue("ServerConf",key);
}	
std::string AppSDKInterface::GetConfigValue(const std::string& element_name, const std::string& key)
{
	std::string value;
	std::wstring server_conf_path = QPath::GetAppPath();
	server_conf_path.append(L"global_conf.txt");
	TiXmlDocument document;
	if (shared::LoadXmlFromFile(document, server_conf_path))
	{

		TiXmlElement* root = document.RootElement();
		TiXmlElement* element = nullptr;
		if (element_name.compare(root->Value()) == 0)
		{
			element = root;
		}
		else
		{
			element = root->FirstChildElement(element_name);
		}
		if (element != nullptr)
		{
			if (auto pchar = element->Attribute(key.c_str()))
			{
				value = pchar;
			}
		}
	}
	return value;
}
int AppSDKInterface::GetConfigFileVersion()
{
	int version = 0;
	std::string value;
	std::wstring server_conf_path = QPath::GetAppPath();
	server_conf_path.append(L"global_conf.txt");
	TiXmlDocument document;
	if (shared::LoadXmlFromFile(document, server_conf_path))
	{

		TiXmlElement* root = document.RootElement();		
		if (root != nullptr)
		{
			if (auto pchar = root->Attribute("kConfVersion"))
			{
				value = pchar;
			}
		}
	}
	if (!value.empty())
		nbase::StringToInt(value, &version);
	return version;
}
std::string AppSDKInterface::GetAppKey()
{
	const static std::string config_key_AppKey = "kAppKey";
	std::string app_key = kAppKey;
	std::string new_app_key = GetConfigValue(config_key_AppKey);
	if (!new_app_key.empty())
	{
		app_key = new_app_key;
	}
	return app_key;
}

bool AppSDKInterface::IsSafeUrl(const std::string& safe_url)
{
	std::string temp(safe_url);
	std::string params;
	std::vector<std::string> param_list;
	std::map<std::string, std::string> param_pair_list;
	auto param_pos = ((temp.find("?") == std::string::npos) ? (0) : (temp.find("?") + strlen("?")));
	params = temp.substr(param_pos, temp.length());
	nbase::LowerString(params);
	shared::tools::SplitString(params, "&", param_list);
	for (auto&it : param_list)
	{
		std::vector<std::string> param_pair;
		shared::tools::SplitString(it, "=", param_pair);
		if (param_pair.empty())
			continue;
		param_pair_list.insert(std::make_pair(*param_pair.begin(), *param_pair.rbegin()));
	}
	static const std::string safe_url_param_key = "_im_url";
	static const int safe_url_param_value = 0x00000001;
	if (param_pair_list.find(safe_url_param_key) != param_pair_list.end())
	{
		return (std::atoi(param_pair_list[safe_url_param_key].c_str()) & safe_url_param_value) == safe_url_param_value;
	}
	return false;
}

std::string AppSDKInterface::GetAppHost()
{
	return kAppHost;
}

std::string AppSDKInterface::GetAppVersion()
{
	return kAppVersion;
}


void AppSDKInterface::InvokeRegisterAccount(const std::string &username, const std::string &password, const std::string &nickname, const OnRegisterAccountCallback& cb)
{
	//�ڹ��캯���д����������
	auto&& req = app_sdk::CreateHttpRequest<app_sdk_pro::RegisterAccountReq>(username,password,nickname);

	SDKManager::GetInstance()->Invoke_Request<app_sdk_pro::RegisterAccountReq, app_sdk_pro::RegisterAccountRsp>(req, 
		ToWeakCallback([cb](const app_sdk_pro::RegisterAccountReq& req, const app_sdk_pro::RegisterAccountRsp& rsp){
		if (cb != nullptr)
		{
			cb((rsp->GetResponseCode() == nim::kNIMResSuccess ? rsp->GetProtocolReplyCode() : rsp->GetResponseCode()), rsp->err_msg_);
		}
	}));
}

}