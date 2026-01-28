#include "emoji_form.h"
#include "module/emoji/emoji_info.h"
#include "export/nim_ui_login_manager.h"
#include <io.h>  
#include "shared/ui/ui_menu.h"

using namespace std;
using namespace ui;
namespace nim_comp
{
const LPCTSTR EmojiForm::kClassName = L"EmojiForm";

EmojiForm::EmojiForm()
{
	only_emoj_ = false;
	is_closing_ = false;
}

EmojiForm::~EmojiForm()
{

}

std::wstring EmojiForm::GetSkinFolder()
{
	return L"emoji";
}

std::wstring EmojiForm::GetSkinFile()
{
	return L"emoji_form.xml";
}

std::wstring EmojiForm::GetWindowClassName() const
{
	return kClassName;
}

std::wstring EmojiForm::GetWindowId() const
{
	return kClassName;
}

UINT EmojiForm::GetClassStyle() const
{
	return (UI_CLASSSTYLE_FRAME | CS_DBLCLKS);
}

void EmojiForm::InitWindow()
{
	m_pRoot->AttachBubbledEvent(ui::kEventSelect, nbase::Bind(&EmojiForm::OnSelChanged, this, std::placeholders::_1));
	emoj_ = (TileBox*) FindControl(L"tile_emoj");

	if (only_emoj_)
	{
		FindControl(L"sticker_vector_container")->SetVisible(false);
	}

	std::vector<emoji::Emoticon> vec;
	emoji::GetEmoticon(vec);
	if( vec.empty() )
		return;

	for(std::vector<emoji::Emoticon>::const_iterator i = vec.begin(); i != vec.end(); i++)
	{
		ButtonBox* box = new ButtonBox;
		GlobalManager::FillBoxWithCache(box, L"emoji/emoji_item.xml");
		emoj_->Add(box);

		box->SetKeyboardEnabled(false);
		box->AttachClick(nbase::Bind(&EmojiForm::OnEmojiClicked, this, std::placeholders::_1));
		std::wstring tag = i->tag;
		assert(tag.size() > 2);
		box->SetToolTipText( tag.substr(1, tag.size() - 2) );

		Control* c = box->FindSubControl(L"ctrl_emoj");
		c->SetBkImage(i->path);
	}
	//AddSticker(L"ajmd", 48);
	//AddSticker(L"lt", 20);
}

LRESULT EmojiForm::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_KILLFOCUS)
	{
		POINT point;
		::GetCursorPos(&point);

		ui::UiRect rect = this->GetPos(true);
		if (!rect.IsPointIn(point))
		{
			this->Close();
		}
	}
	return __super::HandleMessage(uMsg, wParam, lParam);
}

LRESULT EmojiForm::OnClose(UINT u, WPARAM w, LPARAM l, BOOL& bHandled)
{
	if (close_cb_)
	{
		Post2UI(nbase::Bind(close_cb_));
	}
	return __super::OnClose(u, w, l, bHandled);
}

void EmojiForm::Close(UINT nRet /*= IDOK*/)
{
	is_closing_ = true;
	__super::Close(nRet);
}

void EmojiForm::ShowEmoj(POINT pt, OnSelectEmotion sel, OnSelectSticker sel_sticker, OnSelectSticker sel_custom, OnSelectSticker del_custom, OnEmotionClose close_cb, bool only_emoj)
{
	del_custom_cb_ = del_custom;
	sel_custom_cb_ = sel_custom;
	ShowEmoj(pt, sel, sel_sticker, close_cb, only_emoj);
}

void EmojiForm::ShowEmoj(POINT pt, OnSelectEmotion sel, OnSelectSticker sel_sticker, OnEmotionClose close_cb, bool only_emoj)
{
	sel_cb_ = sel;
	sel_sticker_cb_ = sel_sticker;
	close_cb_ = close_cb;
	only_emoj_ = only_emoj;

	HWND hwnd = WindowEx::Create(NULL, L"", WS_POPUP, WS_EX_TOOLWINDOW);
	if (hwnd == NULL)
		return;

	UiRect rc(pt.x, pt.y, 0, 0);
	this->SetPos(rc, false, SWP_NOSIZE | SWP_SHOWWINDOW, HWND_TOPMOST);
}

void EmojiForm::AddSticker(std::wstring name, int num)
{
	std::wstring sticker_id = nbase::StringPrintf(L"sticker_%s", name.c_str());
	ui::TileBox* sticker = (TileBox*)FindControl(sticker_id);
	for (int i = 1; i <= num; i++)
	{
		ButtonBox* box = new ButtonBox;
		if (name == L"ww")
		{
			GlobalManager::FillBoxWithCache(box, L"emoji/sticker_ww.xml");
		}
		else
		{
			GlobalManager::FillBoxWithCache(box, L"emoji/sticker.xml");
		}
		sticker->Add(box);
		sticker->SetDataID(name);

		box->SetKeyboardEnabled(false);
		box->AttachClick(nbase::Bind(&EmojiForm::OnStickerClicked, this, std::placeholders::_1));

		Control* c = box->FindSubControl(L"sticker");
		std::wstring sticker_name = nbase::StringPrintf(L"%s%.3d", name.c_str(), i);

		box->SetDataID(sticker_name);
		std::wstring dir = QPath::GetAppPath() + L"res\\stickers\\" + name + L"\\";

		std::wstring path = dir + sticker_name + L".png";
		c->SetBkImage(path);
	}
}

void EmojiForm::AddSticker(std::wstring name, vector<std::string>& files, bool type/* = false*/)
{
	int num = files.size();
	if (num <= 0)
	{
		return;
	}
	std::wstring sticker_id = nbase::StringPrintf(L"sticker_%s", name.c_str());
	ui::TileBox* sticker = (TileBox*)FindControl(sticker_id);
	
	sticker->SetDataID(name);

	std::string userId = nim_ui::LoginManager::GetInstance()->GetAccount();
	std::wstring dir = L"";
	if (type)
	{
		dir = QPath::GetUserAppDataDir(userId) + L"customImg\\custom\\";
	}
	else
	{
		dir = QPath::GetUserAppDataDir(userId) + L"like\\";
	}

	for (int i = 0; i < num; i++)
	{
		ButtonBox* box = new ButtonBox;
		GlobalManager::FillBoxWithCache(box, L"emoji/sticker_custom.xml");
		sticker->Add(box);
		
		box->SetKeyboardEnabled(false);
		//box->AttachMenu(nbase::Bind(&EmojiForm::OnMenu, this, std::placeholders::_1));
		box->AttachAllEvents(nbase::Bind(&EmojiForm::OnMenu, this, std::placeholders::_1));
		if (type)
		{
			box->AttachClick(nbase::Bind(&EmojiForm::OnCustomClicked, this, std::placeholders::_1));
		}
		else
		{
			box->AttachClick(nbase::Bind(&EmojiForm::OnStickerClicked, this, std::placeholders::_1));
		}
		
		Control* c = box->FindSubControl(L"sticker");
		//std::wstring sticker_name = nbase::StringPrintf(L"%s%.3d", name.c_str(), i);

		std::wstring sticker_name = nbase::UTF8ToUTF16(files[i]);
		box->SetDataID(sticker_name);
		

		std::wstring path = dir + sticker_name;
		c->SetBkImage(path);
	}
}

void getFiles(std::string path, vector<std::string>& files) {
	long hFile = 0;
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1) {
		do {
			if ((fileinfo.attrib & _A_SUBDIR)) {
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFiles(p.assign(path).append("\\").append(fileinfo.name), files);
			}
			else {
				//files.push_back(p.assign(path).append("\\").append(fileinfo.name));
				files.push_back(fileinfo.name);
			}
		} while (_findnext(hFile, &fileinfo) == 0); _findclose(hFile);
	}
}

bool EmojiForm::OnSelChanged(ui::EventArgs* param)
{
	std::wstring name = param->pSender->GetName();
	std::wstring dataid = param->pSender->GetDataID();
	int pos = name.find(L"sticker_op_");
	if (pos != -1 && !dataid.empty())
	{
		int num = 0;
		nbase::StringToInt(dataid, &num);
		
		if (0 == num && name == L"sticker_op_like")
		{
			std::string userId = nim_ui::LoginManager::GetInstance()->GetAccount();
			std::wstring dir = QPath::GetUserAppDataDir(userId) + L"like\\";
			std::string path = nbase::UTF16ToUTF8(dir);;
			vector<std::string> files;
			getFiles(path, files);
			AddSticker(name.substr(11), files);
			param->pSender->SetDataID(L"");
			return true;
		}
		else if (0 == num && name == L"sticker_op_custom")
		{
			std::string userId = nim_ui::LoginManager::GetInstance()->GetAccount();
			std::wstring dir = QPath::GetUserAppDataDir(userId) + L"customImg\\custom\\";
			std::string path = nbase::UTF16ToUTF8(dir);;
			vector<std::string> files;
			getFiles(path, files);
			AddSticker(name.substr(11), files, true);
			param->pSender->SetDataID(L"");
			return true;
		}
		
		if (num > 0)
		{
			AddSticker(name.substr(11), num);
			param->pSender->SetDataID(L"");
		}
	}
	return true;
}

bool EmojiForm::OnEmojiClicked( ui::EventArgs* arg )
{
	if (is_closing_)
		return false;

	std::wstring tip = arg->pSender->GetToolTipText();
	if( tip.empty() )
	{
		this->Close();
		return false;
	}

	if( sel_cb_ )
	{
		std::wstring face_tag = L"[" + tip + L"]";
		Post2UI( nbase::Bind(sel_cb_, face_tag) );
	}

	this->Close();
	return false;
}

bool EmojiForm::OnStickerClicked(ui::EventArgs* arg)
{
	if (is_closing_)
		return false;

	std::wstring id = arg->pSender->GetDataID();
	if (id.empty())
	{
		this->Close();
		return false;
	}
	std::wstring sticker_name = arg->pSender->GetParent()->GetDataID();
	if (sticker_name.empty())
	{
		this->Close();
		return false;
	}

	if (sel_sticker_cb_)
	{
		Post2UI(nbase::Bind(sel_sticker_cb_, sticker_name, id));
	}

	this->Close();
	return false;
}

bool EmojiForm::OnRightHandMenu(ui::EventArgs* arg)
{
	std::wstring name = arg->pSender->GetName();
	if (name == L"delete")
	{
		/*std::wstring sticker_name = arg->pSender->GetParent()->GetDataID();
		if (sticker_name.empty())
		{
			this->Close();
			return false;
		}*/
		if (del_custom_cb_)
		{
			Post2UI(nbase::Bind(del_custom_cb_, L"", del_imgId_));
		}

		this->Close();
		return false;
	}
		//OnMenuDelete();
	return false;
}

bool EmojiForm::OnMenu(ui::EventArgs* arg)
{
	if (kEventMouseRightButtonUp == arg->Type)
	{
		if (is_closing_)
			return false;

		del_imgId_ = arg->pSender->GetDataID();
		if (del_imgId_.empty())
		{
			this->Close();
			return false;
		}

		POINT point;
		::GetCursorPos(&point);

		CMenuWnd* pMenu = new CMenuWnd(NULL);
		STRINGorID xml(L"customImage_menu.xml");
		pMenu->Init(xml, _T("xml"), point);

		CMenuElementUI* del = (CMenuElementUI*)pMenu->FindControl(L"delete");
		del->AttachSelect(nbase::Bind(&EmojiForm::OnRightHandMenu, this, std::placeholders::_1));
		pMenu->Show();
		return true;
	}
	return true;
}

bool EmojiForm::OnCustomClicked(ui::EventArgs* arg)
{
	if (is_closing_)
		return false;

	std::wstring id = arg->pSender->GetDataID();
	if (id.empty())
	{
		this->Close();
		return false;
	}
	std::wstring sticker_name = arg->pSender->GetParent()->GetDataID();
	if (sticker_name.empty())
	{
		this->Close();
		return false;
	}

	if (sel_sticker_cb_)
	{
		Post2UI(nbase::Bind(sel_custom_cb_, sticker_name, id));
	}

	this->Close();
	return false;
}

}