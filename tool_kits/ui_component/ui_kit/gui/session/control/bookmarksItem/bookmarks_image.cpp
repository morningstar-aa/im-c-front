#include "bookmarks_image.h"
#include "image_view/src/image_view_manager.h"
#include "util/user.h"
#include "shared/zoom_image.h"
#include "export/nim_ui_login_manager.h"

using namespace ui;

namespace nim_comp
{
void BookmarksBubbleImage::InitControl(bool bubble_right)
{
	__super::InitControl(bubble_right);

	msg_image_ = new ButtonBox;
	if(bubble_right)
		GlobalManager::FillBoxWithCache(msg_image_, L"bookmarks/image_right.xml");
	else
		GlobalManager::FillBoxWithCache(msg_image_, L"bookmarks/image_left.xml");
	bubble_box_->Add( msg_image_ );

	image_ = msg_image_->FindSubControl(L"image");

	msg_image_->AttachClick(nbase::Bind(&BookmarksBubbleImage::OnClicked, this, std::placeholders::_1));
	this->AttachMenu(nbase::Bind(&BookmarksBubbleImage::OnMenu, this, std::placeholders::_1));
}

long BookmarksBubbleImage::URLDownloadToImgFile(std::string url, std::wstring name)
{
	size_t len = url.length();//获取字符串长度
	int nmlen = MultiByteToWideChar(CP_ACP, 0, url.c_str(), len + 1, NULL, 0);//如果函数运行成功，并且cchWideChar为零，

	//返回值是接收到待转换字符串的缓冲区所需求的宽字符数大小。
	wchar_t* buffer = new wchar_t[nmlen];
	MultiByteToWideChar(CP_ACP, 0, url.c_str(), len + 1, buffer, nmlen);

	return URLDownloadToFile(0, buffer, name.data(), 0, 0);
}

void BookmarksBubbleImage::InitInfo(const BookmarksInfo &msg, OnDelBookmarksMsgCallback del_fun)
{
	__super::InitInfo(msg, del_fun);

	std::string userId = nim_ui::LoginManager::GetInstance()->GetAccount();
	std::string url = msg.imgCompPath;
	std::wstring dir = QPath::GetUserAppDataDir(userId) + L"bookmarks\\";
	if (!nbase::FilePathIsExist(dir, true))
	{
		nbase::win32::CreateDirectoryRecursively(dir.c_str());
	}
	std::string imgId = std::to_string(msg.id);
	std::wstring name = dir + nbase::UTF8ToUTF16(imgId) + L".png";
	if (!nbase::FilePathIsExist(name, false))
	{
		Post2GlobalMisc(nbase::Bind(&BookmarksBubbleImage::DownLoadImage, this, url, name));
		//URLDownloadToImgFile(url, name);
		name = L"../session/def_sticker.png";
	}
	thumb_ = name;
	if (nbase::FilePathIsExist(thumb_, false)) //thumb_图片存在
	{
		if (CheckImageBubble())
		{
			SetCanView(true);
		}
		else //图片有错误
		{
			//SetLoadStatus(RS_LOAD_NO);
		}
			
	}
}

void BookmarksBubbleImage::DownLoadImage(std::string url, std::wstring path)
{
	URLDownloadToImgFile(url, path);
	image_->SetBkImage(path);
	thumb_ = path;
}

bool BookmarksBubbleImage::OnClicked( ui::EventArgs* arg )
{
	std::string userId = nim_ui::LoginManager::GetInstance()->GetAccount();
	std::string url = msg_.path;

	std::wstring dir = QPath::GetUserAppDataDir(userId) + L"bookmarks_o\\";
	if (!nbase::FilePathIsExist(dir, true))
	{
		nbase::win32::CreateDirectoryRecursively(dir.c_str());
	}
	std::string imgId = std::to_string(msg_.id);
	std::wstring name = dir + nbase::UTF8ToUTF16(imgId) + L".png";
	if (!nbase::FilePathIsExist(name, false))
	{
		URLDownloadToImgFile(url, name);
	}
	if (nbase::FilePathIsExist(name, false))
	{
		ImageViewManager::GetInstance()->StartViewPic(name, L"", true);
	}
	return true;
}

bool BookmarksBubbleImage::CheckImageBubble()
{
	if (image_checked_)
		return true;

	if (thumb_.empty() || !nbase::FilePathIsExist(thumb_, false))
		return false;

	//gif图片经过压缩下载后，虽然格式仍是gif，但变成只有一帧，没有动画，尺寸也可能变小了。
	Gdiplus::Image thumb_image(thumb_.c_str());
	Gdiplus::Status status = thumb_image.GetLastStatus();
	if (status != Gdiplus::Ok) //图片有错误
	{
		QLOG_ERR(L"Image {0} error {1}") << thumb_ << status;
		return false;
	}

	int width = thumb_image.GetWidth();
	int height = thumb_image.GetHeight();
	if (width == 0 || height == 0)
	{
		QLOG_ERR(L"Get image size error: {0}, {1}.") << width << height;
		return false;
	}
	
	image_->SetFixedWidth(width, false); //压缩下载的图片直接做消息气泡的图片
	image_->SetFixedHeight(height);
	image_->SetBkImage(thumb_);
	image_checked_ = true;

	return true;
}

void BookmarksBubbleImage::SetCanView(bool can)
{
	msg_image_->SetEnabled(can);
}

bool BookmarksBubbleImage::OnMenu( ui::EventArgs* arg )
{
	PopupMenu(arg);
	return false;
}
}