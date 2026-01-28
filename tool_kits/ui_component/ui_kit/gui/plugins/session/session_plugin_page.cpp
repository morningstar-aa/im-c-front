#include "stdafx.h"
#include "session_plugin_page.h"
#include "module\session\session_manager.h"
#include "gui\session\session_box.h"
#include "export\nim_ui_session_list_manager.h"
using namespace nim_comp;

namespace
{
	const int kDragImageWidth = 300;
	const int kDragImageHeight = 300;
}

SessionPluginPage::SessionPluginPage() :
active_session_box_(nullptr)
{
	old_drag_point_.x = old_drag_point_.y = 0;
}
SessionPluginPage::~SessionPluginPage()
{
	if (session_box_tab_ != nullptr)
		session_box_tab_->GetWindow()->RemoveMessageFilter(this);
}
void SessionPluginPage::DoInit()
{
	session_box_tab_ = dynamic_cast<ui::TabBox*>(FindSubControl(L"session_container"));
	if (session_box_tab_ != nullptr)
		session_box_tab_->GetWindow()->AddMessageFilter(this);
	GetPlugin()->AttachSelect([this](ui::EventArgs* param) {
		if (active_session_box_ != nullptr)
		{
			BOOL handle = false;
			active_session_box_->HandleMessage(WM_ACTIVATE, WA_ACTIVE, 0, handle);
			//::SetActiveWindow(NULL);
			auto session_id = active_session_box_->GetSessionId();
			nbase::ThreadManager::PostTask(active_session_box_->ToWeakCallback([this, session_id]() {
				//::SetActiveWindow(GetHWND());
				nim_ui::SessionListManager::GetInstance()->InvokeSelectSessionItem(session_id, true, false);
			}));			
		}
		return true;
	});
}
void SessionPluginPage::OnSessionListAttached()
{
	unregister_cb.Add(nim_ui::SessionListManager::GetInstance()->RegAddItem(ToWeakCallback([this](const std::string& id){
		if (IsActiveSessionBox(nbase::UTF8ToUTF16(id)))
			nim_ui::SessionListManager::GetInstance()->InvokeSelectSessionItem(id);
	})));
	unregister_cb.Add(nim_ui::SessionListManager::GetInstance()->RegRemoveItem(ToWeakCallback([this](const std::string& id){
		CloseSessionBox(id);
	})));
}
LRESULT SessionPluginPage::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// 处理Ctrl+Tab快捷键
	//if (uMsg == WM_KEYDOWN)
	//{
	//	if (wParam == VK_DOWN)
	//	{
	//		int next = session_box_tab_->GetCurSel();
	//		next = (next + 1) % session_box_tab_->GetCount();
	//		session_box_tab_->SelectItem(next);
	//		nim_ui::SessionListManager::GetInstance()->UpDownSessionItem(true);
	//		/*int next = merge_list_->GetCurSel();
	//		next = (next + 1) % merge_list_->GetCount();
	//		merge_list_->SelectItem(next);*/
	//		return 0;
	//	}
	//	else if (wParam == VK_UP)
	//	{
	//		int next = session_box_tab_->GetCurSel();
	//		next = (next + 1) % session_box_tab_->GetCount();
	//		session_box_tab_->SelectItem(next);
	//		nim_ui::SessionListManager::GetInstance()->UpDownSessionItem(false);
	//		/*int next = merge_list_->GetCurSel();
	//		next = (next + 1) % merge_list_->GetCount();
	//		merge_list_->SelectItem(next);*/
	//		return 0;
	//	}
	//}

	LRESULT ret = S_FALSE;
	auto active_session_box = GetSelectedSessionBox();
	if (active_session_box == nullptr)
		return S_FALSE;
	if (uMsg == WM_KEYDOWN)
	{
		if (wParam == 'V')
		{
			if (::GetKeyState(VK_CONTROL) < 0)
			{
				active_session_box->HandleMessage(uMsg, wParam, lParam, bHandled);
				if(bHandled)
					return S_OK;
			}
		}
	}
	if (uMsg == WM_NOTIFY)
	{
		if (wParam == EN_LINK)
		{
			std::wstring url = *(std::wstring*)lParam;
			if (!url.empty())
			{
				std::wstring ws = url;
				nbase::LowerString(ws);
				// 以"file:"开头 或者 包含".." 的超链接不允许打开
				if (!(ws.find(L"file:", 0, 5) == 0 || ws.find(L"..") != std::wstring::npos))
				{
					Post2GlobalMisc(nbase::Bind(&shared::tools::SafeOpenUrl, url, SW_SHOW));
				}
			}
			bHandled = true;
			ret = S_OK;
		}
		/*if (wParam == VK_DOWN || wParam == VK_UP)
		{
			return true;
		}*/
	}
	else if (uMsg == WM_DROPFILES)
	{
		//Run desktop helper from 360 or tencent
		QLOG_APP(L"##Receive dropfiles msg.");
		POINT pt;
		GetCursorPos(&pt);
		POINTL ppt;
		ppt.x = pt.x;
		ppt.y = pt.y;
		
		if (NULL != active_session_box && active_session_box->CheckDropEnable(ppt))
			active_session_box->OnDropFile((HDROP)wParam);
		return 0;
	}
	else if (uMsg == WM_KEYDOWN && wParam == VK_RETURN)
	{
		if (::GetKeyState(VK_CONTROL) >= 0)
		{
			if (RunTimeDataManager::GetInstance()->IsAttingSomeOne())
			{
				bHandled = true;
				return S_OK;
			}
		}
	}
	if (NULL != active_session_box)
	{	
		if (uMsg == WM_CHAR)
		{
			if (wParam != VK_BACK)
			{
				if (lParam == 0xFFFF)
				{
					bHandled = true;
					return S_OK;
				}					
				else
				{
					lParam = 0xFFFF;
				}					
			}
		}
		BOOL handle = false;
		ret = active_session_box->HandleMessage(uMsg, wParam, lParam, handle);
		if (handle)
			return ret;
	}
	return ret;
}


bool SessionPluginPage::OnProcessSessionBoxHeaderDrag(ui::EventArgs* param)
{
	if (!SessionManager::GetInstance()->IsEnableMerge())
		return true;

	switch (param->Type)
	{
	case ui::EventType::kEventMouseMove:
	{
		if (::GetKeyState(VK_LBUTTON) >= 0)
			break;

		LONG cx = abs(param->ptMouse.x - old_drag_point_.x);
		LONG cy = abs(param->ptMouse.y - old_drag_point_.y);

		if (!is_drag_state_ && (cx > 5 || cy > 5))
		{
			if (NULL == active_session_box_)
				break;

			is_drag_state_ = true;

			// 把被拖拽的会话盒子生成一个宽度300的位图
			HBITMAP bitmap = GenerateSessionBoxBitmap(session_box_tab_->GetPos(true));

			// pt应该指定相对bitmap位图的左上角(0,0)的坐标,这里设置为bitmap的中上点
			POINT pt = { kDragImageWidth / 2, 0 };

			StdClosure cb = [=]{
				SessionManager::GetInstance()->DoDragSessionBox(active_session_box_, bitmap, pt);
			};
			nbase::ThreadManager::PostTask(ThreadId::kThreadUI, cb);
		}
	}
	break;
	case ui::EventType::kEventMouseButtonDown:
	{
		// 如果当前会话窗口里只有一个会话盒子，则可以通过拖拽头像来拖拽会话盒子
		// 否则只能拖拽merge_list的merge_item来拖拽
		if (1 == session_box_tab_->GetCount())
		{
			old_drag_point_ = param->ptMouse;
			is_drag_state_ = false;
		}
	}
	break;
	}
	return true;
}

bool SessionPluginPage::OnProcessMergeItemDrag(ui::EventArgs* param)
{
	if (!SessionManager::GetInstance()->IsEnableMerge())
		return true;

	switch (param->Type)
	{
	case ui::EventType::kEventMouseMove:
	{
		if (::GetKeyState(VK_LBUTTON) >= 0)
			break;

		LONG cx = abs(param->ptMouse.x - old_drag_point_.x);
		LONG cy = abs(param->ptMouse.y - old_drag_point_.y);

		if (!is_drag_state_ && (cx > 5 || cy > 5))
		{
			if (NULL == active_session_box_)
				break;

			is_drag_state_ = true;

			// 把被拖拽的会话盒子生成一个宽度300的位图
			HBITMAP bitmap = GenerateSessionBoxBitmap(session_box_tab_->GetPos(true));

			// pt应该指定相对bitmap位图的左上角(0,0)的坐标,这里设置为bitmap的中上点
			POINT pt = { kDragImageWidth / 2, 0 };

			StdClosure cb = [=]{
				SessionManager::GetInstance()->DoDragSessionBox(active_session_box_, bitmap, pt);
			};
			nbase::ThreadManager::PostTask(ThreadId::kThreadUI, cb);
		}
	}
	break;
	case ui::EventType::kEventMouseButtonDown:
	{
		old_drag_point_ = param->ptMouse;
		is_drag_state_ = false;
	}
	break;
	}
	return true;
}

HBITMAP SessionPluginPage::GenerateSessionBoxBitmap(const ui::UiRect &src_rect)
{
	ASSERT(!src_rect.IsRectEmpty());
	int src_width = src_rect.right - src_rect.left;
	int src_height = src_rect.bottom - src_rect.top;

	auto render = ui::GlobalManager::CreateRenderContext();
	if (render->Resize(kDragImageWidth, kDragImageHeight))
	{
		int dest_width = 0;
		int dest_height = 0;
		float scale = (float)src_width / (float)src_height;
		if (scale >= 1.0)
		{
			dest_width = kDragImageWidth;
			dest_height = (int)(kDragImageWidth * (float)src_height / (float)src_width);
		}
		else
		{
			dest_height = kDragImageHeight;
			dest_width = (int)(kDragImageHeight * (float)src_width / (float)src_height);
		}

		render->AlphaBlend((kDragImageWidth - dest_width) / 2, 0, dest_width, dest_height, this->GetRenderContext()->GetDC(),
			src_rect.left, src_rect.top, src_rect.right - src_rect.left, src_rect.bottom - src_rect.top);
	}

	return render->DetachBitmap();
}

SessionBox * SessionPluginPage::getActiveSessionBox()
{
	return active_session_box_;
}

ui::TabBox * SessionPluginPage::getActiveSessionBoxTab()
{
	return session_box_tab_;
}

bool SessionPluginPage::RemoveItemAt(SessionBox *dragingSessionBox, ui::EventArgs* item_param)
{
	ui::ListBox* list = dynamic_cast<ui::ListBox*>(FindSubControl(L"session_list"));
	SessionList * sessiong_list = (SessionList *)list;
	if (sessiong_list->DetachSessionItem(dragingSessionBox->GetSessionId(), item_param))
	{
		//session_box_tab_->Remove(dragingSessionBox);
		return true;
	}
	return false;
}