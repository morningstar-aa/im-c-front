#pragma once

namespace nim_comp
{

struct MEDIA_SHORTCUTS_EDIT_INFO
{
	std::string type;
	std::string value;
	//std::string newline_shortcuts;
	//std::string screenshots_shortcuts;
};

/** @class ShortcutsManager
  * @brief 快捷键设置
  * @copyright (c) 2016, NetEase Inc. All rights reserved
  * @date 2016/09/18
  */
class ShortcutsManager : public nbase::SupportWeakCallback
{
public:
	SINGLETON_DEFINE(ShortcutsManager);

	ShortcutsManager();
	~ShortcutsManager();

	/** 
	* 显示群视频人员选择窗口
	*/
	bool ShowMultiVideoSelectForm();


	/**
	* 显示快捷键设置窗口
	* @param[in] 
	* @return bool true 成功，false 失败
	*/
	bool ShowShortcutsSetting(bool pageTag = false);
	
	/**
	* 获取设备信息
	* @param[in] type 设备类型
	* @return std::list<MEDIA_DEVICE_DRIVE_INFO> 设备信息列表
	*/
	std::list<MEDIA_SHORTCUTS_EDIT_INFO> GetEditInfo();

	/**
	* 获取发送快捷键状态
	*/
	bool GetSendShortcutsStatus();

	/**
	* 启动发送快捷键并保存设置
	* @param[in] 
	*/
	void StartSendShortcuts(std::string key);

	/**
	* 保存快捷键设置配置
	* @param[in]
	*/
	void SaveShortcutsSetting(std::string screenshots, std::string popMsg);

	/**
	* 读取快捷键设置配置
	* @param[in]
	*/
	void GetShortcutsSetting(std::string &screenshots, std::string &popMsg);

	/**
	* 注册快捷键
	* @param[in]
	*/
	void ResShortcutsSetting(std::string screenshots, std::string popMsg);

public:
	//VideoFrameMng video_frame_mng_;
	//AudioFrameMng audio_frame_mng_;
private:
	std::list<MEDIA_SHORTCUTS_EDIT_INFO> edit_info_list_;
	/*std::string def_device_path_[4];
	DWORD device_session_type_[4];
	//ConnectCallback			chatroom_connect_cb_ = nullptr;
	//PeopleChangeCallback	chatroom_people_cb_ = nullptr;
	/*bool webrtc_setting_;
	std::string multi_vchat_session_id_;
	std::string multi_vchat_creator_id_;
	bool setting_audio_in_;
	bool setting_audio_out_;
	bool setting_video_in_;
	//MultiVChatStatusType vchat_status_;
	nbase::NLock vchat_status_lock_;
	nbase::WeakCallbackFlag join_vchat_timer_;*/

	std::wstring setting_path_;
};
}