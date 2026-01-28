#include "bookmarks_text.h"
#include "module/emoji/emoji_info.h"

using namespace ui;

namespace nim_comp
{
	BookmarksBubbleText::BookmarksBubbleText() :
		msg_text_(nullptr), text_(nullptr), text_has_emoji_(false)
	{

	}
	BookmarksBubbleText::~BookmarksBubbleText()
	{

	}
void BookmarksBubbleText::InitControl(bool bubble_right)
{
	__super::InitControl(bubble_right);
	
	msg_text_ = new Box;
	if(bubble_right)
		GlobalManager::FillBoxWithCache(msg_text_, L"bookmarks/text_right.xml");
	else
		GlobalManager::FillBoxWithCache(msg_text_, L"bookmarks/text_left.xml");
	bubble_box_->Add(msg_text_);

	text_ = (RichEdit*) msg_text_->FindSubControl(L"text");

	ITextServices* text_services = text_->GetTextServices();
	IRichEditOleCallbackEx* richedit_cb = new IRichEditOleCallbackEx( text_services, std::function<void()>() );
	text_services->Release();
	text_->SetOleCallback( richedit_cb );
	text_->SetEventMask(ENM_LINK);
	text_->SetAutoURLDetect(true);
	this->AttachMenu(nbase::Bind(&BookmarksBubbleText::OnMenu, this, std::placeholders::_1));
	text_->AttachGetNaturalSize([this](LONG width, LONG height, CSize& sz)->bool{
		if (text_has_emoji_)
			return false;
		ITextServices *text_service = text_->GetTextServices();
		if (text_service == nullptr)
			return false;
		LONG lWidth = width;
		LONG lHeight = height;
		SIZEL szExtent = { -1, -1 };
		std::wstring text(std::move(text_->GetText()));
		std::wstring text_temp(text);
		ui::StringHelper::ReplaceAll(L" ", L"!", text_temp);
		text_->SetText(text_temp);
		text_service->TxGetNaturalSize(
			DVASPECT_CONTENT,
			GetWindow()->GetPaintDC(),
			NULL,
			NULL,
			TXTNS_FITTOCONTENT,
			&szExtent,
			&lWidth,
			&lHeight);
		text_->SetText(text);
		sz.cx = (int)lWidth;
		sz.cy = (int)lHeight;
		return true;		
	});
}

void BookmarksBubbleText::InitInfo(const BookmarksInfo &msg, OnDelBookmarksMsgCallback del_fun)
{
	__super::InitInfo(msg, del_fun);

	std::wstring str = nbase::UTF8ToUTF16(msg.content);

	SetMsgText(str);
}

void BookmarksBubbleText::SetMsgText( const std::wstring &str )
{
	if(str.empty())
	{
		text_->SetFixedWidth(10);
		text_->SetFixedHeight(20);
	}
	else
	{
		text_has_emoji_ = InsertTextToEdit(text_, str) > 0;
	}
}

ui::CSize BookmarksBubbleText::EstimateSize(ui::CSize szAvailable)
{
	if (msg_.content.empty())
		return Box::EstimateSize(szAvailable);

	int sub_width = 40;
	int width = szAvailable.cx - DpiManager::GetInstance()->ScaleInt(sub_width);

	ui::CSize sz = text_->GetNaturalSize(width, 0);
	text_->SetFixedWidth(sz.cx, true, false);
	text_->SetFixedHeight(sz.cy, false);

	return Box::EstimateSize(szAvailable);
}

bool BookmarksBubbleText::OnMenu( ui::EventArgs* arg )
{
	PopupMenu(arg);
	return false;
}

}