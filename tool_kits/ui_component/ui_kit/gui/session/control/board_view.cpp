#include "board_view.h"
#include "module/session/force_push_manager.h"

using namespace ui;
namespace nim_comp
{
void BoardView::InitControl()
{
	GlobalManager::FillBoxWithCache(this, L"session/board_view.xml");

	content_ = static_cast<RichEdit*>(FindSubControl(L"msg_body"));
	sender_name_ = static_cast<Label*>(FindSubControl(L"sender_name"));
}

void BoardView::AddMessage(const BoardInfo &at_me_info, const bool tag)
{
	if (at_me_info.uuid.empty() || at_me_info.sender_name.empty() || at_me_info.msg_body.empty())
		return;

	at_me_info_.push_back(at_me_info);
	UpdateView(tag);
}

void BoardView::UpdateView(const bool tag)
{
	if (at_me_info_.size() == 0)
	{
		SetVisible(false);
		return;
	}
	BoardInfo item = at_me_info_[at_me_info_.size() - 1];
	std::string title = "";
	std::string content = "";
	if (!tag)
	{
		std::string strBoard = item.msg_body;
		int pos = strBoard.find(":");
		if (std::string::npos != pos)
		{
			title = strBoard.substr(0, pos + 1);
			content = strBoard.substr(pos + 1, strBoard.length() - pos);
		}
		else
		{
			content = strBoard;
		}
		std::wstring show_text = nbase::UTF8ToUTF16(title);
		//show_text.append(L" : ");
		sender_name_->SetText(show_text);
		content_->SetUTF8Text(content);
		SetVisible(true);
		return;
	}
	
	Json::Value json;
	if (StringToJson(item.msg_body, json) && json.isObject())
	{
		Json::Value tinfo_json = json[nim::kNIMNotificationKeyData][nim::kNIMNotificationKeyTeamInfo];
		if (tinfo_json.isMember(nim::kNIMTeamInfoKeyAnnouncement))
		{
			
			std::string tinfo1_json = tinfo_json[nim::kNIMTeamInfoKeyAnnouncement].asString();
			if (StringToJson(tinfo1_json, json) && json.isArray())
			{
				Json::Value &broad = json;
				for (int i = 0; i < (int)broad.size(); i++)
				{
					title = broad[i]["title"].asString();
					content = broad[i]["content"].asString();
				}
			}
			if (!title.empty() && !content.empty())
			{
				std::wstring show_text = nbase::UTF8ToUTF16(title);
				show_text.append(L" : ");
				sender_name_->SetText(show_text);
				content_->SetText(nbase::UTF8ToUTF16(content));
				SetVisible(true);
			}
		}
	}

	/*std::wstring show_text = item.sender_name;
	show_text.append(L" : ");

	sender_name_->SetText(show_text);
	//content_->SetText(item.msg_body);

	SetVisible(true);*/
}

void BoardView::ShowNextMessage()
{
	if (at_me_info_.size() > 0)
	{
		at_me_info_.pop_back();
	}

	UpdateView();
}

std::wstring BoardView::GetUuid()
{
	if (at_me_info_.size() == 0)
		return L"";

	BoardInfo item = at_me_info_[at_me_info_.size() - 1];
	return item.uuid;
}

void BoardView::ShowLoadingTip(bool show)
{
	if (show)
	{
		content_->SetText(MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_SESSION_MSG_LOADING"));
		sender_name_->SetVisible(false);
		this->SetEnabled(false);
	}
	else
	{
		sender_name_->SetVisible(true);
		this->SetEnabled(true);
	}
}

UINT BoardView::GetAtMeCount()
{
	return at_me_info_.size();
}
}