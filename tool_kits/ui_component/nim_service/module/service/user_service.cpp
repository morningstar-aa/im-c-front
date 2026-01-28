#include "user_service.h"
#include "module/local/local_helper.h"
#include "module/login/login_manager.h"
#include "module/subscribe_event/subscribe_event_manager.h"
#include "shared/xml_util.h"
#include "module/winhttp/HttpUploadFiles.h"

std::string GetConfigValue(const std::string& key)
{
	return app_sdk::AppSDKInterface::GetConfigValue(key);
}

std::string yl_api(const std::string& api) {
	return app_sdk::AppSDKInterface::GetInstance()->GetAppHost() + api;
}

namespace nim_comp
{

UserService::UserService()
{
	//向SDK注册监听好友列表变化
	nim::Friend::RegChangeCb(nbase::Bind(&UserService::OnFriendListChangeBySDK, this, std::placeholders::_1));

	//向SDK注册监听用户名片变化
	nim::User::RegUserNameCardChangedCb(nbase::Bind(&UserService::OnUserInfoChangeBySDK, this, std::placeholders::_1));
}


void UserService::InvokeGetRegisterCan(const std::string os, const OnCommonCallback& cb)
{
	auto reg_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		if (ret && response_code == 200) {
			Json::Value json;
			Json::Reader reader;
			bool res = reader.parse(reply, json);
			if (!res)
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 0, "未知错误"));
				return;
			}
			int code = json["code"].asInt();
			std::string err_msg = json["msg"].asString();
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, err_msg));
		}
		else {
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 0, nbase::UTF16ToUTF8(L"网络出现问题，请确认网络连接")));
		}
	};

	std::string body = "os=" + os;
	//cjy
	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(yl_api("/api/User/canregister"), body.c_str(), body.size(), reg_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

/**
* 获取客户端设置
*/
void UserService::InvokeGetClientSetting(const OnGetClientSettingCallback& cb)
{
	auto reg_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		std::map<std::string,std::string> clientInfo;
		if (ret && response_code == 200) {
			Json::Value json;
			Json::Reader reader;
			bool res = reader.parse(reply, json);
			if (!res)
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, -1, "", clientInfo));
				return;
			}

			int code = json["code"].asInt();
			std::string msg = json["msg"].asString();
			if (0 != code) {
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, msg, clientInfo));
				return;
			}

			Json::Value data = json["data"];
			clientInfo["iseveryman"] = data["iseveryman"].asString();
			clientInfo["everyonecount"] = data["everyonecount"].asString();
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, msg, clientInfo));
		}
		else {
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, -1, "", clientInfo));
		}
	};

	std::string body = "&token=" + userinfo_.token;
	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(yl_api("/api/User/clientsetting"), body.c_str(), body.size(), reg_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

void UserService::InvokeGetRegisterCode(const std::string userMobile, const OnGetRegisterCodeCallback& cb)
{
	auto reg_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		if (ret && response_code == 200) {
			Json::Value json;
			Json::Reader reader;
			bool res = reader.parse(reply, json);
			if (!res)
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 0, "未知错误"));
				return;
			}
			int code = json["code"].asInt();
			std::string err_msg = json["msg"].asString();
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, err_msg));
		}
		else {
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 0, nbase::UTF16ToUTF8(L"网络出现问题，请确认网络连接")));
		}
	};

	std::string body = "mobile=" + userMobile;

	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(yl_api("/api/Other/sendCode"), body.c_str(), body.size(), reg_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

void UserService::InvokeVersionUpdate(const std::string &os, const std::string &version, const OnUpdateCallback& cb)
{
	auto reg_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		VersionInfo info;
		if (ret && response_code == 200) {
			Json::Value json;
			Json::Reader reader;
			bool res = reader.parse(reply, json);
			if (!res)
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 0, "未知错误", info));
				return;
			}

			int code = json["code"].asInt();
			std::string err_msg = json["msg"].asString();
			if (0 != code)
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, err_msg, info));
				return;
			}

			Json::Value dataJson = json["data"];
			info.currVersion = dataJson["currVersion"].asString();
			info.url = dataJson["url"].asString();
			info.description = dataJson["description"].asString();
			info.updateType = dataJson["updateType"].asString();
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, err_msg, info));
		}
		else {
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 0, nbase::UTF16ToUTF8(L"网络出现问题，请确认网络连接"), info));
		}
	};

	std::string body;
	body += "os=" + os;
	body += "&version=" + version;

	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(yl_api("/api/Other/checkupdate"), body.c_str(), body.size(), reg_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

void UserService::InvokeRegisterAccountEx(const std::string &username, const std::string &password, const std::string &nickname, const std::string &invitecode, const OnRegisterAccountCallback& cb)
{
	auto reg_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		if (ret && response_code == 200) {
			Json::Value json;
			Json::Reader reader;
			bool res = reader.parse(reply, json);
			if (!res)
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 0, "未知错误"));
				return;
			}

			int code = json["code"].asInt();
			std::string err_msg = json["msg"].asString();
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, err_msg));
		}
		else if (response_code == 500)
		{
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 1, nbase::UTF16ToUTF8(L"服务器注册失败！")));
		}
		else {
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 1, nbase::UTF16ToUTF8(L"网络出现问题，请确认网络连接")));
		}
	};

	std::string body;
	body += "account=" + username;
	body += "&password=" + password;
	body += "&nickname=" + nickname;
	body += "&invitecode=" + invitecode;
	body += "&client=pc";
	//body += "&repassword=" + password;

	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(yl_api("/api/User/register"), body.c_str(), body.size(), reg_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

/**
* 是否有建群的权限
*/
void UserService::InvokeCheckCreateTeam(const std::string &userId, const OnCheckCreateTeamCallback& cb)
{
	auto query_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		if (ret && response_code == 200) {
			Json::Value json;
			Json::Reader reader;
			bool res = reader.parse(reply, json);
			if (!res)
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, -1, "系统异常"));
				return;
			}

			int code = json["code"].asInt();
			std::string msg = json["msg"].asString();
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, msg));
		}
		else {
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, -1, "网络异常"));
		}
	};

	std::string body;
	body += "uid=" + userId;
	body += "&token=" + userinfo_.token;

	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(yl_api("/api/User/checkcreateteam"), body.c_str(), body.size(), query_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

/**
* 是否有加好友的权限
*/
void UserService::InvokeCheckAddFriendAuth(const std::string &userId, const OnCheckAddFriendCallback& cb)
{
	auto query_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		if (ret && response_code == 200) {
			Json::Value json;
			Json::Reader reader;
			bool res = reader.parse(reply, json);
			if (!res)
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, -1, "系统异常"));
				return;
			}

			int code = json["code"].asInt();
			std::string msg = json["msg"].asString();
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, msg));
		}
		else {
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, -1, "网络异常"));
		}
	};

	std::string body;
	body += "uid=" + userId;
	body += "&token=" + userinfo_.token;

	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(yl_api("/api/User/checkaddfriend"), body.c_str(), body.size(), query_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

/**
* 查看能否加好友
* accid 需要添加的用户id
*/
void UserService::InvokeCheckAddFriend(const std::string &userId, const std::string &accid, const OnQueryAddFriendCallback& cb)
{
	auto query_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		if (ret && response_code == 200) {
			Json::Value json;
			Json::Reader reader;
			bool res = reader.parse(reply, json);
			if (!res)
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, -1, "系统异常"));
				return;
			}

			int code = json["code"].asInt();
			std::string msg = json["msg"].asString();
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, msg));
		}
		else {
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, -1, "网络异常"));
		}
	};

	std::string body;
	body += "fuid=" + userId;
	body += "&tuid=" + accid;
	body += "&token=" + userinfo_.token;
	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(yl_api("/api/User/checkAddFriend"), body.c_str(), body.size(), query_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

void UserService::InvokeSendWelcome(const std::string &uid, const OnCommonCallback& cb)
{
	auto query_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, -1, "finish"));
	};
	std::string body;
	body += "uid=" + uid;
	body += "&token=" + userinfo_.token;
	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(yl_api("/api/other/firstrun"), body.c_str(), body.size(), query_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

/**
* 登录
* @return void	无返回值
*/
void UserService::InvokeLoginAccount(const std::string &username, const std::string &password, const OnLoginCallback& cb) {
	auto log_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		UserInfo info;
		if (ret && response_code == 200) {
			Json::Value json;
			Json::Reader reader;
			bool res = reader.parse(reply, json);
			if (!res)
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, -1, "", info));
				return;
			}

			int code = json["code"].asInt();
			std::string msg = json["msg"].asString();
			if (0 != code) {
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, msg, info));
				return;
			}

			Json::Value data = json["data"];
			info.uid = data["id"].asString();
			info.token = data["token"].asString();
			info.nimtoken = data["nimtoken"].asString();
			info.ip = data["ip"].asString();
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, msg, info));
		}
		else {
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, -1, "", info));
		}
	};

	std::string body;
	body += "account=" + username;
	body += "&password=" + password;
	body += "&os=pc&v=" + app_sdk::AppSDKInterface::GetAppVersion();
	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(yl_api("/api/User/login"), body.c_str(), body.size(), log_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

void UserService::SetAccountToken(const UserInfo userinfo)
{
	userinfo_ = userinfo;
}

void UserService::QueryBookmarksInfo(const std::string userId, const std::string page, const OnQueryBookmarksInfoCallback& cb)
{
	auto UploadImg_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		std::list<BookmarksInfo> InfoList;
		BookmarksPageInfo pageInfo;
		if (ret && response_code == 200) {
			Json::Value json;
			Json::Reader reader;
			bool res = reader.parse(reply, json);
			if (!res)
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 1, "", pageInfo, InfoList));
				return;
			}

			int code = json["code"].asInt();
			if (0 != code) {
				std::string msg = json["msg"].asString();

				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, msg, pageInfo, InfoList));
				return;
			}
			Json::Value jsonData = json["data"];
			if (!jsonData.isArray())
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 1, "", pageInfo, InfoList));
				return;
			}
			int len = jsonData.size();
			for (int i = 0; i < len; i++)
			{
				BookmarksInfo info;
				info.id = jsonData[i]["id"].asInt64();
				info.uid = jsonData[i]["uid"].asString();
				info.type = jsonData[i]["type"].asInt();
				info.path = jsonData[i]["path"].asString();;
				info.imgCompPath = jsonData[i]["imgCompPath"].asString();;
				info.content = jsonData[i]["content"].asString();;
				info.senderId = jsonData[i]["senderId"].asString();;
				info.senderName = jsonData[i]["senderName"].asString();;
				info.time = jsonData[i]["time"].asString();
				InfoList.push_back(info);
			}

			Json::Value jsonPageInfo = json["pageInfo"];
			if (jsonPageInfo.isObject())
			{
				pageInfo.count = jsonPageInfo["count"].asInt();
				pageInfo.total = jsonPageInfo["total"].asInt();
				pageInfo.pageCount = jsonPageInfo["pageCount"].asInt();
				pageInfo.page = jsonPageInfo["page"].asInt();
				pageInfo.hasMore = jsonPageInfo["hasMore"].asBool();
			}
			
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, "", pageInfo, InfoList));
		}
		else {
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, response_code, "", pageInfo, InfoList));
		}
	};
	std::string body;
	body += "uid=" + userId;
	body += "&page=" + page;
	body += "&number=10";

	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(yl_api("/bookmarks/api/search"), body.c_str(), body.size(), UploadImg_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

void UserService::DeleteBookmarksInfo(const std::string userId, const std::string msgId, const OnDeleteImgPathCallback& cb)
{
	auto UploadImg_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		if (ret && response_code == 200) {
			Json::Value json;
			Json::Reader reader;
			bool res = reader.parse(reply, json);
			if (!res)
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 1, ""));
				return;
			}

			int code = json["code"].asInt();
			if (0 != code) {
				std::string msg = json["msg"].asString();

				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, msg));
				return;
			}
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, ""));
		}
		else {
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, response_code, ""));
		}
	};
	std::string body;
	body += "uid=" + userId;
	body += "&id=" + msgId;

	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(yl_api("/bookmarks/api/delete"), body.c_str(), body.size(), UploadImg_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

void UserService::QueryImgInfo(const std::string userId, const OnQueryImgInfoCallback& cb)
{
	auto UploadImg_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		std::list<CustomImgInfo> imgInfoList;
		if (ret && response_code == 200) {
			Json::Value json;
			Json::Reader reader;
			bool res = reader.parse(reply, json);
			if (!res)
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 1, imgInfoList));
				return;
			}

			int code = json["code"].asInt();
			if (0 != code) {
				std::string msg = json["msg"].asString();

				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, imgInfoList));
				return;
			}
			Json::Value jsonData = json["data"];
			if (!jsonData.isArray())
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 1, imgInfoList));
				return;
			}
			int len = jsonData.size();
			for (int i = 0; i < len; i++)
			{
				CustomImgInfo info;
				info.imgId = jsonData[i]["id"].asString();
				info.OriginalPath = jsonData[i]["imgpath"].asString();
				info.thumbnailPath = jsonData[i]["imgCompPath"].asString();
				imgInfoList.push_back(info);
			}
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, imgInfoList));
		}
		else {
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, response_code, imgInfoList));
		}
	};
	bool isCompress = true;
	std::string body;
	body += "uid=" + userId;
	body += "&token=" + userinfo_.token;

	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(yl_api("/api/Emoji/search"), body.c_str(), body.size(), UploadImg_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

void UserService::InvokeUploadImgPath(const std::string session_id, const std::string &imgPath, const std::string &imgCompPath, const OnUploadImgPathCallback& cb)
{
	auto UploadImg_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		UserInfo info;
		if (ret && response_code == 200) {
			Json::Value json;
			Json::Reader reader;
			bool res = reader.parse(reply, json);
			if (!res)
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 1, ""));
				return;
			}

			int code = json["code"].asInt();
			if (0 != code) {
				std::string msg = json["msg"].asString();

				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, ""));
				return;
			}

			std::string imgID = json["data"]["imgId"].asString();

			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, imgID));
		}
		else {
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, response_code, ""));
		}
	};
	bool isCompress = true;
	std::string body;
	body += "uid=" + session_id;
	body += "&imgpath=" + imgPath;
	body += "&imgCompPath=" + imgCompPath;
	body += "&token=" + userinfo_.token;

	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(yl_api("/api/Emoji/save"), body.c_str(), body.size(), UploadImg_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

void UserService::InvokeUploadBookmarks(const curBookmarksInfo &info, const OnUploadImgPathCallback& cb)
{
	auto UploadImg_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		UserInfo info;
		if (ret && response_code == 200) {
			Json::Value json;
			Json::Reader reader;
			bool res = reader.parse(reply, json);
			if (!res)
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 1, ""));
				return;
			}

			int code = json["code"].asInt();
			if (0 != code) {
				std::string msg = json["msg"].asString();

				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, ""));
				return;
			}

			std::string imgID = json["id"].asString();

			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, imgID));
		}
		else {
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, response_code, ""));
		}
	};
	bool isCompress = true;
	std::string body;
	body += "uid=" + info.uid;

	if (info.type == 0)
	{
		body += "&type=0";
		body += "&content=" + info.content;
		body += "&senderId=" + info.senderId;
		body += "&senderName=" + info.senderName;
	}
	else if (info.type == 1)
	{
		body += "&type=1";
		body += "&path=" + info.path;
		body += "&imgCompPath=" + info.imgCompPath;
		body += "&senderId=" + info.senderId;
		body += "&senderName=" + info.senderName;
	}
	else if (info.type == 6)
	{
		/*body += "&path=" + info.path;
		body += "&imgCompPath=" + info.imgCompPath;
		body += "&senderId=" + info.senderId;
		body += "&senderName=" + info.senderName;*/
	}

	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(yl_api("/bookmarks/api/save"), body.c_str(), body.size(), UploadImg_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}
//cjy2
void UserService::InvokeDeleteImgPath(const std::string userId, const std::string &imgId, const OnDeleteImgPathCallback& cb)
{
	auto UploadImg_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		UserInfo info;
		if (ret && response_code == 200) {
			Json::Value json;
			Json::Reader reader;
			bool res = reader.parse(reply, json);
			if (!res)
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 1, ""));
				return;
			}

			int code = json["code"].asInt();
			/*if (0 != code) {
				std::string msg = json["msg"].asString();

				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, ""));
				return;
			}*/
			std::string msg = json["msg"].asString();

			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 0, "收藏成功"));
		}
		else {
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, response_code, ""));
		}
	};
	bool isCompress = true;
	std::string body;
	body += "uid=" + userId;
	body += "&imgID=" + imgId;
	body += "&token=" + userinfo_.token;

	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(yl_api("/api/Emoji/delete"), body.c_str(), body.size(), UploadImg_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

void UploadImg(const std::wstring wstrUrl, VecStParam& VecExtInfo,
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

void UploadFile(const std::wstring wstrUrl, VecStParam& VecExtInfo,
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
	std::string filePath = data["filePath"].asString();
	//std::string compressImage = data["imgCompPath"].asString();
	nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, filePath, ""));
}

void UserService::InvokeUploadFile(const std::string &imgContent, const OnUploadImgCallback& cb)
{
	std::wstring wstrFilePath = nbase::UTF8ToUTF16(imgContent);
	std::wstring wstrUrl = nbase::UTF8ToUTF16(yl_api("/other/api/uploadfile"));
	std::wstring wstrFileKey = L"file";
	VecStParam VecExtInfo;
	nbase::ThreadManager::PostTask(kThreadGlobalMisc, nbase::Bind(UploadFile, wstrUrl, VecExtInfo, wstrFilePath, wstrFileKey, cb));
}

void UserService::InvokeUploadImg(const std::string &imgContent, const OnUploadImgCallback& cb)
{
	std::wstring wstrFilePath = nbase::UTF8ToUTF16(imgContent);
	std::wstring wstrUrl = nbase::UTF8ToUTF16(yl_api("/api/other/uploadimage?isCompress=true&token=" + userinfo_.token));
	std::wstring wstrFileKey = L"image";
	VecStParam VecExtInfo;

	nbase::ThreadManager::PostTask(kThreadGlobalMisc, nbase::Bind(UploadImg, wstrUrl, VecExtInfo, wstrFilePath, wstrFileKey, cb));
}

void UserService::InvokeUpdateMyInfoToBaiCai(const nim::UserNameCard &new_info)//, const OnUpdateMyInfo& cb)
{
	auto update_cb = [this](bool ret, int response_code, const std::string& reply)
	{
	};

	std::string body;
	body += "uid=" + new_info.GetAccId();
	body += "&name=" + new_info.GetName();
	body += "&icon=" + new_info.GetIconUrl();
	body += "&sign=" + new_info.GetSignature();
	body += "&email=" + new_info.GetEmail();
	body += "&birth=" + new_info.GetBirth();
	body += "&mobile=" + new_info.GetMobile();
	body += "&gender=" + nbase::IntToString(new_info.GetGender());
	body += "&token=" + userinfo_.token;

	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(yl_api("/api/User/edit"), body.c_str(), body.size(), update_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

void UserService::InvokeSignIn(const std::string userId, const std::string teamId, const OnCommonCallback& cb)
{
	auto singIn_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		if (ret && response_code == 200) {
			Json::Value json;
			Json::Reader reader;
			bool res = reader.parse(reply, json);
			if (!res)
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 1, ""));
				return;
			}

			int code = json["code"].asInt();
			if (0 != code) {
				std::string msg = json["msg"].asString();

				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, ""));
				return;
			}
			std::string msg = json["msg"].asString();

			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, msg));
		}
		else {
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, response_code, ""));
		}
	};

	std::string body;
	body += "user_id=" + userId;
	body += "&group_id=" + teamId;
	body += "&token=" + userinfo_.token;

	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(yl_api("/sign/api/sign"), body.c_str(), body.size(), singIn_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

void UserService::InvokeIsSignIn(const std::string userId, const std::string teamId, const OnIsSignInCallback& cb)
{
	auto singIn_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		if (ret && response_code == 200) {
			Json::Value json;
			Json::Reader reader;
			bool res = reader.parse(reply, json);
			if (!res)
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 1, "", false));
				return;
			}

			int code = json["code"].asInt();
			if (0 != code) {
				std::string msg = json["msg"].asString();

				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, "", false));
				return;
			}
			std::string msg = json["msg"].asString();
			Json::Value dataJson = json["data"];
			bool issign = dataJson["issign"].asBool();
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, msg, issign));
		}
		else {
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, response_code, "", false));
		}
	};

	std::string body;
	body += "user_id=" + userId;
	body += "&group_id=" + teamId;

	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(yl_api("/sign/api/issign"), body.c_str(), body.size(), singIn_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

void UserService::QueryTeamSignIn(const std::string teamId, const std::string startTime, const std::string endTime, const OnSearchSignCallback& cb)
{
	auto singIn_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		std::list<SignInfo> infoList;
		if (ret && response_code == 200) {
			Json::Value json;
			Json::Reader reader;
			bool res = reader.parse(reply, json);
			if (!res)
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 1, "", infoList));
				return;
			}

			int code = json["code"].asInt();
			if (0 != code) {
				std::string msg = json["msg"].asString();

				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, "", infoList));
				return;
			}
			std::string msg = json["msg"].asString();
			Json::Value dataJson = json["data"];
			int len = dataJson.size();
			for (size_t i = 0; i < len; i++)
			{
				SignInfo info;
				info.signDate = dataJson[i]["tm"].asString();
				Json::Value idListJson = dataJson[i]["list"];
				int len = idListJson.size();
				for (size_t j = 0; j < len; j++)
				{
					std::string id = idListJson[j]["id"].asString();
					std::string account = "";
					if (!idListJson[j]["account"].isNull())
					{
						account = idListJson[j]["account"].asString();
					}

					info.idMap.insert(std::map<std::string, std::string>::value_type(id, account));
				}

				infoList.push_back(info);
			}
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, msg, infoList));
		}
		else {
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, response_code, "", infoList));
		}
	};

	std::string body;
	body += "group_id=" + teamId;
	body += "&start_tm=" + startTime;
	body += "&end_tm=" + endTime;

	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(yl_api("/sign/api/lists"), body.c_str(), body.size(), singIn_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

void UserService::QueryMemberInfoCallback(bool ret, int response_code, const std::string& reply, const OnQueryMemberCallback& cb)
{
	if (ret && response_code == 200) {
		Json::Value json;
		Json::Reader reader;
		bool res = reader.parse(reply, json);
		if (!res)
		{
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 1, "", "", "", "", ""));
			return;
		}

		int code = json["code"].asInt();
		if (0 != code) {
			std::string msg = json["msg"].asString();

			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, "", "", "", "", ""));
			return;
		}
		std::string msg = json["msg"].asString();
		Json::Value dataJson = json["data"];
		std::string uid = dataJson["uid"].asString();
		std::string nickname = dataJson["nickname"].asString();
		std::string avatar = dataJson["avatar"].asString();
		std::string username = "";

		if (!dataJson["username"].isNull()) {
			username = dataJson["username"].asString();
		}
		nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, msg, uid, nickname, avatar, username));
	}
	else {
		nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, response_code, "", "", "", "", ""));
	}
}

void UserService::QueryMemberInfoByAcc(const std::string account, const OnQueryMemberCallback& cb)
{
	auto singIn_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		QueryMemberInfoCallback(ret, response_code, reply, cb);
	};

	std::string body;
	body += "account=" + account;
	body += "&token=" + userinfo_.token;

	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(yl_api("/api/User/search"), body.c_str(), body.size(), singIn_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

void UserService::QueryMemberInfo(const std::string account, const OnQueryMemberCallback& cb)
{
	auto singIn_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		QueryMemberInfoCallback(ret, response_code, reply, cb);
	};

	std::string body;
	body += "userid=" + account;
	body += "&token=" + userinfo_.token;

	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(yl_api("/api/User/search_userid"), body.c_str(), body.size(), singIn_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

void UserService::QueryOtherUserInfoCallback(bool ret, int response_code, const std::string& reply, const OnQueryUserInfoCallback& cb)
{
	UserBaseInfo userInfo;
	if (ret && response_code == 200) {
		Json::Value json;
		Json::Reader reader;
		bool res = reader.parse(reply, json);
		if (!res)
		{
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 1, "", userInfo));
			return;
		}

		int code = json["code"].asInt();
		if (0 != code) {
			std::string msg = json["msg"].asString();

			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, "", userInfo));
			return;
		}
		std::string msg = json["msg"].asString();
		Json::Value dataJson = json["data"];
		userInfo.address = dataJson["info"].asString();
		userInfo.ip = dataJson["ip"].asString();
		nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, msg, userInfo));
	}
	else {
		nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, response_code, "", userInfo));
	}
}

void UserService::QueryOtherUserInfo(const std::string uId, const OnQueryUserInfoCallback& cb)
{
	auto singIn_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		UserService::QueryOtherUserInfoCallback(ret, response_code, reply, cb);
	};
	std::string body;
	body += "user_id=" + uId;

	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(yl_api("/api/index/get_user_ip_by_id?")+ body.c_str(),nullptr , 0, singIn_cb);
	//request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	//request.AddHeader("charset", "utf-8");
	//request.AddHeader("appkey", app_key);
	//request.AddHeader("User-Agent", "nim_demo_pc");
	//request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

void UserService::QueryCurMemberInfo(const std::string uId, const OnQueryUserInfoCallback& cb)
{
	auto singIn_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		UserBaseInfo userInfo;
		if (ret && response_code == 200) {
			Json::Value json;
			Json::Reader reader;
			bool res = reader.parse(reply, json);
			if (!res)
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 1, "", userInfo));
				return;
			}

			int code = json["code"].asInt();
			if (0 != code) {
				std::string msg = json["msg"].asString();

				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, "", userInfo));
				return;
			}
			std::string msg = json["msg"].asString();
			Json::Value dataJson = json["data"];
			userInfo.account = dataJson["account"].asString();
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, msg, userInfo));
		}
		else {
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, response_code, "", userInfo));
		}
	};

	std::string body;
	body += "uid=" + uId;
	body += "&token=" + userinfo_.token;

	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(yl_api("/api/User/detail"), body.c_str(), body.size(), singIn_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

void UserService::InvokeCreateTeam(const std::string tId, const std::string tName, const std::string uId, const nim::NIMTeamMemberAddFriendMode addFriend, const OnCommonCallback& cb)
{
	auto singIn_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		if (ret && response_code == 200) {
			Json::Value json;
			Json::Reader reader;
			bool res = reader.parse(reply, json);
			if (!res)
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 1, ""));
				return;
			}

			int code = json["code"].asInt();
			if (0 != code) {
				std::string msg = json["msg"].asString();

				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, msg));
				return;
			}
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, ""));
		}
		else {
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, response_code, ""));
		}
	};

	std::string body;
	body += "id=" + tId;
	body += "&name=" + tName;
	body += "&addfriend=" + std::to_string(addFriend);
	//body += "&uid=" + uId;
	body += "&token=" + userinfo_.token;

	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(yl_api("/api/Team/create"), body.c_str(), body.size(), singIn_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

void UserService::InvokeUpdateTeamInfo(const std::string tId, const std::string tName, const std::string uId, const nim::NIMTeamMemberAddFriendMode addFriend, const OnCommonCallback& cb)
{
	auto singIn_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		if (ret && response_code == 200) {
			Json::Value json;
			Json::Reader reader;
			bool res = reader.parse(reply, json);
			if (!res)
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 1, ""));
				return;
			}

			int code = json["code"].asInt();
			if (0 != code) {
				std::string msg = json["msg"].asString();

				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, msg));
				return;
			}
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, ""));
		}
		else {
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, response_code, ""));
		}
	};

	std::string body;
	body += "id=" + tId;
	body += "&name=" + tName;
	//body += "&uid=" + uId;
	body += "&addfriend=" + std::to_string(addFriend);
	body += "&token=" + userinfo_.token;

	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(yl_api("/api/Team/modifyname"), body.c_str(), body.size(), singIn_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

void UserService::InvokeDissmissTeam(const std::string tId, const OnCommonCallback& cb)
{
	auto singIn_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		if (ret && response_code == 200) {
			Json::Value json;
			Json::Reader reader;
			bool res = reader.parse(reply, json);
			if (!res)
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 1, ""));
				return;
			}

			int code = json["code"].asInt();
			if (0 != code) {
				std::string msg = json["msg"].asString();

				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, msg));
				return;
			}
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, ""));
		}
		else {
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, response_code, ""));
		}
	};

	std::string body;
	body += "id=" + tId;
	body += "&token=" + userinfo_.token;

	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(yl_api("/api/Team/destory"), body.c_str(), body.size(), singIn_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

void UserService::InvokeGetMemberAddFriend(const std::string tId, const OnGetTeamMemberAddfriendCallback& cb)
{
	auto singIn_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		if (ret && response_code == 200) {
			Json::Value json;
			Json::Reader reader;
			bool res = reader.parse(reply, json);
			if (!res)
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 1, "", ""));
				return;
			}

			int code = json["code"].asInt();
			if (0 != code) {
				std::string msg = json["msg"].asString();

				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, msg, ""));
				return;
			}
			Json::Value dataJson = json["data"];
			//std::string canAddFriend = dataJson["canAddFriend"].asString();
			std::string canAddFriend = dataJson["tprivate"].asString();
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, "",canAddFriend));
		}
		else {
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, response_code, "", ""));
		}
	};

	std::string body;
	body += "id=" + tId;
	body += "&token=" + userinfo_.token;

	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	//nim_http::HttpRequest request(yl_api("/api/Team/getmemberauthority"), body.c_str(), body.size(), singIn_cb);
	nim_http::HttpRequest request(yl_api("/api/Team/getTeamSetting"), body.c_str(), body.size(), singIn_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

void UserService::InvokeTransferTeamOwner(const std::string tId, const std::string uId, const OnCommonCallback& cb)
{
	auto singIn_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		if (ret && response_code == 200) {
			Json::Value json;
			Json::Reader reader;
			bool res = reader.parse(reply, json);
			if (!res)
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 1, ""));
				return;
			}

			int code = json["code"].asInt();
			if (0 != code) {
				std::string msg = json["msg"].asString();

				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, msg));
				return;
			}
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, ""));
		}
		else {
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, response_code, ""));
		}
	};

	std::string body;
	body += "id=" + tId;
	body += "&new_owner_id=" + uId;
	body += "&token=" + userinfo_.token;

	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(yl_api("/api/Team/modifyowner"), body.c_str(), body.size(), singIn_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

void UserService::InvokeAddFriend(const std::string uId, const std::string accId, const OnCommonCallback& cb)
{
	auto singIn_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		if (ret && response_code == 200) {
			Json::Value json;
			Json::Reader reader;
			bool res = reader.parse(reply, json);
			if (!res)
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 1, ""));
				return;
			}

			int code = json["code"].asInt();
			if (0 != code) {
				std::string msg = json["msg"].asString();

				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, msg));
				return;
			}
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, ""));
		}
		else {
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, response_code, ""));
		}
	};

	std::string body;
	body += "uid=" + uId;
	body += "&fuid=" + accId;
	body += "&token=" + userinfo_.token;

	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(yl_api("/api/User/addFriend"), body.c_str(), body.size(), singIn_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

void UserService::InvokeDeleteFriend(const std::string uId, const std::string accId, const OnCommonCallback& cb)
{
	auto singIn_cb = [cb](bool ret, int response_code, const std::string& reply)
	{
		if (ret && response_code == 200) {
			Json::Value json;
			Json::Reader reader;
			bool res = reader.parse(reply, json);
			if (!res)
			{
				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, 1, ""));
				return;
			}

			int code = json["code"].asInt();
			if (0 != code) {
				std::string msg = json["msg"].asString();

				nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, msg));
				return;
			}
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, code, ""));
		}
		else {
			nbase::ThreadManager::PostTask(kThreadUI, nbase::Bind(cb, response_code, ""));
		}
	};

	std::string body;
	body += "uid=" + uId;
	body += "&fuid=" + accId;
	body += "&token=" + userinfo_.token;

	std::string app_key = app_sdk::AppSDKInterface::GetAppKey();
	nim_http::HttpRequest request(yl_api("/api/User/deleteFriend"), body.c_str(), body.size(), singIn_cb);
	request.AddHeader("Content-Type", "application/x-www-form-urlencoded");
	request.AddHeader("charset", "utf-8");
	request.AddHeader("appkey", app_key);
	request.AddHeader("User-Agent", "nim_demo_pc");
	request.SetMethodAsPost();
	nim_http::PostRequest(request);
}

void UserService::InvokeGetAllUserInfo(const OnGetUserInfoCallback& cb)
{
	nim::Friend::GetList(ToWeakCallback([this, cb](nim::NIMResCode res_code, const std::list<nim::FriendProfile>& user_profile_list)
	{
		std::list<std::string> account_list;
		for (auto& it : user_profile_list)
		{
			if (it.GetRelationship() == nim::kNIMFriendFlagNormal)
				friend_list_[it.GetAccId()] = it; //插入friend_list_（类的成员变量）好友列表

			account_list.push_back(it.GetAccId());
		}

		std::list<nim::UserNameCard> uinfos;
		GetUserInfos(account_list, uinfos); // 从db和服务器查询用户信息
		if (cb)
			cb(uinfos);
	}));
}

void UserService::InvokeGetAllUserInfo(const std::list<std::string>& accids, const OnGetUserInfoCallback& cb)
{
	nim::User::GetUserNameCardOnline(accids, [this, cb](const std::list<nim::UserNameCard>& uinfos)
	{
		std::list<std::string> accids;
		for (auto& it : uinfos)
		{
			nim::FriendProfile pro;
			pro.SetAccId(it.GetAccId());
			pro.SetRelationship(nim::kNIMFriendFlagNormal);
			friend_list_[it.GetAccId()].Update(pro);
			accids.push_back(it.GetAccId());
		}
		if (cb)
			cb(uinfos);
	});
}

void UserService::InvokeUpdateMyInfo(const nim::UserNameCard &new_info, const OnUpdateUserInfoCallback& cb)
{
	nim::UserNameCard info = new_info;

	auto update_uinfo_cb = ToWeakCallback([this, info, cb](nim::NIMResCode res) {
		if (res == nim::kNIMResSuccess)
		{
			assert(nbase::MessageLoop::current()->ToUIMessageLoop());
			std::list<nim::UserNameCard> lst;
			lst.push_back(info);
			OnUserInfoChangeBySDK(lst);
		}
		if (cb != nullptr)
			cb(res);
	});

	InvokeUpdateMyInfoToBaiCai(new_info);

	nim::User::UpdateMyUserNameCard(info, update_uinfo_cb);
}

void UserService::InvokeUpdateMyIp(const OnUpdateUserInfoCallback& cb)
{
	nim::UserNameCard new_info;
	GetUserInfo(userinfo_.uid, new_info);
	new_info.SetExpand(Json::Value(userinfo_.ip));
	InvokeUpdateMyInfo(new_info, cb);
}

void UserService::InvokeUpdateMyPhoto(const std::string &url, const OnUpdateUserInfoCallback& cb)
{
	nim::UserNameCard my_info;
	my_info.SetAccId(LoginManager::GetInstance()->GetAccount());
	my_info.SetIconUrl(url);
	InvokeUpdateMyInfo(my_info, cb);
}

const std::map<std::string, nim::UserNameCard>& UserService::GetAllUserInfos()
{
	assert(nbase::MessageLoop::current()->ToUIMessageLoop());
	return all_user_;
}

const std::map<std::string, nim::FriendProfile>& UserService::GetAllFriendInfos()
{
	assert(nbase::MessageLoop::current()->ToUIMessageLoop());
	return friend_list_;
}

bool UserService::GetUserInfo(const std::string &id, nim::UserNameCard &info)
{
	auto iter = all_user_.find(id);
	if (iter != all_user_.cend())
	{
		info = iter->second;
		return true;
	}
	else
	{
		//info.SetName(id);
		info.SetAccId(id);
		if(on_query_list_.find(id) == on_query_list_.cend())
			InvokeGetUserInfo(std::list<std::string>(1, id));
		return false;
	}
}

bool UserService::IsRobotAccount(const std::string &accid)
{
	return robot_list_.find(accid) != robot_list_.end();
}

void UserService::GetUserInfos(const std::list<std::string>& ids, std::list<nim::UserNameCard>& uinfos)
{
	uinfos.clear();
	std::list<std::string> not_get_list;

	for (const auto &id : ids)
	{
		auto iter = all_user_.find(id);
		if (iter != all_user_.cend())
			uinfos.push_back(iter->second);
		else
		{
			nim::UserNameCard info(id);
			//info.SetName(id);
			uinfos.push_back(info);

			if(on_query_list_.find(id) == on_query_list_.end()) //不在all_user_里面，也不在查询途中
				not_get_list.push_back(id);
		}
	}

	if (!not_get_list.empty())
		InvokeGetUserInfo(not_get_list);
}

void UserService::DoQueryUserInfos(const std::list<std::string>& ids)
{
	if (ids.empty())
		return;
	
	std::list<std::string> not_get_list;

	for (const auto &id : ids)
	{
		if (id.empty())
			continue;

		if (all_user_.find(id) == all_user_.end())
		{
			if (on_query_list_.find(id) == on_query_list_.end()) //不在all_user_里面，也不在查询途中
				not_get_list.push_back(id);
		}
	}

	if (!not_get_list.empty())
		InvokeGetUserInfo(not_get_list);
}

void UserService::DoQueryUserInfos(const std::set<std::string>& ids)
{
	if (ids.empty())
		return;

	std::list<std::string> not_get_list;

	for (const auto &id : ids)
	{
		if (id.empty())
			continue;

		if (all_user_.find(id) == all_user_.end())
		{
			if (on_query_list_.find(id) == on_query_list_.end()) //不在all_user_里面，也不在查询途中
				not_get_list.push_back(id);
		}
	}

	if (!not_get_list.empty())
		InvokeGetUserInfo(not_get_list);
}

nim::NIMFriendFlag UserService::GetUserType(const std::string &id)
{
	assert(nbase::MessageLoop::current()->ToUIMessageLoop());
	return (friend_list_.find(id) != friend_list_.end() ? nim::kNIMFriendFlagNormal : nim::kNIMFriendFlagNotFriend);
}

std::wstring UserService::GetUserName(const std::string &id, bool alias_prior/* = true */)
{
	if(alias_prior && GetUserType(id) == nim::kNIMFriendFlagNormal && !friend_list_.at(id).GetAlias().empty()) //优先使用备注名
		return nbase::UTF8ToUTF16(friend_list_.at(id).GetAlias());

	nim::UserNameCard info;
	GetUserInfo(id, info);
	return nbase::UTF8ToUTF16(info.GetName());
}

Json::Value UserService::GetUserCustom(const std::string &id)
{
	nim::UserNameCard info;
	GetUserInfo(id, info);
	return info.GetExpand();
}

std::wstring UserService::GetFriendAlias(const std::string & id)
{
	auto iter = friend_list_.find(id);
	if (iter == friend_list_.cend())
		return L"";
	return nbase::UTF8ToUTF16(iter->second.GetAlias());
}

UnregisterCallback UserService::RegFriendListChange(const OnFriendListChangeCallback& callback)
{
	OnFriendListChangeCallback* new_callback = new OnFriendListChangeCallback(callback);
	int cb_id = (int)new_callback;
	assert(nbase::MessageLoop::current()->ToUIMessageLoop());
	friend_list_change_cb_list_[cb_id].reset(new_callback);
	auto cb = ToWeakCallback([this, cb_id]() {
		friend_list_change_cb_list_.erase(cb_id);
	});
	return cb;
}

UnregisterCallback UserService::RegUserInfoChange(const OnUserInfoChangeCallback& callback)
{
	OnUserInfoChangeCallback* new_callback = new OnUserInfoChangeCallback(callback);
	int cb_id = (int)new_callback;
	assert(nbase::MessageLoop::current()->ToUIMessageLoop());
	uinfo_change_cb_list_[cb_id].reset(new_callback);
	auto cb = ToWeakCallback([this, cb_id]() {
		uinfo_change_cb_list_.erase(cb_id);
	});
	return cb;
}

UnregisterCallback nim_comp::UserService::RegMiscUInfoChange(const OnUserInfoChangeCallback & callback)
{
	OnUserInfoChangeCallback* new_callback = new OnUserInfoChangeCallback(callback);
	int cb_id = (int)new_callback;
	assert(nbase::MessageLoop::current()->ToUIMessageLoop());
	misc_uinfo_change_cb_list_[cb_id].reset(new_callback);
	auto cb = ToWeakCallback([this, cb_id]() {
		misc_uinfo_change_cb_list_.erase(cb_id);
	});
	return cb;
}

void UserService::OnFriendListChangeBySDK(const nim::FriendChangeEvent& change_event)
{
	std::list<std::string> add_list;
	std::list<std::string> delete_list;
	std::list<std::string> update_list; // 需要更换备注名的用户列表

	switch (change_event.type_)
	{
	case nim::kNIMFriendChangeTypeDel:
	{
		nim::FriendDelEvent del_event;
		nim::Friend::ParseFriendDelEvent(change_event, del_event);
		delete_list.push_back(del_event.accid_);
		update_list.push_back(del_event.accid_); // 删除好友之后，其原来的备注名改为其昵称
		friend_list_.erase(del_event.accid_); // 从friend_list_删除
		break;
	}
	case nim::kNIMFriendChangeTypeRequest:
	{
		nim::FriendAddEvent add_event;
		nim::Friend::ParseFriendAddEvent(change_event, add_event);
		if (add_event.add_type_ == nim::kNIMVerifyTypeAdd || add_event.add_type_ == nim::kNIMVerifyTypeAgree)
		{
			// 此处根据accid获取该好友的FriendProfile，添加到friend_list_中。
			nim::Friend::GetFriendProfileCallback cb = ToWeakCallback([this](const std::string& accid, const nim::FriendProfile& user_profile)
			{
				if (accid.empty() || user_profile.GetAccId().empty())
				{
					QLOG_ERR(L"UserService::OnFriendListChangeBySDK kNIMFriendChangeTypeRequest error, accid:{0}, profile_accid:{1}") << accid.c_str() << user_profile.GetAccId().c_str();
					return;
				}
					
				friend_list_[user_profile.GetAccId()] = user_profile;
				SubscribeEventManager::GetInstance()->SubscribeFriendEvent(user_profile.GetAccId());
				InvokeFriendListChangeCallback(kChangeTypeAdd, user_profile.GetAccId());
			});
			nim::Friend::GetFriendProfile(add_event.accid_, cb);
		}
		break;
	}
	case nim::kNIMFriendChangeTypeSyncList:
	{
		// 单独处理好友同步消息
		OnSyncFriendList(change_event);
		return;
	}
	case nim::kNIMFriendChangeTypeUpdate:
	{
		nim::FriendProfileUpdateEvent update_event;
		nim::Friend::ParseFriendProfileUpdateEvent(change_event, update_event);

		std::string accid = update_event.profile_.GetAccId();
		if (friend_list_.find(accid) != friend_list_.end())
		{
			update_list.push_back(accid);
			friend_list_.at(accid).Update(update_event.profile_);
		}		
		break;
	}
	default:
		break;
	}

	SubscribeEventManager::GetInstance()->SubscribeFriendEvent(add_list);
	SubscribeEventManager::GetInstance()->UnSubscribeFriendEvent(delete_list);

	for each (const auto& id in add_list)
		InvokeFriendListChangeCallback(kChangeTypeAdd, id);

	for each (const auto& id in delete_list)
		InvokeFriendListChangeCallback(kChangeTypeDelete, id);

	if (!update_list.empty())
	{
		std::list<nim::UserNameCard> uinfos;
		GetUserInfos(update_list, uinfos);
		for (auto& it : uinfo_change_cb_list_) //通知上层修改用户的备注名
			(*(it.second))(uinfos);
	}
}

void UserService::OnSyncFriendList(const nim::FriendChangeEvent& change_event)
{
	std::list<std::string> add_list;
	std::list<std::string> delete_list;
	std::list<std::string> update_list; // 需要更换备注名的用户列表

	nim::FriendProfileSyncEvent sync_event;
	nim::Friend::ParseFriendProfileSyncEvent(change_event, sync_event);
	for (auto& info : sync_event.profiles_)
	{
		std::string accid = info.GetAccId();
		if (info.GetRelationship() == nim::kNIMFriendFlagNormal)
		{
			if (GetUserType(accid) == nim::kNIMFriendFlagNotFriend) //不在friend_list_里面，就添加进去
			{
				add_list.push_back(accid);
				friend_list_.insert(decltype(friend_list_)::value_type(accid, info));
			}
			else //在friend_list_里面，则更新之
			{
				update_list.push_back(accid);
				friend_list_.at(accid).Update(info);
			}
		}
		else
		{
			delete_list.push_back(accid);
			update_list.push_back(accid); // 删除好友之后，其原来的备注名改为其昵称
			friend_list_.erase(accid); // 从friend_list_删除
		}
	}

	// 好友同步消息会在登录后较早的收到，这时批量的拉取用户信息把这些好友添加到查询队列里
	// 避免创建好友列表项时，列表项控件查询用户信息而导致频繁调用用户信息获取接口
	DoQueryUserInfos(add_list);
	DoQueryUserInfos(update_list);

	SubscribeEventManager::GetInstance()->SubscribeFriendEvent(add_list);
	SubscribeEventManager::GetInstance()->UnSubscribeFriendEvent(delete_list);

	for each (const auto& id in add_list)
		InvokeFriendListChangeCallback(kChangeTypeAdd, id);

	for each (const auto& id in delete_list)
		InvokeFriendListChangeCallback(kChangeTypeDelete, id);

	for each (const auto& id in update_list)
		InvokeFriendListChangeCallback(kChangeTypeUpdate, id);
}

void UserService::OnUserInfoChangeBySDK(const std::list<nim::UserNameCard> &uinfo_list)
{
	assert(nbase::MessageLoop::current()->ToUIMessageLoop());

	std::list<nim::UserNameCard> name_photo_list;
	std::list<nim::UserNameCard> misc_uinfo_list;

	for (auto& info : uinfo_list)
	{
		auto iter = all_user_.find(info.GetAccId());
		if (iter != all_user_.end()) //all_user_中存在，就更新
			iter->second.Update(info);
		else
			all_user_[info.GetAccId()] = info;

		if (!info.GetIconUrl().empty())
			PhotoService::GetInstance()->DownloadUserPhoto(info);

		if (info.ExistValue(nim::kUserNameCardKeyName) || info.ExistValue(nim::kUserNameCardKeyIconUrl)) //用户名或头像变化了
			name_photo_list.push_back(info);
		if (info.ExistValue((nim::UserNameCardValueKey)(nim::kUserNameCardKeyAll - nim::kUserNameCardKeyName - nim::kUserNameCardKeyIconUrl))) //用户其他信息变化了
			misc_uinfo_list.push_back(info);
	}

	// 执行回调列表中所有回调
	for (auto& it : uinfo_change_cb_list_)
		(*(it.second))(name_photo_list);
	for (auto& it : misc_uinfo_change_cb_list_)
		(*(it.second))(misc_uinfo_list);
}

void UserService::OnUserInfoChange(const std::list<nim::UserNameCard> &uinfo_list)
{
	assert(nbase::MessageLoop::current()->ToUIMessageLoop());

	std::list<nim::UserNameCard> name_photo_list;
	std::list<nim::UserNameCard> misc_uinfo_list;

	for (auto& info : uinfo_list)
	{
		if (!info.GetIconUrl().empty())
			PhotoService::GetInstance()->DownloadUserPhoto(info);

		if (info.ExistValue(nim::kUserNameCardKeyName) || info.ExistValue(nim::kUserNameCardKeyIconUrl)) //用户名或头像变化了
			name_photo_list.push_back(info);
		if (info.ExistValue((nim::UserNameCardValueKey)(nim::kUserNameCardKeyAll - nim::kUserNameCardKeyName - nim::kUserNameCardKeyIconUrl))) //用户其他信息变化了
			misc_uinfo_list.push_back(info);
	}

	// 执行回调列表中所有回调
	for (auto& it : uinfo_change_cb_list_)
		(*(it.second))(name_photo_list);
	for (auto& it : misc_uinfo_change_cb_list_)
		(*(it.second))(misc_uinfo_list);
}

void UserService::InvokeGetUserInfo(const std::list<std::string>& account_list)
{
	// 先在本地db中找
	nim::User::GetUserNameCardCallback cb1 = ToWeakCallback([this, account_list](const std::list<nim::UserNameCard> &json_result)
	{
		std::set<std::string> not_get_set(account_list.cbegin(), account_list.cend());
		for (auto& card : json_result)
		{
			all_user_[card.GetAccId()] = card; // 插入all_user
			on_query_list_.erase(card.GetAccId()); //已经查到，就从on_query_list_删除
			not_get_set.erase(card.GetAccId());
		}

		OnUserInfoChange(json_result); //触发监听

		if (not_get_set.empty()) // 全部从本地db找到，直接返回
			return;

		// 有些信息本地db没有，再从服务器获取
		auto closure = [this](const std::list<std::string>& ids)
		{
			nim::User::GetUserNameCardOnline(ids,
				this->ToWeakCallback([this, ids](const std::list<nim::UserNameCard> &json_result)
				{
					std::set<std::string> tmp_set(ids.cbegin(), ids.cend());
					for (auto& card : json_result)
					{
						all_user_[card.GetAccId()] = card; // 插入all_user

						if (card.ExistValue(nim::kUserNameCardKeyIconUrl))
							PhotoService::GetInstance()->DownloadUserPhoto(card); // 下载头像

						on_query_list_.erase(card.GetAccId()); //已经查到，就从on_query_list_删除
						tmp_set.erase(card.GetAccId());
					}

					//OnUserInfoChangeBySdk(json_result); //sdk会自动触发此回调

					for (const auto& id : tmp_set) //从服务器也查不到的用户
					{
						QLOG_APP(L"Can't get user's name card from server. Account id: {0}.") << id;
						on_query_list_.erase(id); //从on_query_list_删除，以免积压
					}
			}));
		};
				
		//SDK限制一次服务器查询数量不超过150	
		std::list<std::string> ids;
		if (not_get_set.size() > 150)
		{			
			for (auto iter = not_get_set.begin(); iter != not_get_set.end(); ++iter)
			{
				ids.push_back(*iter);
				if (ids.size() == 150)
				{
					closure(ids);
					ids.clear();
				}
			}
			if (!ids.empty())
			{
				closure(ids);
				ids.clear();
			}
		}
		else
		{
			for (auto iter = not_get_set.begin(); iter != not_get_set.end(); ++iter)
			{
				ids.push_back(*iter);
			}
			closure(ids);
		}
	});

	for (const auto& id : account_list)
		on_query_list_.insert(id);

	nim::User::GetUserNameCard(account_list, cb1);
}

void UserService::InvokeFriendListChangeCallback(FriendChangeType change_type, const std::string& accid)
{
	if (accid.empty())
		return;

	assert(nbase::MessageLoop::current()->ToUIMessageLoop());
	for (auto& it : friend_list_change_cb_list_)
	{
		(*(it.second))(change_type, accid);
	}
}

}
