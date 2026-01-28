#include "stdafx.h"
#include "../../../tool_kits/shared/closure.h"
#include "shortcutMsgBoxEx.h" 
//#include "../../../tool_kits/ui_component/ui_kit/guiex/main/main_form_ex.h"

using namespace ui;

const LPCTSTR shortcutMsgBoxEx::kClassName = L"shortcutMsgBoxEx";
shortcutMsgBoxEx *shortcutMsgBoxEx::m_instence = NULL;

shortcutMsgBoxEx::shortcutMsgBoxEx()
{
	isGetMsg = false;
	iScreenshotsTag = 0;
	iPopMsgTag = iScreenshotsTag + 1;
}

shortcutMsgBoxEx::~shortcutMsgBoxEx()
{
	HWND hConsole = GetActiveWindow();
	UnregisterHotKey(hConsole, 1);
	UnregisterHotKey(hConsole, 2);
}

shortcutMsgBoxEx *shortcutMsgBoxEx::getInstence()
{
	if (NULL == m_instence)
	{
		m_instence = new shortcutMsgBoxEx();
	}
	return m_instence;
}

std::wstring shortcutMsgBoxEx::GetSkinFolder()
{
	return L"msgbox";
}

std::wstring shortcutMsgBoxEx::GetSkinFile()
{
	return L"shortcutMsgBox.xml";
}

ui::UILIB_RESOURCETYPE shortcutMsgBoxEx::GetResourceType() const
{
	return ui::UILIB_FILE;
}

std::wstring shortcutMsgBoxEx::GetZIPFileName() const
{
	return (L"msgbox.zip");
}

std::wstring shortcutMsgBoxEx::GetWindowClassName() const
{
	return kClassName;
}

std::wstring shortcutMsgBoxEx::GetWindowId() const
{
	return kClassName;
}

UINT shortcutMsgBoxEx::GetClassStyle() const
{
	return (UI_CLASSSTYLE_FRAME | CS_DBLCLKS);
}

LRESULT shortcutMsgBoxEx::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return __super::HandleMessage(uMsg, wParam, lParam);
}

void shortcutMsgBoxEx::OnEsc(BOOL &bHandled)
{
	bHandled = TRUE;
}
void shortcutMsgBoxEx::Close(UINT nRet)
{
	// 提示框关闭之前先Enable父窗口，防止父窗口隐到后面去。
	HWND hWndParent = GetWindowOwner(m_hWnd);
	if (hWndParent)
	{
		::EnableWindow(hWndParent, TRUE);
		::SetFocus(hWndParent);
	}

	__super::Close(nRet);
}

void shortcutMsgBoxEx::InitWindow()
{

}

void shortcutMsgBoxEx::reRegisterShortcuts(std::string &screenshots, std::string &popMsg)
{
	isGetMsg = false;
	HWND hConsole = GetActiveWindow();
	UnregisterHotKey(hConsole, 1);
	UnregisterHotKey(hConsole, 2);
	registerShortcuts(screenshots, popMsg);
	setReceivedMsg();
}

void shortcutMsgBoxEx::unRegisterShortcuts()
{
	HWND hConsole = GetActiveWindow();
	UnregisterHotKey(hConsole, iScreenshotsTag);
	UnregisterHotKey(hConsole, iPopMsgTag);
}

void shortcutMsgBoxEx::registerShortcuts(std::string &screenshots, std::string &popMsg)
{
	if (isGetMsg)
	{
		isGetMsg = false;
		HWND hConsole = GetActiveWindow();
		UnregisterHotKey(hConsole, iScreenshotsTag);
		UnregisterHotKey(hConsole, iPopMsgTag);
		iScreenshotsTag = iPopMsgTag + 1;
		iPopMsgTag = iScreenshotsTag + 1;
	}
	if (screenshots == popMsg)
	{
		return;
	}
	isGetMsg = true;
	HWND hConsole = GetActiveWindow();
	char screenshotsKey = *(screenshots.end() - 1);
	char popMsgKey = *(popMsg.end() - 1);
	if (!(std::string::npos == screenshots.find("Ctrl")))
	{
		if (!(std::string::npos == screenshots.find("Alt")))
		{
			RegisterHotKey(hConsole, iScreenshotsTag, MOD_CONTROL | MOD_ALT, screenshotsKey);
		}
		else
		{
			RegisterHotKey(hConsole, iScreenshotsTag, MOD_CONTROL, screenshotsKey);
		}
	}
	else
	{
		if (!(std::string::npos == screenshots.find("Alt")))
		{
			RegisterHotKey(hConsole, iScreenshotsTag, MOD_ALT, screenshotsKey);
		}
		else
		{
			RegisterHotKey(hConsole, iScreenshotsTag, MOD_CONTROL | MOD_ALT, 'A');
		}
		
	}

	if (!(std::string::npos == popMsg.find("Ctrl")))
	{
		if (!(std::string::npos == popMsg.find("Alt")))
		{
			RegisterHotKey(hConsole, iPopMsgTag, MOD_CONTROL | MOD_ALT, popMsgKey);
		}
		else
		{
			RegisterHotKey(hConsole, iPopMsgTag, MOD_CONTROL, popMsgKey);
		}
	}
	else
	{
		if (!(std::string::npos == popMsg.find("Alt")))
		{
			RegisterHotKey(hConsole, iPopMsgTag, MOD_ALT, popMsgKey);
		}
		else
		{
			RegisterHotKey(hConsole, iPopMsgTag, MOD_CONTROL | MOD_ALT, 'A');
		}

	}
}
	
void shortcutMsgBoxEx::setReceivedMsg()
{
	MSG msg = { 0 };
	//HWND hConsole = GetActiveWindow();
	while (GetMessage(&msg, NULL, 0, 0) != 0 && isGetMsg)
	{
		// 当收到快捷键消息时
		if (msg.message == WM_HOTKEY)
		{
			std::wstring mainClassName = L"MainFormEx";
			if (msg.wParam == iScreenshotsTag)
			{
				//((MainFormEx *)(nim_ui::WindowsManager::GetInstance()->GetWindow(mainClassName, mainClassName)))->starClip();
				Post2UI(nbase::Bind(shortcut_cb_));
			}
			else if (msg.wParam == iPopMsgTag)
			{
				//((MainFormEx *)(nim_ui::WindowsManager::GetInstance()->GetWindow(mainClassName, mainClassName)))->showSessionBox();
				Post2UI(nbase::Bind(pop_msg_cb_));
			}
		}
	}
}

void shortcutMsgBoxEx::setCallbackFuntion(ShortcutCallBack callback1, ShortcutCallBack callback2)
{
	pop_msg_cb_ = callback2;
	shortcut_cb_ = callback1;
}
