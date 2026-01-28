#include "bubble_sticker.h"
#include "export/nim_ui_login_manager.h"
#include <UrlMon.h>
#include<cstdio>
#include "shared/zoom_image.h"

#pragma comment(lib,"urlmon.lib")
using namespace ui;

namespace nim_comp
{
void MsgBubbleSticker::InitControl(bool bubble_right)
{
	__super::InitControl(bubble_right);

	msg_sticker_ = new Box;
	if(bubble_right)
		GlobalManager::FillBoxWithCache(msg_sticker_, L"session/sticker_right.xml");
	else
		GlobalManager::FillBoxWithCache(msg_sticker_, L"session/sticker_left.xml");
	bubble_box_->Add(msg_sticker_);
	msg_sticker_->AttachMenu(nbase::Bind(&MsgBubbleSticker::OnMenu, this, std::placeholders::_1));
}

long URLDownloadToLikeImgFile(std::string url, std::wstring name)
{
	size_t len = url.length();//获取字符串长度
	int nmlen = MultiByteToWideChar(CP_ACP, 0, url.c_str(), len + 1, NULL, 0);//如果函数运行成功，并且cchWideChar为零，

	//返回值是接收到待转换字符串的缓冲区所需求的宽字符数大小。
	wchar_t* buffer = new wchar_t[nmlen];
	MultiByteToWideChar(CP_ACP, 0, url.c_str(), len + 1, buffer, nmlen);

	return URLDownloadToFile(0, buffer, name.data(), 0, 0);
}

void MsgBubbleSticker::InitInfo(const nim::IMMessage &msg)
{
	__super::InitInfo(msg);

	Json::Value json;
	if (StringToJson(msg.attach_, json) && json.isObject())
	{
		int sub_type = json["type"].asInt();
		if (sub_type == CustomMsgType_Sticker && json["data"].isObject()) //finger
		{
			std::string catalog = json["data"]["catalog"].asString();
			//std::string imgId = json["data"]["imgId"].asString();
			std::wstring dir = L"";
			std::wstring path = L"";
			curItemType_ = catalog;
			if (catalog == "like")
			{
				//为缓存到本地备用
				/*std::string userId = msg.sender_accid_;
				dir = QPath::GetUserAppDataDir(userId) + L"like\\";
				path = dir + nbase::UTF8ToUTF16(imgId) + L".png";*/
				std::string userId = nim_ui::LoginManager::GetInstance()->GetAccount();
				std::string url = json["data"]["chartlet"].asString();
				std::wstring dir = QPath::GetUserAppDataDir(userId) + L"like_buf\\";
				if (!nbase::FilePathIsExist(dir, true))
					nbase::win32::CreateDirectoryRecursively(dir.c_str());

				std::string name = "";
				int nPos = url.rfind('\\');
				if (-1 == nPos) {
					nPos = url.rfind('/');
				}
				if (-1 != nPos) {
					name = url.substr(nPos + 1, url.length() - nPos - 1);
				}

				std::wstring namePath = dir + nbase::UTF8ToUTF16(name);// +L".png";

				if (!nbase::FilePathIsExist(namePath, false))
				{
					if (!my_msg_)
					{
						SetLoadStatus(RS_LOADING);
					}
					else
					{
						__super::SetMsgStatus(nim::kNIMMsgLogStatusSending); //SetMsgStatus(nim::kNIMMsgLogStatusSending);
					}
					
					//下载线程
					
					Post2GlobalMisc(nbase::Bind(&MsgBubbleSticker::DownLoadImage, this, url, namePath));

					//nbase::ThreadManager::PostTask(kThreadGlobalMisc, nbase::Bind(BeginAsynCustomImg));
					//URLDownloadToLikeImgFile(url, namePath);

					//msg_sticker_->SetBkImage(path);
				}
				else
				{
					__super::SetMsgStatus(nim::kNIMMsgLogStatusSent);
				}
				path = namePath;
			}
			else
			{
				std::string sticker_name = json["data"]["chartlet"].asString();
				if (sticker_name.find_first_of("ww") == 0)
				{
					sticker_name += ".gif";
				}
				else
				{
					sticker_name += ".png";
				}
				dir = QPath::GetAppPath() + L"res\\stickers\\";
				path = dir + nbase::UTF8ToUTF16(catalog) + L"\\" + nbase::UTF8ToUTF16(sticker_name);
			}
			
			if (!path.empty() && nbase::FilePathIsExist(path, false))
			{
				SIZE sz = { 0, 0 };
				bool resize = CalculateImageSize(path, sz, 270, 180);

				msg_sticker_->SetFixedWidth(sz.cx, false);
				msg_sticker_->SetFixedHeight(sz.cy);
				msg_sticker_->SetBkImage(path);
				path_ = path;
				return;
			}
		}
	}
	QLOG_ERR(L"parse location msg attach fail: {0}") << msg.attach_;
}

void MsgBubbleSticker::SetMsgStatus(nim::NIMMsgLogStatus status)
{
	if (curItemType_ == "like")
	{
		return;
	}
	__super::SetMsgStatus(status);
}

bool MsgBubbleSticker::OnMenu(ui::EventArgs* arg)
{
	//PopupMenu(false, true);
	PopupMenu(true, true, true, true);
	return false;
}

void MsgBubbleSticker::DownLoadImage(std::string url, std::wstring path)
{
	URLDownloadToLikeImgFile(url, path);
	if (!my_msg_)
	{
		SetLoadStatus(RS_LOAD_OK);
	}
	else
	{
		//SetMsgStatus(nim::kNIMMsgLogStatusSent);
		__super::SetMsgStatus(nim::kNIMMsgLogStatusSent);
	}
	
	SIZE sz = { 0, 0 };
	if (!path.empty())
		CalculateImageSize(path, sz, 270, 180);
	if (msg_sticker_ != nullptr)
	{
		msg_sticker_->SetFixedWidth(sz.cx, false);
		msg_sticker_->SetFixedHeight(sz.cy);
		msg_sticker_->SetBkImage(path);
	}
	
	path_ = path;
}

void MsgBubbleSticker::OnMenuCopy()
{
	//写剪贴板
	std::string path = nbase::UTF16ToUTF8(path_);
	CopyFileToClipboard(path.data());
}

}