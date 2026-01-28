#include "bubble_video.h"
#include "image_view/src/image_view_manager.h"
#include "util/user.h"
#include "shared/zoom_image.h"
#include "gui/local_video_player/local_video_player_form.h"
#include "export/nim_ui_login_manager.h"
#include "module/session/session_manager.h"

using namespace ui;

namespace nim_comp
{
void MsgBubbleVideo::InitControl(bool bubble_right)
{
	__super::InitControl(bubble_right);

	msg_video_ = new ButtonBox;
	if(bubble_right)
		GlobalManager::FillBoxWithCache(msg_video_, L"session/video_right.xml");
	else
		GlobalManager::FillBoxWithCache(msg_video_, L"session/video_left.xml");
	bubble_box_->Add( msg_video_ );

	image_ = msg_video_->FindSubControl(L"image");

	msg_video_->AttachClick(nbase::Bind(&MsgBubbleVideo::OnClicked, this, std::placeholders::_1));
	msg_video_->AttachMenu(nbase::Bind(&MsgBubbleVideo::OnMenu, this, std::placeholders::_1));

	prog_download_panel_ = dynamic_cast<ui::Box*>(FindSubControl(L"prog_download_panel"));
	prog_download_ = dynamic_cast<ui::Label*>(FindSubControl(L"prog_download"));
}

void MsgBubbleVideo::InitInfo(const nim::IMMessage &msg)
{
	__super::InitInfo(msg);
	InitResPath(msg_);
	if(CheckThumbImage())
		image_->SetBkImage(thumb_);
}
bool MsgBubbleVideo::OnClicked( ui::EventArgs* arg )
{
	if (NeedDownloadResource())
	{
		prog_download_panel_->SetVisible(true);
		SetEnabled(false);
		nim::NOS::FetchMedia(this->msg_,
			ToWeakCallback(std::bind(&MsgBubbleVideo::OnVideoDownLoadResult, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)),
			ToWeakCallback(std::bind(&MsgBubbleVideo::OnVideoDownLoadProgress, this, std::placeholders::_1, std::placeholders::_2)));
	}
	else
	{
		PlayVideo();
	}
	
	return true;
}

//bool MsgBubbleVideo::OnClicked( ui::EventArgs* arg )
//{
//	if( nbase::FilePathIsExist(path_, false) )
//	{
//		std::string strPath = nbase::UTF16ToUTF8(path_);
//		std::string strPath_mp4 = nbase::UTF16ToUTF8(path_) + ".mp4";
//		std::wstring wstrPath = nbase::UTF8ToUTF16(strPath_mp4);
//		if (!nbase::FilePathIsExist(wstrPath, false))
//		{
//			CopyFile(path_.data(), wstrPath.data(),true);
//		}
//		//rename(strPath.data(), strPath_mp4.data());
//		HINSTANCE hRslt = ShellExecute(NULL, L"open", wstrPath.data(), NULL, NULL, SW_SHOWNORMAL);
//		if (hRslt <= (HINSTANCE)HINSTANCE_ERROR)
//		{
//			ShowMsgBox(this->GetWindow()->GetHWND(), MsgboxCallback(), L"STRID_SESSION_PLEASE_INSTALL_MP4APP", true);
//		}
//	}
//	else //原视频不存在
//	{
//		//ImageViewManager::GetInstance()->StartViewPic(path_, L"", true);
//		nim::NOS::DownloadMediaCallback cb = [](nim::NIMResCode res_code, const std::string& file_path, const std::string&, const std::string&)
//		{
//			if (res_code == nim::kNIMResSuccess)
//			{
//				/*ImageViewManager* image_view_mgr = ImageViewManager::GetInstance();
//				if (image_view_mgr)
//					image_view_mgr->OnImageReady(nbase::UTF8ToUTF16(file_path), true);*/
//				std::wstring path = nbase::UTF8ToUTF16(file_path);
//				std::string strPath_mp4 = file_path + ".mp4";
//				std::wstring wstrPath = nbase::UTF8ToUTF16(strPath_mp4);////
//
//				if (!nbase::FilePathIsExist(wstrPath, false))
//				{
//					CopyFile(path.data(), wstrPath.data(), true);
//				}
//				//rename(strPath.data(), strPath_mp4.data());
//				
//				HINSTANCE hRslt = ShellExecute(NULL, L"open", wstrPath.data(), NULL, NULL, SW_SHOWNORMAL);
//				//rename(strPath_mp4.data(), strPath.data());
//				if (hRslt <= (HINSTANCE)HINSTANCE_ERROR)
//				{
//					//返回失败提示
//					std::string id = nim_ui::LoginManager::GetInstance()->GetAccount();
//					SessionBox* session = SessionManager::GetInstance()->FindSessionBox(id);
//					ShowMsgBox(session->GetWindow()->GetHWND(), MsgboxCallback(), L"STRID_SESSION_PLEASE_INSTALL_MP4APP", true);//
//				}
//			}
//			else
//			{
//				//返回失败提示
//				std::string id = nim_ui::LoginManager::GetInstance()->GetAccount();
//				SessionBox* session = SessionManager::GetInstance()->FindSessionBox(id);
//				ShowMsgBox(session->GetWindow()->GetHWND(), MsgboxCallback(), L"STRID_SESSION_DOWNLOAD_FAIL", true);
//			}
//		};
//		nim::NOS::FetchMedia(msg_, cb, nim::NOS::ProgressCallback());
//	}
//	return true;
//}

bool MsgBubbleVideo::OnMenu( ui::EventArgs* arg )
{
	PopupMenu(false, true);
	return false;
}

bool MsgBubbleVideo::NeedDownloadResource()
{
	if( nbase::FilePathIsExist(path_, false) )
		return false;
	else
		return true;
}
void MsgBubbleVideo::OnDownloadCallback(bool success, const std::string& file_path)
{
	ASSERT(!file_path.empty());
	std::wstring wpath = nbase::UTF8ToUTF16(file_path);
	if (wpath == thumb_) //下载的是缩略图
	{
		if (success)
		{
			if (CheckThumbImage())
				image_->SetBkImage(thumb_);
		}		
	}
	else if (wpath == path_) //下载的是源资源
	{
		if (success)
		{
		}
		else
		{			
		}
	}
}
void MsgBubbleVideo::PlayVideo()
{
#ifdef SUPPORTLOCALPLAYER
	{
		auto osversion = shared::tools::GetOSVersion();
		if (osversion >= 600)
		{
			static LocalVideoPlayerForm* player = nullptr;
			if (player == nullptr)
			{
				player = nim_comp::WindowsManager::SingletonShow<LocalVideoPlayerForm>(LocalVideoPlayerForm::kClassName, [&]() {
					player = nullptr;
				}, L"");
			}
			if (player != nullptr)
			{
				if (!player->SamePath(path_))
					player->CenterWindow();
				player->Play(path_);
				player->ToTopMost(false);
			}
		}
		else
		{
			auto tempPath = path_;
			tempPath.append(L".mp4");
			if (!nbase::FilePathIsExist(tempPath, false))
				nbase::CopyFile(path_, tempPath);
			Post2GlobalMisc(nbase::Bind(&shared::tools::SafeOpenUrl, tempPath, SW_SHOW));
		}
	}
#else
	{
		auto tempPath = path_;
		tempPath.append(L".mp4");
		if (!nbase::FilePathIsExist(tempPath, false))
			nbase::CopyFile(path_, tempPath);
		Post2GlobalMisc(nbase::Bind(&shared::tools::SafeOpenUrl, tempPath, SW_SHOW));
	}	
#endif
}
void  MsgBubbleVideo::OnVideoDownLoadResult(nim::NIMResCode res_code, const std::string& file_path, const std::string& call_id, const std::string& res_id)
{
	prog_download_panel_->SetVisible(false);
	SetEnabled(true);
	prog_download_->SetText(L"0%%");
	if (res_code == nim::NIMResCode::kNIMResSuccess)
	{
		PlayVideo();
	}
}
/**
* 使下载视频文件进度回调
* @return void 无返回值
*/
void  MsgBubbleVideo::OnVideoDownLoadProgress(int64_t completed_size, int64_t file_size)
{
	if (completed_size >= file_size)
		return;
	prog_download_->SetText(nbase::StringPrintf(L"%2d%%", (int)(completed_size*1.0 / file_size * 100)));	
}
}