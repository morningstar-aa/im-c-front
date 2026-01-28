#pragma once
#include "bubble_item.h"

namespace nim_comp
{
/** @class MsgBubbleUnknown
  * @brief 会话窗体中聊天框内的未知消息项
  * @copyright (c) 2015, NetEase Inc. All rights reserved
  * @author Redrain
  * @date 2015/9/11
  */
class MsgBubbleVisitingCard : public MsgBubbleItem
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
	virtual void InitInfo(const nim::IMMessage &msg);
	//Svoid SetWalletType(int isSendWallet);

	void DownLoadImage(std::string url, std::wstring path);

	virtual bool OnClicked(ui::EventArgs* arg);
private:
	ui::HBox*	msg_unknown_;

	ui::Control *card_heard_img_;
	ui::Label*  visitingcard_account_;
	ui::Label*  visitingcard_name_;
	ui::Label*  visitingcard_title_;

	nim::UserNameCard userInfo_;
};
}