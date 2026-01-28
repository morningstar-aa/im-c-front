#pragma once
#include "bookmarks_item.h"

namespace nim_comp
{
/** @class BookmarksBubbleImage
  * @brief 会话窗体中聊天框内的图片消息项
  * @copyright (c) 2015, NetEase Inc. All rights reserved
  * @author Redrain
  * @date 2015/9/11
  */
class BookmarksBubbleImage : public BookmarksBubbleItem
{
public:
	/**
	* 初始化控件内部指针
	* @param[in] bubble_right 是否显示到右侧
	* @return void 无返回值
	*/
	virtual void InitControl(bool bubble_right);

	/**
	* 初始化控件外观
	* @param[in] msg 消息信息结构体
	* @return void 无返回值
	*/
	virtual void InitInfo(const BookmarksInfo &msg, OnDelBookmarksMsgCallback del_fun = NULL);
	
	
	/**
	* 响应此消息项的单击消息，打开图片预览
	*@param[in] param 被单击的按钮的相关信息
	* @return bool 返回值true: 继续传递控件消息， false: 停止传递控件消息
	*/
	bool OnClicked(ui::EventArgs* arg);

	/**
	* 响应此消息项的右击消息，弹出菜单
	* @param[in] param 被单击的菜单项的相关信息
	* @return bool 返回值true: 继续传递控件消息， false: 停止传递控件消息
	*/
	bool OnMenu(ui::EventArgs* arg);

private:
	//下载图片
	long URLDownloadToImgFile(std::string url, std::wstring name);

	/**
	* 检查用于在消息气泡中展示的缩略图是否已存在，如果存在就展示
	* @return bool 返回值true: 缩略图存在且完好， false: 缩略图不存在或图片有错误
	*/
	bool CheckImageBubble();

	/** 
	* 设置图片消息项的图片是否可以预览
	* @param[in] can 是否可预览		
	* @return void 无返回值
	*/
	void SetCanView(bool can);

	/** 
	* 设置图片消息项显示的缩略图	
	* @return void 无返回值
	*/
	//void DoZoom();

	void DownLoadImage(std::string url, std::wstring path);
protected:
	ui::ButtonBox*	msg_image_;
	ui::Control*	image_;

	std::wstring	thumb_;
	//std::wstring	path_;

	bool			image_checked_ = false;
};
}