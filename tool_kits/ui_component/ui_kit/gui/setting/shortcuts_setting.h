#ifndef NIM_WIN_GUI_SHORTCUTS_SETTING_H_
#define NIM_WIN_GUI_SHORTCUTS_SETTING_H_

#include "util/window_ex.h"

namespace nim_comp
{
/** @class ShortcutsSettingForm
  * @brief 快捷键设置窗口
  * @copyright (c) 2016, NetEase Inc. All rights reserved
  * @date 2016/09/21
  */
class ShortcutsSettingForm : public WindowEx
{
public:
	ShortcutsSettingForm();
	~ShortcutsSettingForm();
	
	// 覆盖虚函数
	virtual std::wstring GetSkinFolder() override;
	virtual std::wstring GetSkinFile() override;
	virtual std::wstring GetWindowClassName() const override { return kClassName; };
	virtual std::wstring GetWindowId() const override { return kClassName; };
	virtual UINT GetClassStyle() const override { return UI_CLASSSTYLE_FRAME | CS_DBLCLKS; };
	
	/**
	* 窗口初始化函数
	* @return void	无返回值
	*/
	virtual void InitWindow() override;

	/**
	* 处理截屏快捷键编辑框控件的所有消息
	* @param[in] param 消息的相关信息
	* @return bool true 继续传递控件消息，false 停止传递控件消息
	*/
	bool ScreenshotsNotify(ui::EventArgs* param);
	/**
	* 处理弹出消息快捷键编辑框控件的所有消息
	* @param[in] param 消息的相关信息
	* @return bool true 继续传递控件消息，false 停止传递控件消息
	*/
	bool PopMsgNotify(ui::EventArgs* param);
	
	/**
	* 显示的页面内容
	* @return void	无返回值
	*/
	void ShowPage();

	/**
	* 初始化列表
	* @return void	无返回值
	*/
	void InitEditList();

	/**
	* 切换发送快捷键
	* @param[in] device_path 设备路径
	* @return void	无返回值
	*/
	void ChangeSendButtonStatus(const std::string text);

private:
	/**
	* 处理所有控件的所有消息
	* @param[in] msg 消息的相关信息
	* @return bool true 继续传递控件消息，false 停止传递控件消息
	*/
	bool Notify(ui::EventArgs* msg);

public:
	bool getSendButtonStatus();
public:
	static const LPCTSTR kClassName;
private:
	ui::Combo*      send_select_text_;
	bool            send_button_status_;

	ui::RichEdit*   Screenshots_key_;
	std::string     default_Screenshots_key;
	std::string     default_Screenshots_char;

	ui::RichEdit*   PopMsg_key_;
	std::string     default_PopMsg_key;
	std::string     default_PopMsg_char;

};
}

#endif
