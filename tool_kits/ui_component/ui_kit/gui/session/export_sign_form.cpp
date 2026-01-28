#include "export_sign_form.h"
//#include "module/emoji/richedit_util.h"
//#include "module/login/login_manager.h"
//#include "export/nim_ui_user_config.h"
#include "shared/modal_wnd/file_dialog_ex.h"

using namespace ui;

namespace nim_comp
{
const LPCTSTR ExportSignForm::kClassName = L"ExportSignForm";

ExportSignForm::ExportSignForm()
{
}

ExportSignForm::~ExportSignForm()
{

}

std::wstring ExportSignForm::GetSkinFolder()
{
	return L"session";
}

std::wstring ExportSignForm::GetSkinFile()
{
	return L"export_sign_form.xml";
}

std::wstring ExportSignForm::GetWindowClassName() const
{
	return ExportSignForm::kClassName;
}

std::wstring ExportSignForm::GetWindowId() const
{
	return ExportSignForm::kClassName;
}

UINT ExportSignForm::GetClassStyle() const
{
	return (UI_CLASSSTYLE_FRAME | CS_DBLCLKS);
}

void ExportSignForm::InitWindow()
{

	path_edit_ = (RichEdit*)FindControl(L"path_edit");

	m_pRoot->AttachBubbledEvent(ui::kEventClick, nbase::Bind(&ExportSignForm::OnClicked, this, std::placeholders::_1));
	start_year_combo = static_cast<ui::Combo*>(FindControl(L"start_year"));
	start_month_combo = static_cast<ui::Combo*>(FindControl(L"start_month"));
	start_day_combo = static_cast<ui::Combo*>(FindControl(L"start_day"));

	end_year_combo = static_cast<ui::Combo*>(FindControl(L"end_year"));
	end_month_combo = static_cast<ui::Combo*>(FindControl(L"end_month"));
	end_day_combo = static_cast<ui::Combo*>(FindControl(L"end_day"));

	start_year_combo->AttachSelect(nbase::Bind(&ExportSignForm::OnStartTimeComboSelect, this, std::placeholders::_1));
	start_month_combo->AttachSelect(nbase::Bind(&ExportSignForm::OnStartTimeComboSelect, this, std::placeholders::_1));
	end_year_combo->AttachSelect(nbase::Bind(&ExportSignForm::OnEndTimeComboSelect, this, std::placeholders::_1));
	end_month_combo->AttachSelect(nbase::Bind(&ExportSignForm::OnEndTimeComboSelect, this, std::placeholders::_1));
	
	((Option*)FindControl(L"option_today"))->Selected(true);
	
	InitTimeCombo();

	int birth[3] = { 0, 0, 0 }; //生日
	size_t pos = std::string::npos;
	int count = 0;
	std::string time = "";
	do
	{
		size_t temp = pos + 1;
		pos = time.find_first_of('-', temp);
		std::string num_str = time.substr(temp, pos - temp);
		if (!num_str.empty())
			birth[count] = std::stoi(num_str);
		count++;
	} while (pos != std::string::npos && count < 3);
	int this_year = nbase::Time::Now().ToTimeStruct(true).year();
	if (birth[0] >= 1900 && birth[0] < this_year && start_year_combo->GetItemAt(this_year - birth[0] - 1) != NULL)
		start_year_combo->SelectItem(this_year - birth[0] - 1);
	if (birth[1] >= 1 && birth[1] <= 12 && start_month_combo->GetItemAt(birth[1] - 1) != NULL)
		start_month_combo->SelectItem(birth[1] - 1);
	if (birth[2] >= 1 && birth[2] <= 31 && start_day_combo->GetItemAt(birth[2] - 1) != NULL)
		start_day_combo->SelectItem(birth[2] - 1);
	OnStartTimeComboSelect(NULL);
	OnEndTimeComboSelect(NULL);
}

void ExportSignForm::SetCallBack(OnSearchSignDataCallBack cb)
{
	cb_search_ = cb;
}

void ExportSignForm::InitTimeCombo()
{
	start_year_combo->RemoveAll();
	start_month_combo->RemoveAll();
	start_day_combo->RemoveAll();

	end_year_combo->RemoveAll();
	end_month_combo->RemoveAll();
	end_day_combo->RemoveAll();

	auto create_elem = [](int i)
	{
		ui::ListContainerElement* elem = new ui::ListContainerElement;
		elem->SetClass(L"listitem");
		elem->SetFixedHeight(30);
		elem->SetTextPadding(ui::UiRect(10, 1, 1, 1));
		elem->SetText(nbase::IntToString16(i));
		elem->SetFont(2);
		elem->SetStateTextColor(ui::kControlStateNormal, L"profile_account");
		return elem;
	};

	int this_year = nbase::Time::Now().ToTimeStruct(true).year();
	for (int i = this_year; i >= 1900; i--)
		start_year_combo->Add(create_elem(i));
	for (int i = 1; i <= 12; i++)
		start_month_combo->Add(create_elem(i));
	for (int i = 1; i <= 31; i++)
		start_day_combo->Add(create_elem(i));

	//int this_year = nbase::Time::Now().ToTimeStruct(true).year();
	for (int i = this_year; i >= 1900; i--)
		end_year_combo->Add(create_elem(i));
	for (int i = 1; i <= 12; i++)
		end_month_combo->Add(create_elem(i));
	for (int i = 1; i <= 31; i++)
		end_day_combo->Add(create_elem(i));
}

bool ExportSignForm::OnStartTimeComboSelect(ui::EventArgs* args)
{
	ui::ListContainerElement* cur_year = (ui::ListContainerElement*)start_year_combo->GetItemAt(start_year_combo->GetCurSel());
	ui::ListContainerElement* cur_month = (ui::ListContainerElement*)start_month_combo->GetItemAt(start_month_combo->GetCurSel());
	if (cur_year == NULL || cur_month == NULL)
		return false;

	int year = std::stoi(cur_year->GetText());
	int month = std::stoi(cur_month->GetText());

	std::set<int> big_months({ 1, 3, 5, 7, 8, 10, 12 });
	std::set<int> small_months({ 4, 6, 9, 11 });
	if (big_months.find(month) != big_months.cend()) //大月
	{
		for (int i = 28; i < 31; i++)
		{
			auto item = start_day_combo->GetItemAt(i);
			if (item) item->SetVisible(true);
		}
	}
	else if (small_months.find(month) != small_months.cend()) //小月
	{
		for (int i = 28; i < 30; i++)
		{
			auto item = start_day_combo->GetItemAt(i);
			if (item) item->SetVisible(true);
		}
		auto item = start_day_combo->GetItemAt(30);
		if (item)
			item->SetVisible(false);

		if (start_day_combo->GetCurSel() > 29)
			start_day_combo->SelectItem(29);
	}
	else //二月
	{
		for (int i = 29; i < 31; i++)
		{
			auto item = start_day_combo->GetItemAt(i);
			if (item) item->SetVisible(false);
		}

		auto item = start_day_combo->GetItemAt(28);
		if (item)
		{
			bool bissextile = (year % 4 == 0 && year % 100 != 0 || year % 400 == 0);
			start_day_combo->GetItemAt(28)->SetVisible(bissextile);

			if (start_day_combo->GetCurSel() > 27 + bissextile)
				start_day_combo->SelectItem(27 + bissextile);
		}
	}

	return true;
}
bool ExportSignForm::OnEndTimeComboSelect(ui::EventArgs* args)
{
	ui::ListContainerElement* cur_year = (ui::ListContainerElement*)end_year_combo->GetItemAt(end_year_combo->GetCurSel());
	ui::ListContainerElement* cur_month = (ui::ListContainerElement*)end_month_combo->GetItemAt(end_month_combo->GetCurSel());
	if (cur_year == NULL || cur_month == NULL)
		return false;

	int year = std::stoi(cur_year->GetText());
	int month = std::stoi(cur_month->GetText());

	std::set<int> big_months({ 1, 3, 5, 7, 8, 10, 12 });
	std::set<int> small_months({ 4, 6, 9, 11 });
	if (big_months.find(month) != big_months.cend()) //大月
	{
		for (int i = 28; i < 31; i++)
		{
			auto item = end_day_combo->GetItemAt(i);
			if (item) item->SetVisible(true);
		}
	}
	else if (small_months.find(month) != small_months.cend()) //小月
	{
		for (int i = 28; i < 30; i++)
		{
			auto item = end_day_combo->GetItemAt(i);
			if (item) item->SetVisible(true);
		}
		auto item = end_day_combo->GetItemAt(30);
		if (item)
			item->SetVisible(false);

		if (end_day_combo->GetCurSel() > 29)
			end_day_combo->SelectItem(29);
	}
	else //二月
	{
		for (int i = 29; i < 31; i++)
		{
			auto item = end_day_combo->GetItemAt(i);
			if (item) item->SetVisible(false);
		}

		auto item = end_day_combo->GetItemAt(28);
		if (item)
		{
			bool bissextile = (year % 4 == 0 && year % 100 != 0 || year % 400 == 0);
			end_day_combo->GetItemAt(28)->SetVisible(bissextile);

			if (end_day_combo->GetCurSel() > 27 + bissextile)
				end_day_combo->SelectItem(27 + bissextile);
		}
	}

	return true;
}

bool ExportSignForm::OnClicked(ui::EventArgs* param)
{
	std::wstring name = param->pSender->GetName();
	if (name == L"export_ok")
	{
		SearchSignInData();
	}
	else if (name == L"export_cancel")
	{
		this->Close();
	}
	else if (name == L"btn_sel")
	{
		SelectPath();
	}
	return true;
}

void ExportSignForm::SearchSignInData()
{
	std::string filePath = path_edit_->GetUTF8Text();
	if (filePath.empty())
	{
		ShowMsgBox(this->GetHWND(), MsgboxCallback(), L"STRID_MSGLOG_MANAGE_SELECT_EXPORT_PATH", true);
		return;
	}
	if (((Option*)FindControl(L"option_today"))->IsSelected())
	{
		nbase::Time::TimeStruct time = nbase::Time::Now().ToTimeStruct(true);
		int cur_year = time.year();
		int cur_month = time.month();
		int cur_day = time.day_of_month();
		std::string cur_date = nbase::StringPrintf("%04d%02d%02d", cur_year, cur_month, cur_day);
		Post2UI(nbase::Bind(cb_search_, cur_date, cur_date, filePath));
		return;
	}
	std::string start_date, end_date;
	if (start_year_combo->GetCurSel() != -1 && start_month_combo->GetCurSel() != -1 && start_day_combo->GetCurSel() != -1)
	{
		int this_year = nbase::Time::Now().ToTimeStruct(true).year();
		int start_year = this_year - start_year_combo->GetCurSel();
		int start_month = start_month_combo->GetCurSel() + 1;
		int start_day = start_day_combo->GetCurSel() + 1;
		start_date = nbase::StringPrintf("%04d%02d%02d", start_year, start_month, start_day);
	}
	else
	{
		ShowMsgBox(this->GetHWND(), MsgboxCallback(), L"STRID_TEAM_SIGN_IN_EXPORT_TIME_TIP", true);
		return;
	}
	if (end_year_combo->GetCurSel() != -1 && end_month_combo->GetCurSel() != -1 && end_day_combo->GetCurSel() != -1)
	{
		int this_year = nbase::Time::Now().ToTimeStruct(true).year();
		int end_year = this_year - end_year_combo->GetCurSel();
		int end_month = end_month_combo->GetCurSel() + 1;
		int end_day = end_day_combo->GetCurSel() + 1;
		end_date = nbase::StringPrintf("%04d%02d%02d", end_year, end_month, end_day);
	}
	else
	{
		ShowMsgBox(this->GetHWND(), MsgboxCallback(), L"STRID_TEAM_SIGN_IN_EXPORT_TIME_TIP", true);
		return;
	}
	if (start_date > end_date)
	{
		ShowMsgBox(this->GetHWND(), MsgboxCallback(), L"STRID_TEAM_SIGN_IN_EXPORT_TIME_TIP", true);
		return;
	}
	Post2UI(nbase::Bind(cb_search_, start_date, end_date, filePath));
	this->Close();
}

void ExportSignForm::SelectPath()
{
	open_file_ = true;
	std::wstring file_type = MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_MSGLOG_MANAGE_FORMAT");
	std::wstring text = nbase::StringPrintf(L"%s(*.csv)", file_type.c_str());
	std::wstring file_exten = L".csv";
	std::wstring file_name;
	//nbase::FilePathExtension(file_name, file_exten);
	// 要保存的文件名
	CFileDialogEx* file_dlg = new CFileDialogEx();
	std::map<LPCTSTR, LPCTSTR> filters;
	filters[text.c_str()] = L"*.csv";
	file_dlg->SetFilter(filters);
	file_dlg->SetFileName(file_name.c_str());
	file_dlg->SetDefExt(file_exten.c_str());
	file_dlg->SetParentWnd(GetHWND());
	// 弹出非模态对话框
	CFileDialogEx::FileDialogCallback2 callback2 = nbase::Bind(&ExportSignForm::OnSelectPathCallback, this, std::placeholders::_1, std::placeholders::_2);
	file_dlg->AyncShowSaveFileDlg(callback2);
}

void ExportSignForm::OnSelectPathCallback(BOOL ret, std::wstring file_path)
{
	open_file_ = false;
	if (ret)
	{
		path_edit_->SetText(file_path);
	}
}
}