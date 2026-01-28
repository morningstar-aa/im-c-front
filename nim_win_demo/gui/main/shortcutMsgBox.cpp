#include "stdafx.h"
#include "../../../tool_kits/shared/closure.h"
#include "shortcutMsgBox.h" 

using namespace ui;

const LPCTSTR shortcutMsgBox::kClassName = L"shortcutMsgBox";
shortcutMsgBox *shortcutMsgBox::m_instence = NULL;

shortcutMsgBox::shortcutMsgBox()
{
	isGetMsg = false;
	iScreenshotsTag = 0;
	iPopMsgTag = iScreenshotsTag + 1;
}

shortcutMsgBox::~shortcutMsgBox()
{
	HWND hConsole = GetActiveWindow();
	UnregisterHotKey(hConsole, 1);
	UnregisterHotKey(hConsole, 2);
}

shortcutMsgBox *shortcutMsgBox::getInstence()
{
	if (NULL == m_instence)
	{
		m_instence = new shortcutMsgBox();
	}
	return m_instence;
}

std::wstring shortcutMsgBox::GetSkinFolder()
{
	return L"msgbox";
}

std::wstring shortcutMsgBox::GetSkinFile()
{
	return L"shortcutMsgBox.xml";
}

ui::UILIB_RESOURCETYPE shortcutMsgBox::GetResourceType() const
{
	return ui::UILIB_FILE;
}

std::wstring shortcutMsgBox::GetZIPFileName() const
{
	return (L"msgbox.zip");
}

std::wstring shortcutMsgBox::GetWindowClassName() const
{
	return kClassName;
}

std::wstring shortcutMsgBox::GetWindowId() const
{
	return kClassName;
}

UINT shortcutMsgBox::GetClassStyle() const
{
	return (UI_CLASSSTYLE_FRAME | CS_DBLCLKS);
}

LRESULT shortcutMsgBox::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return __super::HandleMessage(uMsg, wParam, lParam);
}

void shortcutMsgBox::OnEsc( BOOL &bHandled )
{
	bHandled = TRUE;
}
void shortcutMsgBox::Close(UINT nRet)
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

void shortcutMsgBox::InitWindow()
{

}

void shortcutMsgBox::reRegisterShortcuts(std::string &screenshots, std::string &popMsg)
{
	isGetMsg = false;
	HWND hConsole = GetActiveWindow();
	UnregisterHotKey(hConsole, 1);
	UnregisterHotKey(hConsole, 2);
	registerShortcuts(screenshots, popMsg);
	setReceivedMsg();
}

void shortcutMsgBox::unRegisterShortcuts()
{
	HWND hConsole = GetActiveWindow();
	UnregisterHotKey(hConsole, iScreenshotsTag);
	UnregisterHotKey(hConsole, iPopMsgTag);
}

void shortcutMsgBox::registerShortcuts(std::string &screenshots, std::string &popMsg)
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
	
void shortcutMsgBox::setReceivedMsg()
{
	MSG msg = { 0 };
	//HWND hConsole = GetActiveWindow();
	while (GetMessage(&msg, NULL, 0, 0) != 0 && isGetMsg)
	{
		// 当收到快捷键消息时
		if (msg.message == WM_HOTKEY)
		{
			std::wstring mainClassName = L"MainForm";
			if (msg.wParam == iScreenshotsTag)
			{
				((MainForm *)(nim_ui::WindowsManager::GetInstance()->GetWindow(mainClassName, mainClassName)))->starClip();
			}
			else if (msg.wParam == iPopMsgTag)
			{
				((MainForm *)(nim_ui::WindowsManager::GetInstance()->GetWindow(mainClassName, mainClassName)))->showSessionBox();
			}
		}
	}
}
