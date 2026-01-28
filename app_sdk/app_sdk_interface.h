#pragma once
#include "app_sdk_define.h"
namespace app_sdk
{
class AppSDKInterface : public nbase::SupportWeakCallback
{		
public:
	AppSDKInterface() = default;
	~AppSDKInterface() = default;
	SINGLETON_DEFINE(AppSDKInterface);

	/**
	 * 获取连接服务器的某一个配置信息
	 * @param[in] key 需要获取的信息关键字
	 * @return string 配置信息
	 */
	static std::string GetConfigValue(const std::string& key);

	/**
	 * 获取连接服务器的某一个配置信息
	 * @param[in] element_name 节点关键字
	 * @param[in] key 需要获取的信息关键字
	 * @return string 配置信息
	 */
	static std::string GetConfigValue(const std::string& element_name,const std::string& key);

	/**
	 * 获取配置文件的版本信息
	 * @return string 版本
	 */
	static int GetConfigFileVersion();

	/**
	 * 获取应用的app key
	 * @return string 配置信息app key
	 */
	static std::string GetAppKey();

	static std::string GetAppVersion();

	/**
	 * 判断 URL 是否是安全链接
	 * @param[in] safe_url 要检测的 URL
	 * @return 返回 true 表示是一个安全短链接，否则为 false
	 */
	static bool IsSafeUrl(const std::string& safe_url);


public:		
	/**
	 * 获取应用发送http请求的host,host与api拼接成一个完整的http请求地址
	 * 开发者可以修改为具体应用的host
	 * @return string http请求的host
	 */
	std::string GetAppHost();

	/**
	 * 注册一个新账号
	 * @param[in] username 要注册的用户名
	 * @param[in] password 用户密码
	 * @param[in] nickname 用户昵称
	 * @param[in] cb 注册完毕的回调通知函数
	 * @return void	无返回值
	 */
	void InvokeRegisterAccount(const std::string &username, const std::string &password, const std::string &nickname, const OnRegisterAccountCallback& cb);

	/**
	 * 获取聊天室列表
	 * @param[in] cb 回调通知函数
	 * @return void	无返回值
	 */
	//void InvokeGetChatroomList(const OnGetChatroomListCallback& cb);

	/**
	 * 获取聊天室连接地址,多用于匿名登录时
	 * @param[in] room_id 聊天室ID
	 * @param[in] uid 用户UID
	 * @param[in] type 进入聊天室的模式 Logined = 1,Anonymous = 2,
	 * @param[in] cb 回调通知函数
	 * @return void	无返回值
	 */
	//void InvokeGetChatroomAddress(__int64 room_id,const std::string& uid,int type, const OnGetChatroomAddressCallback& cb);

		
private:
	static const std::string kAppKey;		
	static const std::string kAppHost;
	static const std::string kAppVersion;
};

}