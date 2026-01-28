#include "shared/image_ole_i.h"
#include "richedit_olecallback.h"
#include "richedit_util.h"
#include "module/dragdrop/drag_drop.h"
#include <gdiplus.h>  
using namespace Gdiplus;

namespace nim_comp
{
IRichEditOleCallbackEx::IRichEditOleCallbackEx(ITextServices * text_services, std::function<void()> callback)
{
	pStorage = NULL;
	m_iNumStorages = 0;
	m_dwRef = 0;
	cf_nim_format_ = RegisterClipboardFormat(L"Nim_RichEdit_Format");
	cf_html_format_ = RegisterClipboardFormat(L"HTML Format");
	cf_qq_format_ = RegisterClipboardFormat(L"QQ_Unicode_RichEdit_Format");
	//cf_img_format_ = RegisterClipboardFormat(L"Rich Text Format");
	text_services_ = text_services;
	callback_ = callback;
	custom_mode_ = true;
}

IRichEditOleCallbackEx::~IRichEditOleCallbackEx()
{
	if (pStorage)
	{
		pStorage->Release();
	}
}
void IRichEditOleCallbackEx::SetCustomMode(bool custom)
{
	custom_mode_ = custom;
}

HRESULT STDMETHODCALLTYPE IRichEditOleCallbackEx::GetNewStorage(LPSTORAGE* lplpstg)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE IRichEditOleCallbackEx::QueryInterface(REFIID iid, void ** ppvObject)
{

	HRESULT hr = S_OK;
	*ppvObject = NULL;

	if ( iid == IID_IUnknown ||
		iid == IID_IRichEditOleCallback )
	{
		*ppvObject = this;
		AddRef();
		hr = NOERROR;
	}
	else
	{
		hr = E_NOINTERFACE;
	}

	return hr;
}

ULONG STDMETHODCALLTYPE IRichEditOleCallbackEx::AddRef()
{
	return ++m_dwRef;
}

ULONG STDMETHODCALLTYPE IRichEditOleCallbackEx::Release()
{
	if ( --m_dwRef == 0 )
	{
		delete this;
		return 0;
	}

	return m_dwRef;
}

HRESULT STDMETHODCALLTYPE IRichEditOleCallbackEx::GetInPlaceContext(LPOLEINPLACEFRAME FAR *lplpFrame,
	LPOLEINPLACEUIWINDOW FAR *lplpDoc, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE IRichEditOleCallbackEx::ShowContainerUI(BOOL fShow)
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE IRichEditOleCallbackEx::QueryInsertObject(LPCLSID lpclsid, LPSTORAGE lpstg, LONG cp)
{
	if (*lpclsid == CLSID_ImageOle)
		return S_OK;
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE IRichEditOleCallbackEx::DeleteObject(LPOLEOBJECT lpoleobj)
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE IRichEditOleCallbackEx::QueryAcceptData(LPDATAOBJECT lpdataobj, CLIPFORMAT FAR *lpcfFormat,
	DWORD reco, BOOL fReally, HGLOBAL hMetaPict)
{
	if (lpdataobj && fReally && (reco == RECO_DROP || reco == RECO_PASTE))
	{
		STGMEDIUM stgMedium;
		HRESULT ret;
		FORMATETC cFmt = {(CLIPFORMAT) cf_nim_format_, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
		ret = lpdataobj->GetData(&cFmt, &stgMedium);
		if (ret == S_OK)
		{
			std::string fragment;
			fragment = (LPSTR)GlobalLock(stgMedium.hGlobal);
			GlobalUnlock(stgMedium.hGlobal);
			Re_InsertNimTextInfo(text_services_, fragment, custom_mode_, callback_);
			::ReleaseStgMedium(&stgMedium);
			return S_FALSE;
		}
		
		/*cFmt = { (CLIPFORMAT)cf_qq_format_, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		ret = lpdataobj->GetData(&cFmt, &stgMedium);
		if (ret == S_OK)
		{
			std::string fragment;
			fragment = (LPSTR)GlobalLock(stgMedium.hGlobal);
			GlobalUnlock(stgMedium.hGlobal);
			bool tag = Re_InsertQQTextInfo(text_services_, fragment, custom_mode_, callback_);
			::ReleaseStgMedium(&stgMedium);
			if (tag)
			{
				return S_FALSE;
			}
		}*/

		/*cFmt = { (CLIPFORMAT)cf_html_format_, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		ret = lpdataobj->GetData(&cFmt, &stgMedium);
		if (ret == S_OK)
		{
			std::string fragment;
			fragment = (LPSTR)GlobalLock(stgMedium.hGlobal);
			GlobalUnlock(stgMedium.hGlobal);
			Re_InsertHtmlInfo(text_services_, fragment, custom_mode_, callback_);
			::ReleaseStgMedium(&stgMedium);
			return S_FALSE;
		}

		cFmt = { (CLIPFORMAT)CF_BITMAP, NULL, DVASPECT_CONTENT, -1, TYMED_GDI };
		ret = lpdataobj->GetData(&cFmt, &stgMedium);
		if (ret == S_OK)
		{
			//BitmapData *data;
			//std::string fragment;
			std::wstring szfilename = L"";
			std::wstring dir = nbase::win32::GetLocalAppDataDir() + L"screenshot/";
			if (!nbase::FilePathIsExist(dir, true))
			{
				nbase::win32::CreateDirectoryRecursively(dir.c_str());
			}
			szfilename = dir + L"temp.png";
			SaveBitmapToFile(stgMedium.hBitmap, szfilename);
			//data->Scan0 = (LPSTR)GlobalLock(stgMedium.hBitmap);
			//fragment = (LPSTR)GlobalLock(stgMedium.hBitmap);
			GlobalUnlock(stgMedium.hGlobal);
			Re_InsertScreenshotInfo(text_services_, szfilename, custom_mode_, callback_);
			::ReleaseStgMedium(&stgMedium);
			return S_FALSE;
		}

		cFmt = { (CLIPFORMAT)CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		ret = lpdataobj->GetData(&cFmt, &stgMedium);
		if (ret == S_OK)
		{
			HDROP hdrop = (HDROP)GlobalLock(stgMedium.hGlobal);
			int count = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
			for (int i = 0; i < count; ++i) {
				wchar_t path[_MAX_PATH];
				DragQueryFile(hdrop, i, path, _MAX_PATH);
				std::wstring nim_text = path;
				Re_InsertFileInfo(text_services_, nim_text, custom_mode_, callback_);
			}
			GlobalUnlock(stgMedium.hGlobal);
			ReleaseStgMedium(&stgMedium);
			return S_FALSE;
		}*/

		/*if (OpenClipboard(NULL) &&
			IsClipboardFormatAvailable(CF_BITMAP))
		{
			// 取得剪切板中的图片数据  
			HBITMAP hBitmap = (HBITMAP) ::GetClipboardData(CF_BITMAP);
			if (hBitmap)
			{
				//从剪贴板中取出一个内存的句柄  
				// 获取内存大小  
				int nSize = GlobalSize(hBitmap);
				//对内存块进行加锁，将内存句柄值转化为一个指针,并将内存块的引用计数器加一，内存中的数据也返回到指针型变量中  
				char *pchPic = new char[nSize];
				memcpy(pchPic, GlobalLock(hBitmap), nSize);
				// 将数据存储  
				//将内存块的引用计数器减一  
				GlobalUnlock(hBitmap);
				//关闭剪贴板，释放剪贴板资源的占用权  
				CloseClipboard();
			}
		}
		char szText[MAX_PATH];
		if (::OpenClipboard(NULL)) {
			HANDLE hData = ::GetClipboardData(CF_TEXT);
			if (hData != NULL) {
				LPCSTR pData = (LPCSTR) ::GlobalLock(hData);
				//if (::lstrlen(pData) < MAX_PATH)
					//::lstrcpy(szText, pData);
				::GlobalUnlock(hData);
			}
			::CloseClipboard();
		}*/
	}
	return S_OK;  
}

BOOL IRichEditOleCallbackEx::SaveBitmapToFile(HBITMAP   hBitmap, std::wstring szfilename)
{
	HDC     hDC;
	//当前分辨率下每象素所占字节数            
	int     iBits;
	//位图中每象素所占字节数            
	WORD     wBitCount;
	//定义调色板大小，     位图中像素字节大小     ，位图文件大小     ，     写入文件字节数                
	DWORD     dwPaletteSize = 0, dwBmBitsSize = 0, dwDIBSize = 0, dwWritten = 0;
	//位图属性结构                
	BITMAP     Bitmap;
	//位图文件头结构            
	BITMAPFILEHEADER     bmfHdr;
	//位图信息头结构                
	BITMAPINFOHEADER     bi;
	//指向位图信息头结构                    
	LPBITMAPINFOHEADER     lpbi;
	//定义文件，分配内存句柄，调色板句柄                
	HANDLE     fh, hDib, hPal, hOldPal = NULL;

	//计算位图文件每个像素所占字节数                
	hDC = CreateDC(L"DISPLAY", NULL, NULL, NULL);
	iBits = GetDeviceCaps(hDC, BITSPIXEL)     *     GetDeviceCaps(hDC, PLANES);
	DeleteDC(hDC);
	if (iBits <= 1)
		wBitCount = 1;
	else  if (iBits <= 4)
		wBitCount = 4;
	else if (iBits <= 8)
		wBitCount = 8;
	else
		wBitCount = 24;

	GetObject(hBitmap, sizeof(Bitmap), (LPSTR)&Bitmap);
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = Bitmap.bmWidth;
	bi.biHeight = Bitmap.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = wBitCount;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrImportant = 0;
	bi.biClrUsed = 0;

	dwBmBitsSize = ((Bitmap.bmWidth *wBitCount + 31) / 32) * 4 * Bitmap.bmHeight;

	//为位图内容分配内存                
	hDib = GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
	lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
	*lpbi = bi;

	//     处理调色板                    
	hPal = GetStockObject(DEFAULT_PALETTE);
	if (hPal)
	{
		hDC = ::GetDC(NULL);
		hOldPal = ::SelectPalette(hDC, (HPALETTE)hPal, FALSE);
		RealizePalette(hDC);
	}

	//     获取该调色板下新的像素值                
	GetDIBits(hDC, hBitmap, 0, (UINT)Bitmap.bmHeight,
		(LPSTR)lpbi + sizeof(BITMAPINFOHEADER) + dwPaletteSize,
		(BITMAPINFO *)lpbi, DIB_RGB_COLORS);

	//恢复调色板                    
	if (hOldPal)
	{
		::SelectPalette(hDC, (HPALETTE)hOldPal, TRUE);
		RealizePalette(hDC);
		::ReleaseDC(NULL, hDC);
	}

	//创建位图文件                    
	fh = CreateFile(szfilename.data(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (fh == INVALID_HANDLE_VALUE)         return     FALSE;

	//     设置位图文件头                
	bmfHdr.bfType = 0x4D42;     //     "BM"                
	dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;
	bmfHdr.bfSize = dwDIBSize;
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;
	//     写入位图文件头                
	WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
	//     写入位图文件其余内容                
	WriteFile(fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL);
	//清除                    
	GlobalUnlock(hDib);
	GlobalFree(hDib);
	CloseHandle(fh);

	return     TRUE;

}

HRESULT STDMETHODCALLTYPE IRichEditOleCallbackEx::ContextSensitiveHelp(BOOL fEnterMode)
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE IRichEditOleCallbackEx::GetClipboardData(CHARRANGE FAR *lpchrg, DWORD reco, LPDATAOBJECT FAR *lplpdataobj)
{
	if (text_services_)
	{
		std::wstring strText;
		std::wstring nim_format;
		bool bCustomObject = Re_GetNimTextInfo(text_services_, lpchrg, strText, nim_format);

		if (strText.empty() && nim_format.empty())
		{
			return E_NOTIMPL;
		}
		if (*lplpdataobj == NULL)
		{
			*lplpdataobj = new SdkDataObject;
		}
		SaveStr2DataObject(*lplpdataobj, strText, CF_TEXT);
		SaveStr2DataObject(*lplpdataobj, strText, CF_UNICODETEXT);
		if (bCustomObject)
		{
			SaveStr2DataObject(*lplpdataobj, nim_format, cf_nim_format_);
		}
		return S_OK;
	} 
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE IRichEditOleCallbackEx::GetDragDropEffect(BOOL fDrag, DWORD grfKeyState, LPDWORD pdwEffect)
{
	//return S_OK;
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE IRichEditOleCallbackEx::GetContextMenu(WORD seltyp, LPOLEOBJECT lpoleobj, CHARRANGE FAR *lpchrg,
	HMENU FAR *lphmenu)
{
	return S_OK;
}
HRESULT IRichEditOleCallbackEx::SaveStr2DataObject(LPDATAOBJECT pDataSource, std::wstring str, CLIPFORMAT cfFormat)
{
	HGLOBAL hG = NULL;
	if (cfFormat == CF_UNICODETEXT)
	{
		int  strBytes=  str.length() * sizeof(wchar_t);  
		wchar_t *temp = new wchar_t[str.length()+1];
		ZeroMemory(temp, strBytes+sizeof(wchar_t));
		memcpy(temp, str.c_str(), strBytes);
		hG = GlobalAlloc(GMEM_DDESHARE, strBytes+sizeof(wchar_t));  
		void* pBuffer = GlobalLock(hG);  
		{  
			memcpy(pBuffer, temp, strBytes+sizeof(wchar_t)); 
			GlobalUnlock(hG);  
		} 
		delete[] temp;
	} 
	else
	{
		std::string tmp;
		if (cfFormat == CF_TEXT)
		{
			int len = WideCharToMultiByte(CP_ACP, 0, str.c_str(), -1, NULL, 0, NULL, NULL);
			if (len > 0)
			{
				std::unique_ptr<char[]> gbk_text(new char[len]);
				WideCharToMultiByte(CP_ACP, 0, str.c_str(), -1, gbk_text.get(), len, NULL, NULL);
				tmp = gbk_text.get();
			}

		}
		else
		{
			tmp = nbase::UTF16ToUTF8(str);
		}
		int  strBytes=  tmp.length() * sizeof(char);  
		char *temp = new char[tmp.length()+1];
		ZeroMemory(temp, strBytes+sizeof(char));
		memcpy(temp, tmp.c_str(), strBytes);
		hG = GlobalAlloc(GMEM_DDESHARE, strBytes+sizeof(char));  
		void* pBuffer = GlobalLock(hG);  
		{  
			memcpy(pBuffer, temp, strBytes+sizeof(char)); 
			GlobalUnlock(hG);  
		} 
		delete[] temp;
	}

	FORMATETC fmt;  
	fmt.cfFormat = cfFormat;  
	fmt.dwAspect = DVASPECT_CONTENT;  
	fmt.lindex = -1;  
	fmt.ptd = NULL;  
	fmt.tymed = TYMED_HGLOBAL;  

	STGMEDIUM stg;  
	stg.tymed = TYMED_HGLOBAL;  
	stg.hGlobal = hG;  
	stg.pUnkForRelease = NULL;  

	pDataSource->SetData(&fmt, &stg, FALSE);

	return S_OK;
}

}