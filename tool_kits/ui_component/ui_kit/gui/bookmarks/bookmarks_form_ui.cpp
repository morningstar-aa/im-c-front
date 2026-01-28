#include "resource.h"
#include "bookmarks_form.h"
#include "export/nim_ui_user_manager.h"
#include "export/nim_ui_window_manager.h"

using namespace ui;
//using namespace nim_ui;

namespace nim_comp
{
	const LPCTSTR BookmarksForm::kClassName = L"BookmarksForm";

	BookmarksForm::BookmarksForm()
	{
		cur_page_ = 1;
	}

	BookmarksForm::~BookmarksForm()
	{

	}

	std::wstring BookmarksForm::GetSkinFolder()
	{
		return L"bookmarks";
	}

	std::wstring BookmarksForm::GetSkinFile()
	{
		return L"bookmarks.xml";
	}

	std::wstring BookmarksForm::GetWindowClassName() const
	{
		return kClassName;
	}

	std::wstring BookmarksForm::GetWindowId() const
	{
		return kClassName;
	}

	UINT BookmarksForm::GetClassStyle() const
	{
		return (UI_CLASSSTYLE_FRAME | CS_DBLCLKS);
	}

	void BookmarksForm::InitBookmarks()
	{
		SearchBookmarks(1);
	}

	void BookmarksForm::SearchBookmarks(int page)
	{
		std::string strPage = std::to_string(page);
		std::string userId = LoginManager::GetInstance()->GetAccount();
		//std::list<BookmarksInfo> infoList;
		//向服务器发送请求获取
		nim_ui::UserManager::GetInstance()->QueryBookmarksInfo(userId, strPage, ToWeakCallback([this](int code, std::string msg, const BookmarksPageInfo &pageInfo, const std::list<BookmarksInfo>& infoList) {
			if (code == 0)
			{
				BookmarksForm *form = (BookmarksForm*)(nim_ui::WindowsManager::GetInstance()->GetWindow(nim_comp::BookmarksForm::kClassName, nim_comp::BookmarksForm::kClassName));
				//form->ClearBookmarksCtrl();
				form->CreateBookmarksCtrl(infoList);

				if (pageInfo.hasMore)
				{
					form->setNextBtn(true);
				}
				else
				{
					form->setNextBtn(false);
				}
				if (cur_page_ == 1)
				{
					form->setPreviousBtn(false);
				}
				else
				{
					form->setPreviousBtn(true);
				}
			}
			else
			{

			}
		}));
	}

	void BookmarksForm::OnDeleteBookmarksMsg(std::string id, ListContainerElement * item)
	{
		msg_list_->Remove(item);
		std::string userId = LoginManager::GetInstance()->GetAccount();
		//std::list<BookmarksInfo> infoList;
		//向服务器发送请求获取
		nim_ui::UserManager::GetInstance()->DeleteBookmarksInfo(userId, id, ToWeakCallback([this](int code, const std::string msg) {
			if (code == 0)
			{
				BookmarksForm *form = (BookmarksForm*)(nim_ui::WindowsManager::GetInstance()->GetWindow(nim_comp::BookmarksForm::kClassName, nim_comp::BookmarksForm::kClassName));
				
				ShowMsgBox(this->GetHWND(), MsgboxCallback(), L"STRID_SESSIONBOX_CUSTOMEMOTION_DEL_SUCCESS", true);
			}
			else
			{
				//弹框展示失败
				ShowMsgBox(this->GetHWND(), MsgboxCallback(), L"STRID_SESSIONBOX_CUSTOMEMOTION_DEL_FAIL", true);
			}
		}));
	}

	void BookmarksForm::setPreviousBtn(bool isEnable)
	{
		btn_previous_->SetEnabled(isEnable);
	}

	void BookmarksForm::setNextBtn(bool isEnable)
	{
		btn_next_->SetEnabled(isEnable);
	}

	bool BookmarksForm::nextPage(ui::EventArgs* param)
	{
		btn_previous_->SetEnabled(false);
		btn_next_->SetEnabled(false);
		cur_page_++;
		SearchBookmarks(cur_page_);
		return true;
	}

	bool BookmarksForm::previousPage(ui::EventArgs* param)
	{
		btn_previous_->SetEnabled(false);
		btn_next_->SetEnabled(false);
		cur_page_--;
		SearchBookmarks(cur_page_);
		return true;
	}

	void BookmarksForm::InitWindow()
	{
		//UI初始化
		btn_next_ = (Button*)FindControl(L"btn_next");
		btn_next_->AttachClick(nbase::Bind(&BookmarksForm::nextPage, this, std::placeholders::_1));

		btn_previous_ = (Button*)FindControl(L"btn_previous");
		btn_previous_->AttachClick(nbase::Bind(&BookmarksForm::previousPage, this, std::placeholders::_1));

		msg_content_ = (Box*)FindControl(L"msg_content");
		msg_list_ = (ListBox*)FindControl(L"msg_list");

		/*BookmarksBubbleItem* item = NULL;
		item = new BookmarksBubbleImage;
		msg_list_->Add(item);
		*/
		//查找收藏线程
		InitBookmarks();
		//nbase::ThreadManager::PostTask(kThreadGlobalMisc, nbase::Bind(&BookmarksForm::InitBookmarks, this));
	}

	void BookmarksForm::CreateBookmarksCtrl(const std::list<BookmarksInfo>& infoList)
	{
		BookmarksBubbleItem* item = NULL;
		msg_list_->RemoveAll();
		for (auto info : infoList)
		{
			if (info.type == nim::kNIMMessageTypeText)
			{
				item = new BookmarksBubbleText;
			}
			else if (info.type == nim::kNIMMessageTypeImage)
			{
				item = new BookmarksBubbleImage;
			}
			else if (info.type == nim::kNIMMessageTypeFile)
			{
				//item = new MsgBubbleFile;
			}
			GlobalManager::FillBoxWithCache(item, L"bookmarks/bubble_left.xml");
			msg_list_->Add(item);

			item->InitControl(false);
			//nim::IMMessage msg;

			OnDelBookmarksMsgCallback del = nbase::Bind(&BookmarksForm::OnDeleteBookmarksMsg, this, std::placeholders::_1, std::placeholders::_2);
			item->InitInfo(info,del);

			/*item->SetSessionId(session_id_);
			item->SetSessionType(session_type_);
			item->SetShowTime(show_time);*/
			//item->SetShowName(false, "");
		}
		
	}

	void BookmarksForm::ClearBookmarksCtrl()
	{
		msg_list_->RemoveAll();
	}
}