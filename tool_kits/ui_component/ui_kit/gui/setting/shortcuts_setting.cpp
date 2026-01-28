#include "shortcuts_setting.h"
#include "module/shortcuts/shortcuts_manager.h"

using namespace ui;

namespace nim_comp
{
const LPCTSTR ShortcutsSettingForm::kClassName = L"ShortcutsSettingForm";

ShortcutsSettingForm::ShortcutsSettingForm()
{
	//camera_is_open_ = false;
}

ShortcutsSettingForm::~ShortcutsSettingForm()
{
}

std::wstring ShortcutsSettingForm::GetSkinFolder()
{
	return L"setting";
}

std::wstring ShortcutsSettingForm::GetSkinFile()
{
	return L"shortcuts_setting.xml";
}


void ShortcutsSettingForm::InitWindow()
{
	send_select_text_ = (Combo*)FindControl(L"send_select_text");
	m_pRoot->AttachBubbledEvent(ui::kEventAll, nbase::Bind(&ShortcutsSettingForm::Notify, this, std::placeholders::_1));

	Screenshots_key_ = (RichEdit*)FindControl(L"Screenshots_select_text");
	Screenshots_key_->AttachBubbledEvent(ui::kEventAll, nbase::Bind(&ShortcutsSettingForm::ScreenshotsNotify, this, std::placeholders::_1));

	PopMsg_key_ = (RichEdit*)FindControl(L"popmsg_select_text");
	PopMsg_key_->AttachBubbledEvent(ui::kEventAll, nbase::Bind(&ShortcutsSettingForm::PopMsgNotify, this, std::placeholders::_1));
}

bool ShortcutsSettingForm::ScreenshotsNotify(ui::EventArgs* param)
{
	static bool isAlt = false;
	static bool isCtrl = false;

	QLOG_APP(L"param msg: param->chKey={0}  param->Type={1}") << param->chKey << param->Type;

	if (param->chKey == 18 && param->Type == 8)
	{
		isAlt = true;
		isCtrl = false;
	}
	else if (param->chKey == 17 && param->Type == 8)
	{
		isAlt = false;
		isCtrl = true;
	}
	else
	{
		if (param->chKey >= 65 && param->chKey <= 90 && param->Type == 8)
		{
			if (isAlt)
			{
				default_Screenshots_key = "Ctrl+Alt+";
			}
			else if (isCtrl)
			{
				default_Screenshots_key = "Ctrl+";
			}
			char a[2];
			a[0] = param->chKey;
			a[1] = '\0';
			default_Screenshots_char = a;
			std::wstring str = nbase::UTF8ToUTF16(default_Screenshots_key + default_Screenshots_char);
			if ((default_Screenshots_key + default_Screenshots_char) == (default_PopMsg_key + default_PopMsg_char))
			{
				PopMsg_key_->SetText(L"");
				return false;
			}
			Screenshots_key_->SetText(str);
			ShortcutsManager::GetInstance()->SaveShortcutsSetting(default_Screenshots_key + default_Screenshots_char, default_PopMsg_key + default_PopMsg_char);
		}
		
		if (param->Type == kEventMouseButtonDown)
		{
			Screenshots_key_->SetText(L"");
		}
		else if (param->Type == kEventKillFocus)
		{
			std::wstring str = nbase::UTF8ToUTF16(default_Screenshots_key + default_Screenshots_char);
			Screenshots_key_->SetText(str);
		}
		isAlt = false;
		isCtrl = false;
	}
	return true;
}

bool ShortcutsSettingForm::PopMsgNotify(ui::EventArgs* param)
{
	static bool isAlt = false;
	static bool isCtrl = false;
	if (param->chKey == 18 && param->Type == 8)
	{
		isAlt = true;
		isCtrl = false;
	}
	else if (param->chKey == 17 && param->Type == 8)
	{
		isAlt = false;
		isCtrl = true;
	}
	else
	{
		if (param->chKey >= 65 && param->chKey <= 90 && param->Type == 8)
		{
			if (isAlt)
			{
				default_PopMsg_key = "Ctrl+Alt+";
			}
			else if (isCtrl)
			{
				default_PopMsg_key = "Ctrl+";
			}
			char a[2];
			a[0] = param->chKey;
			a[1] = '\0';
			default_PopMsg_char = a;
			std::wstring str = nbase::UTF8ToUTF16(default_PopMsg_key + default_PopMsg_char);
			if ((default_Screenshots_key + default_Screenshots_char) == (default_PopMsg_key + default_PopMsg_char))
			{
				PopMsg_key_->SetText(L"");
				return false;
			}
			PopMsg_key_->SetText(str);
			ShortcutsManager::GetInstance()->SaveShortcutsSetting(default_Screenshots_key + default_Screenshots_char, default_PopMsg_key + default_PopMsg_char);
		}

		if (param->Type == kEventMouseButtonDown)
		{
			PopMsg_key_->SetText(L"");
		}
		else if (param->Type ==kEventKillFocus)
		{
			std::wstring str = nbase::UTF8ToUTF16(default_PopMsg_key + default_PopMsg_char);
			PopMsg_key_->SetText(str);
		}
		isAlt = false;
		isCtrl = false;
	}
	return true;
}


bool ShortcutsSettingForm::Notify(ui::EventArgs* msg)
{
	std::wstring name = msg->pSender->GetName();
	if(msg->Type == ui::kEventSelect)
	{
		if(name == L"send_select_text")
		{
			int k = send_select_text_->GetCurSel();
			ASSERT(k >= 0 && k < send_select_text_->GetCount());

			ListContainerElement* label = (ListContainerElement*)send_select_text_->GetItemAt(k);
			//string text = label->GetText();
			ChangeSendButtonStatus(label->GetUTF8DataID());
		}
	}
	return true;
}

void ShortcutsSettingForm::InitEditList()
{
	Combo* combo = NULL;
	combo = send_select_text_;
	send_button_status_ = false;
	ListContainerElement* label = new ListContainerElement;
	label->SetClass(L"listitem");
	label->SetFixedHeight(30);
	label->SetTextPadding(UiRect(10, 1, 30, 1));
	label->SetUTF8Text("Enter");
	label->SetUTF8DataID("Enter");
	combo->Add(label);

	label = new ListContainerElement;
	label->SetClass(L"listitem");
	label->SetFixedHeight(30);
	label->SetTextPadding(UiRect(10, 1, 30, 1));
	label->SetUTF8Text("Ctrl + Enter");
	label->SetUTF8DataID("Ctrl + Enter");
	combo->Add(label);
	std::list<MEDIA_SHORTCUTS_EDIT_INFO> shortcuts;

	shortcuts = ShortcutsManager::GetInstance()->GetEditInfo();
	for (auto i = shortcuts.begin(); i != shortcuts.end(); i++)
	{
		if (i->type == "send")
		{
			if (i->value == "Enter")
			{
				combo->SelectItem(0);
				send_button_status_ = false;
			}
			else
			{
				combo->SelectItem(1);
				send_button_status_ = true;
			}
		}
		else if (i->type == "Screenshots")
		{
			if (std::string::npos == i->value.find("Alt"))
			{ 
				default_Screenshots_key = "Ctrl+";
			}
			else
			{
				default_Screenshots_key = "Ctrl+Alt+";
			}
			default_Screenshots_char = i->value.substr(i->value.size() - 1);
			//std::wstring str = nbase::UTF8ToUTF16(i->value);
			Screenshots_key_->SetUTF8Text(i->value);

			//调用注册热键函数

		}
		else if (i->type == "PopMsg")
		{
			if (std::string::npos == i->value.find("Alt"))
			{
				default_PopMsg_key = "Ctrl+";
			}
			else
			{
				default_PopMsg_key = "Ctrl+Alt+";
			}
			default_PopMsg_char = i->value.substr(i->value.size() - 1);
			//std::wstring str = nbase::UTF8ToUTF16(i->value);
			//PopMsg_key_->SetText(str);
			PopMsg_key_->SetUTF8Text(i->value);

			//调用注册热键函数

		}
	}
	combo->SetEnabled(true);
}

bool ShortcutsSettingForm::getSendButtonStatus()
{
	return send_button_status_;
}

void ShortcutsSettingForm::ShowPage()
{
	InitEditList();
}

void ShortcutsSettingForm::ChangeSendButtonStatus(const std::string text)
{
	ShortcutsManager::GetInstance()->StartSendShortcuts(text);
}

}