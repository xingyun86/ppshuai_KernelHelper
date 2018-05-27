#pragma once

#ifndef __COMMONWINDOW_H_
#define __COMMONWINDOW_H_

#include <richedit.h>
#include <gdiplus.h>
#include <base64.h>

namespace PPSHUAI
{
	namespace COMMONWINDOW
	{
		class CAnimationDisplayer : public Gdiplus::Image
		{
		public:

			CAnimationDisplayer(const WCHAR* filename, BOOL useEmbeddedColorManagement = FALSE)
				: Image(filename, useEmbeddedColorManagement)
			{
				Initialize();

				m_bIsInitialized = true;

				TestForAnimatedGIF();
			}

			~CAnimationDisplayer()
			{
				Destroy();
			}

		public:

			void Draw(HDC hDC);

			void GetSize(SIZE & size)
			{
				size.cx = GetWidth();
				size.cy = GetHeight();
				//CSize(GetWidth(), GetHeight());
			}

			bool	IsAnimatedGIF()
			{
				return m_nFrameCount > 1;
			}
			void	SetPause(bool bPaused)
			{
				if (!IsAnimatedGIF())
					return;

				if (bPaused && !m_bPaused)
				{
					::ResetEvent(m_hPaused);
				}
				else
				{
					if (m_bPaused && !bPaused)
					{
						::SetEvent(m_hPaused);
					}
				}

				m_bPaused = bPaused;
			}
			bool	IsPaused()
			{
				return m_bPaused;
			}
			bool	InitAnimation(HWND hWnd, POINT pt)
			{
				m_hWnd = hWnd;
				m_pt = pt;

				if (!m_bIsInitialized)
				{
					//TRACE(_T("GIF not initialized\n"));
					return false;
				};

				if (IsAnimatedGIF())
				{
					if (m_hThread == NULL)
					{

						DWORD dwThreadID = 0;

						m_hThread = (HANDLE)::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)_ThreadAnimationProc, this, CREATE_SUSPENDED, &dwThreadID);

						if (!m_hThread)
						{
							//TRACE(_T("Couldn't start a GIF animation thread\n"));
							return true;
						}
						else
						{
							::ResumeThread(m_hThread);
						}
					}
				}

				return false;

			}
			void Destroy()
			{
				if (m_hThread)
				{
					// If pause un pause
					SetPause(false);

					SetEvent(m_hExitEvent);
					WaitForSingleObject(m_hThread, INFINITE);
				}

				CloseHandle(m_hThread);
				CloseHandle(m_hExitEvent);
				CloseHandle(m_hPaused);

				free(m_pPropertyItem);

				m_pPropertyItem = NULL;
				m_hThread = NULL;
				m_hExitEvent = NULL;
				m_hPaused = NULL;

				if (m_pStream)
				{
					m_pStream->Release();
				}

				m_bIsInitialized = false;
			}

		protected:

			bool TestForAnimatedGIF()
			{
				UINT count = 0;
				count = GetFrameDimensionsCount();
				GUID* pDimensionIDs = new GUID[count];

				// Get the list of frame dimensions from the Image object.
				GetFrameDimensionsList(pDimensionIDs, count);

				// Get the number of frames in the first dimension.
				m_nFrameCount = GetFrameCount(&pDimensionIDs[0]);

				// Assume that the image has a property item of type PropertyItemEquipMake.
				// Get the size of that property item.
				int nSize = GetPropertyItemSize(PropertyTagFrameDelay);

				// Allocate a buffer to receive the property item.
				m_pPropertyItem = (Gdiplus::PropertyItem*) malloc(nSize);

				GetPropertyItem(PropertyTagFrameDelay, nSize, m_pPropertyItem);

				delete  pDimensionIDs; pDimensionIDs = NULL;

				return m_nFrameCount > 1;
			}
			void Initialize()
			{
				m_pStream = NULL;
				m_nFramePosition = 0;
				m_nFrameCount = 0;
				m_hThread = NULL;
				m_bIsInitialized = false;
				m_pPropertyItem = NULL;
				//lastResult = InvalidParameter;

#ifdef INDIGO_CTRL_PROJECT
				m_hInst = _Module.GetResourceInstance();
#else
				m_hInst = GetModuleHandle(NULL);//AfxGetResourceHandle();
#endif

				m_bPaused = false;

				m_hExitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

				m_hPaused = CreateEvent(NULL, TRUE, TRUE, NULL);

			}
			bool				DrawFrameGIF()
			{

				::WaitForSingleObject(m_hPaused, INFINITE);

				GUID pageGuid = Gdiplus::FrameDimensionTime;

				LONG hmWidth = GetWidth();
				LONG hmHeight = GetHeight();

				HDC hDC = GetDC(m_hWnd);
				if (hDC)
				{
					Gdiplus::Graphics graphics(hDC);
					graphics.DrawImage((Gdiplus::Image *)this, (INT)m_pt.x, (INT)m_pt.y, (INT)hmWidth, (INT)hmHeight);
					ReleaseDC(m_hWnd, hDC);
				}

				SelectActiveFrame(&pageGuid, m_nFramePosition++);

				if (m_nFramePosition == m_nFrameCount)
					m_nFramePosition = 0;


				long lPause = ((long*)m_pPropertyItem->value)[m_nFramePosition] * 10;

				DWORD dwErr = WaitForSingleObject(m_hExitEvent, lPause);

				return dwErr == WAIT_OBJECT_0;
			}

			IStream*			m_pStream;

			bool LoadFromBuffer(BYTE* pBuff, int nSize)
			{
				bool bResult = false;

				HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, nSize);
				if (hGlobal)
				{
					void* pData = GlobalLock(hGlobal);
					if (pData)
						memcpy(pData, pBuff, nSize);

					GlobalUnlock(hGlobal);

					if (CreateStreamOnHGlobal(hGlobal, TRUE, &m_pStream) == S_OK)
						bResult = true;

				}


				return bResult;
			}
			bool GetResource(LPCTSTR lpName, LPCTSTR lpType, void* pResource, int& nBufSize)
			{
				HRSRC		hResInfo;
				HANDLE		hRes;
				LPSTR		lpRes = NULL;
				int			nLen = 0;
				bool		bResult = FALSE;

				// Find the resource

				hResInfo = FindResource(m_hInst, lpName, lpType);
				if (hResInfo == NULL)
				{
					DWORD dwErr = GetLastError();
					return false;
				}

				// Load the resource
				hRes = LoadResource(m_hInst, hResInfo);

				if (hRes == NULL)
					return false;

				// Lock the resource
				lpRes = (char*)LockResource(hRes);

				if (lpRes != NULL)
				{
					if (pResource == NULL)
					{
						nBufSize = SizeofResource(m_hInst, hResInfo);
						bResult = true;
					}
					else
					{
						if (nBufSize >= (int)SizeofResource(m_hInst, hResInfo))
						{
							memcpy(pResource, lpRes, nBufSize);
							bResult = true;
						}
					}

					UnlockResource(hRes);
				}

				// Free the resource
				FreeResource(hRes);

				return bResult;
			}

			bool Load(LPCTSTR sResourceType, LPCTSTR sResource)
			{
				bool bResult = false;


				BYTE*	pBuff = NULL;
				int		nSize = 0;
				if (GetResource(sResource, sResourceType, pBuff, nSize))
				{
					if (nSize > 0)
					{
						pBuff = new BYTE[nSize];

						if (GetResource(sResource, sResourceType, pBuff, nSize))
						{
							if (LoadFromBuffer(pBuff, nSize))
							{

								bResult = true;
							}
						}

						delete[] pBuff;
					}
				}


				m_bIsInitialized = bResult;

				return bResult;
			}

			void ThreadAnimation()
			{
				m_nFramePosition = 0;

				bool bExit = false;
				while (!(bExit = DrawFrameGIF()));
			}

			static UINT WINAPI _ThreadAnimationProc(LPVOID pParam)
			{
				//ASSERT(pParam);
				CAnimationDisplayer *pImage = reinterpret_cast<CAnimationDisplayer *> (pParam);
				pImage->ThreadAnimation();

				return 0;
			}

			HANDLE			m_hThread;
			HANDLE			m_hPaused;
			HANDLE			m_hExitEvent;
			HINSTANCE		m_hInst;
			HWND			m_hWnd;
			UINT			m_nFrameCount;
			UINT			m_nFramePosition;
			bool			m_bIsInitialized;
			bool			m_bPaused;
			Gdiplus::PropertyItem*	m_pPropertyItem;
			POINT			m_pt;
		};

		//  This function is called by the Windows function DispatchMessage()
		//LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
		ShadowWindow::CShadowBorder G_ShadowBorder;
		class CWindowsManager{
		public:

			CWindowsManager(const _TCHAR * ptszFileName = _T("demo.gif"))
			{
				CWindowsManager::m_pAnimation = NULL;
				memset((void *)&CWindowsManager::m_rcWindow, 0, sizeof(RECT));
				if (ptszFileName && _tcslen(ptszFileName))
				{
					_tcscpy(CWindowsManager::m_tszGifFileName, ptszFileName);
				}
				else
				{
					ExitProcess(0L);
				}
			}
			virtual ~CWindowsManager()
			{

			}

		public:

			//  This function is called by the Windows function DispatchMessage()
			__inline static LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
			{
				LRESULT lResult = 0;
				// handle the messages
				switch (message)
				{
				case WM_NCCREATE:
				{
					GUI::CenterWindowInScreen(hWnd);
					lResult = (-1L);
				}
				break;
				case WM_CREATE:
				{
					// Initiation of the shadow
					ShadowWindow::CShadowBorder::Initialize(GetModuleHandle(NULL));

					::SetWindowLong(hWnd, GWL_STYLE, ::GetWindowLong(hWnd, GWL_STYLE) & (~WS_CAPTION));
					::SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_DRAWFRAME);
					//添加阴影效果
					SetClassLong(hWnd, GCL_STYLE, GetClassLong(hWnd, GCL_STYLE) | CS_DROPSHADOW);

					G_ShadowBorder.Create(hWnd);
					G_ShadowBorder.SetSize(4);
					G_ShadowBorder.SetColor(RGB(255, 0, 0));
					G_ShadowBorder.SetPosition(0, 0);

					// GDI+
					m_pAnimation = new CAnimationDisplayer(Convert::TToW(CWindowsManager::m_tszGifFileName).c_str());

					RECT rc = { 0 };
					::GetClientRect(hWnd, &rc);

					int cx = (rc.right - CWindowsManager::m_pAnimation->GetWidth()) / 2;

					POINT pt = { cx, 10 };
					CWindowsManager::m_pAnimation->InitAnimation(hWnd, pt);

					GUI::NotifyUpdate(hWnd, &CWindowsManager::m_rcWindow);
				}
				break;

				case WM_SIZE:
				case WM_SIZING:
				case WM_ENTERSIZEMOVE:
				case WM_EXITSIZEMOVE:
				{
					GUI::NotifyUpdate(hWnd, &CWindowsManager::m_rcWindow);
				}
				break;
				case WM_LBUTTONDOWN:
				{
					GUI::DragMoveFull(hWnd);
				}
				break;
				case WM_PAINT:
				{
					ULONG uARGB[2] = { ARGB(0xFF, 0x7F, 0xFF, 0x7F), ARGB(0xFF, 0xFF, 0x7F, 0x7F) };
					RECT rcWnd = { 0 };
					PAINTSTRUCT ps = { 0 };
					RECT rcMemory = { 8, 8, 8, 8 };
					HDC hDC = ::BeginPaint(hWnd, &ps);
					GetClientRect(hWnd, &rcWnd);
					GUI::DrawMemoryBitmap(hDC, hWnd, rcWnd.right, rcWnd.bottom,
						GUI::DrawAlphaBlendRect(hWnd, uARGB, hDC, &rcMemory));

					::EndPaint(hWnd, &ps);
				}
				break;
				case WM_DESTROY:
				{
					if (CWindowsManager::m_pAnimation)
					{
						delete CWindowsManager::m_pAnimation;
						CWindowsManager::m_pAnimation = NULL;
					}
					// send a WM_QUIT to the message queue
					PostQuitMessage(0);
				}
				break;
				default:
				{
					// for messages that we don't deal with
					return DefWindowProc(hWnd, message, wParam, lParam);
				}
				break;
				}

				return lResult;
			}

			UINT_PTR RunApp()
			{
				HWND hWnd = NULL;
				UINT_PTR uResult = 0;
				HMODULE hModule = NULL;
				INITCOMMONCONTROLSEX iccex = { 0 };
				_TCHAR tzClassName[] = _T("PPSHUAIWINDOW");
				HINSTANCE hInstance = GetModuleHandle(NULL);

				hModule = ::LoadLibrary(_T("msftedit.dll"));
				
				::InitCommonControls();

				iccex.dwSize = sizeof(iccex);
				// 将它设置为包括所有要在应用程序中使用的公共控件类。
				iccex.dwICC = ICC_WIN95_CLASSES;
				::InitCommonControlsEx(&iccex);

				GdiplusDisplay::GdiplusInitialize();

				if (GUI::WindowClassesRegister(hInstance, tzClassName, &CWindowsManager::WindowProcedure))
				{
					//return 0;
				}
				hWnd = GUI::CreateCustomWindow(hInstance, tzClassName);

				uResult = GUI::StartupWindows(hWnd, SW_SHOW);

				GdiplusDisplay::GdiplusExitialize();

				if (hModule)
				{
					::FreeLibrary(hModule);
					hModule = NULL;
				}

				return uResult;
			}

		public:
			//static ShadowWindow::CShadowBorder G_ShadowBorder;
			static CAnimationDisplayer * CWindowsManager::m_pAnimation;
			static RECT CWindowsManager::m_rcWindow;
			static _TCHAR m_tszGifFileName[MAX_PATH + 1];

		private:

		};

		RECT CWindowsManager::m_rcWindow = { 0 };
		_TCHAR CWindowsManager::m_tszGifFileName[MAX_PATH + 1] = { 0 };
		CAnimationDisplayer * CWindowsManager::m_pAnimation = NULL;

		class CFileIconWindow{
		public:

			CFileIconWindow()
			{
				this->ResetParam();
			}
			virtual ~CFileIconWindow()
			{
			}

		public:

			//  This function is called by the Windows function DispatchMessage()
			__inline static LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
			{
				LRESULT lResult = 0;
				std::map<TSTRING, SIZE_T> * pTS = NULL;

				pTS = (std::map<tstring, SIZE_T> *)GUI::GetWindowUserData(hWnd);

				// handle the messages
				switch (message)
				{
				case WM_NCCREATE:
				{
					GUI::CenterWindowInScreen(hWnd);
					lResult = (-1L);
				}
				break;
				case WM_CREATE:
				{
					//::SetWindowLong(hWnd, GWL_STYLE, ::GetWindowLong(hWnd, GWL_STYLE) & (~WS_CAPTION));
					//::SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_DRAWFRAME);

					//添加阴影效果
					//SetClassLong(hWnd, GCL_STYLE, GetClassLong(hWnd, GCL_STYLE) | CS_DROPSHADOW);
					{
						HICON hIcon = NULL;
						RECT rcRect = { 0 };
						HICON hOldIcon = NULL;
						HICON hNewIcon = NULL;
						HWND hOldIconWnd = NULL;
						HWND hNewIconWnd = NULL;
						HWND hOldTextWnd = NULL;
						HWND hNewTextWnd = NULL;
						SIZE sTextSize = { 128, 20 };
						SIZE sIconSize = { 128, 128 };
						if (!pTS)
						{
							pTS = (std::map<TSTRING, SIZE_T> *)*(LPVOID *)(((CREATESTRUCT *)lParam)->lpCreateParams);
						}
						if (pTS)
						{
							GUI::SetWindowUserData(hWnd, (LONG_PTR)pTS);

							SetWindowText(hWnd, _T("修改程序图标"));
							////////////////////////////////////////////////////////////
							//	左上(图标窗口)
							rcRect.left = 0;
							rcRect.top = 0;
							rcRect.right = rcRect.left + sIconSize.cx;
							rcRect.bottom = rcRect.top + sIconSize.cy;

							hOldIconWnd = GUI::CreateCustomWindow(GetModuleHandle(NULL),
								WC_STATIC,
								_T("旧图标"),
								WS_BORDER | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SS_ICON | SS_CENTERIMAGE,
								NULL,
								rcRect, hWnd);

							////////////////////////////////////////////////////////////
							//	右上(图标窗口)
							rcRect.left = sIconSize.cx;
							rcRect.top = 0;
							rcRect.right = rcRect.left + sIconSize.cx;
							rcRect.bottom = rcRect.top + sIconSize.cy;

							hNewIconWnd = GUI::CreateCustomWindow(GetModuleHandle(NULL),
								WC_STATIC,
								_T("新图标"),
								WS_BORDER | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SS_ICON | SS_CENTERIMAGE,
								NULL,
								rcRect, hWnd);

							////////////////////////////////////////////////////////////
							//	左下(文本窗口)
							rcRect.left = 0;
							rcRect.top = sIconSize.cy;
							rcRect.right = rcRect.left + sTextSize.cx;
							rcRect.bottom = rcRect.top + sTextSize.cy;

							hOldTextWnd = GUI::CreateCustomWindow(GetModuleHandle(NULL),
								WC_STATIC,
								_T("旧图标"),
								WS_BORDER | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SS_CENTER,
								NULL,
								rcRect, hWnd);

							////////////////////////////////////////////////////////////
							//	右下(文本窗口)
							rcRect.left = sIconSize.cx;
							rcRect.top = sIconSize.cy;
							rcRect.right = rcRect.left + sTextSize.cx;
							rcRect.bottom = rcRect.top + sTextSize.cy;

							hNewTextWnd = GUI::CreateCustomWindow(GetModuleHandle(NULL),
								WC_STATIC,
								_T("新图标"),
								WS_BORDER | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SS_CENTER,
								NULL,
								rcRect, hWnd);

							hIcon = (HICON)SendMessage(hOldIconWnd, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hOldIcon);
							if (hIcon)
							{
								DeleteObject(hIcon);
								hIcon = NULL;
							}
							hIcon = (HICON)SendMessage(hNewIconWnd, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hNewIcon);
							if (hIcon)
							{
								DeleteObject(hIcon);
								hIcon = NULL;
							}

							pTS->insert(std::map<TSTRING, SIZE_T>::value_type(_T("hOldIconWnd"), (SIZE_T)hOldIconWnd));
							pTS->insert(std::map<TSTRING, SIZE_T>::value_type(_T("hNewIconWnd"), (SIZE_T)hNewIconWnd));
							pTS->insert(std::map<TSTRING, SIZE_T>::value_type(_T("hOldTextWnd"), (SIZE_T)hOldTextWnd));
							pTS->insert(std::map<TSTRING, SIZE_T>::value_type(_T("hNewTextWnd"), (SIZE_T)hNewTextWnd));

							ShowWindow(hOldIconWnd, SW_SHOW);
							ShowWindow(hNewIconWnd, SW_SHOW);
							ShowWindow(hOldTextWnd, SW_SHOW);
							ShowWindow(hNewTextWnd, SW_SHOW);
						}
						break;
					}
				}
				break;
				case WM_NOTIFY:
				{
					//GUI::ListCtrlOnNotify((HWND)GUI::GetWindowUserData(hWnd), (LPNMHDR)lParam);
				}
				break;
				case WM_LBUTTONDOWN:
				{
					GUI::DragMoveFull(hWnd);
				}
				break;
				case WM_DROPFILES:
				{
					HICON hIcon = NULL;
					HICON hIconLarge = NULL;
					HICON hIconSmall = NULL;
					HWND hOldIconWnd = NULL;
					HWND hNewIconWnd = NULL;
					tstring tsFileName = _T("");
					HDROP hDrop = (HDROP)wParam;
					std::map<TSTRING, TSTRING> ttmap;
					std::map<TSTRING, TSTRING> * pTT = NULL;
					
					if (pTS)
					{
						hOldIconWnd = (HWND)pTS->at(_T("hOldIconWnd"));
						hNewIconWnd = (HWND)pTS->at(_T("hNewIconWnd"));
						pTT = (std::map<TSTRING, TSTRING> *)(pTS->at(_T("m_ttmap")));
						if (pTT)
						{
							GUI::GetDropFiles(&ttmap, hDrop);

							tsFileName = ToLowerCase(ttmap.begin()->first.c_str());
							if (hOldIconWnd)
							{
								if ((tsFileName.find(ToLowerCase(_T(".exe"))) != TSTRING::npos) ||
									(tsFileName.find(ToLowerCase(_T(".dll"))) != TSTRING::npos))
								{
									pTT->clear();
									pTT->insert(std::map<TSTRING, TSTRING>::value_type(tsFileName, tsFileName));
									ExtractIconEx(ttmap.begin()->first.c_str(), 0, &hIconLarge, &hIconSmall, 1);
									GUI::StaticSetIconImage(hOldIconWnd, NULL);
									GUI::StaticSetIconImage(hOldIconWnd, hIconLarge);
									GUI::NotifyUpdate(hOldIconWnd);
								}
								else if ((tsFileName.find(ToLowerCase(_T(".ico"))) != TSTRING::npos))
								{
									if (pTT->size() && PE::ChangeExeIcon(tsFileName.c_str(), pTT->begin()->first.c_str()))
									{
										GUI::StaticSetIconImage(hNewIconWnd, NULL);
										GUI::StaticSetIconImage(hNewIconWnd, GUI::HIconFromFile(tsFileName.c_str()));
										GUI::NotifyUpdate(hNewIconWnd);
										::MessageBoxEx(NULL, _T("修改图标成功!"), _T("提示"), MB_ICONINFORMATION, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT));
									}
									else
									{
										::MessageBoxEx(NULL, _T("修改图标失败!"), _T("提示"), MB_ICONINFORMATION, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT));
									}
								}
								else
								{
									// no-op
								}
							}
						}
					}
				}
				break;
				case WM_SIZE:
					//case WM_EXITSIZEMOVE:
				{
					/*HWND hListViewWnd = (HWND)GUI::GetWindowUserData(hWnd);
					if (hListViewWnd)
					{
					RECT rcWnd = { 0 };
					GetClientRect(hWnd, &rcWnd);
					MoveWindow(hListViewWnd, rcWnd.left, rcWnd.top, rcWnd.right - rcWnd.left, rcWnd.bottom - rcWnd.top, TRUE);
					//SetWindowPos(hListViewWnd, HWND_NOTOPMOST, rcWnd.left, rcWnd.top, rcWnd.right - rcWnd.left, rcWnd.bottom - rcWnd.top, SWP_NOMOVE | SWP_NOSIZE);
					}*/
				}
				break;
				case WM_PAINT:
				{
					ULONG uARGB[2] = { ARGB(0xFF, 0x7F, 0xFF, 0x7F), ARGB(0xFF, 0xFF, 0x7F, 0x7F) };
					RECT rcWnd = { 0 };
					PAINTSTRUCT ps = { 0 };
					RECT rcMemory = { 8, 8, 8, 8 };
					HDC hDC = ::BeginPaint(hWnd, &ps);
					GetClientRect(hWnd, &rcWnd);
					GUI::DrawMemoryBitmap(hDC, hWnd, rcWnd.right, rcWnd.bottom, GUI::HBITMAPFromHWND(hWnd));

					::EndPaint(hWnd, &ps);
				}
				break;
				case WM_DESTROY:
				{
					// send a WM_QUIT to the message queue
					PostQuitMessage(0);
				}
				break;
				default:
				{
					// for messages that we don't deal with
					return DefWindowProc(hWnd, message, wParam, lParam);
				}
				break;
				}

				return lResult;
			}

			UINT_PTR RunApp()
			{
				HWND hWnd = NULL;
				UINT_PTR uResult = 0;
				HMODULE hModule = NULL;
				INITCOMMONCONTROLSEX iccex = { 0 };
				_TCHAR tzClassName[] = _T("CFileIconWindow");
				HINSTANCE hInstance = GetModuleHandle(NULL);
				RECT rcRect = { CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT + 263, CW_USEDEFAULT + 177 };
				
				hModule = ::LoadLibrary(_T("msftedit.dll"));

				::InitCommonControls();
				
				iccex.dwSize = sizeof(iccex);
				// 将它设置为包括所有要在应用程序中使用的公共控件类。
				iccex.dwICC = ICC_WIN95_CLASSES;
				::InitCommonControlsEx(&iccex);

				if (GUI::WindowClassesRegister(hInstance, tzClassName, &CFileIconWindow::WindowProcedure))
				{
					//return 0;
				}

				hWnd = GUI::CreateCustomWindow(hInstance, tzClassName, _T("CFileIconWindow"),
					WS_OVERLAPPEDWINDOW & (~WS_MAXIMIZEBOX) & (~WS_THICKFRAME), WS_EX_ACCEPTFILES, rcRect, NULL, NULL, &m_cs);

				uResult = GUI::StartupWindows(hWnd, SW_SHOW);

				if (hModule)
				{
					::FreeLibrary(hModule);
					hModule = NULL;
				}

				return uResult;
			}

			void ResetParam()
			{
				m_tsmap.clear();
				m_ttmap.clear();
				m_tsmap.insert(std::map<TSTRING, SIZE_T>::value_type(_T("m_ttmap"), (SIZE_T)&m_ttmap));
				memset(&m_cs, 0, sizeof(m_cs));
				m_cs.lpCreateParams = &m_tsmap;
			}

		private:
			CREATESTRUCT m_cs;

			std::map<TSTRING, SIZE_T> m_tsmap;
			std::map<TSTRING, TSTRING> m_ttmap;

		};
		class CSelectProcessWindow {
		public:

			CSelectProcessWindow()
			{
				this->ResetParam();
			}
			CSelectProcessWindow(std::map<TSTRING, std::vector<TSTRING>> * pTVMAP, HIMAGELIST * hImageList = NULL)
			{
				this->ResetParam();
				this->SetListData(pTVMAP);
				if (hImageList)
				{
					this->ClearParam();
					m_hImageList = *hImageList;
				}
			}
			virtual ~CSelectProcessWindow()
			{
				this->ClearParam();
			}

		public:

			//  This function is called by the Windows function DispatchMessage()
			__inline static LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
			{
				LRESULT lResult = 0;
				std::map<TSTRING, SIZE_T> * pTS = NULL;
				pTS = (std::map<tstring, SIZE_T> *)GUI::GetWindowUserData(hWnd);
				
				// handle the messages
				switch (message)
				{
				case WM_CREATE:
				{
					//::SetWindowLong(hWnd, GWL_STYLE, ::GetWindowLong(hWnd, GWL_STYLE) & (~WS_CAPTION));
					//::SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_DRAWFRAME);

					//添加阴影效果
					//SetClassLong(hWnd, GCL_STYLE, GetClassLong(hWnd, GCL_STYLE) | CS_DROPSHADOW);
					HWND hListViewWnd = NULL;
					HIMAGELIST hImageList = NULL;
					SIZE_T * pSPWI = NULL;
					GUI::SORTDATAINFO * pSDI = NULL;
					std::map<TSTRING, std::vector<TSTRING>> * pTVMAP = NULL;
					if (!pTS)
					{
						pTS = (std::map<TSTRING, SIZE_T> *)*(LPVOID *)(((CREATESTRUCT *)lParam)->lpCreateParams);
					}
					if (pTS)
					{
						GUI::SetWindowUserData(hWnd, (LONG_PTR)pTS);

						pSDI = (GUI::SORTDATAINFO *)(pTS->at(_T("m_sdi")));
						hImageList = (HIMAGELIST)(pTS->at(_T("m_hImageList")));
						pTVMAP = (std::map<TSTRING, std::vector<TSTRING>> *)(pTS->at(_T("m_tvmap")));
						if (pSDI && pTVMAP)
						{
							RECT rcRect = { 0 };
							GetClientRect(hWnd, &rcRect);
							hListViewWnd = GUI::CreateCustomWindow(GetModuleHandle(NULL),
								WC_LISTVIEW,
								_T("ListView"),
								WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | LVS_REPORT | LVS_SINGLESEL,
								LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT,
								rcRect, hWnd);
							::SetWindowLong(hListViewWnd, GWL_STYLE, ::GetWindowLong(hListViewWnd, GWL_STYLE) | LVS_REPORT);
							::SetWindowLong(hListViewWnd, GWL_EXSTYLE, ::GetWindowLong(hListViewWnd, GWL_EXSTYLE) | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
							ListView_SetExtendedListViewStyle(hListViewWnd, ListView_GetExtendedListViewStyle(hListViewWnd) | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

							pTS->insert(std::map<TSTRING, SIZE_T>::value_type(_T("hListViewWnd"), (SIZE_T)hListViewWnd));
							
							GUI::SetWindowUserData(hListViewWnd, (LONG_PTR)pTVMAP);

							pSDI->hListCtrlWnd = hListViewWnd;

							ListView_SetImageList(hListViewWnd, hImageList, LVSIL_NORMAL);
							ListView_SetImageList(hListViewWnd, hImageList, LVSIL_SMALL);
							
							ListCtrlSetSortDataInfo(hListViewWnd, pSDI);

							GUI::ListCtrlDeleteAllRows(hListViewWnd);
							GUI::ListCtrlDeleteAllColumns(hListViewWnd);
							GUI::ListCtrlInsertData(pTVMAP, hListViewWnd, hImageList);

							ShowWindow(hListViewWnd, SW_SHOW);
						}
					}
				}
				break;
				case WM_NOTIFY:
				{
					GUI::ListCtrlOnNotify((HWND)pTS->at(_T("hListViewWnd")), (LPNMHDR)lParam);
				}
				break;
				case WM_LBUTTONDOWN:
				{
					GUI::DragMoveFull(hWnd);
				}
				break;
				case WM_SIZE:
					//case WM_EXITSIZEMOVE:
				{
					if (pTS)
					{
						HWND hListViewWnd = (HWND)pTS->at(_T("hListViewWnd"));
						if (hListViewWnd)
						{
							RECT rcWnd = { 0 };
							GetClientRect(hWnd, &rcWnd);
							MoveWindow(hListViewWnd, rcWnd.left, rcWnd.top, rcWnd.right - rcWnd.left, rcWnd.bottom - rcWnd.top, TRUE);
							//SetWindowPos(hListViewWnd, HWND_NOTOPMOST, rcWnd.left, rcWnd.top, rcWnd.right - rcWnd.left, rcWnd.bottom - rcWnd.top, SWP_NOMOVE | SWP_NOSIZE);
						}
					}
				}
				break;
				case WM_PAINT:
				{
					ULONG uARGB[2] = { ARGB(0xFF, 0x7F, 0xFF, 0x7F), ARGB(0xFF, 0xFF, 0x7F, 0x7F) };
					RECT rcWnd = { 0 };
					PAINTSTRUCT ps = { 0 };
					RECT rcMemory = { 8, 8, 8, 8 };
					HDC hDC = ::BeginPaint(hWnd, &ps);
					GetClientRect(hWnd, &rcWnd);
					GUI::DrawMemoryBitmap(hDC, hWnd, rcWnd.right, rcWnd.bottom, GUI::HBITMAPFromHWND(hWnd));

					::EndPaint(hWnd, &ps);
				}
				break;
				case WM_DESTROY:
				{
					// send a WM_QUIT to the message queue
					PostQuitMessage(0);
				}
				break;
				default:
				{
					// for messages that we don't deal with
					return DefWindowProc(hWnd, message, wParam, lParam);
				}
				break;
				}

				return lResult;
			}

			UINT_PTR RunApp()
			{
				HWND hWnd = NULL;
				UINT_PTR uResult = 0;
				HMODULE hModule = NULL;
				INITCOMMONCONTROLSEX iccex = { 0 };
				_TCHAR tzClassName[] = _T("SelectProcessWindow");
				HINSTANCE hInstance = GetModuleHandle(NULL);
				RECT rcRect = { CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT + 300, CW_USEDEFAULT + 200 };

				hModule = ::LoadLibrary(_T("msftedit.dll"));

				::InitCommonControls();

				iccex.dwSize = sizeof(iccex);
				// 将它设置为包括所有要在应用程序中使用的公共控件类。
				iccex.dwICC = ICC_WIN95_CLASSES;
				::InitCommonControlsEx(&iccex);

				if (GUI::WindowClassesRegister(hInstance, tzClassName, &CSelectProcessWindow::WindowProcedure))
				{
					//return 0;
				}

				hWnd = GUI::CreateCustomWindow(hInstance, tzClassName, _T("SelectProcessWindow"),
					WS_OVERLAPPEDWINDOW, NULL, rcRect, NULL, NULL, &m_cs);

				uResult = GUI::StartupWindows(hWnd, SW_SHOW);

				if (hModule)
				{
					::FreeLibrary(hModule);
					hModule = NULL;
				}

				return uResult;
			}

			void ResetParam()
			{
				m_tsmap.clear();
				m_tvmap.clear();
				memset(&m_sdi, 0, sizeof(m_sdi));
				m_hImageList = ImageList_Create(32, 32, ILC_COLOR8 | ILC_MASK, 3, 1);
				m_tsmap.insert(std::map<TSTRING, SIZE_T>::value_type(_T("m_sdi"), (SIZE_T)&m_sdi));
				m_tsmap.insert(std::map<TSTRING, SIZE_T>::value_type(_T("m_tvmap"), (SIZE_T)&m_tvmap));
				m_tsmap.insert(std::map<TSTRING, SIZE_T>::value_type(_T("m_hImageList"), (SIZE_T)m_hImageList));
				memset(&m_cs, 0, sizeof(m_cs));
				m_cs.lpCreateParams = &m_tsmap;
			}
			void ClearParam()
			{				
				ImageList_Destroy(m_hImageList);
			}
			void SetListData(std::map<TSTRING, std::vector<TSTRING>> * pTTMMAP)
			{
				m_tvmap.insert(pTTMMAP->begin(), pTTMMAP->end());
			}

			std::map<TSTRING, std::vector<TSTRING>> * GetListData()
			{
				return &m_tvmap;
			}
		private:
			CREATESTRUCT m_cs;
			std::map<TSTRING, SIZE_T> m_tsmap;
			
		private:
			GUI::SORTDATAINFO m_sdi;
			HIMAGELIST m_hImageList;
			std::map<TSTRING, std::vector<TSTRING>> m_tvmap;
			
		};

		class CCryptoWindow{
		public:

			CCryptoWindow()
			{
				this->ResetParam();
			}
			virtual ~CCryptoWindow()
			{
				this->ClearParam();
			}

		public:

			//  This function is called by the Windows function DispatchMessage()
			__inline static LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
			{
				LRESULT lResult = 0;
				std::map<TSTRING, SIZE_T> * pTS = NULL;

				pTS = (std::map<tstring, SIZE_T> *)GUI::GetWindowUserData(hWnd);
				
				// handle the messages
				switch (message)
				{
				case WM_NCCREATE:
				{
					GUI::CenterWindowInScreen(hWnd);
					lResult = (-1L);
				}
				break;
				case WM_CREATE:
				{
					//::SetWindowLong(hWnd, GWL_STYLE, ::GetWindowLong(hWnd, GWL_STYLE) & (~WS_CAPTION));
					//::SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_DRAWFRAME);

					//添加阴影效果
					//SetClassLong(hWnd, GCL_STYLE, GetClassLong(hWnd, GCL_STYLE) | CS_DROPSHADOW);
					{
						HICON hIcon = NULL;
						RECT rcRect = { 0 };
						HICON hOldIcon = NULL;
						HICON hNewIcon = NULL;
						HWND hEdit1Wnd = NULL;
						HWND hSplit1Wnd = NULL;
						HWND hButton1Wnd = NULL;
						HWND hButton2Wnd = NULL;
						HWND hSplit2Wnd = NULL;
						HWND hEdit2Wnd = NULL;
						if (!pTS)
						{
							pTS = (std::map<TSTRING, SIZE_T> *)*(LPVOID *)(((CREATESTRUCT *)lParam)->lpCreateParams);
						}
						if (pTS)
						{
							GUI::SetWindowUserData(hWnd, (LONG_PTR)pTS);

							SetWindowText(hWnd, _T("Base64加密解密"));
							////////////////////////////////////////////////////////////
							//	左上(图标窗口)
							rcRect.left = 0;
							rcRect.top = 0;
							rcRect.right = rcRect.left + 100;
							rcRect.bottom = rcRect.top + 100;

							hEdit1Wnd = GUI::CreateCustomWindow(GetModuleHandle(NULL),
								Convert::WToT(MSFTEDIT_CLASS).c_str(),//WC_EDIT,
								_T("LEFT_EDIT"),
								WS_BORDER | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
								NULL,
								rcRect, hWnd);

							////////////////////////////////////////////////////////////
							//	右上(图标窗口)
							rcRect.left = rcRect.right;
							rcRect.top = 0;
							rcRect.right = rcRect.left + 100;
							rcRect.bottom = rcRect.top + 100;

							hEdit2Wnd = GUI::CreateCustomWindow(GetModuleHandle(NULL),
								Convert::WToT(MSFTEDIT_CLASS).c_str(),//WC_EDIT,
								_T("RIGHT_EDIT"),
								WS_BORDER | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
								NULL,
								rcRect, hWnd);

							pTS->insert(std::map<TSTRING, SIZE_T>::value_type(_T("hEdit1Wnd"), (SIZE_T)hEdit1Wnd));
							pTS->insert(std::map<TSTRING, SIZE_T>::value_type(_T("hEdit2Wnd"), (SIZE_T)hEdit2Wnd));
							
							ShowWindow(hEdit1Wnd, SW_SHOW);
							ShowWindow(hEdit2Wnd, SW_SHOW);
							
							HMENU hMenu = GetSystemMenu(hWnd, FALSE);
							if (hMenu)
							{
								AppendMenu(hMenu, MF_BYCOMMAND, 0, _T("保存解码到文件"));
								AppendMenu(hMenu, MF_BYCOMMAND, 1, _T("保存编码到文件"));
							}
						}
						break;
					}
				}
				break;
				case WM_SYSCOMMAND:
				{
					if (pTS)
					{
						HWND hEdit1Wnd = (HWND)pTS->at(_T("hEdit1Wnd"));
						HWND hEdit2Wnd = (HWND)pTS->at(_T("hEdit2Wnd"));
						switch (LOWORD(wParam))
						{
						case 0:
						{
							DWORD dwSize = GetWindowTextLength(hEdit1Wnd);
							if (dwSize)
							{
								tstring tsData(dwSize, _T('\0'));
								GetWindowText(hEdit1Wnd, (LPTSTR)tsData.c_str(), tsData.size());
								String::file_writer(Convert::TToA(tsData), Convert::TToA(FilePath::GetProgramPath() + _T("decode.txt")));
								::MessageBoxEx(NULL, tstring(_T("保存解码信息到：") + FilePath::GetProgramPath() + _T("decode.txt")).c_str(), _T("提示"), MB_ICONINFORMATION, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT));
							}
						}
						break;
						case 1:
						{
							DWORD dwSize = GetWindowTextLength(hEdit2Wnd);
							if (dwSize)
							{
								tstring tsData(dwSize, _T('\0'));
								GetWindowText(hEdit2Wnd, (LPTSTR)tsData.c_str(), tsData.size());
								String::file_writer(Convert::TToA(tsData), Convert::TToA(PPSHUAI::FilePath::GetProgramPath() + _T("encode.txt")));
								::MessageBoxEx(NULL, tstring(_T("保存编码信息到：") + FilePath::GetProgramPath() + _T("encode.txt")).c_str(), _T("提示"), MB_ICONINFORMATION, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT));
							}
						}
						break;
						default:
						{
							// for messages that we don't deal with
							return DefWindowProc(hWnd, message, wParam, lParam);
						}
						break;
						}
					}
				}
				break;
				case WM_NOTIFY:
				{
					//GUI::ListCtrlOnNotify((HWND)GUI::GetWindowUserData(hWnd), (LPNMHDR)lParam);
				}
				break;
				case WM_LBUTTONDOWN:
				{
					GUI::DragMoveFull(hWnd);
				}
				break;
				case WM_DROPFILES:
				{
					HWND hEdit1Wnd = NULL;
					HWND hEdit2Wnd = NULL;
					HDROP hDrop = (HDROP)wParam;
					std::map<TSTRING, TSTRING> ttmap;
					POINT point = { 0 };
					RECT rcEdit1 = { 0 };
					RECT rcEdit2 = { 0 };

					if (pTS)
					{
						GUI::GetDropFiles(&ttmap, hDrop);
						hEdit1Wnd = (HWND)pTS->at(_T("hEdit1Wnd"));
						hEdit2Wnd = (HWND)pTS->at(_T("hEdit2Wnd"));

						GetCursorPos(&point);
						GetWindowRect(hEdit1Wnd, &rcEdit1);
						GetWindowRect(hEdit2Wnd, &rcEdit2);

						if (ttmap.size())
						{
							std::string strDecodeData("");
							std::string strEncodeData("");
							if (PtInRect(&rcEdit1, point))
							{
								String::file_reader(strDecodeData, Convert::TToA(ttmap.begin()->first).c_str());

								SetWindowText(hEdit1Wnd, Convert::AToT(strDecodeData).c_str());

								strEncodeData = CRYPTO::Base64::base64Encode((const unsigned char *)strDecodeData.c_str(), strDecodeData.length());
								SetWindowText(hEdit2Wnd, Convert::AToT(strEncodeData).c_str());
							}
							else if (PtInRect(&rcEdit2, point))
							{
								String::file_reader(strEncodeData, Convert::TToA(ttmap.begin()->first).c_str());
								SetWindowText(hEdit2Wnd, Convert::AToT(strEncodeData).c_str());
								
								strDecodeData = CRYPTO::Base64::base64Decode(strEncodeData);
																
								SetWindowText(hEdit1Wnd, Convert::AToT(strDecodeData).c_str());
							}
						}
					}
				}
				break;
				case WM_SIZE:
					//case WM_EXITSIZEMOVE:
				{
					HWND hEdit1Wnd = NULL;
					HWND hEdit2Wnd = NULL;
					HDROP hDrop = (HDROP)wParam;

					if (pTS)
					{
						int nWndNum = 2;
						int nWndWidth = 0;
						hEdit1Wnd = (HWND)pTS->at(_T("hEdit1Wnd"));
						hEdit2Wnd = (HWND)pTS->at(_T("hEdit2Wnd"));
						if (hEdit1Wnd && hEdit2Wnd)
						{
							RECT rcWnd = { 0 };
							GetClientRect(hWnd, &rcWnd);
							nWndWidth = (rcWnd.right - rcWnd.left) / nWndNum;

							MoveWindow(hEdit1Wnd, rcWnd.left, rcWnd.top, nWndWidth, rcWnd.bottom - rcWnd.top, TRUE);
							//SetWindowPos(hListViewWnd, HWND_NOTOPMOST, rcWnd.left, rcWnd.top, rcWnd.right - rcWnd.left, rcWnd.bottom - rcWnd.top, SWP_NOMOVE | SWP_NOSIZE);

							MoveWindow(hEdit2Wnd, rcWnd.left + nWndWidth, rcWnd.top, nWndWidth, rcWnd.bottom - rcWnd.top, TRUE);
							//SetWindowPos(hListViewWnd, HWND_NOTOPMOST, rcWnd.left, rcWnd.top, rcWnd.right - rcWnd.left, rcWnd.bottom - rcWnd.top, SWP_NOMOVE | SWP_NOSIZE);

						}
					}
				}
				break;
				/*case WM_PAINT:
				{
					ULONG uARGB[2] = { ARGB(0xFF, 0x7F, 0xFF, 0x7F), ARGB(0xFF, 0xFF, 0x7F, 0x7F) };
					RECT rcWnd = { 0 };
					PAINTSTRUCT ps = { 0 };
					RECT rcMemory = { 8, 8, 8, 8 };
					HDC hDC = ::BeginPaint(hWnd, &ps);
					GetClientRect(hWnd, &rcWnd);
					GUI::DrawMemoryBitmap(hDC, hWnd, rcWnd.right, rcWnd.bottom, GUI::HBITMAPFromHWND(hWnd));

					::EndPaint(hWnd, &ps);
				}
				break;*/
				case WM_DESTROY:
				{
					// send a WM_QUIT to the message queue
					PostQuitMessage(0);
				}
				break;
				default:
				{
					// for messages that we don't deal with
					return DefWindowProc(hWnd, message, wParam, lParam);
				}
				break;
				}

				return lResult;
			}

			UINT_PTR RunApp()
			{
				HWND hWnd = NULL;				
				UINT_PTR uResult = 0;
				HMODULE hModule = NULL;
				INITCOMMONCONTROLSEX iccex = { 0 };
				_TCHAR tzClassName[] = _T("CCryptoWindow");
				HINSTANCE hInstance = GetModuleHandle(NULL);
				RECT rcRect = { CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT + 300, CW_USEDEFAULT + 200 };
				
				hModule = ::LoadLibrary(_T("msftedit.dll"));

				::InitCommonControls();

				iccex.dwSize = sizeof(iccex);
				// 将它设置为包括所有要在应用程序中使用的公共控件类。
				iccex.dwICC = ICC_WIN95_CLASSES;
				::InitCommonControlsEx(&iccex);

				if (GUI::WindowClassesRegister(hInstance, tzClassName, &CCryptoWindow::WindowProcedure))
				{
					//return 0;
				}

				hWnd = GUI::CreateCustomWindow(hInstance, tzClassName, _T("CCryptoWindow"),
					WS_OVERLAPPEDWINDOW, WS_EX_ACCEPTFILES, rcRect, NULL, NULL, &m_cs);

				uResult = GUI::StartupWindows(hWnd, SW_SHOW);

				if (hModule)
				{
					::FreeLibrary(hModule);
					hModule = NULL;
				}
				return uResult;
			}

			void ResetParam()
			{
				m_tsmap.clear();
				memset(&m_cs, 0, sizeof(m_cs));
				m_cs.lpCreateParams = &m_tsmap;
			}

			void ClearParam()
			{

			}
		private:
			CREATESTRUCT m_cs;
			std::map<TSTRING, SIZE_T> m_tsmap;

		};

#define DEFAULT_ELAPSE_TIMEOUT	750 //750ms
#define IDC_TIMER_EVENT			1001 //事件消息标识定义
		class CAnimationWindow{
		public:

			CAnimationWindow()
			{
				this->ResetParam();
			}
			CAnimationWindow(LPCTSTR pRootPathName = _T(""), LPCTSTR pExtension = _T(""), LPCTSTR pShowTips = _T(""))
			{
				FilePath::DirectoryTraversal(&m_stmap, pRootPathName, pExtension);
				m_nElapse = DEFAULT_ELAPSE_TIMEOUT;
				m_nCount = m_stmap.size();
				m_nIndex = 0;
				m_tsRootPathName = pRootPathName;
				m_tsShowTips = pShowTips;
				m_rect.left = m_rect.top = 0;
				m_rect.right = m_rect.bottom = 360;
				this->ResetParam();
			}
			virtual ~CAnimationWindow()
			{
			}

		public:

			//  This function is called by the Windows function DispatchMessage()
			__inline static LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
			{
				LRESULT lResult = 0;
				HDC * m_phRenderDC = NULL;//设备环境
				int * m_pnIndex = NULL;//图片动画索引
				int * m_pnCount = NULL;//图片动画个数
				int * m_pnElapse = NULL;//倒计时时间
				RECT * m_prect = NULL;//窗口大小
				TSTRING * m_ptsRootPathName = NULL;//图片路径前缀
				TSTRING * m_ptsShowTips = NULL;//显示提示信息
				std::map<SIZE_T, TSTRING> * m_pstmap = NULL;
				std::map<TSTRING, SIZE_T> * pTS = NULL;

				pTS = (std::map<tstring, SIZE_T> *)GUI::GetWindowUserData(hWnd);

				// handle the messages
				switch (message)
				{
				case WM_NCCREATE:
				{
					GUI::CenterWindowInScreen(hWnd);
					lResult = (-1L);
				}
				break;
				case WM_CREATE:
				{
					//::SetWindowLong(hWnd, GWL_STYLE, ::GetWindowLong(hWnd, GWL_STYLE) & (~WS_CAPTION));
					//::SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_DRAWFRAME);

					//添加阴影效果
					//SetClassLong(hWnd, GCL_STYLE, GetClassLong(hWnd, GCL_STYLE) | CS_DROPSHADOW);
					
					::SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) & (~WS_CAPTION));
					
					{
						if (!pTS)
						{
							pTS = (std::map<TSTRING, SIZE_T> *)*(LPVOID *)(((CREATESTRUCT *)lParam)->lpCreateParams);
						}
						if (pTS)
						{
							GUI::SetWindowUserData(hWnd, (LONG_PTR)pTS);

							SetWindowText(hWnd, _T("动画窗口测试"));
							
							m_phRenderDC = (HDC *)pTS->at(_T("m_hRenderDC"));
							m_pnIndex = (int *)pTS->at(_T("m_nIndex"));
							m_pnCount = (int *)pTS->at(_T("m_nCount"));
							m_pnElapse = (int *)pTS->at(_T("m_nElapse"));
							m_prect = (RECT *)pTS->at(_T("m_rect"));
							m_ptsRootPathName = (TSTRING *)pTS->at(_T("m_tsRootPathName"));
							m_ptsShowTips = (TSTRING *)pTS->at(_T("m_tsShowTips"));
							m_pstmap = (std::map<SIZE_T, TSTRING> *)pTS->at(_T("m_stmap"));

							//pTS->insert(std::map<TSTRING, SIZE_T>::value_type(_T("hStaticWnd"), (SIZE_T)hStaticWnd));

							::SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0,
								m_prect->right - m_prect->left, m_prect->bottom - m_prect->top, SWP_HIDEWINDOW);
							GUI::CenterWindowInScreen(hWnd);
							
							RECT rect = { 0 };
							GetClientRect(hWnd, &rect);

							(*m_phRenderDC) = ::GetDC(hWnd);//设备环境

							GUI::ImagesRenderDisplay((*m_phRenderDC), &rect, m_pstmap->at((*m_pnIndex)++).c_str());

							(*m_pnIndex) %= (*m_pnCount);

							::ShowWindow(hWnd, SW_SHOW);
							::SetForegroundWindow(hWnd);

							::SetTimer(hWnd, IDC_TIMER_EVENT, (*m_pnElapse), NULL);

							HMENU hMenu = ::GetSystemMenu(hWnd, FALSE);
							if (hMenu)
							{
								::AppendMenu(hMenu, MF_BYCOMMAND, 0, _T("测试1"));
								::AppendMenu(hMenu, MF_BYCOMMAND, 1, _T("测试2"));
							}
						}
						break;
					}
				}
				break;
				case WM_SYSCOMMAND:
				{
					if (pTS)
					{
						switch (LOWORD(wParam))
						{
						case 0:
						{
							::MessageBoxEx(NULL, _T("保存解码信息到!"), _T("提示"), MB_ICONINFORMATION, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT));
						}
						break;
						case 1:
						{
							::MessageBoxEx(NULL, _T("保存编码信息到："), _T("提示"), MB_ICONINFORMATION, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT));
						}
						break;
						default:
						{
							// for messages that we don't deal with
							return DefWindowProc(hWnd, message, wParam, lParam);
						}
						break;
						}
					}
				}
				break;
				case WM_TIMER:
				{
					m_phRenderDC = (HDC *)pTS->at(_T("m_hRenderDC"));
					m_pnIndex = (int *)pTS->at(_T("m_nIndex"));
					m_pnCount = (int *)pTS->at(_T("m_nCount"));
					m_pnElapse = (int *)pTS->at(_T("m_nElapse"));
					m_prect = (RECT *)pTS->at(_T("m_rect"));
					m_ptsRootPathName = (TSTRING *)pTS->at(_T("m_tsRootPathName"));
					m_ptsShowTips = (TSTRING *)pTS->at(_T("m_tsShowTips"));
					m_pstmap = (std::map<SIZE_T, TSTRING> *)pTS->at(_T("m_stmap"));

					switch (LOWORD(wParam))
					{
					case IDC_TIMER_EVENT:
					{
						RECT rect = { 0 };
						GetClientRect(hWnd, &rect);
						GUI::ImagesRenderDisplay((*m_phRenderDC), &rect, m_pstmap->at((*m_pnIndex)++).c_str());
						
						(*m_pnIndex) %= (*m_pnCount);
					}
					break;
					default:
					{

					}
					break;
					}
				}
				break;
				case WM_NOTIFY:
				{
					//GUI::ListCtrlOnNotify((HWND)GUI::GetWindowUserData(hWnd), (LPNMHDR)lParam);
				}
				break;
				case WM_LBUTTONDOWN:
				{
					GUI::DragMoveFull(hWnd);
				}
				break;
				case WM_DROPFILES:
				{
					HDROP hDrop = (HDROP)wParam;
					std::map<TSTRING, TSTRING> ttmap;
					POINT point = { 0 };
					
					if (pTS)
					{
						GUI::GetDropFiles(&ttmap, hDrop);
						
						GetCursorPos(&point);

						if (ttmap.size())
						{
							
						}
					}
				}
				break;
				case WM_SIZE:
					//case WM_EXITSIZEMOVE:
				{
					HDROP hDrop = (HDROP)wParam;

					if (pTS)
					{
						//HWND hStaticWnd = (HWND)pTS->at(_T("hStaticWnd"));
					}
				}
				break;
				/*case WM_PAINT:
				{
					ULONG uARGB[2] = { ARGB(0xFF, 0x7F, 0xFF, 0x7F), ARGB(0xFF, 0xFF, 0x7F, 0x7F) };
					RECT rcWnd = { 0 };
					PAINTSTRUCT ps = { 0 };
					RECT rcMemory = { 8, 8, 8, 8 };
					HDC hDC = ::BeginPaint(hWnd, &ps);
					GetClientRect(hWnd, &rcWnd);
					//GUI::DrawMemoryBitmap(hDC, hWnd, rcWnd.right, rcWnd.bottom, GUI::HBITMAPFromHWND(hWnd));
					{
						m_phRenderDC = (HDC *)pTS->at(_T("m_hRenderDC"));
						m_pnIndex = (int *)pTS->at(_T("m_nIndex"));
						m_pnCount = (int *)pTS->at(_T("m_nCount"));
						m_pnElapse = (int *)pTS->at(_T("m_nElapse"));
						m_prect = (RECT *)pTS->at(_T("m_rect"));
						m_ptsImagesName = (TSTRING *)pTS->at(_T("m_tsImagesName"));
						m_ptsShowTips = (TSTRING *)pTS->at(_T("m_tsShowTips"));
						HWND hStaticWnd = (HWND)pTS->at(_T("hStaticWnd"));

						_TCHAR tImagePath[MAX_PATH] = { 0 };
						wsprintf(tImagePath, _T("%s%d.jpg"), m_ptsImagesName->c_str(), (*m_pnIndex)++);
						RECT rc = { 0 };
						GetClientRect(hWnd, &rc);
						(*m_phRenderDC) = GetDC(hStaticWnd);
						GUI::ImagesRenderDisplay((*m_phRenderDC), &rc, tImagePath);
					}
					::EndPaint(hWnd, &ps);
				}
				break;*/
				case WM_DESTROY:
				{
					// send a WM_QUIT to the message queue
					PostQuitMessage(0);
				}
				break;
				default:
				{
					// for messages that we don't deal with
					return DefWindowProc(hWnd, message, wParam, lParam);
				}
				break;
				}

				return lResult;
			}

			UINT_PTR RunApp()
			{
				HWND hWnd = NULL;
				UINT_PTR uResult = 0;
				HMODULE hModule = NULL;
				INITCOMMONCONTROLSEX iccex = { 0 };
				_TCHAR tzClassName[] = _T("CAnimationWindow");
				HINSTANCE hInstance = GetModuleHandle(NULL);
				RECT rcRect = { CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT + 300, CW_USEDEFAULT + 200 };

				hModule = ::LoadLibrary(_T("msftedit.dll"));
				
				::InitCommonControls();

				iccex.dwSize = sizeof(iccex);
				// 将它设置为包括所有要在应用程序中使用的公共控件类。
				iccex.dwICC = ICC_WIN95_CLASSES;
				::InitCommonControlsEx(&iccex);

				if (GUI::WindowClassesRegister(hInstance, tzClassName, &CAnimationWindow::WindowProcedure))
				{
					//return 0;
				}

				hWnd = GUI::CreateCustomWindow(hInstance, tzClassName, _T("CAnimationWindow"),
					WS_OVERLAPPEDWINDOW, WS_EX_ACCEPTFILES, rcRect, NULL, NULL, &m_cs);

				uResult = GUI::StartupWindows(hWnd, SW_SHOW);

				if (hModule)
				{
					::FreeLibrary(hModule);
					hModule = NULL;
				}
				return uResult;
			}

			void ResetParam()
			{
				m_tsmap.clear();
				m_tsmap.insert(std::map<TSTRING, SIZE_T>::value_type(_T("m_hRenderDC"), (SIZE_T)&m_hRenderDC));
				m_tsmap.insert(std::map<TSTRING, SIZE_T>::value_type(_T("m_nIndex"), (SIZE_T)&m_nIndex));
				m_tsmap.insert(std::map<TSTRING, SIZE_T>::value_type(_T("m_nCount"), (SIZE_T)&m_nCount));
				m_tsmap.insert(std::map<TSTRING, SIZE_T>::value_type(_T("m_nElapse"), (SIZE_T)&m_nElapse));
				m_tsmap.insert(std::map<TSTRING, SIZE_T>::value_type(_T("m_rect"), (SIZE_T)&m_rect));
				m_tsmap.insert(std::map<TSTRING, SIZE_T>::value_type(_T("m_tsRootPathName"), (SIZE_T)&m_tsRootPathName));
				m_tsmap.insert(std::map<TSTRING, SIZE_T>::value_type(_T("m_tsShowTips"), (SIZE_T)&m_tsShowTips));
				m_tsmap.insert(std::map<TSTRING, SIZE_T>::value_type(_T("m_stmap"), (SIZE_T)&m_stmap));
				memset(&m_cs, 0, sizeof(m_cs));
				m_cs.lpCreateParams = &m_tsmap;
			}
			
		private:
			CREATESTRUCT m_cs;
			std::map<TSTRING, SIZE_T> m_tsmap;

		private:
			HDC m_hRenderDC;//设备环境
			int m_nIndex;//图片动画索引
			int m_nCount;//图片动画个数
			int m_nElapse;//倒计时时间
			RECT m_rect;//窗口大小
			TSTRING m_tsRootPathName;//图片路径前缀
			TSTRING m_tsShowTips;//显示提示信息
			std::map<SIZE_T, TSTRING> m_stmap;//图片路径列表
		};


		__inline static void ShowFrameUI()
		{
#define WINDOW_TITLE_INFO				"WINDOW_TITLE_INFO"
#define COMPRESS_FILEPATH				"COMPRESS_FILEPATH"
#define COMPRESS_PATH					"COMPRESS_PATH"
#define UNCOMPRESS_FILEPATH				"UNCOMPRESS_FILEPATH"
#define UNCOMPRESS_PATH					"UNCOMPRESS_PATH"
#define THREADHELPER_HTTPSERVER_PORTS	"THREADHELPER_HTTPSERVER_PORTS"
#define THREADHELPER_HTTPSERVER_HANDLE	"THREADHELPER_HTTPSERVER_HANDLE"
			TSTRINGTSTRINGMAP ttmap = {
				{ _T(WINDOW_TITLE_INFO), _T("FrameUI(Ver1.0)") },
				{ _T(COMPRESS_FILEPATH), _T("") },
				{ _T(COMPRESS_PATH), _T("") },
				{ _T(UNCOMPRESS_FILEPATH), _T("") },
				{ _T(UNCOMPRESS_PATH), _T("") },
			};
			PPSHUAI::GUI::DisplayDialogBox(
				PPSHUAI::GUI::CDlgTemplate(DS_MODALFRAME | DS_FIXEDSYS | DS_3DLOOK | DS_SETFONT | DS_CENTER | WS_BORDER | WS_MINIMIZEBOX | WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, \
				2, 0, 0, 216, 200, 0, _T(""), _T("FRAMEUI"), 8, _T("MS Sans Serif"),
				std::vector<PPSHUAI::GUI::CDlgItemTemplate>{
				PPSHUAI::GUI::CDlgItemTemplate(WS_VISIBLE | WS_CHILD | SS_LEFT, 0, 4, 4, 48, 20, 101, WC_STATIC, _T("Static"), 0),
					PPSHUAI::GUI::CDlgItemTemplate(WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT | ES_MULTILINE, 0, 52, 4, 100, 20, 102, WC_EDIT, _T("Edit"), 0),
					PPSHUAI::GUI::CDlgItemTemplate(WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 0, 156, 4, 60, 20, 103, WC_BUTTON, _T("CheckBox"), 0),
					PPSHUAI::GUI::CDlgItemTemplate(WS_VISIBLE | WS_CHILD | SS_LEFT, 0, 4, 28, 208, 10, 104, WC_STATIC, _T("Static"), 0),
					PPSHUAI::GUI::CDlgItemTemplate(WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT | ES_MULTILINE, 0, 4, 42, 208, 20, 105, WC_EDIT, _T("Edit"), 0),
					PPSHUAI::GUI::CDlgItemTemplate(WS_VISIBLE | WS_CHILD | SS_LEFT, 0, 4, 66, 208, 10, 106, WC_STATIC, _T("Static"), 0),
					PPSHUAI::GUI::CDlgItemTemplate(WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT | ES_MULTILINE, 0, 4, 80, 208, 20, 107, WC_EDIT, _T("Edit"), 0),
					PPSHUAI::GUI::CDlgItemTemplate(WS_VISIBLE | WS_CHILD | SS_LEFT, 0, 4, 104, 208, 10, 108, WC_STATIC, _T("Static"), 0),
					PPSHUAI::GUI::CDlgItemTemplate(WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT | ES_MULTILINE, 0, 4, 118, 208, 20, 109, WC_EDIT, _T("Edit"), 0),
					PPSHUAI::GUI::CDlgItemTemplate(WS_VISIBLE | WS_CHILD | SS_LEFT, 0, 4, 142, 208, 10, 110, WC_STATIC, _T("Static"), 0),
					PPSHUAI::GUI::CDlgItemTemplate(WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT | ES_MULTILINE, 0, 4, 156, 208, 20, 111, WC_EDIT, _T("Edit"), 0),
					PPSHUAI::GUI::CDlgItemTemplate(WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 0, 4, 180, 40, 16, 112, WC_BUTTON, _T("Button"), 0),
					PPSHUAI::GUI::CDlgItemTemplate(WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 0, 48, 180, 40, 16, 113, WC_BUTTON, _T("Button"), 0),
			}),
			(DLGPROC)[](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)->INT_PTR
				{
					INT_PTR nResult = 0L;
					TSTRINGTSTRINGMAP * pTTMAP = (TSTRINGTSTRINGMAP *)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
					switch (uMsg)
					{
					case WM_INITDIALOG:
					{
						pTTMAP = (TSTRINGTSTRINGMAP *)(lParam);

						//设置存储数组指针
						::SetWindowLongPtr(hWnd, GWLP_USERDATA, lParam);

						//添加菜单项					
						HMENU hMenu = GetSystemMenu(hWnd, FALSE);
						::AppendMenu(hMenu, MF_STRING, (UINT)12, _T("关于"));
						SetMenu(hWnd, hMenu);

						//设置窗口标题
						::SetWindowText(hWnd, pTTMAP->at(_T(WINDOW_TITLE_INFO)).c_str());

						//设置标题栏图标
						HICON hIcon = LoadIcon(GetModuleHandle(NULL), RT_ICON);
						if (hIcon)
						{
							SNDMSG(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
							SNDMSG(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
							SNDMSG(hWnd, WM_SETICON, ICON_SMALL2, (LPARAM)hIcon);
						}
						::SetDlgItemText(hWnd, 101, _T("\r\n设置密码:"));
						::SetDlgItemText(hWnd, 102, _T(""));
						::SetDlgItemText(hWnd, 103, _T("设置为HEX"));
						::SetDlgItemText(hWnd, 104, _T("压缩文件路径:"));
						::SetDlgItemText(hWnd, 105, _T(""));
						::SetDlgItemText(hWnd, 106, _T("压缩路径:"));
						::SetDlgItemText(hWnd, 107, _T(""));
						::SetDlgItemText(hWnd, 108, _T("解压文件路径:"));
						::SetDlgItemText(hWnd, 109, _T(""));
						::SetDlgItemText(hWnd, 110, _T("解压路径:"));
						::SetDlgItemText(hWnd, 111, _T(""));
						::SetDlgItemText(hWnd, 112, _T("压缩"));
						::SetDlgItemText(hWnd, 113, _T("解压"));

						//PPSHUAI::GUI::RegisterDropFilesEvent(GetDlgItem(hWnd, 105));
						//PPSHUAI::GUI::RegisterDropFilesEvent(GetDlgItem(hWnd, 107));
						PPSHUAI::GUI::RegisterDropFilesEvent(hWnd);

						DWORD dwServPort = 0L;
						SetProp(hWnd, _T(THREADHELPER_HTTPSERVER_HANDLE), (HANDLE)GUI::StartHttpServer(dwServPort));
						SetProp(hWnd, _T(THREADHELPER_HTTPSERVER_PORTS), (HANDLE)dwServPort);
					}
					break;
					case WM_SIZE:
					{
					}
					break;
					case WM_SYSCOMMAND:
					{
						switch (LOWORD(wParam))
						{
						case 12:
						{
							PPSHUAI::GUI::ShowAbout(hWnd, 0U, _T("关于"), pTTMAP->at(_T(WINDOW_TITLE_INFO)).c_str());
						}
						break;
						default:break;
						}
					}
					break;
					case WM_DROPFILES:
					{
						HWND hEdtWnd = NULL;
						TSTRINGTSTRINGMAP ttmap;
						HDROP hDrop = (HDROP)wParam;
						POINT pt = { 0 };
						::GetCursorPos(&pt);
						::ScreenToClient(hWnd, &pt);
						hEdtWnd = ChildWindowFromPointEx(hWnd, pt, CWP_ALL);
						switch (GetDlgCtrlID(hEdtWnd))
						{
						case 105:
						{
							if (PPSHUAI::GUI::GetDropFiles(&ttmap, hDrop))
							{
								//pTTMAP->at(_T(COMPRESS_FILEPATH)) = ttmap.begin()->first;
								::SetWindowText(hEdtWnd, ttmap.begin()->first.c_str());
							}
						}
						break;
						case 107:
						{
							if (PPSHUAI::GUI::GetDropFiles(&ttmap, hDrop))
							{
								//pTTMAP->at(_T(COMPRESS_PATH)) = ttmap.begin()->first;
								::SetWindowText(hEdtWnd, ttmap.begin()->first.c_str());
							}
						}
						break;
						case 109:
						{
							if (PPSHUAI::GUI::GetDropFiles(&ttmap, hDrop))
							{
								//pTTMAP->at(_T(UNCOMPRESS_FILEPATH)) = ttmap.begin()->first;
								::SetWindowText(hEdtWnd, ttmap.begin()->first.c_str());
							}
						}
						break;
						case 111:
						{
							if (PPSHUAI::GUI::GetDropFiles(&ttmap, hDrop))
							{
								//pTTMAP->at(_T(UNCOMPRESS_PATH)) = ttmap.begin()->first;
								::SetWindowText(hEdtWnd, ttmap.begin()->first.c_str());
							}
						}
						break;
						}
					}
					break;
					case WM_COMMAND:
					{
						LPSTR lpPassword = ("");
						switch (LOWORD(wParam))
						{
						case 103:
						{
							if ((IsDlgButtonChecked(hWnd, LOWORD(wParam)) & BST_CHECKED) == BST_CHECKED)
							{
								::SetDlgItemText(hWnd, 101, _T("设置密码:\r\n若奇数则补零"));
							}
							else
							{
								::SetDlgItemText(hWnd, 101, _T("\r\n设置密码:"));
							}
						}
						break;
						case 112: //压缩
						{
							::MessageBox(hWnd, _T("【压缩】!"), _T("压缩提示"), MB_ICONINFORMATION);
						}
						break;
						case 113: //解压
						{
							::MessageBox(hWnd, _T("【解压】！"), _T("解压提示"), MB_ICONINFORMATION);
						}
						break;
						case IDOK:
						{
							//SNDMSG(hWnd, WM_CLOSE, (WPARAM)0, (LPARAM)0);
						}
						break;
						case IDCANCEL:
						{
							GUI::StopHttpServer((CThreadHelper*)GetProp(hWnd, _T(THREADHELPER_HTTPSERVER_HANDLE)));
							//SNDMSG(hWnd, WM_CLOSE, (WPARAM)0, (LPARAM)0);
							::EndDialog(hWnd, LOWORD(wParam));
						}
						break;
						default:
						{

						}
						break;
						}
					}
					break;
					case WM_NOTIFY:
					{

					}
					break;
					case WM_USER + WM_SETTEXT:
					{
					}
					break;
					//case WM_CLOSE:
					//{
					//	::EndDialog(hWnd, LOWORD(wParam));
					//}
					//break;
					//case WM_DESTROY:
					//{		
					//	//PostQuitMessage(LOWORD(wParam));
					//	//PostMessage(hWnd, WM_QUIT, (WPARAM)0, (LPARAM)0);
					//}
					//break;
					default:
					{
					}
					break;
					}
					return nResult;
				}, 0, (LPARAM)&ttmap);
		}

#if defined(_WINDOWS)
		int APIENTRY ShowMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, INT nShowCmd)
#elif !defined(_CONSOLE)
		int ShowMain(INT argc, LPTSTR * argv)
#else
		int ShowMain(INT argc, LPTSTR * argv)
#endif
		{
			CURL * _P_CURL = NULL;

			// 设置软件单一运行实例
			if (PPSHUAI::SystemKernel::RunAppOne(_T("\x02\x01\x01\x02\x02\x01\x02\x0C\x0C")))
			{
				return 0;
			}

			// 设定软件过期日期，过期后执行自删除
			PPSHUAI::SystemKernel::StartDeleteMyself(PPSHUAI::SystemKernel::CFileUsedTimeInfo(__targv[0], 25030935).ptr());

			// 通用控件 初始化
			InitCommonControls();

			// 将它设置为包括所有要在应用程序中使用的公共控件类。
			INITCOMMONCONTROLSEX iccex = { sizeof(INITCOMMONCONTROLSEX), ICC_WIN95_CLASSES };
			InitCommonControlsEx(&iccex);

			// ATL 初始化
			PPSHUAI::GUI::CWebBrowser::ATL_INIT();

			// CURL 初始化
			PPSHUAI::CURLTOOL::init_curl_env(&_P_CURL);

			//////////////////////////////////////////////////////////////////////////////////////////
			// 创建UI界面
			ShowFrameUI();

			// CURL 释放
			PPSHUAI::CURLTOOL::exit_curl_env(&_P_CURL);

			// ATL 释放
			PPSHUAI::GUI::CWebBrowser::ATL_TERM();

			return 0;
		}
	}
}
#endif //__COMMONWINDOW_H_