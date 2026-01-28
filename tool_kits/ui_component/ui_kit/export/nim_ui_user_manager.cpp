#include "nim_ui_user_manager.h"

namespace nim_ui
{
	void UserManager::InvokeRegisterAccountEx(const std::string &username, const std::string &password, const std::string &nickname, const std::string &invitecode, const OnRegisterAccountCallback& cb)
	{
		nim_comp::UserService::GetInstance()->InvokeRegisterAccountEx(username, password, nickname, invitecode, cb);
	}

	void UserManager::InvokeGetRegisterCan(const std::string os, const OnCommonCallback& cb)
	{
		nim_comp::UserService::GetInstance()->InvokeGetRegisterCan(os, cb);
	}

	void UserManager::InvokeGetClientSetting(const OnGetClientSettingCallback& cb)
	{
		nim_comp::UserService::GetInstance()->InvokeGetClientSetting(cb);
	}

	void UserManager::InvokeGetRegisterCode(const std::string userMobile, const OnGetRegisterCodeCallback& cb)
	{
		nim_comp::UserService::GetInstance()->InvokeGetRegisterCode(userMobile, cb);
	}

	void UserManager::InvokeVersionUpdate(const std::string &os, const std::string &version, const OnUpdateCallback& cb)
	{
		nim_comp::UserService::GetInstance()->InvokeVersionUpdate(os, version, cb);
	}
	/**
	* 登录
	*/
	void UserManager::InvokeLoginAccount(const std::string &username, const std::string &password, const OnLoginCallback& cb)
	{
		nim_comp::UserService::GetInstance()->InvokeLoginAccount(username, password, cb);
	}

	void UserManager::SetAccountToken(const UserInfo userinfo)
	{
		nim_comp::UserService::GetInstance()->SetAccountToken(userinfo);
	}

	void UserManager::InvokeSendWelcome(const std::string &uid, const OnCommonCallback& cb)
	{
		nim_comp::UserService::GetInstance()->InvokeSendWelcome(uid, cb);
	}

	void UserManager::InvokeCheckCreateTeam(const std::string &userId, const OnCheckCreateTeamCallback& cb)
	{
		nim_comp::UserService::GetInstance()->InvokeCheckCreateTeam(userId, cb);
	}

	void UserManager::InvokeCheckAddFriendAuth(const std::string &userId, const OnCheckAddFriendCallback& cb)
	{
		nim_comp::UserService::GetInstance()->InvokeCheckAddFriendAuth(userId, cb);
	}

	void UserManager::InvokeCheckAddFriend(const std::string &userId, const std::string &accid, const OnQueryAddFriendCallback& cb)
	{
		nim_comp::UserService::GetInstance()->InvokeCheckAddFriend(userId, accid, cb);
	}

	void UserManager::InvokeUploadImg(const std::string &imgContent, const OnUploadImgCallback& cb)
	{
		nim_comp::UserService::GetInstance()->InvokeUploadImg(imgContent, cb);
	}

	void UserManager::InvokeUploadFile(const std::string &filePath, const OnUploadImgCallback& cb)
	{
		nim_comp::UserService::GetInstance()->InvokeUploadFile(filePath, cb);
	}
	void UserManager::InvokeUploadImgPath(const std::string session_id, const std::string &imgPath, const std::string &imgCompPath, const OnUploadImgPathCallback& cb)
	{
		nim_comp::UserService::GetInstance()->InvokeUploadImgPath(session_id, imgPath, imgCompPath, cb);
	}

	void UserManager::InvokeDeleteImgPath(const std::string userId, const std::string &imgId, const OnDeleteImgPathCallback& cb)
	{
		nim_comp::UserService::GetInstance()->InvokeDeleteImgPath(userId, imgId, cb);
	}

	void UserManager::QueryImgInfo(const std::string userId, const OnQueryImgInfoCallback& cb)
	{
		nim_comp::UserService::GetInstance()->QueryImgInfo(userId, cb);
	}

	void UserManager::QueryBookmarksInfo(const std::string userId, const std::string page, const OnQueryBookmarksInfoCallback& cb)
	{
		nim_comp::UserService::GetInstance()->QueryBookmarksInfo(userId, page, cb);
	}

	void UserManager::DeleteBookmarksInfo(const std::string userId, const std::string msgId, const OnDeleteImgPathCallback& cb)
	{
		nim_comp::UserService::GetInstance()->DeleteBookmarksInfo(userId, msgId, cb);
	}

	void UserManager::InvokeUploadBookmarks(const curBookmarksInfo &info, const OnUploadImgPathCallback& cb)
	{
		nim_comp::UserService::GetInstance()->InvokeUploadBookmarks(info, cb);
	}

	nim::NIMFriendFlag UserManager::GetUserType(const std::string &id)
	{
		return nim_comp::UserService::GetInstance()->GetUserType(id);
	}

	bool UserManager::GetUserInfo(const std::string &account, nim::UserNameCard &info)
	{
		return nim_comp::UserService::GetInstance()->GetUserInfo(account, info);
	}

	void UserManager::GetUserInfos(const std::list<std::string>& ids, std::list<nim::UserNameCard>& uinfos)
	{
		nim_comp::UserService::GetInstance()->GetUserInfos(ids, uinfos);
	}

	const std::map<std::string, nim::UserNameCard>& UserManager::GetAllUserInfos()
	{
		return nim_comp::UserService::GetInstance()->GetAllUserInfos();
	}

	std::wstring UserManager::GetUserName(const std::string &id, bool alias_prior/* = true */)
	{
		return nim_comp::UserService::GetInstance()->GetUserName(id, alias_prior);
	}

	std::wstring UserManager::GetFriendAlias(const std::string & id)
	{
		return nim_comp::UserService::GetInstance()->GetFriendAlias(id);
	}

	UnregisterCallback UserManager::RegFriendListChange(const OnFriendListChangeCallback & callback)
	{
		return nim_comp::UserService::GetInstance()->RegFriendListChange(callback);
	}

	UnregisterCallback UserManager::RegUserInfoChange(const OnUserInfoChangeCallback& callback)
	{
		return nim_comp::UserService::GetInstance()->RegUserInfoChange(callback);
	}

	UnregisterCallback UserManager::RegMiscUInfoChange(const OnUserInfoChangeCallback & callback)
	{
		return nim_comp::UserService::GetInstance()->RegMiscUInfoChange(callback);
	}

	void UserManager::InvokeSignIn(const std::string userId, const std::string teamId, const OnCommonCallback& cb)
	{
		return nim_comp::UserService::GetInstance()->InvokeSignIn(userId, teamId, cb);
	}

	void UserManager::InvokeIsSignIn(const std::string userId, const std::string teamId, const OnIsSignInCallback& cb)
	{
		return nim_comp::UserService::GetInstance()->InvokeIsSignIn(userId, teamId, cb);
	}

	void UserManager::QueryTeamSignIn(const std::string teamId, const std::string startTime, const std::string endTime, const OnSearchSignCallback& cb)
	{
		return nim_comp::UserService::GetInstance()->QueryTeamSignIn(teamId, startTime, endTime, cb);
	}

	void UserManager::QueryMemberInfo(const std::string account, const OnQueryMemberCallback& cb)
	{
		return nim_comp::UserService::GetInstance()->QueryMemberInfo(account, cb);
	}

	void UserManager::QueryMemberInfoByAcc(const std::string account, const OnQueryMemberCallback& cb)
	{
		return nim_comp::UserService::GetInstance()->QueryMemberInfoByAcc(account, cb);
	}

	void UserManager::QueryCurMemberInfo(const std::string uId, const OnQueryUserInfoCallback& cb)
	{
		return nim_comp::UserService::GetInstance()->QueryCurMemberInfo(uId, cb);
	}

	void UserManager::InvokeCreateTeam(const std::string tId, const std::string tName, const std::string uId, const nim::NIMTeamMemberAddFriendMode addFriend, const OnCommonCallback& cb)
	{
		return nim_comp::UserService::GetInstance()->InvokeCreateTeam(tId, tName, uId, addFriend, cb);
	}

	void UserManager::InvokeGetMemberAddFriend(const std::string tId, const OnGetTeamMemberAddfriendCallback& cb)
	{
		return nim_comp::UserService::GetInstance()->InvokeGetMemberAddFriend(tId, cb);
	}

	void UserManager::InvokeUpdateTeamInfo(const std::string tId, const std::string tName, const std::string uId, const nim::NIMTeamMemberAddFriendMode addFriend, const OnCommonCallback& cb)
	{
		return nim_comp::UserService::GetInstance()->InvokeUpdateTeamInfo(tId, tName, uId, addFriend, cb);
	}

	void UserManager::InvokeDissmissTeam(const std::string tId, const OnCommonCallback& cb)
	{
		return nim_comp::UserService::GetInstance()->InvokeDissmissTeam(tId, cb);
	}

	void UserManager::InvokeTransferTeamOwner(const std::string tId, const std::string uId, const OnCommonCallback& cb)
	{
		return nim_comp::UserService::GetInstance()->InvokeTransferTeamOwner(tId, uId, cb);
	}

	void UserManager::InvokeAddFriend(const std::string uId, const std::string accId, const OnCommonCallback& cb)
	{
		return nim_comp::UserService::GetInstance()->InvokeAddFriend(uId, accId, cb);
	}

	void UserManager::InvokeDeleteFriend(const std::string uId, const std::string accId, const OnCommonCallback& cb)
	{
		return nim_comp::UserService::GetInstance()->InvokeDeleteFriend(uId, accId, cb);
	}
}

