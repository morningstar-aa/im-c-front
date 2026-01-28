#include "force_push_manager.h"
#include "module/db/user_db.h""
#include "base/encrypt/encrypt_impl.h"
#include "base/util/base64.h"
#include <locale>
#include <codecvt>
#include <string>

namespace nim_comp
{
void ForcePushManager::Load()
{
	std::map<std::string, std::string> data;
	UserDB::GetInstance()->QueryAllForcePushData(data);

	Json::Reader reader;
	Json::Value value;
	ForcePushInfo info;
	std::vector<ForcePushInfo> info_list;
	for (auto &i : data)
	{
		if (reader.parse(i.second, value) && value.isArray())
		{
			for (int j = 0; j < (int)value.size(); ++j)
			{
				info.sender_id = value[j]["id"].asString();
				info.msg_body = value[j]["msg_body"].asString();
				info.uuid = value[j]["uuid"].asString();

				info_list.push_back(info);
			}
		}
		
		session_id_atme_[i.first] = info_list;
		info_list.clear();
	}
}

void ForcePushManager::Save()
{
	UserDB::GetInstance()->ClearForcePushData();

	Json::FastWriter writer;
	Json::Value root;
	Json::Value value;

	std::map<std::string, std::string> data;
	std::string info;
	for (auto &i : session_id_atme_)
	{
		for (auto &j : i.second)
		{
			value["id"] = j.sender_id;
			value["msg_body"] = j.msg_body;
			value["uuid"] = j.uuid;
			root.append(value);
		}

		data[i.first] = writer.write(root);
		root.clear();
	}

	UserDB::GetInstance()->InsertForcePushData(data);
}

bool ForcePushManager::IsAtMeMsg(const nim::IMMessage &msg)
{
	// 是否包含atme消息，如果当前msg包含atme消息，就不显示提示条，否则显示
	if (msg.session_type_ == nim::kNIMSessionTypeTeam && msg.type_ == nim::kNIMMessageTypeText && !LoginManager::GetInstance()->IsEqual(msg.sender_accid_))
	{
		if (msg.msg_setting_.is_force_push_ == BS_TRUE)
		{
			//@所有人
			if (msg.msg_setting_.force_push_ids_list_.empty())
			{
				return true;
			}
			else
			{
				for (auto &id : msg.msg_setting_.force_push_ids_list_)
				{
					if (LoginManager::GetInstance()->IsEqual(id))
						return true;
				}
			}
		}
	}

	return false;
}

bool ForcePushManager::IsAddTeamMsg(const nim::IMMessage &msg, std::string &id)
{
	Json::Value json;
	if (StringToJson(msg.attach_, json) && json.isObject())
	{
		if (json.isMember(nim::kNIMNotificationKeyId))
		{
			if (json[nim::kNIMNotificationKeyId].asInt() == nim::kNIMNotificationIdTeamApplyPass)
			{
				id = json[nim::kNIMNotificationKeyData][nim::kNIMNotificationKeyId].asString();
				return true;
			}
		}
	}
	return false;
}

bool ForcePushManager::IsPassiveAddTeamMsg(const nim::IMMessage &msg)
{
	Json::Value json;
	if (StringToJson(msg.attach_, json) && json.isObject())
	{
		if (json.isMember(nim::kNIMNotificationKeyId))
		{
			if (json[nim::kNIMNotificationKeyId].asInt() == nim::kNIMNotificationIdTeamInviteAccept)
			{
				return true;
			}
		}
	}
	return false;
}

bool ForcePushManager::IsBoardMsg(const nim::IMMessage &msg)
{
	// 是否包含群公告消息，如果当前msg包含群公告消息，就不显示提示条，否则显示
	if (msg.session_type_ == nim::kNIMSessionTypeTeam && msg.type_ == nim::kNIMMessageTypeNotification && !LoginManager::GetInstance()->IsEqual(msg.sender_accid_))
	{
		Json::Value json;
		if (StringToJson(msg.attach_, json) && json.isObject())
		{

			nim::NIMNotificationId id = (nim::NIMNotificationId)json[nim::kNIMNotificationKeyId].asInt();
			if (id == nim::kNIMNotificationIdTeamMuteMember)
			{
					return false;
			}
			Json::Value tinfo_json = json[nim::kNIMNotificationKeyData][nim::kNIMNotificationKeyTeamInfo];
			if (tinfo_json.isMember(nim::kNIMTeamInfoKeyAnnouncement))
			{
				return true;
			}
			/*else if (json.isMember(nim::kNIMNotificationKeyId) && (json[nim::kNIMNotificationKeyId].asInt() == nim::kNIMNotificationIdTeamApplyPass || json[nim::kNIMNotificationKeyId].asInt() == nim::kNIMNotificationIdTeamInviteAccept))
			{
				return true;
			}*/
		}
		/*if (msg.msg_setting_.is_force_push_ == nim::BS_TRUE)
		{
			//@所有人
			if (msg.msg_setting_.force_push_ids_list_.empty())
			{
				return true;
			}
			else
			{
				for (auto &id : msg.msg_setting_.force_push_ids_list_)
				{
					if (LoginManager::GetInstance()->IsEqual(id))
						return true;
				}
			}
		}*/
	}

	return false;
}

void ForcePushManager::AddAtMeMsg(const std::string &session_id, const nim::IMMessage &msg)
{
	ForcePushInfo info;

	std::wstring str = nbase::UTF8ToUTF16(msg.content_);
	// decode
	try {
		const std::string key = "12345678766666661234567876666666";
		const std::string iv = "1112222211111121";

		std::string cipher(str.begin(), str.end());
		std::string base64_de;
		nbase::EncryptInterface_var encrypt_enc(new nbase::Encrypt_Impl());
		nbase::Base64Decode(cipher, &base64_de);
		encrypt_enc->SetMethod(nbase::ENC_AES256_CBC);
		encrypt_enc->SetDecryptKey(key);
		encrypt_enc->SetDecryptIvParameterSpec(iv);
		encrypt_enc->EnableDecryptPadding(true, 7);
		UTF8String password_aes_de;
		encrypt_enc->Decrypt(base64_de, password_aes_de);

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		str = converter.from_bytes(password_aes_de);
	}
	catch (const std::exception&) {
		str = nbase::UTF8ToUTF16(msg.content_);
	}

	info.msg_body = std::string(str.begin(), str.end());
	info.sender_id = msg.sender_accid_;
	info.uuid = msg.client_msg_id_;

	auto i = session_id_atme_.find(session_id);
	if (i != session_id_atme_.end())
	{
		i->second.push_back(info);
	}
	else
	{
		std::vector<ForcePushInfo> list;
		list.push_back(info);
		session_id_atme_[session_id] = list;	
	}
}

void ForcePushManager::AddBoardMsg(const std::string &session_id, const nim::IMMessage &msg)
{
	ForcePushInfo info;
	info.msg_body = msg.attach_;
	info.sender_id = msg.sender_accid_;
	info.uuid = msg.client_msg_id_;

	auto i = session_id_board_.find(session_id);
	if (i != session_id_board_.end())
	{
		i->second.push_back(info);
	}
	else
	{
		std::vector<ForcePushInfo> list;
		list.push_back(info);
		session_id_board_[session_id] = list;
	}
}

bool ForcePushManager::GetAtMeMsgs(const std::string &session_id, std::vector<ForcePushInfo> &infos)
{
	auto i = session_id_atme_.find(session_id);
	if (i != session_id_atme_.end())
	{
		infos = i->second;
		return true;
	}
	else
	{
		return false;
	}
}

bool ForcePushManager::GetBoardMsgs(const std::string &session_id, std::vector<ForcePushInfo> &infos)
{
	auto i = session_id_board_.find(session_id);
	if (i != session_id_board_.end())
	{
		infos = i->second;
		return true;
	}
	else
	{
		return false;
	}
}

void ForcePushManager::ResetAtMeMsg(const std::string &session_id)
{
	// 重置对应会话中的@me消息为已读
	auto i = session_id_atme_.find(session_id);
	if (i != session_id_atme_.end())
		session_id_atme_.erase(i);
}

void ForcePushManager::ResetBoardMsg(const std::string &session_id)
{
	// 重置对应会话中的群公告消息为已读
	auto i = session_id_board_.find(session_id);
	if (i != session_id_board_.end())
		session_id_board_.erase(i);
}

bool ForcePushManager::IsContainAtMeMsg(const std::string &session_id)
{
	auto i = session_id_atme_.find(session_id);
	if (i != session_id_atme_.end())
		return true;
	
	return false;
}

bool ForcePushManager::IsContainBoard(const std::string &session_id)
{
	auto i = session_id_board_.find(session_id);
	if (i != session_id_board_.end())
		return true;

	return false;
}

}