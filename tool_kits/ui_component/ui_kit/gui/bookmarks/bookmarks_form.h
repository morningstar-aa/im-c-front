#pragma once

//#include "gui/session/control/bubbles/bubble_text.h"
//#include "gui/session/control/bubbles/bubble_image.h"
/*#include "gui/session/control/bubbles/bubble_snapchat.h"
#include "gui/session/control/bubbles/bubble_audio.h"
#include "gui/session/control/bubbles/bubble_location.h"
#include "gui/session/control/bubbles/bubble_notice.h"
#include "gui/session/control/bubbles/bubble_finger.h"
#include "gui/session/control/bubbles/bubble_unknown.h"
#include "gui/session/control/bubbles/bubble_writing.h"*/
//#include "gui/session/control/bubbles/bubble_file.h"
/*#include "gui/session/control/bubbles/bubble_sticker.h"
#include "gui/session/control/bubbles/bubble_video.h"
#include "gui/session/control/bubbles/bubble_robot.h"*/

#include "gui/session/control/bookmarksItem/bookmarks_text.h"
#include "gui/session/control/bookmarksItem/bookmarks_image.h"
//#include "gui/session/control/bookmarks/bubble_file.h"

namespace nim_comp
{

	//typedef std::function<void()> OnShortcutsCallBack;

	/** @class BookmarksForm
	  * @brief demo程序收藏界面窗口类
	  * @copyright (c) 2015, NetEase Inc. All rights reserved
	  * @author towik
	  * @date 2015/1/1
	  */
	typedef std::function<void(const std::list<BookmarksInfo>& bookmarksInfoList)> FillInfoFun;

	class BookmarksForm :
		public nim_comp::WindowEx
		//public ITrayIconDelegate
	{
	public:
		BookmarksForm();
		~BookmarksForm();

		/**
		* 虚函数，指定本界面的xml布局文件和图片素材所在的目录的相对路径
		* @return std::wstring 返回该目录相对于[安装目录]/bin/themes/default的路径
		*/
		virtual std::wstring GetSkinFolder() override;

		//覆盖虚函数
		virtual std::wstring GetSkinFile() override;
		virtual std::wstring GetWindowClassName() const override;
		virtual std::wstring GetWindowId() const override;
		virtual UINT GetClassStyle() const override;

		/**
		* 窗口初始化函数
		* @return void	无返回值
		*/
		virtual void InitWindow() override;

		/**
		* 查找收藏内容线程
		* @return void	无返回值
		*/
		void InitBookmarks();

		/**
		* 根据收藏内容增加控件
		* @return void	无返回值
		*/
		void CreateBookmarksCtrl(const std::list<BookmarksInfo>& infoList);

		/**
		* 清除列表中的控件
		* @return void	无返回值
		*/
		void ClearBookmarksCtrl();

		/**
		* 按页查找收藏内容
		* @return void	无返回值
		*/
		void SearchBookmarks(int page);

		/**
		* 下一页
		* @return void	无返回值
		*/
		bool nextPage(ui::EventArgs* param);

		/**
		* 上一页
		* @return void	无返回值
		*/
		bool previousPage(ui::EventArgs* param);

		void setNextBtn(bool isEnable);

		void setPreviousBtn(bool isEnable);

		/**删除收藏记录
		*/
		void OnDeleteBookmarksMsg(std::string id, ui::ListContainerElement * item);
	public:
		static const LPCTSTR kClassName;
		//UserInfo info_;
	private:
		ui::Button*     btn_next_;
		ui::Button*     btn_previous_;

		ui::Box*		msg_content_;		// 消息列表控件的父控件
		ui::ListBox*	msg_list_;
		int             cur_page_;
		/*ui::Button*		btn_header_;
		ui::Label*		label_name_;
		ui::Button*		btn_online_state_;
		ui::HBox* option_panel_ = nullptr;
		bool			is_busy_;

		nim_comp::CustomButtonBox*	box_unread_;
		ui::Label*		label_unread_;

		ui::RichEdit*	search_edit_;
		ui::Button*		btn_clear_input_;
		ui::ListBox*	search_result_list_;
		bool			is_trayicon_left_double_clicked_;

		AutoUnregister	unregister_cb;*/
	};

	//using namespace nbase;
	//DISABLE_RUNNABLE_METHOD_REFCOUNT(BookmarksForm);
}