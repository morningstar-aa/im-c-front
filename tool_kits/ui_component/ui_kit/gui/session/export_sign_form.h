#pragma once

typedef std::function<void(std::string, std::string, std::string)> OnSearchSignDataCallBack;
namespace nim_comp
{
/** @class ExportSignForm
  * @brief 自定义通知消息窗体
  * @copyright (c) 2015, NetEase Inc. All rights reserved
  * @author Redrain
  * @date 2015/9/10
  */
class ExportSignForm : public WindowEx
{
public:
	ExportSignForm();
	~ExportSignForm();
	
	//覆盖虚函数
	virtual std::wstring GetSkinFolder() override;
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
	* 处理所有控件单击消息
	* @param[in] msg 消息的相关信息
	* @return bool true 继续传递控件消息，false 停止传递控件消息
	*/
	bool OnClicked(ui::EventArgs* msg);

	/** 
	* 设置回调
	*/
	void SetCallBack(OnSearchSignDataCallBack cb);

private:
	/**
	* 处理设置组合框的选择消息
	* @param[in] msg 消息的相关信息
	* @return bool true 继续传递控件消息，false 停止传递控件消息
	*/
	bool OnStartTimeComboSelect(ui::EventArgs* args);

	/**
	* 处理设置组合框的选择消息
	* @param[in] msg 消息的相关信息
	* @return bool true 继续传递控件消息，false 停止传递控件消息
	*/
	bool OnEndTimeComboSelect(ui::EventArgs* args);

	/**
	* 初始化生日下拉框
	* @return void	无返回值
	*/
	void InitTimeCombo();

	/**
	**查找签到数据
	*/
	void SearchSignInData();

	/**
	* 选择导出\导入路径
	* @return void	无返回值
	*/
	void SelectPath();

	/**
	* 响应路径被选择的回调函数
	* @param[in] ret 是否选择了路径
	* @param[in] file_path 路径值
	* @return void	无返回值
	*/
	void OnSelectPathCallback(BOOL ret, std::wstring file_path);
public:
	static const LPCTSTR kClassName;
private:
	/*std::string		session_id_;
	nim::NIMSessionType		session_type_;

	ui::RichEdit* richedit_apns_;
	ui::RichEdit* richedit_msg_;
	ui::RichEdit* richedit_attach_;
	ui::Label* rec_name_;
	ui::CheckBox* msg_mode_;*/
	ui::RichEdit* path_edit_;
	ui::Combo*		start_year_combo = NULL;
	ui::Combo*		start_month_combo = NULL;
	ui::Combo*		start_day_combo = NULL;

	ui::Combo*		end_year_combo = NULL;
	ui::Combo*		end_month_combo = NULL;
	ui::Combo*		end_day_combo = NULL;

	OnSearchSignDataCallBack cb_search_;

	bool open_file_;
};
}