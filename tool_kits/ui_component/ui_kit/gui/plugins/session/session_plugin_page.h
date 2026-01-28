#pragma once
#include "gui/session/session_dock_def.h"
#include "module/plugins/main_plugins_base.h"
#include "gui/session/taskbar/taskbar_manager.h"
namespace nim_comp
{
	class SessionBox;
	class SessionPluginPage : public MainPluginPage, public ISessionDock, public ui::IUIMessageFilter
	{
	public:
		SessionPluginPage();
		virtual ~SessionPluginPage();
	public:
		virtual HWND Create() override;
		virtual void CenterWindow() override;
		virtual HWND GetHWND() override;
		virtual ui::UiRect GetPos(bool bContainShadow = false) const override;
		virtual void SetPos(const ui::UiRect& rc, bool bNeedDpiScale, UINT uFlags, HWND hWndInsertAfter = NULL, bool bContainShadow = false) override;
		virtual void ActiveWindow() override;
		virtual LRESULT PostMessage(UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0L) override;		
		virtual void SetTaskbarTitle(const std::wstring &title) override;	
		virtual void SetTaskbarIcon(const std::wstring &icon) override;		
		virtual SessionBox* CreateSessionBox(const std::string &session_id, nim::NIMSessionType session_type) override;
		virtual void CloseSessionBox(const std::string &session_id) override;
		virtual bool AttachSessionBox(SessionBox *session_box) override;
		virtual bool DetachSessionBox(SessionBox *session_box) override;
		virtual SessionBox* GetSelectedSessionBox() override;
		virtual void SetActiveSessionBox(const std::string &session_id) override;
		virtual bool IsActiveSessionBox(const SessionBox *session_box) override;
		virtual bool IsActiveSessionBox(const std::wstring &session_id) override;
		virtual int GetSessionBoxCount() const override;
		virtual void OnBeforeDragSessionBoxCallback(const std::wstring &session_id) override;
		virtual void OnAfterDragSessionBoxCallback(bool drop_succeed) override;
		virtual void InvokeSetSessionUnread(const std::string &id, int unread) override;
		virtual void SetMergeItemName(const std::wstring &session_id, const std::wstring &name) override;
		virtual void SetMergeItemHeaderPhoto(const std::wstring &session_id, const std::wstring &photo) override;
		virtual void OnNewMsg(SessionBox &session_box, bool create, bool flash) override;

		/**
		* 在任务栏闪动
		* @param[in] session_box 要闪动的会话盒子
		* @return void	无返回值
		*/
		void FlashTaskbar(SessionBox &session_box);

		virtual void AdjustFormSize() override;
		virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
		virtual LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;
		virtual LRESULT HostWindowHandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	public:
		virtual void DoInit() override;
		void OnSessionListAttached();

		SessionBox * getActiveSessionBox();
		ui::TabBox * getActiveSessionBoxTab();
		bool RemoveItemAt(SessionBox *dragingSessionBox, ui::EventArgs* item_param);

	private:		
		SessionBox* GetSessionBoxByID(const std::string& session_id) const;

	public:
		/**
		* 判断是否要拖拽会话盒子
		* @param[in] param 处理会话盒子的头像控件发送的事件
		* @return bool 返回值true: 继续传递控件消息， false: 停止传递控件消息
		*/
		bool OnProcessSessionBoxHeaderDrag(ui::EventArgs* param);

		/**
		* 判断是否要拖拽会话盒子
		* @param[in] param 处理会话窗口左侧会话合并列表项发送的事件
		* @return bool 返回值true: 继续传递控件消息， false: 停止传递控件消息
		*/
		bool OnProcessMergeItemDrag(ui::EventArgs* param);

		/**
		* 生成当前窗体中某个区域对应的位图
		* @param[in] src_rect 目标位图的位置
		* @return HBITMAP 生成的位图
		*/
		HBITMAP GenerateSessionBoxBitmap(const ui::UiRect &src_rect);

	private:
		// 处理会话盒子拖拽事件
		bool				is_drag_state_;
		POINT				old_drag_point_;
		std::wstring		draging_session_id_;
	private:
		ui::TabBox* session_box_tab_;
		SessionBox* active_session_box_;
		AutoUnregister	unregister_cb;

		// 任务栏缩略图管理器
		TaskbarManager		taskbar_manager_;
	};
}