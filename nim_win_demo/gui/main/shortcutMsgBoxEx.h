#pragma once

typedef std::function<void()> ShortcutCallBack;

class shortcutMsgBoxEx : public ui::WindowImplBase//WindowEx
{
public:
	shortcutMsgBoxEx();
	virtual ~shortcutMsgBoxEx();

	virtual std::wstring GetSkinFolder() override;
	virtual std::wstring GetSkinFile() override;
	virtual ui::UILIB_RESOURCETYPE GetResourceType() const;
	virtual std::wstring GetZIPFileName() const;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual void OnEsc(BOOL &bHandled);
	virtual void Close(UINT nRet = IDOK) override;

	virtual std::wstring GetWindowClassName() const override;
	virtual std::wstring GetWindowId() const /*override*/;
	virtual UINT GetClassStyle() const override;
	virtual void InitWindow() override;
	static shortcutMsgBoxEx *getInstence();

	void registerShortcuts(std::string &screenshots, std::string &popMsg);
	void unRegisterShortcuts();
	void setReceivedMsg();
	void reRegisterShortcuts(std::string &screenshots, std::string &popMsg);

	void setCallbackFuntion(ShortcutCallBack callback1, ShortcutCallBack callback2);
public:
	static const LPCTSTR kClassName;
	static shortcutMsgBoxEx *m_instence;
	ShortcutCallBack pop_msg_cb_;
	ShortcutCallBack shortcut_cb_;
	bool isGetMsg;
	UINT iScreenshotsTag;
	UINT iPopMsgTag;
};
