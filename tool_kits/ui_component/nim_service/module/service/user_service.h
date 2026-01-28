#pragma once
#include "shared/threads.h"
#include "shared/auto_unregister.h"

//好友变化类型
enum FriendChangeType
{
	kChangeTypeAdd,
	kChangeTypeDelete,
	kChangeTypeUpdate
};

//用户性别
enum UserGender
{
	UG_UNKNOWN,
	UG_MALE,
	UG_FEMALE
};

//用户登录信息
struct UserInfo
{
	std::string uid;
	std::string token;
	std::string nimtoken;
	std::string ip;
	UserInfo()
	{
		uid = "";
		token = "";
		nimtoken = "";
		ip = "";
	}
};
struct CustomImgInfo
{
	std::string imgId;
	std::string OriginalPath;
	std::string thumbnailPath;
};
typedef struct MsgInfo
{
	std::string senderId;
	std::string senderName;
}curMsgInfo;

typedef struct BookmarksInfo
{
	long id;
	std::string uid;
	int type;
	std::string path;
	std::string imgCompPath;
	std::string content;
	std::string senderId;
	std::string senderName;
	std::string time;
	BookmarksInfo()
	{
		id = -1;
		uid = "";
		type = -1;
		path = "";
		imgCompPath = "";
		content = "";
		senderId = "";
		senderName = "";
		time = "";
	}
}curBookmarksInfo;

typedef struct pageInfo
{
	int total;// 10,
	int count;// : 10,
	int pageCount;// : 1
	int page;// : 1,
	bool hasMore;// : 0
}BookmarksPageInfo;

//用户登录信息
typedef struct VersionInfo_
{
	std::string currVersion;
	std::string url;
	std::string description;
	std::string updateType;
	VersionInfo_()
	{
		currVersion = "";
		url = "";
		description = "";
		updateType = "";
	}
}VersionInfo;

//用户登录信息
typedef struct SignInfo_
{
	std::string signDate;
	std::map<std::string, std::string> idMap;
	//std::list<std::map<std::string,std::string>> idList;
	//std::list<std::string> accountList;
	SignInfo_()
	{
		signDate = "";
	}
}SignInfo;

//用户登录信息
typedef struct UserBaseInfo_
{
	std::string account;
	std::string address;
	std::string ip;
	UserBaseInfo_()
	{
		account = "";
		address = "";
		ip = "";
	}
}UserBaseInfo;

//用户登录信息
//typedef struct ClientSetting_
//{
//	std::string key;
//	std::string value;
//	ClientSetting_()
//	{
//		key = "";
//		value = "";
//	}
//}ClientSetting;

typedef std::function<void(int res, const std::string& err_msg)> OnCommonCallback;
typedef std::function<void(int res, const std::string& err_msg, const bool isSignIn)> OnIsSignInCallback;
typedef std::function<void(int code, const std::string& err_msg, const std::list<SignInfo> &info)> OnSearchSignCallback;
typedef std::function<void(int res, const std::string& err_msg)> OnGetRegisterCodeCallback;
typedef std::function<void(int code, const std::string& err_msg, const UserInfo &info)> OnLoginCallback;
typedef std::function<void(int code, const std::string& err_msg, const VersionInfo &info)> OnUpdateCallback;
typedef std::function<void(int code, const std::string OriginalPath, const std::string thumbnailPath)> OnUploadImgCallback;
typedef std::function<void(int code, const std::string imgId)> OnUploadImgPathCallback;
typedef std::function<void(int res, const std::string& err_msg)> OnDeleteImgPathCallback;
typedef std::function<void(int code, const std::list<CustomImgInfo>& imgInfoList)> OnQueryImgInfoCallback;
typedef std::function<void(int code, const std::string &err_msg, const BookmarksPageInfo &pageInfo, std::list<BookmarksInfo>& bookmarksInfoList)> OnQueryBookmarksInfoCallback;
typedef std::function<void(int res, const std::string& err_msg)> OnRegisterAccountCallback;
typedef std::function<void(FriendChangeType change_type, const std::string& accid)> OnFriendListChangeCallback;
typedef std::function<void(const std::list<nim::UserNameCard>&)> OnUserInfoChangeCallback;
typedef std::function<void(const std::list<nim::UserNameCard>&)> OnGetUserInfoCallback;
typedef std::function<void(nim::NIMResCode res)> OnUpdateUserInfoCallback; 
typedef std::function<void(int res, const std::string& err_msg, const std::string uid, const std::string nickname, const std::string avatar, const std::string username)> OnQueryMemberCallback;
typedef std::function<void(int res, const std::string& err_msg, const UserBaseInfo userInfo)> OnQueryUserInfoCallback;
typedef std::function<void(int res, const std::string& err_msg)> OnCheckCreateTeamCallback;
typedef std::function<void(int res, const std::string& err_msg)> OnCheckAddFriendCallback;
typedef std::function<void(int res, const std::string& err_msg)> OnQueryAddFriendCallback;
typedef std::function<void(int code, const std::string& err_msg, const std::map<std::string, std::string>&)> OnGetClientSettingCallback;
typedef std::function<void(int code, const std::string& err_msg, const std::string&)> OnGetTeamMemberAddfriendCallback;

/**
* 获取连接服务器的某一个配置信息
* @param[in] key 需要获取的信息关键字
* @return string 配置信息
*/
std::string GetConfigValue(const std::string& key);

namespace nim_comp
{
/** @class UserService
  * @brief 用户注册、用户信息查询服务
  * @copyright (c) 2016, NetEase Inc. All rights reserved
  * @date 2016/09/14
  */
class UserService : public nbase::SupportWeakCallback
{
public:
	SINGLETON_DEFINE(UserService);
	UserService();

public:

	/**
	* 不使用手机号注册一个新账号
	* @param[in] username 要注册的用户名
	* @param[in] password 用户密码
	* @param[in] nickname 用户昵称
	* @param[in] cb 注册完毕的回调通知函数
	* @return void	无返回值
	*/
	void InvokeRegisterAccountEx(const std::string &username, const std::string &password, const std::string &nickname, const std::string &invitecode, const OnRegisterAccountCallback& cb);

	/**
	* 登录
	* @return void	无返回值
	*/
	void InvokeVersionUpdate(const std::string &os, const std::string &version, const OnUpdateCallback& cb);

	/**
	* 获取能否注册
	*/
	void InvokeGetRegisterCan(const std::string os, const OnCommonCallback& cb);

	/**
	* 获取客户端设置
	*/
	void InvokeGetClientSetting(const OnGetClientSettingCallback& cb);

	/**
	* 获取验证码
	* @return void	无返回值
	*/
	void InvokeGetRegisterCode(const std::string userMobile, const OnGetRegisterCodeCallback& cb);

	/**
	* 登录
	* @return void	无返回值
	*/
	void InvokeLoginAccount(const std::string &username, const std::string &password, const OnLoginCallback& cb);

	/**
	* 设置用户token
	*/
	void SetAccountToken(const UserInfo userinfo);

	/**
	* 发送欢迎词
	*
	*/
	void InvokeSendWelcome(const std::string &uid, const OnCommonCallback& cb);

	/**
	* 是否有建群的权限
	*/
	void InvokeCheckCreateTeam(const std::string &userId, const OnCheckCreateTeamCallback& cb);

	/**
	* 是否有加好友的权限
	*/
	void InvokeCheckAddFriendAuth(const std::string &userId, const OnCheckAddFriendCallback& cb);

	/**
	* 查看能否加好友
	* accid 需要添加的用户id
	*/
	void InvokeCheckAddFriend(const std::string &userId, const std::string &accid, const OnQueryAddFriendCallback& cb);

	/**
	* 上传图片
	* @return void	无返回值
	*/
	void InvokeUploadImg(const std::string &imgContent, const OnUploadImgCallback& cb);

	/**
	* 上传文件
	* @return void	无返回值
	*/
	void InvokeUploadFile(const std::string &imgContent, const OnUploadImgCallback& cb);

	/**
	* 上传图片路径
	*/
	void InvokeUploadImgPath(const std::string session_id, const std::string &imgPath, const std::string &imgCompPath, const OnUploadImgPathCallback& cb);

	/**
	* 删除图片路径
	*/
	void InvokeDeleteImgPath(const std::string userId, const std::string &imgId, const OnDeleteImgPathCallback& cb);

	/**
	* 查询收藏图片信息
	*/
	void QueryImgInfo(const std::string userId, const OnQueryImgInfoCallback& cb);

	/**
	* 查询收藏信息
	*/
	void QueryBookmarksInfo(const std::string userId, const std::string page, const OnQueryBookmarksInfoCallback& cb);

	/**
	* 删除收藏信息
	*/
	void DeleteBookmarksInfo(const std::string userId, const std::string msgId, const OnDeleteImgPathCallback& cb);

	/**
	* 收藏保存
	*/
	void InvokeUploadBookmarks(const curBookmarksInfo &info, const OnUploadImgPathCallback& cb);

	/**
	* 更新用户信息
	* @return void	无返回值
	*/
	void InvokeUpdateMyInfoToBaiCai(const nim::UserNameCard &new_info);//, const OnUpdateMyInfo& cb);

	/**
	* 群用户签到
	* @return void	无返回值
	*/
	void InvokeSignIn(const std::string userId, const std::string teamId, const OnCommonCallback& cb);

	/**
	* 群用户是否签到
	* @return void	无返回值
	*/
	void InvokeIsSignIn(const std::string userId, const std::string teamId, const OnIsSignInCallback& cb);

	/**
	* 群用户签到信息查询
	* @return void	无返回值
	*/
	void QueryTeamSignIn(const std::string teamId, const std::string startTime, const std::string endTime, const OnSearchSignCallback& cb);

	/*
	* 由用户注册的帐号查询昵称、注册id等
	*/
	void QueryMemberInfo(const std::string account, const OnQueryMemberCallback& cb);
	void QueryMemberInfoByAcc(const std::string account, const OnQueryMemberCallback& cb);
	static void QueryMemberInfoCallback(bool ret, int response_code, const std::string& reply, const OnQueryMemberCallback& cb);
	/*
	* 由id到后台服务器获取用户的基本信息
	*/
	void QueryCurMemberInfo(const std::string uId, const OnQueryUserInfoCallback& cb);

	void QueryOtherUserInfo(const std::string uId, const OnQueryUserInfoCallback& cb);
	static void QueryOtherUserInfoCallback(bool ret, int response_code, const std::string& reply, const OnQueryUserInfoCallback& cb);
	/*
	**创建群聊
	*/
	void InvokeCreateTeam(const std::string tId, const std::string tName, const std::string uId, const nim::NIMTeamMemberAddFriendMode addFriend, const OnCommonCallback& cb);

	/*
	**更新群信息
	*/
	void InvokeUpdateTeamInfo(const std::string tId, const std::string tName, const std::string uId, const nim::NIMTeamMemberAddFriendMode addFriend, const OnCommonCallback& cb);

	/*
	**解散群上传服务器
	*/
	void InvokeDissmissTeam(const std::string tId, const OnCommonCallback& cb);

	/*
	**获取群成员添加好友权限
	*/
	void InvokeGetMemberAddFriend(const std::string tId, const OnGetTeamMemberAddfriendCallback& cb);

	/*
	**移交群主上传服务器
	*/
	void InvokeTransferTeamOwner(const std::string tId, const std::string uId, const OnCommonCallback& cb);

	/*
	**添加好友
	*/
	void InvokeAddFriend(const std::string uId, const std::string accId, const OnCommonCallback& cb);

	/*
	**获取用户基本信息
	*/
	void InvokeDeleteFriend(const std::string uId, const std::string accId, const OnCommonCallback& cb);

	/**
	* 向数据库和服务器获取全部好友以及近期获取过的用户信息
	* @param[in] cb 回调函数
	* @return void	无返回值
	*/
	void InvokeGetAllUserInfo(const OnGetUserInfoCallback& cb);

	/**
	* 向数据库和服务器获取指定用户以及近期获取过的用户信息
	* @param[in] cb 回调函数
	* @return void	无返回值
	*/
	void InvokeGetAllUserInfo(const std::list<std::string>& accids, const OnGetUserInfoCallback& cb);

	/**
	* 向数据库获取全部机器人以及近期获取过的机器人信息
	* @param[in] cb 回调函数
	* @return void	无返回值
	*/
	nim::RobotInfos InvokeGetAllRobotsInfoBlock();

	/**
	* 修改用户自己的个人信息
	* @param[in] new_info 新的用户信息
	* @param[in] cb 回调函数
	* @return void	无返回值
	*/
	void InvokeUpdateMyInfo(const nim::UserNameCard &new_info, const OnUpdateUserInfoCallback& cb);

	/**
	* 推送自己的ip到云信
	* @param[in] cb 回调函数
	* @return void	无返回值
	*/
	void InvokeUpdateMyIp(const OnUpdateUserInfoCallback& cb);

	/**
	* 修改用户自己的头像
	* @param[in] new_info 新的用户信息
	* @param[in] cb 回调函数
	* @return void	无返回值
	*/
	void InvokeUpdateMyPhoto(const std::string &url, const OnUpdateUserInfoCallback& cb);

	/**
	* 获取本地保存的所有用户信息
	* @return std::map<std::string, nim::UserNameCard>& 用户信息列表
	*/
	const std::map<std::string, nim::UserNameCard>& GetAllUserInfos();

	/**
	* 获取本地保存的所有好友信息
	* @return std::map<std::string, nim::FriendProfile>& 好友信息列表
	*/
	const std::map<std::string, nim::FriendProfile>& GetAllFriendInfos();

	/**
	* 获取用户信息,如果查询不到则查询服务器
	* @param[in] id 用户id
	* @param[out] info 用户信息
	* @return bool true 查询到，false 没有查询到
	*/
	bool GetUserInfo(const std::string &id, nim::UserNameCard &info);

	/**
	* 获取Id对应的用户是不是机器人
	* @param[in] accid 机器人云信ID
	* @return bool true 是，false 不是
	*/
	bool IsRobotAccount(const std::string &accid);

	/**
	* 获取用户信息,如果查询不到则查询服务器
	* @param[in] ids 用户id列表
	* @param[out] uinfos 用户信息列表
	* @return void 无返回值
	*/
	void GetUserInfos(const std::list<std::string> &ids, std::list<nim::UserNameCard>&uinfos);

	/**
	* 执行批量查询用户信息操作,在可能要使用某些用户信息前进行批量查询,用于优化用户信息查询操作
	* @param[in] ids 用户id列表
	* @return void 无返回值
	*/
	void DoQueryUserInfos(const std::set<std::string>& ids);

	/**
	* 执行批量查询用户信息操作,在可能要使用某些用户信息前进行批量查询,用于优化用户信息查询操作
	* @param[in] ids 用户id列表
	* @return void 无返回值
	*/
	void DoQueryUserInfos(const std::list<std::string>& ids);

	/**
	* 获取某个用户的好友类型
	* @param[in] id 用户id
	* @return NIMFriendFlag 好友类型
	*/
	nim::NIMFriendFlag GetUserType(const std::string &id);

	/**
	* 获取好友昵称或者备注名
	* @param[in] id 用户id
	* @param[in] alias_prior 是否有限查备注名
	* @return wstring 用户昵称或备注名
	*/
	std::wstring GetUserName(const std::string &id, bool alias_prior = true);

	/**
	* 获取好友扩展字段
	* @param[in] id 用户id
	* @return Json::Value 用户扩展字段
	*/
	Json::Value GetUserCustom(const std::string &id);

	/**
	* 获取好友备注名
	* @param[in] id 用户id
	* @return wstring 用户备注名
	*/
	std::wstring GetFriendAlias(const std::string &id);

	/**
	* 注册好友列表改变的回调
	* @param[in] callback 回调函数
	* @return UnregisterCallback 反注册对象
	*/
	UnregisterCallback RegFriendListChange(const OnFriendListChangeCallback& callback);

	/**
	* 注册用户名、头像改变的回调
	* @param[in] callback 回调函数
	* @return UnregisterCallback 反注册对象
	*/
	UnregisterCallback RegUserInfoChange(const OnUserInfoChangeCallback& callback);

	/**
	* 注册用户其他信息改变的回调
	* @param[in] callback 回调函数
	* @return UnregisterCallback 反注册对象
	*/
	UnregisterCallback RegMiscUInfoChange(const OnUserInfoChangeCallback& callback);

	/**
	* 好友列表改变的回调
	* @param[in] change_event 好友变更事件
	* @return void 无返回值
	*/
	void OnFriendListChangeBySDK(const nim::FriendChangeEvent& change_event);

	/**
	* 同步好友列表的回调
	* @param[in] change_event 同步好友列表事件
	* @return void 无返回值
	*/
	void OnSyncFriendList(const nim::FriendChangeEvent& change_event);

	/**
	* 用户名、头像改变的回调(由SDk触发)
	* @param[in] uinfo_list 用户名片列表
	* @return void 无返回值
	*/
	void OnUserInfoChangeBySDK(const std::list<nim::UserNameCard> &uinfo_list);

	/**
	* 用户名、头像改变的回调（由Demo触发）
	* @param[in] uinfo_list 用户名片列表
	* @return void 无返回值
	*/
	void OnUserInfoChange(const std::list<nim::UserNameCard> &uinfo_list);

	/**
	* 响应机器人信息改变的回调函数
	* @param[in] rescode 错误码
	* @param[in] type 类型
	* @param[in] robots 机器人列表
	* @return void 无返回值
	*/
	void OnRobotChange(nim::NIMResCode rescode, nim::NIMRobotInfoChangeType type, const nim::RobotInfos& robots);
private:
	/**
	* 向数据库和服务器获取指定id的用户信息
	* @param[in] account_list 用户id列表
	* @return void	无返回值
	*/
	void InvokeGetUserInfo(const std::list<std::string>& account_list);

	/**
	* 触发好友列表变更的的回调
	* @param[in] change_type 好友变化类型
	* @param[in] accid 用户id
	* @return void	无返回值
	*/
	void InvokeFriendListChangeCallback(FriendChangeType change_type, const std::string& accid);

private:
	std::map<std::string, nim::UserNameCard> all_user_; //好友+陌生人
	std::set<std::string> post_ids_; //已经发出请求的id们
	std::map<std::string, nim::FriendProfile> friend_list_; //好友列表
	std::map<std::string, nim::RobotInfo> robot_list_; //机器人列表
	std::map<int, std::unique_ptr<OnFriendListChangeCallback>> friend_list_change_cb_list_; //有好友增减回调列表
	std::map<int, std::unique_ptr<OnUserInfoChangeCallback>> uinfo_change_cb_list_; //用户名、头像变化回调列表
	std::map<int, std::unique_ptr<OnUserInfoChangeCallback>> misc_uinfo_change_cb_list_; //用户其他信息变化回调列表
	std::map<int, std::unique_ptr<nim::Robot::RobotChangedCallback>> robot_change_cb_list_; //机器人变化回调列表

	std::set<std::string> on_query_list_; //已经要求查询，但还未返回结果的

	UserInfo userinfo_;
};

}
