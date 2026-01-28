#pragma once
#include "main_form.h"

class shortcutMsgBox : public ui::WindowImplBase//WindowEx
{
public:
	shortcutMsgBox();
	virtual ~shortcutMsgBox();

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
	static shortcutMsgBox *getInstence();

	void registerShortcuts(std::string &screenshots, std::string &popMsg);
	void unRegisterShortcuts();
	void setReceivedMsg();
	void reRegisterShortcuts(std::string &screenshots, std::string &popMsg);
public:
	static const LPCTSTR kClassName;
	static shortcutMsgBox *m_instence;

	bool isGetMsg;
	UINT iScreenshotsTag;
	UINT iPopMsgTag;
};
