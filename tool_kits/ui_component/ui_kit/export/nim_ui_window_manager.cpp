#include "nim_ui_window_manager.h"
#include "module/login/login_manager.h"
#include "shared/ui/toast/toast.h"
#include "gui/contact_select_sendmsg/contact_select_sendmsg.h"
#include "tool_kits/base/callback/callback.h"
#include "gui/bookmarks/bookmarks_form.h"

namespace nim_ui
{

nim_comp::WindowList WindowsManager::GetAllWindows()
{
	return nim_comp::WindowsManager::GetInstance()->GetAllWindows();
}

nim_comp::WindowEx* WindowsManager::GetWindow(const std::wstring &wnd_class_name, const std::wstring &wnd_id)
{
	return nim_comp::WindowsManager::GetInstance()->GetWindow(wnd_class_name, wnd_id);
}

nim_comp::WindowList WindowsManager::GetWindowsByClassName(LPCTSTR classname)
{
	return nim_comp::WindowsManager::GetInstance()->GetWindowsByClassName(classname);
}

void WindowsManager::DestroyAllWindows()
{
	return nim_comp::WindowsManager::GetInstance()->DestroyAllWindows();
}

void WindowsManager::ShowProfileForm(UTF8String uid)
{
	nim_comp::ProfileForm::ShowProfileForm(uid);
}

void WindowsManager::ShowLinkForm()
{
	nim_comp::ShowLinkForm((nim::NIMResCode)nim_comp::LoginManager::GetInstance()->GetErrorCode(), true);
}

void WindowsManager::ShowVideoSettingForm()
{
	nim_comp::VideoManager::GetInstance()->ShowVideoSetting();
}

void WindowsManager::ShowShortcutsSettingForm()
{
	nim_comp::ShortcutsManager::GetInstance()->ShowShortcutsSetting();
}

void WindowsManager::ShowGroupSendForm()
{
	//nim_comp::ShortcutsManager::GetInstance()->ShowShortcutsSetting();
	nim_comp::ContactSelectSendForm *contact_select_send_form = (nim_comp::ContactSelectSendForm *)nim_comp::WindowsManager::GetInstance()->GetWindow\
		(nim_comp::ContactSelectSendForm::kClassName, nbase::UTF8ToUTF16(nim_comp::ContactSelectSendForm::kCreateGroup));

	if (!contact_select_send_form)
	{
		std::string my_id = nim_comp::LoginManager::GetInstance()->GetAccount();
		std::list<UTF8String> exnclude_ids;
		nim_comp::ContactSelectSendForm::SelectedCompletedCallback completedCallback;
		contact_select_send_form = new nim_comp::ContactSelectSendForm(my_id, exnclude_ids, completedCallback);// nbase::Bind(&TeamInfoForm::SelectedCompleted, this, std::placeholders::_1, std::placeholders::_2, create_or_display_));
		contact_select_send_form->Create(NULL, L"", UI_WNDSTYLE_FRAME& ~WS_MAXIMIZEBOX, 0L);
		contact_select_send_form->CenterWindow();
	}

	contact_select_send_form->ActiveWindow();
}

void WindowsManager::ShowBookmarksForm()
{
	//ShowMsgBox(wnd->GetHWND(), MsgboxCallback(), content, false);
	//WindowsManager::SingletonShow<MainForm>(MainForm::kClassName);

	/*nim_comp::BookmarksForm *bookmarks_form = (nim_comp::BookmarksForm *)GetWindow(nim_comp::BookmarksForm::kClassName, nim_comp::BookmarksForm::kClassName);

	if (!bookmarks_form)
	{
		std::string my_id = nim_comp::LoginManager::GetInstance()->GetAccount();
		std::list<UTF8String> exnclude_ids;
		nim_comp::ContactSelectSendForm::SelectedCompletedCallback completedCallback;
		bookmarks_form = new nim_comp::BookmarksForm();//(my_id, exnclude_ids, completedCallback);// nbase::Bind(&TeamInfoForm::SelectedCompleted, this, std::placeholders::_1, std::placeholders::_2, create_or_display_));
		//bookmarks_form->Create(NULL, L"", UI_WNDSTYLE_FRAME& ~WS_MAXIMIZEBOX, 0L);
		//bookmarks_form->CenterWindow();
	}

	bookmarks_form->ActiveWindow();*/
	nim_ui::WindowsManager::GetInstance()->SingletonShow<nim_comp::BookmarksForm>(nim_comp::BookmarksForm::kClassName);
}

void ShowToast(const std::wstring &content, int duration, HWND parent)
{
	shared::Toast::ShowToast(content, duration, parent);
}

}

