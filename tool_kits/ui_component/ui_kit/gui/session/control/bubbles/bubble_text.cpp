#include "bubble_text.h"
#include "module/emoji/emoji_info.h"
#include "base/encrypt/encrypt_impl.h"
#include "base/util/base64.h"
#include <locale>
#include <codecvt>
#include <string>

using namespace ui;

namespace nim_comp
{
	MsgBubbleText::MsgBubbleText() :
		msg_text_(nullptr), text_(nullptr), text_has_emoji_(false)
	{

	}
	MsgBubbleText::~MsgBubbleText()
	{

	}
	void MsgBubbleText::InitControl(bool bubble_right)
	{
		__super::InitControl(bubble_right);

		msg_text_ = new Box;
		if (bubble_right)
			GlobalManager::FillBoxWithCache(msg_text_, L"session/text_right.xml");
		else
			GlobalManager::FillBoxWithCache(msg_text_, L"session/text_left.xml");
		bubble_box_->Add(msg_text_);

		text_ = (RichEdit*)msg_text_->FindSubControl(L"text");
		own_msg_time = (Label*)msg_text_->FindSubControl(L"own_msg_time");

		ITextServices* text_services = text_->GetTextServices();
		IRichEditOleCallbackEx* richedit_cb = new IRichEditOleCallbackEx(text_services, std::function<void()>());
		text_services->Release();
		text_->SetOleCallback(richedit_cb);
		text_->SetEventMask(ENM_LINK);
		text_->SetAutoURLDetect(true);
		text_->AttachMenu(nbase::Bind(&MsgBubbleText::OnMenu, this, std::placeholders::_1));
		text_->AttachGetNaturalSize([this, bubble_right](LONG width, LONG height, CSize& sz) -> bool {
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
			/*ui::StringHelper::ReplaceAll(L" ", L"!", text_temp);
			text_->SetText(text_temp);*/
			text_service->TxGetNaturalSize(
				DVASPECT_CONTENT,
				GetWindow()->GetPaintDC(),
				NULL,
				NULL,
				TXTNS_FITTOCONTENT,
				&szExtent,
				&lWidth,
				&lHeight);
			//text_->SetText(text);
			sz.cx = (int)lWidth;
			sz.cy = (int)lHeight;
			int newLineCount = std::count(text.begin(), text.end(), '\r');
			if (newLineCount > 1) {
				sz.cy += bubble_right ? 25 : 30;
			}

			return true;
		});
		//text_->SetNeedButtonUpWhenKillFocus(false);
		//text_->SetNoSelOnKillFocus(false);
		//text_->SetNoCaretReadonly();
		//text_->HideSelection(true, false);
	}

	void MsgBubbleText::InitInfo(const nim::IMMessage &msg)
	{
		__super::InitInfo(msg);

		std::wstring str = nbase::UTF8ToUTF16(msg.content_);
		// decode
		try {
			const std::string key = "12345678766666661234567876666666";
			const std::string iv = "1112222211111121";

			std::string cipher(str.begin(), str.end());
			std::string base64_de;
			nbase::EncryptInterface_var encrypt_enc(new nbase::Encrypt_Impl());
			nbase::Base64Decode(cipher, &base64_de);
			encrypt_enc->SetMethod(nbase::ENC_AES256_CBC);
			encrypt_enc->SetDecryptKey(key);
			encrypt_enc->SetDecryptIvParameterSpec(iv);
			encrypt_enc->EnableDecryptPadding(true, 7);
			UTF8String password_aes_de;
			encrypt_enc->Decrypt(base64_de, password_aes_de);

			str = nbase::UTF8ToUTF16(password_aes_de);
		}
		catch (const std::exception&) {
			str = nbase::UTF8ToUTF16(msg.content_);
		}
		// check str is empty
		if (str.empty()){
			str = nbase::UTF8ToUTF16(msg.content_);
		}


		if (IsNetCallMsg((nim::NIMMessageType)msg.type_, msg.attach_))
		{
			GetNotifyMsg(msg.attach_, msg.sender_accid_, msg.receiver_accid_, str, sid_);
			msg_.content_ = nbase::UTF16ToUTF8(str);
		}
		else if (msg.type_ == nim::kNIMMessageTypeCustom)
		{
			str = GetCustomMsg(msg.sender_accid_, msg.attach_);
			msg_.content_ = nbase::UTF16ToUTF8(str);
		}
		else
		{
			msg_.content_ = nbase::UTF16ToUTF8(str);
		}

		SetMsgText(str);
		if (own_msg_time != NULL) {
			std::wstring tm = GetMessageTimePart(msg_.timetag_);
			own_msg_time->SetText(tm);
		}
	}

	void MsgBubbleText::SetMsgText(const std::wstring &str)
	{
		if (str.empty())
		{
			text_->SetFixedWidth(10);
			text_->SetFixedHeight(20);
		}
		else
		{
			text_has_emoji_ = InsertTextToEdit(text_, str) > 0;
		}
	}

	ui::CSize MsgBubbleText::EstimateSize(ui::CSize szAvailable)
	{
		if (msg_.content_.empty())
			return Box::EstimateSize(szAvailable);

		int sub_width = 200;
		int width = szAvailable.cx - DpiManager::GetInstance()->ScaleInt(sub_width);

		ui::CSize sz = text_->GetNaturalSize(width, 0);
		text_->SetFixedWidth(sz.cx, true, false);
		text_->SetFixedHeight(sz.cy, false);

		return Box::EstimateSize(szAvailable);
	}

	bool MsgBubbleText::OnMenu(ui::EventArgs* arg)
	{
		bool can_recall = !IsNetCallMsg(msg_.type_, msg_.attach_);
		bool can_retweet = msg_.type_ != nim::kNIMMessageTypeNotification && msg_.type_ != nim::kNIMMessageTypeRobot;
		PopupMenu(true, can_recall, can_retweet, false, true);
		return false;
	}

	void MsgBubbleText::OnMenuCopy()
	{
		long start_pos = 0, end_pos = 0;
		text_->GetSel(start_pos, end_pos);
		if (start_pos == end_pos)
		{
			text_->SetSelAll();
			text_->Copy();
			text_->SetSelNone();
		}
		else
		{
			text_->Copy();
		}
	}

}