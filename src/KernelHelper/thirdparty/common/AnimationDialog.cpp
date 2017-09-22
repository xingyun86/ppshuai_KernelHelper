// CAnimationDialog.cpp : implementation file
//

#include "Player.h"
#include "AnimationDialog.h"
#include "afxdialogex.h"


#define DEFAULT_IMAGES_COUNTER	5 //750ms
#define DEFAULT_ELAPSE_TIMEOUT	750 //750ms
#define IDC_TIMER_EVENT			1001 //事件消息标识定义

// CAnimationDialog dialog

IMPLEMENT_DYNAMIC(CAnimationDialog, CDialogEx)

CAnimationDialog::CAnimationDialog(CWnd* pParent /*=NULL*/, 
	LPCTSTR pImagesName/* = _T("")*/, LPCTSTR pShowTips/* = _T("")*/)
	: CDialogEx(IDD_ANIMATION_DIALOG, pParent)
	, m_nElapse(DEFAULT_ELAPSE_TIMEOUT)
	, m_nCount(DEFAULT_IMAGES_COUNTER)
	, m_nIndex(0)
{
	m_wstrImagesName = pImagesName;
	m_wstrShowTips = pShowTips;
	m_rect.left = m_rect.top = 0;
	m_rect.right = m_rect.bottom = 360;
}

CAnimationDialog::~CAnimationDialog()
{
}

void CAnimationDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAnimationDialog, CDialogEx)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CAnimationDialog message handlers
///////////////////////////////////////////////////////////
//如下代码段实现的功能是从指定的路径中读取图片，并显示出来
//
void ImagesRenderDisplay(HDC hDC, RECT * pRect, _TCHAR * tImagePath)
{
	HANDLE hFile = NULL;

	DWORD dwReadedSize = 0; //保存实际读取的文件大小
	IStream *pIStream = NULL;//创建一个IStream接口指针，用来保存图片流
	IPicture *pIPicture = NULL;//创建一个IPicture接口指针，表示图片对象
	VOID * pImageMemory = NULL;
	HGLOBAL hImageMemory = NULL;
	OLE_XSIZE_HIMETRIC hmWidth = 0;
	OLE_YSIZE_HIMETRIC hmHeight = 0;
	LARGE_INTEGER liFileSize = { 0, 0 };

	hFile = ::CreateFile(tImagePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);//从指定的路径szImagePath中读取文件句柄
	if (hFile != INVALID_HANDLE_VALUE)
	{
		liFileSize.LowPart = ::GetFileSize(hFile, (DWORD *)&liFileSize.HighPart); //获得图片文件的大小，用来分配全局内存
		hImageMemory = ::GlobalAlloc(GMEM_MOVEABLE, liFileSize.QuadPart); //给图片分配全局内存
		if (hImageMemory)
		{
			pImageMemory = ::GlobalLock(hImageMemory); //锁定内存

			::ReadFile(hFile, pImageMemory, liFileSize.QuadPart, &dwReadedSize, NULL); //读取图片到全局内存当中

			::CreateStreamOnHGlobal(hImageMemory, false, &pIStream); //用全局内存初使化IStream接口指针
			if (pIStream)
			{
				::OleLoadPicture(pIStream, 0, false, IID_IPicture, (LPVOID*)&(pIPicture));//用OleLoadPicture获得IPicture接口指针
				if (pIPicture)														  //得到IPicture COM接口对象后，就可以进行获得图片信息、显示图片等操作
				{
					pIPicture->get_Width(&hmWidth); //用接口方法获得图片的宽和高
					pIPicture->get_Height(&hmHeight); //用接口方法获得图片的宽和高
					pIPicture->Render(hDC, pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top, 0, hmHeight, hmWidth, -hmHeight, NULL); //在指定的DC上绘出图片
				}
			}
		}
	}

	if (hImageMemory)
	{
		::GlobalUnlock(hImageMemory); //解锁内存

		::GlobalFree(hImageMemory); //释放全局内存
		hImageMemory = NULL;
	}
	if (pIPicture)
	{
		pIPicture->Release(); //释放pIPicture
		pIPicture = NULL;
	}
	if (pIStream)
	{
		pIStream->Release(); //释放pIStream
		pIStream = NULL;
	}
	if (hFile != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(hFile); //关闭文件句柄
		hFile = INVALID_HANDLE_VALUE;
	}
}
///////////////////////////////////////////////////////////
//如下代码段实现的功能是从指定的路径中读取图片，并显示出来
//
void ImagesDisplayScreen(SIZE & szImageSize, _TCHAR * tImagePath)
{
	HANDLE hFile = NULL;

	DWORD dwReadedSize = 0; //保存实际读取的文件大小
	IStream *pIStream = NULL;//创建一个IStream接口指针，用来保存图片流
	IPicture *pIPicture = NULL;//创建一个IPicture接口指针，表示图片对象
	VOID * pImageMemory = NULL;
	HGLOBAL hImageMemory = NULL;
	OLE_XSIZE_HIMETRIC hmWidth = 0;
	OLE_YSIZE_HIMETRIC hmHeight = 0;
	LARGE_INTEGER liFileSize = { 0, 0 };

	hFile = ::CreateFile(tImagePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);//从指定的路径szImagePath中读取文件句柄
	if (hFile != INVALID_HANDLE_VALUE)
	{
		liFileSize.LowPart = ::GetFileSize(hFile, (DWORD *)&liFileSize.HighPart); //获得图片文件的大小，用来分配全局内存
		hImageMemory = ::GlobalAlloc(GMEM_MOVEABLE, liFileSize.QuadPart); //给图片分配全局内存
		if (hImageMemory)
		{
			pImageMemory = ::GlobalLock(hImageMemory); //锁定内存

			::ReadFile(hFile, pImageMemory, liFileSize.QuadPart, &dwReadedSize, NULL); //读取图片到全局内存当中

			::CreateStreamOnHGlobal(hImageMemory, false, &pIStream); //用全局内存初使化IStream接口指针
			if (pIStream)
			{
				::OleLoadPicture(pIStream, 0, false, IID_IPicture, (LPVOID*)&(pIPicture));//用OleLoadPicture获得IPicture接口指针
				if (pIPicture)														  //得到IPicture COM接口对象后，就可以进行获得图片信息、显示图片等操作
				{
					pIPicture->get_Width(&hmWidth); //用接口方法获得图片的宽和高
					pIPicture->get_Height(&hmHeight); //用接口方法获得图片的宽和高
					szImageSize.cx = hmWidth;
					szImageSize.cy = hmHeight;
				}
			}
		}
	}

	if (hImageMemory)
	{
		::GlobalUnlock(hImageMemory); //解锁内存

		::GlobalFree(hImageMemory); //释放全局内存
		hImageMemory = NULL;
	}
	if (pIPicture)
	{
		pIPicture->Release(); //释放pIPicture
		pIPicture = NULL;
	}
	if (pIStream)
	{
		pIStream->Release(); //释放pIStream
		pIStream = NULL;
	}
	if (hFile != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(hFile); //关闭文件句柄
		hFile = INVALID_HANDLE_VALUE;
	}
}

BOOL CAnimationDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	::SetWindowLong(this->m_hWnd, GWL_STYLE, GetWindowLong(this->m_hWnd, GWL_STYLE) & (~WS_CAPTION));
	::SetWindowPos(this->m_hWnd, HWND_NOTOPMOST, 0, 0, 
		m_rect.right - m_rect.left, m_rect.bottom - m_rect.top, SWP_HIDEWINDOW);
	CenterWindowInScreen(this->m_hWnd);
	
	_TCHAR tImagePath[MAX_PATH] = { 0 };
	_stprintf_s(tImagePath, sizeof(tImagePath) / sizeof(_TCHAR), _T("%s%d.jpg"), m_wstrImagesName.c_str(), m_nIndex++);
	RECT rc = { 0 };
	GetClientRect(&rc);
	ImagesRenderDisplay(m_hRenderDC, &rc, tImagePath);

	m_nIndex %= m_nCount;

	::ShowWindow(this->m_hWnd, SW_SHOW);
	::SetForegroundWindow(this->m_hWnd);

	m_hRenderDC = ::GetDC(this->m_hWnd);//设备环境

	::SetTimer(this->m_hWnd, IDC_TIMER_EVENT, m_nElapse, NULL);

	return TRUE;
}

void CAnimationDialog::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	switch (nIDEvent)
	{
	case IDC_TIMER_EVENT:
	{
		_TCHAR tImagePath[MAX_PATH] = { 0 };
		_stprintf_s(tImagePath, sizeof(tImagePath) / sizeof(_TCHAR), _T("%s%d.jpg"), m_wstrImagesName.c_str(), m_nIndex++);
		RECT rc = { 0 };
		GetClientRect(&rc);
		ImagesRenderDisplay(m_hRenderDC, &rc, tImagePath);

		m_nIndex %= m_nCount;
		//::KillTimer(this->m_hWnd, nIDEvent);
	}
		break;
	default:
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}

void CAnimationDialog::StartAnimation()
{
	this->DoModal();
}

void CAnimationDialog::StartAnimation(int nElapse, RECT * pRect)
{
	m_nElapse = nElapse;
	memcpy(&m_rect, pRect, sizeof(m_rect));	
	this->DoModal();
}

void CAnimationDialog::CloseAnimation()
{
	::KillTimer(this->m_hWnd, IDC_TIMER_EVENT);
	this->OnCancel();//停止并取消所有的用户请求
	this->DestroyWindow();//销毁窗口
	this->PostModal();//通知窗口退出消息循环
}