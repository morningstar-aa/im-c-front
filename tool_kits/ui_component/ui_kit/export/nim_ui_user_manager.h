#pragma once

#include "nim_ui_dll.h"
#include "module/service/user_service.h"
#include "tool_kits/base/memory/singleton.h"

namespace nim_ui
{

/** @class UserManager
  * @brief 提供用户数据获取接口
  * @copyright (c) 2015, NetEase Inc. All rights reserved
  * @author Redrain
  * @date 2015/9/16
  */
class NIM_UI_DLL_API UserManager
{
public:
	SINGLETON_DEFINE(UserManager);

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
	* 获取能否注册
	*/
	void InvokeGetRegisterCan(const std::string os, const OnCommonCallback& cb);

	/**
	* 获取客户端设置
	*/
	void InvokeGetClientSetting(const OnGetClientSettingCallback& cb);

	/**
	* 获取注册码
	*/
	void InvokeGetRegisterCode(const std::string userMobile, const OnGetRegisterCodeCallback& cb);

	/*
	**版本更新信息
	*/
	void InvokeVersionUpdate(const std::string &os, const std::string &version, const OnUpdateCallback& cb);
	/**
	* 登录
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
	* 是否有创建群聊权限
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
	*/
	void InvokeUploadImg(const std::string &imgContent, const OnUploadImgCallback& cb);

	/**
	* 上传图片路径
	*/
	void InvokeUploadImgPath(const std::string session_id, const std::string &imgPath, const std::string &imgCompPath, const OnUploadImgPathCallback& cb);

	/**
	* 删除图片路径
	*/
	void InvokeDeleteImgPath(const std::string userId, const std::string &imgId, const OnDeleteImgPathCallback& cb);

	/**
	* 查询所有收藏图片
	*/
	void QueryImgInfo(const std::string userId, const OnQueryImgInfoCallback& cb);

	/**
	* 查询所有收藏信息
	*/
	void QueryBookmarksInfo(const std::string userId, const std::string page, const OnQueryBookmarksInfoCallback& cb);

	/**
	* 查询所有收藏信息
	*/
	void DeleteBookmarksInfo(const std::string userId, const std::string msgId, const OnDeleteImgPathCallback& cb);

	/**
	* 上传文件
	*/
	void InvokeUploadFile(const std::string &filePath, const OnUploadImgCallback& cb);

	/**
	* 收藏保存
	*/
	void InvokeUploadBookmarks(const curBookmarksInfo &info, const OnUploadImgPathCallback& cb);

	/**
	* 查询收藏
	*/
	void QueryBookmarksInfo(const std::string userId, const OnQueryImgInfoCallback& cb);

	/*
	**签到
	*/
	void InvokeSignIn(const std::string userId, const std::string teamId, const OnCommonCallback& cb);

	/*
	**是否签到
	*/
	void InvokeIsSignIn(const std::string userId, const std::string teamId, const OnIsSignInCallback& cb);

	/*
	**查询签到信息
	*/
	void QueryTeamSignIn(const std::string teamId, const std::string startTime, const std::string endTime, const OnSearchSignCallback& cb);

	/*
	**获取要添加为好友信息
	*/
	void QueryMemberInfo(const std::string account, const OnQueryMemberCallback& cb);
	void QueryMemberInfoByAcc(const std::string account, const OnQueryMemberCallback& cb);
	/*
	**获取用户基本信息
	*/
	void QueryCurMemberInfo(const std::string uId, const OnQueryUserInfoCallback& cb);

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
	* 获取本地保存的所有用户信息
	* @return std::map<std::string, nim::UserNameCard>& 用户信息列表
	*/
	const std::map<std::string, nim::UserNameCard>& GetAllUserInfos();
	/**
	* 查询本地保存的用户信息
	* @param[in] id 用户id
	* @param[out] info 用户信息
	* @return bool true 查询到，false 没有查询到
	*/
	bool GetUserInfo(const std::string &id, nim::UserNameCard &info);

	/**
	* 查询本地保存的用户信息
	* @param[in] ids 用户id列表
	* @param[out] uinfos 用户信息列表
	* @return void 无返回值
	*/
	void GetUserInfos(const std::list<std::string> &ids, std::list<nim::UserNameCard>&uinfos);

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

private:
	UserManager(){};
	~UserManager(){};

};

}