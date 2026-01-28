#include "bubble_visitingcard.h"
#include <nim_user_helper.h>
#include "gui/profile_form/profile_form.h"
using namespace ui;

namespace nim_comp
{
	void MsgBubbleVisitingCard::InitControl(bool bubble_right)
{
	__super::InitControl(bubble_right);

	msg_unknown_ = new HBox;
	if(bubble_right)
		GlobalManager::FillBoxWithCache(msg_unknown_, L"session/visitingcard_right.xml");
	else
		GlobalManager::FillBoxWithCache(msg_unknown_, L"session/visitingcard_left.xml");
	bubble_box_->Add(msg_unknown_);

	card_heard_img_ = (Control*)msg_unknown_->FindSubControl(L"card_heard_img");
	visitingcard_account_ = (Label*)msg_unknown_->FindSubControl(L"visitingcard_account");
	visitingcard_name_ = (Label*)msg_unknown_->FindSubControl(L"visitingcard_name");
	visitingcard_title_ = (Label*)msg_unknown_->FindSubControl(L"visitingcard_title");
	msg_unknown_->FindSubControl(L"msg_visitingcard")->AttachAllEvents(nbase::Bind(&MsgBubbleVisitingCard::OnClicked, this, std::placeholders::_1));
	msg_unknown_->AttachAllEvents(nbase::Bind(&MsgBubbleVisitingCard::OnClicked, this, std::placeholders::_1));
	card_heard_img_->AttachAllEvents(nbase::Bind(&MsgBubbleVisitingCard::OnClicked, this, std::placeholders::_1));
	visitingcard_account_->AttachAllEvents(nbase::Bind(&MsgBubbleVisitingCard::OnClicked, this, std::placeholders::_1));
	visitingcard_title_->AttachAllEvents(nbase::Bind(&MsgBubbleVisitingCard::OnClicked, this, std::placeholders::_1));
	visitingcard_name_->AttachAllEvents(nbase::Bind(&MsgBubbleVisitingCard::OnClicked, this, std::placeholders::_1));

	//msg_image_->AttachMenu(nbase::Bind(&MsgBubbleImage::OnMenu, this, std::placeholders::_1));
}

	bool MsgBubbleVisitingCard::OnClicked(ui::EventArgs* arg)
	{
		if (arg->Type == kEventMouseButtonUp)
		{
			nim_comp::ProfileForm::ShowProfileForm(userInfo_.GetAccId());
		}
		return true;
	}

void MsgBubbleVisitingCard::InitInfo(const nim::IMMessage &msg)
{
	__super::InitInfo(msg); 
	Json::Value json;
	if (StringToJson(msg.attach_, json) && json.isObject())
	{
		//int sub_type = json["type"].asInt();
		Json::Value datajson = json["data"];
		std::string name = datajson["content"].asString();
		std::string personCardId = datajson["personCardId"].asString();
		//std::string img = datajson["title"].asString();

		std::wstring account = nbase::StringPrintf(ui::MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_PROFILE_FORM_ACCOUNT_").c_str(), nbase::UTF8ToUTF16(personCardId).c_str());
		std::wstring nickname = nbase::StringPrintf(ui::MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_PROFILE_FORM_NICKNAME_").c_str(), nbase::UTF8ToUTF16(name).c_str());
		visitingcard_account_->SetText(account);
		visitingcard_name_->SetText(nickname);

		if (msg.session_type_ == nim::kNIMSessionTypeP2P)
		{
			std::wstring title = ui::MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_SESSION_PERSONAL_VISITING_CARD_MSG_TITLE");
			visitingcard_title_->SetText(title);
		}
		std::list<std::string>& account_list = std::list<std::string>(1, personCardId);
		nim::User::GetUserNameCardCallback cb1 = ToWeakCallback([this, account_list](const std::list<nim::UserNameCard> &json_result)
		{
			std::set<std::string> not_get_set(account_list.cbegin(), account_list.cend());
			for (auto& card : json_result)
			{
				userInfo_ = card;
				
				std::string url = card.GetIconUrl();
				//card_heard_img_->SetUTF8BkImage(icon);
				//std::wstring photo_path = GetPhotoDir(kUser) + nbase::UTF8ToUTF16(QString::GetMd5(url));
				if (url == "")
				{
					break;
				}
				std::wstring dir = QPath::GetUserAppDataDir(card.GetAccId()) + L"photo_temp\\";
				if (!nbase::FilePathIsExist(dir, true))
					nbase::win32::CreateDirectoryRecursively(dir.c_str());
				std::wstring namePath = dir + nbase::UTF8ToUTF16(QString::GetMd5(url));
				if (!nbase::FilePathIsExist(namePath, false))
				{
					DownLoadImage(url, namePath);
				}
				else
				{
					//card_heard_img_->SetBkImage(namePath);
					card_heard_img_->SetStateImage(kControlStateNormal, namePath);
				}
				
				//Post2GlobalMisc(nbase::Bind(&MsgBubbleVisitingCard::DownLoadImage, this, url, namePath));
			}

		});
		nim::User::GetUserNameCardOnline(account_list, cb1);

	}
	
}
long URLDownloadToPhotoFile(std::string url, std::wstring name)
{
	size_t len = url.length();//获取字符串长度
	int nmlen = MultiByteToWideChar(CP_ACP, 0, url.c_str(), len + 1, NULL, 0);//如果函数运行成功，并且cchWideChar为零，

	//返回值是接收到待转换字符串的缓冲区所需求的宽字符数大小。
	wchar_t* buffer = new wchar_t[nmlen];
	MultiByteToWideChar(CP_ACP, 0, url.c_str(), len + 1, buffer, nmlen);

	return URLDownloadToFile(0, buffer, name.data(), 0, 0);
}
void MsgBubbleVisitingCard::DownLoadImage(std::string url, std::wstring path)
{
	URLDownloadToPhotoFile(url, path);
	//card_heard_img_->SetBkImage(path);
	card_heard_img_->SetStateImage(kControlStateNormal, path);
}
}