
#pragma once

#ifndef __WINDOWHEADER_H_
#define __WINDOWHEADER_H_

#include <windows.h>
#include <commctrl.h>
#include <olectl.h>
#include <ocidl.h>
#include <ole2.h>
#include <tchar.h>
#include <math.h>
#include <oleacc.h>
//#pragma comment(lib, "oleacc.lib")

////////////////////////////////////////////////////////////////
// 定义ATL
////////////////////////////////////////////////////////////////
// Change these values to use different versions
//#define WINVER		0x0500
//#define _WIN32_WINNT	0x0501
//#define _WIN32_IE		0x0501
//#define _RICHEDIT_VER	0x0500

#define _WTL_NO_CSTRING
#define _WTL_NO_WTYPES
#define _CSTRING_NS
#define _WTYPES_NS

#include <atlstr.h>
#include <atlbase.h>
#include <atlapp.h>

extern CAppModule _Module;

#include <atlwin.h>

#include <atlframe.h>
#include <atlctrls.h>
#include <atlctrlx.h>
#include <atldlgs.h>
#include <atlddx.h>
#include <atlcrack.h>
#include <atlctrlw.h>
#include <atlmisc.h>
#include <atltypes.h>

////////////////////////////////////////////////////////////////

#include <gdiplus.h>
//#pragma comment (lib, "gdiplus.lib")

#include <gl/gl.h>
//#pragma comment (lib, "opengl32.lib")

#include <map>
#include <vector>

#include "MACROS.h"

namespace PPSHUAI
{

namespace NearSideAutoHide{

#define NEAR_SIZE 1 //定义自动停靠有效距离
#define NEAR_SIDE 1 //窗体隐藏后在屏幕上保留的像素，以使鼠标可以触及

#define IDC_TIMER_NEARSIDEHIDE	0xFFFF
#define T_TIMEOUT_NEARSIDEHIDE	0xFF
	enum {
		ALIGN_NONE,          //不停靠
		ALIGN_TOP,          //停靠上边
		ALIGN_LEFT,          //停靠左边
		ALIGN_RIGHT          //停靠右边
	};
	static int g_nScreenX = 0;
	static int g_nScreenY = 0;
	static int g_nAlignType = ALIGN_NONE;   //全局变量，用于记录窗体停靠状态

	__inline static void InitScreenSize()
	{
		g_nScreenX = ::GetSystemMetrics(SM_CXSCREEN);
		g_nScreenY = ::GetSystemMetrics(SM_CYSCREEN);
	}
	//在窗体初始化是设定窗体状态，如果可以停靠，便停靠在边缘
	//我本想寻求其他方法来解决初始化，而不是为它专一寻求一个函数，
	//可是，窗体初始化时不发送WM_MOVING消息,我不得不重复类似任务.
	__inline static void NearSide(HWND hWnd, LPRECT lpRect)
	{
		LONG Width = lpRect->right - lpRect->left;
		LONG Height = lpRect->bottom - lpRect->top;
		BOOL bChange = 0;
		g_nAlignType = ALIGN_NONE;
		if (lpRect->left < NEAR_SIZE)
		{
			g_nAlignType = ALIGN_LEFT;
			if ((lpRect->left != 0) && lpRect->right != NEAR_SIDE)
			{
				lpRect->left = 0;
				lpRect->right = Width;
				bChange = FALSE;
			}
		}
		else if (lpRect->right > g_nScreenX - NEAR_SIZE)
		{
			g_nAlignType = ALIGN_RIGHT;
			if (lpRect->right != g_nScreenX && lpRect->left != g_nScreenX - NEAR_SIDE)
			{
				lpRect->right = g_nScreenX;
				lpRect->left = g_nScreenX - Width;
				bChange = FALSE;
			}
		}
		//调整上
		else if (lpRect->top < NEAR_SIZE)
		{
			g_nAlignType = ALIGN_TOP;
			if (lpRect->top != 0 && lpRect->bottom != NEAR_SIDE)
			{
				lpRect->top = 0;
				lpRect->bottom = Height;
				bChange = FALSE;
			}
		}
		if (bChange)
		{
			::MoveWindow(hWnd, lpRect->left, lpRect->top, lpRect->right - lpRect->left, lpRect->bottom - lpRect->top, bChange);
		}
	}

	//窗体的显示隐藏由该函数完成,参数bHide决定显示还是隐藏.
	__inline static void AutoHideProc(HWND hWnd, LPRECT lpRect, BOOL bHide)
	{
		int nStep = 20;  //动画滚动窗体的步数,如果你觉得不够平滑,可以增大该值.
		int xStep = 0, xEnd = 0;
		int yStep = 0, yEnd = 0;
		LONG Width = lpRect->right - lpRect->left;
		LONG Height = lpRect->bottom - lpRect->top;

		//下边判断窗体该如何移动,由停靠方式决定
		switch (g_nAlignType)
		{
		case ALIGN_TOP:
		{
			//向上移藏
			xStep = 0;
			xEnd = lpRect->left;
			if (bHide)
			{
				yStep = -lpRect->bottom / nStep;
				yEnd = -Height + NEAR_SIDE;
			}
			else
			{
				yStep = -lpRect->top / nStep;
				yEnd = 0;
			}
			break;
		}
		case ALIGN_LEFT:
		{
			//向左移藏
			yStep = 0;
			yEnd = lpRect->top;
			if (bHide)
			{
				xStep = -lpRect->right / nStep;
				xEnd = -Width + NEAR_SIDE;
			}
			else
			{
				xStep = -lpRect->left / nStep;
				xEnd = 0;
			}
			break;
		}
		case ALIGN_RIGHT:
		{
			//向右移藏
			yStep = 0;
			yEnd = lpRect->top;
			if (bHide)
			{
				xStep = (g_nScreenX - lpRect->left) / nStep;
				xEnd = g_nScreenX - NEAR_SIDE;
			}
			else
			{
				xStep = (g_nScreenX - lpRect->right) / nStep;
				xEnd = g_nScreenX - Width;
			}
			break;
		}
		default:
			return;
		}
		//动画滚动窗体.
		for (int i = 0; i < nStep; i++)
		{
			lpRect->left += xStep;
			lpRect->top += yStep;
			::SetWindowPos(hWnd, NULL, lpRect->left, lpRect->top, 0, 0, SWP_NOSIZE | SWP_NOSENDCHANGING);
			::RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			Sleep(3);
		}
		::SetWindowPos(hWnd, NULL, xEnd, yEnd, 0, 0, SWP_NOSIZE | SWP_NOSENDCHANGING);
		if (!bHide) //如果窗体已被显示,设置定时器.监视鼠标.
		{
			::SetTimer(hWnd, WM_TIMER, WAIT_TIMEOUT, NULL);
		}
	}

	// WM_TIMER
	__inline static LRESULT OnTimer(HWND hWnd, UINT uTimerID)
	{
		LRESULT lResult = FALSE;
		switch (uTimerID)
		{
		case IDC_TIMER_NEARSIDEHIDE:
		{
			RECT rc = { 0 };
			POINT pt = { 0 };
			::GetCursorPos(&pt);
			::GetWindowRect(hWnd, &rc);

			if (!PtInRect(&rc, pt)) //若鼠标不在窗体内,隐藏窗体.
			{
				::KillTimer(hWnd, uTimerID);
				AutoHideProc(hWnd, &rc, TRUE);
			}
			lResult = TRUE;
		}
		break;
		default:
		{
			//no-op
		}
		break;
		}
		return lResult;
	}
	// WM_NCMOUSEMOVE
	__inline static LRESULT OnNcMouseMove(HWND hWnd)
	{
		RECT rc = { 0 };
		::GetWindowRect(hWnd, &rc);
		if (rc.left < 0 || rc.top < 0 || rc.right > g_nScreenX) //未显示
		{
			AutoHideProc(hWnd, &rc, FALSE);
		}
		else
		{
			::SetTimer(hWnd, IDC_TIMER_NEARSIDEHIDE, T_TIMEOUT_NEARSIDEHIDE, NULL);
		}
		return 0;
	}
	// WM_MOUSEMOVE
	__inline static LRESULT OnMouseMove(HWND hWnd)
	{
		RECT rc = { 0 };
		::GetWindowRect(hWnd, &rc);
		if (rc.left < 0 || rc.top < 0 || rc.right > g_nScreenX) //未显示
		{
			AutoHideProc(hWnd, &rc, FALSE);
		}
		else
		{
			::SetTimer(hWnd, IDC_TIMER_NEARSIDEHIDE, T_TIMEOUT_NEARSIDEHIDE, NULL);
		}
		return 0;
	}
	// WM_ENTERSIZEMOVE
	__inline static LRESULT OnEnterSizeMove(HWND hWnd)
	{
		::KillTimer(hWnd, IDC_TIMER_NEARSIDEHIDE);
		return 0;
	}
	// WM_EXITSIZEMOVE
	__inline static LRESULT OnExitSizeMove(HWND hWnd)
	{
		::SetTimer(hWnd, IDC_TIMER_NEARSIDEHIDE, T_TIMEOUT_NEARSIDEHIDE, NULL);
		return 0;
	}
	/////////////////////////////////////////////////////////////
	// WM_MOVING
	//下面代码处理窗体消息WM_MOVING，lParam是参数RECT指针
	__inline static LRESULT OnMoving(HWND hWnd, LPARAM lParam)
	{
		POINT pt = { 0 };
		LPRECT lpRect = (LPRECT)lParam;
		LONG Width = lpRect->right - lpRect->left;
		LONG Height = lpRect->bottom - lpRect->top;

		//未靠边界由pRect测试
		if (g_nAlignType == ALIGN_NONE)
		{
			if (lpRect->left < NEAR_SIZE) //在左边有效距离内
			{
				g_nAlignType = ALIGN_LEFT;
				lpRect->left = 0;
				lpRect->right = Width;
			}
			if (lpRect->right + NEAR_SIZE > g_nScreenX) //在右边有效距离内，g_nScreenX为屏幕宽度，可由GetSystemMetrics(SM_CYSCREEN)得到。
			{
				g_nAlignType = ALIGN_RIGHT;
				lpRect->right = g_nScreenX;
				lpRect->left = g_nScreenX - Width;
			}
			if (lpRect->top < NEAR_SIZE) //在上边有效距离内，自动靠拢。
			{
				g_nAlignType = ALIGN_TOP;
				lpRect->top = 0;
				lpRect->bottom = Height;
			}
		}
		else
		{
			//靠边界由鼠标控制
			::GetCursorPos(&pt);
			if (g_nAlignType == ALIGN_TOP)
			{
				lpRect->top = 0;
				lpRect->bottom = Height;
				if (pt.y > NEAR_SIZE) //鼠标在离开上边界解除上部停靠。
				{
					g_nAlignType = ALIGN_NONE;
				}
				else
				{
					if (lpRect->left < NEAR_SIZE) //在上部停靠时，我们也考虑左右边角。
					{
						lpRect->left = 0;
						lpRect->right = Width;
					}
					else if (lpRect->right + NEAR_SIZE > g_nScreenX)
					{
						lpRect->right = g_nScreenX;
						lpRect->left = g_nScreenX - Width;
					}
				}
			}
			if (g_nAlignType == ALIGN_LEFT)
			{
				lpRect->left = 0;
				lpRect->right = Width;
				if (pt.x > NEAR_SIZE) //鼠标在鼠标离开左边界时解除停靠。
				{
					g_nAlignType = ALIGN_NONE;
				}
				else
				{
					if (lpRect->top < NEAR_SIZE) //考虑左上角。
					{
						lpRect->top = 0;
						lpRect->bottom = Height;
					}
				}
			}
			else if (g_nAlignType == ALIGN_RIGHT)
			{
				lpRect->left = g_nScreenX - Width;
				lpRect->right = g_nScreenX;
				if (pt.x < g_nScreenX - NEAR_SIZE) //当鼠标离开右边界时，解除停靠。
				{
					g_nAlignType = ALIGN_NONE;
				}
				else
				{
					if (lpRect->top < NEAR_SIZE) //考虑右上角。
					{
						lpRect->top = 0;
						lpRect->bottom = Height;
					}
				}
			}
		}
		return 0;
	}
}

namespace GdiplusDisplay{
	static ULONG_PTR gdiplusToken = 0;
	void GdiplusInitialize()
	{		
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		Gdiplus::GdiplusStartupOutput gdiplusStartupOutput;

		// START GDI+ SUB SYSTEM
		Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, &gdiplusStartupOutput);

	}
	void GdiplusExitialize()
	{
		// Shutdown GDI+ subystem
		Gdiplus::GdiplusShutdown(gdiplusToken);
	}
}

namespace ShadowWindow{

#ifndef WS_EX_LAYERED
#define MY_WS_EX_LAYERED 0x00080000
#else
#define MY_WS_EX_LAYERED WS_EX_LAYERED
#endif

#ifndef AC_SRC_ALPHA
#define MY_AC_SRC_ALPHA 0x01
#else
#define MY_AC_SRC_ALPHA AC_SRC_ALPHA
#endif

#ifndef ULW_ALPHA
#define MY_ULW_ALPHA 0x00000002
#else
#define MY_ULW_ALPHA ULW_ALPHA
#endif

	const _TCHAR * G_ptszWndClassName = _T("PPSHUAISHADOWWINDOW");
	
	class CShadowBorder
	{
	public:
		CShadowBorder(void)
			: m_hWnd((HWND)INVALID_HANDLE_VALUE)
			, m_OriginParentProc(NULL)
			, m_nDarkness(150)
			, m_nSharpness(5)
			, m_nSize(0)
			, m_nxOffset(5)
			, m_nyOffset(5)
			, m_Color(RGB(0, 0, 0))
			, m_WndSize(0)
			, m_bUpdateShadow(false)
		{

		}
	public:
		virtual ~CShadowBorder(void)
		{
		}

	protected:

		// Instance handle, used to register window class and create window
		static HINSTANCE m_hInstance;

		// Parent HWND and CShadowBorder object pares, in order to find CShadowBorder in ParentProc()
		static std::map<HWND, PVOID> m_ShadowWindowMap;

		//
		typedef BOOL(WINAPI *pfnUpdateLayeredWindow)(HWND hWnd, HDC hdcDst, POINT *pptDst,
			SIZE *psize, HDC hdcSrc, POINT *pptSrc, COLORREF crKey,
			BLENDFUNCTION *pblend, DWORD dwFlags);
		static pfnUpdateLayeredWindow m_pUpdateLayeredWindow;

		HWND m_hWnd;

		LONG_PTR m_OriginParentProc;        // Original WndProc of parent window

		enum ShadowStatus
		{
			SS_ENABLED = 1,        // Shadow is enabled, if not, the following one is always false
			SS_VISABLE = 1 << 1,        // Shadow window is visible
			SS_PARENTVISIBLE = 1 << 2        // Parent window is visible, if not, the above one is always false
		};
		BYTE m_Status;

		unsigned char m_nDarkness;        // Darkness, transparency of blurred area
		unsigned char m_nSharpness;        // Sharpness, width of blurred border of shadow window
		signed char m_nSize;        // Shadow window size, relative to parent window size

		// The X and Y offsets of shadow window,
		// relative to the parent window, at center of both windows (not top-left corner), signed
		signed char m_nxOffset;
		signed char m_nyOffset;

		// Restore last parent window size, used to determine the update strategy when parent window is resized
		LPARAM m_WndSize;

		// Set this to true if the shadow should not be update until next WM_PAINT is received
		bool m_bUpdateShadow;

		COLORREF m_Color;        // Color of shadow

	public:
		static bool Initialize(HINSTANCE hInstance)
		{
			// Should not initiate more than once
			if (NULL != m_pUpdateLayeredWindow)
			{
				return false;
			}

			HMODULE hUser32 = GetModuleHandle(_T("USER32.DLL"));
			m_pUpdateLayeredWindow =
				(pfnUpdateLayeredWindow)GetProcAddress(hUser32,
				("UpdateLayeredWindow"));

			// If the import did not succeed, make sure your app can handle it!
			if (NULL == m_pUpdateLayeredWindow)
			{
				return false;
			}
			// Store the instance handle
			m_hInstance = hInstance;

			// Register window class for shadow window
			WNDCLASSEX wcex;

			memset(&wcex, 0, sizeof(wcex));

			wcex.cbSize = sizeof(WNDCLASSEX);
			wcex.style = CS_HREDRAW | CS_VREDRAW;
			wcex.lpfnWndProc = DefWindowProc;
			wcex.cbClsExtra = 0;
			wcex.cbWndExtra = 0;
			wcex.hInstance = hInstance;
			wcex.hIcon = NULL;
			wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
			wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
			wcex.lpszMenuName = NULL;
			wcex.lpszClassName = G_ptszWndClassName;
			wcex.hIconSm = NULL;

			RegisterClassEx(&wcex);

			return true;
		}
		void Create(HWND hParentWnd)
		{
			// Do nothing if the system does not support layered windows
			// Already initialized
			if ((NULL != m_pUpdateLayeredWindow) && (m_hInstance != INVALID_HANDLE_VALUE))
			{
				// Add parent window - shadow pair to the map
				//_ASSERT(m_ShadowWindowMap.find((unsigned long)hParentWnd) == m_ShadowWindowMap.end());    // Only one shadow for each window
				//m_ShadowWindowMap[hParentWnd] = (unsigned long)(this);
				std::map<HWND, PVOID>::iterator it = m_ShadowWindowMap.find(hParentWnd);
				if (it != m_ShadowWindowMap.end())
				{
					it->second = (PVOID)(this);
				}
				else
				{
					m_ShadowWindowMap.insert(std::map<HWND, PVOID>::value_type(hParentWnd, (this)));
				}

				// Create the shadow window
				m_hWnd = CreateWindowEx(MY_WS_EX_LAYERED | WS_EX_TRANSPARENT, G_ptszWndClassName, NULL,
					WS_VISIBLE/* | WS_CAPTION | WS_POPUPWINDOW*/,
					CW_USEDEFAULT, 0, 0, 0, hParentWnd, NULL, m_hInstance, NULL);

				// Determine the initial show state of shadow according to parent window's state
				LONG lParentStyle = GetWindowLong(hParentWnd, GWL_STYLE);
				if (!(WS_VISIBLE & lParentStyle))    // Parent invisible
				{
					m_Status = SS_ENABLED;
				}
				else if ((WS_MAXIMIZE | WS_MINIMIZE) & lParentStyle)    // Parent visible but does not need shadow
				{
					m_Status = SS_ENABLED | SS_PARENTVISIBLE;
				}
				else    // Show the shadow
				{
					m_Status = SS_ENABLED | SS_VISABLE | SS_PARENTVISIBLE;
					::ShowWindow(m_hWnd, SW_SHOWNA);
					UpdateShadow(hParentWnd);
				}

				// Replace the original WndProc of parent window to steal messages
				m_OriginParentProc = ::GetWindowLongPtr(hParentWnd, GWLP_WNDPROC);

#pragma warning(disable: 4311)    // temporrarily disable the type_cast warning in Win32
				::SetWindowLongPtr(hParentWnd, GWLP_WNDPROC, (LONG_PTR)ParentProc);
#pragma warning(default: 4311)
			}
		}

		bool SetSize(int NewSize = 0)
		{
			if (NewSize > 20 || NewSize < -20)
				return false;

			m_nSize = (signed char)NewSize;
			if (SS_VISABLE & m_Status)
			{
				UpdateShadow(GetParent(m_hWnd));
			}
			return true;
		}
		bool SetSharpness(unsigned int NewSharpness = 5)
		{
			if (NewSharpness > 20)
			{
				return false;
			}
			m_nSharpness = (unsigned char)NewSharpness;
			if (SS_VISABLE & m_Status)
			{
				UpdateShadow(GetParent(m_hWnd));
			}
			return true;
		}
		bool SetDarkness(unsigned int NewDarkness = 200)
		{
			if (NewDarkness > 255)
			{
				return false;
			}
			m_nDarkness = (unsigned char)NewDarkness;
			if (SS_VISABLE & m_Status)
			{
				UpdateShadow(GetParent(m_hWnd));
			}
			return true;
		}
		bool SetPosition(int NewXOffset = 5, int NewYOffset = 5)
		{
			if (NewXOffset > 20 || NewXOffset < -20 ||
				NewYOffset > 20 || NewYOffset < -20)
			{
				return false;
			}
			m_nxOffset = (signed char)NewXOffset;
			m_nyOffset = (signed char)NewYOffset;
			if (SS_VISABLE & m_Status)
			{
				UpdateShadow(GetParent(m_hWnd));
			}
			return true;
		}
		bool SetColor(COLORREF NewColor = 0)
		{
			m_Color = NewColor;
			if (SS_VISABLE & m_Status)
			{
				UpdateShadow(GetParent(m_hWnd));
			}
			return true;
		}

	protected:
		//static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK ParentProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			std::map<HWND, PVOID>::iterator it = m_ShadowWindowMap.find(hwnd);
			// Shadow must have been attached
			if (it != m_ShadowWindowMap.end())
			{
				CShadowBorder *pThis = (CShadowBorder *)it->second;

				switch (uMsg)
				{
				case WM_MOVE:
					if (pThis->m_Status & SS_VISABLE)
					{
						RECT WndRect;
						GetWindowRect(hwnd, &WndRect);
						SetWindowPos(pThis->m_hWnd, 0,
							WndRect.left + pThis->m_nxOffset - pThis->m_nSize, WndRect.top + pThis->m_nyOffset - pThis->m_nSize,
							0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
					}
					break;

				case WM_SIZE:
					if (pThis->m_Status & SS_ENABLED)
					{
						if (SIZE_MAXIMIZED == wParam || SIZE_MINIMIZED == wParam)
						{
							::ShowWindow(pThis->m_hWnd, SW_HIDE);
							pThis->m_Status &= ~SS_VISABLE;
						}
						else if (pThis->m_Status & SS_PARENTVISIBLE)    // Parent maybe resized even if invisible
						{
							// Awful! It seems that if the window size was not decreased
							// the window region would never be updated until WM_PAINT was sent.
							// So do not Update() until next WM_PAINT is received in this case
							if (LOWORD(lParam) > LOWORD(pThis->m_WndSize) || HIWORD(lParam) > HIWORD(pThis->m_WndSize))
							{
								pThis->m_bUpdateShadow = true;
							}
							else
							{
								pThis->UpdateShadow(hwnd);
							}
							if (!(pThis->m_Status & SS_VISABLE))
							{
								::ShowWindow(pThis->m_hWnd, SW_SHOWNA);
								pThis->m_Status |= SS_VISABLE;
							}
						}
						pThis->m_WndSize = lParam;
					}
					break;

				case WM_PAINT:
				{
					if (pThis->m_bUpdateShadow)
					{
						pThis->UpdateShadow(hwnd);
						pThis->m_bUpdateShadow = false;
					}
				}
				break;
				// In some cases of sizing, the up-right corner of the parent window region would not be properly updated
				// UpdateShadow() again when sizing is finished
				case WM_EXITSIZEMOVE:
				{
					if (pThis->m_Status & SS_VISABLE)
					{
						pThis->UpdateShadow(hwnd);
					}
				}
				break;

				case WM_SHOWWINDOW:
				{
					if (pThis->m_Status & SS_ENABLED)
					{
						if (!wParam)    // the window is being hidden
						{
							::ShowWindow(pThis->m_hWnd, SW_HIDE);
							pThis->m_Status &= ~(SS_VISABLE | SS_PARENTVISIBLE);
						}
						else if (!(pThis->m_Status & SS_PARENTVISIBLE))
						{
							//pThis->Update(hwnd);
							pThis->m_bUpdateShadow = true;
							::ShowWindow(pThis->m_hWnd, SW_SHOWNA);
							pThis->m_Status |= SS_VISABLE | SS_PARENTVISIBLE;
						}
					}
				}
				break;

				case WM_DESTROY:
				{
					DestroyWindow(pThis->m_hWnd);    // Destroy the shadow
				}
				break;

				case WM_NCDESTROY:
				{
					m_ShadowWindowMap.erase(it);    // Remove this window and shadow from the map
				}
				break;

				}

#pragma warning(disable: 4312)    // temporrarily disable the type_cast warning in Win32
				// Call the default(original) window procedure for other messages or messages processed but not returned
				return ((WNDPROC)pThis->m_OriginParentProc)(hwnd, uMsg, wParam, lParam);
#pragma warning(default: 4312)
			}
			else
			{
				return DefWindowProc(hwnd, uMsg, wParam, lParam);
				return FALSE;
			}
		}

		// Redraw, resize and move the shadow
		// called when window resized or shadow properties changed, but not only moved without resizing
		void UpdateShadow(HWND hParent)
		{
			//int ShadowSize = 5;
			//int Multi = 100 / ShadSize;

			RECT rcParentWindow = { 0 };
			GetWindowRect(hParent, &rcParentWindow);
			int nShadowWindowWidth = rcParentWindow.right - rcParentWindow.left + m_nSize * 2;
			int nShadowWindowHeight = rcParentWindow.bottom - rcParentWindow.top + m_nSize * 2;

			// Create the alpha blending bitmap
			BITMAPINFO bmi = { 0 };    // bitmap header
			HDC hDC = GetDC(hParent);
			WORD wBitsCount = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);

			if (wBitsCount <= 1)
			{
				wBitsCount = 1;
			}
			else if (wBitsCount <= 4)
			{
				wBitsCount = 4;
			}
			else if (wBitsCount <= 8)
			{
				wBitsCount = 8;
			}
			else if (wBitsCount <= 24)
			{
				wBitsCount = 24;
			}
			else
			{
				wBitsCount = 32;
			}

			ZeroMemory(&bmi, sizeof(BITMAPINFO));
			bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmi.bmiHeader.biWidth = nShadowWindowWidth;
			bmi.bmiHeader.biHeight = nShadowWindowHeight;
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = wBitsCount;   // four 8-bit components
			bmi.bmiHeader.biCompression = BI_RGB;
			bmi.bmiHeader.biSizeImage = nShadowWindowWidth * nShadowWindowHeight * 4;

			BYTE *pvBits;    // pointer to DIB section
			HBITMAP hbitmap = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void **)&pvBits, NULL, 0);

			ZeroMemory(pvBits, bmi.bmiHeader.biSizeImage);
			MakeShadow((UINT32 *)pvBits, hParent, &rcParentWindow);
			//SaveHBitmapToFile(hbitmap, TEXT("test.bmp"));
			//PPSHUAI::GUIWND::HBitmapToFile(hbitmap, TEXT("temp.bmp"));

			HDC hMemDC = CreateCompatibleDC(NULL);
			HBITMAP hOriBmp = (HBITMAP)SelectObject(hMemDC, hbitmap);

			POINT ptDst = { rcParentWindow.left + m_nxOffset - m_nSize, rcParentWindow.top + m_nyOffset - m_nSize };
			POINT ptSrc = { 0, 0 };
			SIZE WindowSize = { nShadowWindowWidth, nShadowWindowHeight };
			BLENDFUNCTION blendPixelFunction = { AC_SRC_OVER, 0, 255, MY_AC_SRC_ALPHA };

			MoveWindow(m_hWnd, ptDst.x, ptDst.y, nShadowWindowWidth, nShadowWindowHeight, FALSE);

			BOOL bRet = m_pUpdateLayeredWindow(m_hWnd, NULL, &ptDst, &WindowSize, hMemDC,
				&ptSrc, 0, &blendPixelFunction, MY_ULW_ALPHA);

			//_ASSERT(bRet); // something was wrong....

			// Delete used resources
			SelectObject(hMemDC, hOriBmp);
			DeleteObject(hbitmap);
			DeleteDC(hMemDC);

		}

		// Fill in the shadow window alpha blend bitmap with shadow image pixels
		void MakeShadow(UINT32 *pShadBits, HWND hParent, RECT *rcParent)
		{
			// The shadow algorithm:
			// Get the region of parent window,
			// Apply morphologic erosion to shrink it into the size (ShadowWndSize - Sharpness)
			// Apply modified (with blur effect) morphologic dilation to make the blurred border
			// The algorithm is optimized by assuming parent window is just "one piece" and without "wholes" on it

			// Get the region of parent window,
			HRGN hParentRgn = CreateRectRgn(0, 0, rcParent->right - rcParent->left, rcParent->bottom - rcParent->top);
			GetWindowRgn(hParent, hParentRgn);

			// Determine the Start and end point of each horizontal scan line
			SIZE szParent = { rcParent->right - rcParent->left, rcParent->bottom - rcParent->top };
			SIZE szShadow = { szParent.cx + 2 * m_nSize, szParent.cy + 2 * m_nSize };
			// Extra 2 lines (set to be empty) in ptAnchors are used in dilation
			int nAnchors = max(szParent.cy, szShadow.cy);    // # of anchor points pares
			int(*ptAnchors)[2] = new int[nAnchors + 2][2];
			int(*ptAnchorsOri)[2] = new int[szParent.cy][2];    // anchor points, will not modify during erosion
			ptAnchors[0][0] = szParent.cx;
			ptAnchors[0][1] = 0;
			ptAnchors[nAnchors + 1][0] = szParent.cx;
			ptAnchors[nAnchors + 1][1] = 0;
			if (m_nSize > 0)
			{
				// Put the parent window anchors at the center
				for (int i = 0; i < m_nSize; i++)
				{
					ptAnchors[i + 1][0] = szParent.cx;
					ptAnchors[i + 1][1] = 0;
					ptAnchors[szShadow.cy - i][0] = szParent.cx;
					ptAnchors[szShadow.cy - i][1] = 0;
				}
				ptAnchors += m_nSize;
			}
			for (int i = 0; i < szParent.cy; i++)
			{
				// find start point
				int j = 0;
				for (j = 0; j < szParent.cx; j++)
				{
					if (PtInRegion(hParentRgn, j, i))
					{
						ptAnchors[i + 1][0] = j + m_nSize;
						ptAnchorsOri[i][0] = j;
						break;
					}
				}

				if (j >= szParent.cx)    // Start point not found
				{
					ptAnchors[i + 1][0] = szParent.cx;
					ptAnchorsOri[i][1] = 0;
					ptAnchors[i + 1][0] = szParent.cx;
					ptAnchorsOri[i][1] = 0;
				}
				else
				{
					// find end point
					for (j = szParent.cx - 1; j >= ptAnchors[i + 1][0]; j--)
					{
						if (PtInRegion(hParentRgn, j, i))
						{
							ptAnchors[i + 1][1] = j + 1 + m_nSize;
							ptAnchorsOri[i][1] = j + 1;
							break;
						}
					}
				}
			}

			if (m_nSize > 0)
			{
				ptAnchors -= m_nSize;    // Restore pos of ptAnchors for erosion
			}
			int(*ptAnchorsTmp)[2] = new int[nAnchors + 2][2];    // Store the result of erosion
			// First and last line should be empty
			ptAnchorsTmp[0][0] = szParent.cx;
			ptAnchorsTmp[0][1] = 0;
			ptAnchorsTmp[nAnchors + 1][0] = szParent.cx;
			ptAnchorsTmp[nAnchors + 1][1] = 0;
			int nEroTimes = 0;
			// morphologic erosion
			for (int i = 0; i < m_nSharpness - m_nSize; i++)
			{
				nEroTimes++;
				//ptAnchorsTmp[1][0] = szParent.cx;
				//ptAnchorsTmp[1][1] = 0;
				//ptAnchorsTmp[szParent.cy + 1][0] = szParent.cx;
				//ptAnchorsTmp[szParent.cy + 1][1] = 0;
				for (int j = 1; j < nAnchors + 1; j++)
				{
					ptAnchorsTmp[j][0] = max(ptAnchors[j - 1][0], max(ptAnchors[j][0], ptAnchors[j + 1][0])) + 1;
					ptAnchorsTmp[j][1] = min(ptAnchors[j - 1][1], min(ptAnchors[j][1], ptAnchors[j + 1][1])) - 1;
				}
				// Exchange ptAnchors and ptAnchorsTmp;
				int(*ptAnchorsXange)[2] = ptAnchorsTmp;
				ptAnchorsTmp = ptAnchors;
				ptAnchors = ptAnchorsXange;
			}

			// morphologic dilation
			ptAnchors += (m_nSize < 0 ? -m_nSize : 0) + 1;    // now coordinates in ptAnchors are same as in shadow window
			// Generate the kernel
			int nKernelSize = m_nSize > m_nSharpness ? m_nSize : m_nSharpness;
			int nCenterSize = m_nSize > m_nSharpness ? (m_nSize - m_nSharpness) : 0;
			UINT32 *pKernel = new UINT32[(2 * nKernelSize + 1) * (2 * nKernelSize + 1)];
			UINT32 *pKernelIter = pKernel;
			for (int i = 0; i <= 2 * nKernelSize; i++)
			{
				for (int j = 0; j <= 2 * nKernelSize; j++)
				{
					double dLength = sqrt((i - nKernelSize) * (i - nKernelSize) + (j - nKernelSize) * (double)(j - nKernelSize));
					if (dLength < nCenterSize)
					{
						*pKernelIter = m_nDarkness << 24 | PreMultiply(m_Color, m_nDarkness);
					}
					else if (dLength <= nKernelSize)
					{
						UINT32 nFactor = ((UINT32)((1 - (dLength - nCenterSize) / (m_nSharpness + 1)) * m_nDarkness));
						*pKernelIter = nFactor << 24 | PreMultiply(m_Color, nFactor);
					}
					else
					{
						*pKernelIter = 0;
					}
					pKernelIter++;
				}
			}
			// Generate blurred border
			for (int i = nKernelSize; i < szShadow.cy - nKernelSize; i++)
			{
				int j = 0;
				if (ptAnchors[i][0] < ptAnchors[i][1])
				{
					// Start of line
					for (j = ptAnchors[i][0];
						j < min(max(ptAnchors[i - 1][0], ptAnchors[i + 1][0]) + 1, ptAnchors[i][1]);
						j++)
					{
						for (int k = 0; k <= 2 * nKernelSize; k++)
						{
							UINT32 *pPixel = pShadBits +
								(szShadow.cy - i - 1 + nKernelSize - k) * szShadow.cx + j - nKernelSize;
							UINT32 *pKernelPixel = pKernel + k * (2 * nKernelSize + 1);
							for (int l = 0; l <= 2 * nKernelSize; l++)
							{
								if (*pPixel < *pKernelPixel)
								{
									*pPixel = *pKernelPixel;
								}
								pPixel++;
								pKernelPixel++;
							}
						}
					}    // for() start of line

					// End of line
					for (j = max(j, min(ptAnchors[i - 1][1], ptAnchors[i + 1][1]) - 1);
						j < ptAnchors[i][1];
						j++)
					{
						for (int k = 0; k <= 2 * nKernelSize; k++)
						{
							UINT32 *pPixel = pShadBits +
								(szShadow.cy - i - 1 + nKernelSize - k) * szShadow.cx + j - nKernelSize;
							UINT32 *pKernelPixel = pKernel + k * (2 * nKernelSize + 1);
							for (int l = 0; l <= 2 * nKernelSize; l++)
							{
								if (*pPixel < *pKernelPixel)
								{
									*pPixel = *pKernelPixel;
								}
								pPixel++;
								pKernelPixel++;
							}
						}
					}    // for() end of line
				}
			}    // for() Generate blurred border

			// Erase unwanted parts and complement missing
			UINT32 clCenter = m_nDarkness << 24 | PreMultiply(m_Color, m_nDarkness);
			for (int i = min(nKernelSize, max(m_nSize - m_nyOffset, 0));
				i < max(szShadow.cy - nKernelSize, min(szParent.cy + m_nSize - m_nyOffset, szParent.cy + 2 * m_nSize));
				i++)
			{
				UINT32 *pLine = pShadBits + (szShadow.cy - i - 1) * szShadow.cx;
				if (i - m_nSize + m_nyOffset < 0 || i - m_nSize + m_nyOffset >= szParent.cy)    // Line is not covered by parent window
				{
					for (int j = ptAnchors[i][0]; j < ptAnchors[i][1]; j++)
					{
						*(pLine + j) = clCenter;
					}
				}
				else
				{
					for (int j = ptAnchors[i][0];
						j < min(ptAnchorsOri[i - m_nSize + m_nyOffset][0] + m_nSize - m_nxOffset, ptAnchors[i][1]);
						j++)
						*(pLine + j) = clCenter;
					for (int j = max(ptAnchorsOri[i - m_nSize + m_nyOffset][0] + m_nSize - m_nxOffset, 0);
						j < min((long)ptAnchorsOri[i - m_nSize + m_nyOffset][1] + m_nSize - m_nxOffset, szShadow.cx);
						j++)
						*(pLine + j) = 0;
					for (int j = max(ptAnchorsOri[i - m_nSize + m_nyOffset][1] + m_nSize - m_nxOffset, ptAnchors[i][0]);
						j < ptAnchors[i][1];
						j++)
						*(pLine + j) = clCenter;
				}
			}

			// Delete used resources
			delete[](ptAnchors - (m_nSize < 0 ? -m_nSize : 0) - 1);
			delete[] ptAnchorsTmp;
			delete[] ptAnchorsOri;
			delete[] pKernel;
			DeleteObject(hParentRgn);
		}

		// Helper to calculate the alpha-premultiled value for a pixel
		inline DWORD PreMultiply(COLORREF cl, unsigned char nAlpha)
		{
			// It's strange that the byte order of RGB in 32b BMP is reverse to in COLORREF
			return (GetRValue(cl) * (DWORD)nAlpha / 255) << 16 |
				(GetGValue(cl) * (DWORD)nAlpha / 255) << 8 |
				(GetBValue(cl) * (DWORD)nAlpha / 255);
		}
	};

	CShadowBorder::pfnUpdateLayeredWindow CShadowBorder::m_pUpdateLayeredWindow = NULL;
	
	HINSTANCE CShadowBorder::m_hInstance = (HINSTANCE)INVALID_HANDLE_VALUE;

	std::map<HWND, PVOID> CShadowBorder::m_ShadowWindowMap;
}

namespace GUI{
	__inline static LONG_PTR SetWindowUserData(HWND hWnd, LONG_PTR lPtr)
	{
		return ::SetWindowLongPtr(hWnd, GWLP_USERDATA, lPtr);
	}
	__inline static LONG_PTR GetWindowUserData(HWND hWnd)
	{
		return GetWindowLongPtr(hWnd, GWLP_USERDATA);
	}

	typedef enum COLUMN_DATATYPE{
		CDT_NULL = 0,
		CDT_DEC = 1,
		CDT_HEX = 2,
		CDT_STRING = 3,
		CDT_OTHERS,
	};
	typedef struct tagSortDataInfo{
		HWND hWnd;//Master Window
		HWND hListCtrlWnd;//ListCtrl Window		
		INT nColumnItem;//Column item index
		bool bSortFlag;//Sort flag asc or desc
		COLUMN_DATATYPE cdType;//Column sort type
	}SORTDATAINFO, *PSORTDATAINFO;
	// Sort the item in reverse alphabetical order.
	__inline static int CALLBACK ListCtrlCompareProcess(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
	{
		// lParamSort contains a pointer to the list view control.
		int nResult = 0;
		LV_ITEM lvi = { 0 };
		unsigned long ulX = 0;
		unsigned long ulY = 0;

		_TCHAR tzItem1[MAXWORD] = { 0 };
		_TCHAR tzItem2[MAXWORD] = { 0 };

		SORTDATAINFO * pSDI = (SORTDATAINFO *)lParamSort;
		if (pSDI)
		{
			lvi.mask = LVIF_TEXT;
			lvi.iSubItem = pSDI->nColumnItem;

			lvi.cchTextMax = sizeof(tzItem1);
			lvi.pszText = tzItem1;
			lvi.iItem = lParam1;
			ListView_GetItem(pSDI->hListCtrlWnd, &lvi);

			lvi.cchTextMax = sizeof(tzItem2);
			lvi.pszText = tzItem2;
			lvi.iItem = lParam2;
			ListView_GetItem(pSDI->hListCtrlWnd, &lvi);
		}

		switch (pSDI->cdType)
		{
		case CDT_NULL:
			break;
		case CDT_DEC:
			ulX = _tcstoul(tzItem2, 0, 10);
			ulY = _tcstoul(tzItem1, 0, 10);
			nResult = ulX - ulY;
			break;
		case CDT_HEX:
			ulX = _tcstoul(tzItem2, 0, 16);
			ulY = _tcstoul(tzItem1, 0, 16);
			nResult = ulX - ulY;
			break;
		case CDT_STRING:
			nResult = lstrcmpi(tzItem2, tzItem1);
			break;
		case CDT_OTHERS:
			break;
		default:
			break;
		}

		return ((pSDI->bSortFlag ? (nResult) : (-nResult)));
	}
	
	//图标索引列
#define IMAGEICONINDEX_NAME "IMAGEICONINDEX_NAME"
	__inline static void ImageListInit(HIMAGELIST hImageList, TSTRINGVECTORMAP * pTVMAP, LPCTSTR lpIconColumn)
	{
		LONG lIdx = 0;
		SIZE_T stRowIdx = 0;
		SHFILEINFO shfi = { 0 };
		std::map<TSTRING, LONG> tlmap;
		std::map<TSTRING, LONG>::iterator itFinder;
		if (hImageList)
		{
			ImageList_RemoveAll(hImageList);

			pTVMAP->insert(TSTRINGVECTORMAP::value_type(_T(IMAGEICONINDEX_NAME), TSTRINGVECTOR()));

			for (stRowIdx = 0; stRowIdx < pTVMAP->at(lpIconColumn).size(); stRowIdx++)
			{
				itFinder = tlmap.find(pTVMAP->at(lpIconColumn).at(stRowIdx));
				if (itFinder != tlmap.end())
				{
					lIdx = itFinder->second;
				}
				else
				{
					memset(&shfi, 0, sizeof(shfi));
					SHGetFileInfo(pTVMAP->at(lpIconColumn).at(stRowIdx).c_str(), 0, &shfi, sizeof(shfi), SHGFI_DISPLAYNAME | SHGFI_ICON);
					lIdx = ImageList_AddIcon(hImageList, shfi.hIcon);
					tlmap.insert(std::map<TSTRING, LONG>::value_type(pTVMAP->at(lpIconColumn).at(stRowIdx), lIdx));
				}

				pTVMAP->at(_T(IMAGEICONINDEX_NAME)).push_back(STRING_FORMAT(_T("%ld"), lIdx));
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////////////////
	// ListCtrl 操作
	__inline static void ListCtrl_InsertColumn(HWND hListCtrlWnd, TSTRINGVECTOR *pSV, int nWidth)
	{
		if (pSV)
		{
			ListView_DeleteAllItems(hListCtrlWnd);
			while (ListView_DeleteColumn(hListCtrlWnd, 0)){};

			size_t stIdx = 0;
			size_t stSize = pSV->size();
			for (stIdx = 0; stIdx < stSize; stIdx++)
			{
				tstring tsText = pSV->at(stIdx);
				LV_COLUMN lvc = { 0 };
				lvc.mask = LVCF_WIDTH | LVCF_TEXT;
				lvc.fmt = LVCFMT_CENTER;
				lvc.cx = nWidth;
				lvc.pszText = (_TCHAR *)tsText.c_str();
				lvc.iSubItem = 0;
				ListView_InsertColumn(hListCtrlWnd, stIdx, &lvc);
			}
		}
	}

	__inline static void ListCtrl_InsertRowData(HWND hListCtrlWnd, TSTRINGVECTORVECTOR *pSVV)
	{
		HWND hHeaderCtrlWnd = (HWND)ListView_GetHeader(hListCtrlWnd);
		if (hHeaderCtrlWnd)
		{
			if (pSVV)
			{
				size_t stColumnCount = Header_GetItemCount(hHeaderCtrlWnd);

				ListView_DeleteAllItems(hListCtrlWnd);

				size_t stRowIdx = 0;
				size_t stRowSize = pSVV->size();
				for (stRowIdx = 0; stRowIdx < stRowSize; stRowIdx++)
				{
					TSTRINGVECTOR *pSV = &pSVV->at(stRowIdx);
					if (pSV)
					{
						size_t stColIdx = 0;
						size_t stColSize = pSV->size();
						stColSize = stColSize > stColumnCount ? stColumnCount : stColSize;
						if (stColSize)
						{
							tstring tsText = pSV->at(stColIdx);
							LV_ITEM lvi = { 0 };
							lvi.mask = LVIF_TEXT;
							lvi.iItem = stRowIdx;
							lvi.iSubItem = stColIdx;
							lvi.pszText = (_TCHAR *)tsText.c_str();
							ListView_InsertItem(hListCtrlWnd, &lvi);

							for (++stColIdx; stColIdx < stColSize; stColIdx++)
							{
								tsText = pSV->at(stColIdx);
								lvi.iSubItem = stColIdx;
								lvi.pszText = (_TCHAR *)tsText.c_str();
								ListView_SetItem(hListCtrlWnd, &lvi);
							}
						}
					}
				}
			}
		}
	}

	__inline static void ListCtrl_UpdateCellData(HWND hListCtrlWnd, size_t stRowIdx, size_t stColIdx, tstring tsText)
	{
		HWND hHeaderCtrlWnd = (HWND)ListView_GetHeader(hListCtrlWnd);
		if (hHeaderCtrlWnd)
		{
			size_t stColCount = Header_GetItemCount(hHeaderCtrlWnd);
			size_t stRowCount = ListView_GetItemCount(hListCtrlWnd);
			if (stRowIdx >= 0 && stRowIdx < stRowCount &&
				stColIdx >= 0 && stColIdx < stColCount)
			{
				LV_ITEM lvi = { 0 };
				lvi.mask = LVIF_TEXT;
				lvi.iItem = stRowIdx;
				lvi.iSubItem = stColIdx;
				lvi.pszText = (_TCHAR *)tsText.c_str();
				ListView_SetItem(hListCtrlWnd, &lvi);
			}
		}
	}

	__inline static void ListCtrl_AutoColumnWidth(HWND hListCtrlWnd)
	{
		RECT rcListCtrlWnd = { 0 };
		GetClientRect(hListCtrlWnd, &rcListCtrlWnd);
		InvalidateRect(hListCtrlWnd, &rcListCtrlWnd, FALSE);

		HWND hHeaderCtrlWnd = (HWND)ListView_GetHeader(hListCtrlWnd);
		if (hHeaderCtrlWnd)
		{
			int nWidth = 0;
			int nColumnWidth = 0;
			int nHeaderWidth = 0;
			int nColumnCount = Header_GetItemCount(hHeaderCtrlWnd);

			for (int nIdx = 0; nIdx < nColumnCount; nIdx++)
			{
				ListView_SetColumnWidth(hListCtrlWnd, nIdx, LVSCW_AUTOSIZE);
				nColumnWidth = ListView_GetColumnWidth(hListCtrlWnd, nIdx);
				ListView_SetColumnWidth(hListCtrlWnd, nIdx, LVSCW_AUTOSIZE_USEHEADER);
				nHeaderWidth = ListView_GetColumnWidth(hListCtrlWnd, nIdx);
				nWidth = nColumnWidth > nHeaderWidth ? nColumnWidth : nHeaderWidth;
				ListView_SetColumnWidth(hListCtrlWnd, nIdx, nWidth);
			}
			InvalidateRect(hListCtrlWnd, &rcListCtrlWnd, TRUE);
		}
	}
	__inline static void ListCtrlDeleteAllColumns(HWND hListViewWnd)
	{
		while (ListView_DeleteColumn(hListViewWnd, ListView_GetHeader(hListViewWnd)));
	}
	__inline static void ListCtrlDeleteAllRows(HWND hListViewWnd)
	{
		ListView_DeleteAllItems(hListViewWnd);
	}
	__inline static void ListCtrlInsertHeaderData(TSTRINGVECTOR & tv, HWND hListViewWnd, UINT nWidth = 100)
	{
		LV_COLUMN lvc = { 0 };

		lvc.iSubItem = 0;
		lvc.cx = nWidth;
		lvc.mask = LVCF_TEXT | LVCF_WIDTH;
		for (auto it:tv)
		{
			lvc.pszText = (LPTSTR)it.c_str();	
			ListView_InsertColumn(hListViewWnd, lvc.iSubItem++, &lvc);
		}
	}
	__inline static UINT ListCtrlInsertCellData(TSTRING ts, HWND hListViewWnd, UINT nColumnIndex = 0, UINT nRowIndex = 0)
	{
		LV_ITEM lvi = { 0 };
		//if ((lvi.iSubItem < Header_GetItemCount(ListView_GetHeader(hListViewWnd))))
		//{
		lvi.mask = LVIF_TEXT;
		//lvi.mask = LVIF_PARAM;
		lvi.iItem = nRowIndex;
		lvi.iSubItem = nColumnIndex;
		//lvi.lParam = lvi.iSubItem;
		lvi.pszText = (LPTSTR)ts.c_str();
		if (lvi.iSubItem != (0))
		{
			ListView_SetItem(hListViewWnd, &lvi);
		}
		else
		{
			ListView_InsertItem(hListViewWnd, &lvi);
		}
		//}
		return (lvi.iSubItem + 1);
	}
	__inline static UINT ListCtrlInsertItemData(TSTRINGVECTOR & tv, HWND hListViewWnd, UINT nColumnIndex = 0, UINT nRowIndex = 0)
	{
		for (auto it : tv)
		{			
			nColumnIndex = ListCtrlInsertCellData(it, hListViewWnd, nColumnIndex, nRowIndex);
		}
		return (nRowIndex + 1);
	}
	__inline static void ListCtrlInsertItemsData(TSTRINGVECTORVECTOR & tvv, HWND hListViewWnd, UINT nColumnIndex = 0, UINT nRowIndex = 0)
	{
		for (auto it : tvv)
		{
			nRowIndex = ListCtrlInsertItemData(it, hListViewWnd, nColumnIndex, nRowIndex);
		}
	}
	__inline static void ListCtrlInsertData(TSTRINGVECTORMAP * pTVMAP, HWND hListViewWnd, HIMAGELIST hImageList = NULL, LPCTSTR lpListCtrlText = _T(""), LPCTSTR lpHeaderText = _T("|3|3|3|3|3|3|3|3|3|3"))
	{
		SIZE_T stIndex = 0;
		SIZE_T stCount = 0;
		SIZE_T stRowIdx = 0;
		SIZE_T stColIdx = 0;
		LV_ITEM lvi = { 0 };
		LV_COLUMN lvc = { 0 };
		TSTRINGVECTORMAP::iterator itEnd;
		TSTRINGVECTORMAP::iterator itIdx;
		
		if (lpListCtrlText)
		{
			SetWindowText(hListViewWnd, lpListCtrlText);
		}

		if (lpHeaderText)
		{
			SetWindowText(ListView_GetHeader(hListViewWnd), lpHeaderText);
		}

		stColIdx = 0;
		itEnd = pTVMAP->end();
		itIdx = pTVMAP->begin();
		lvc.mask = LVCF_TEXT | LVCF_WIDTH;
		for (; itIdx != itEnd; itIdx++, stColIdx++)
		{
			if (!itIdx->first.compare(_T("图标文件")) || !itIdx->first.compare(_T(IMAGEICONINDEX_NAME)))
			{
				stColIdx--;
				continue;
			}

			lvc.iSubItem = stColIdx;
			lvc.pszText = (LPTSTR)itIdx->first.c_str();
			lvc.cx = 120;
			ListView_InsertColumn(hListViewWnd, stColIdx, &lvc);

			stRowIdx = 0;
			lvi.iSubItem = stColIdx;
			lvi.mask = LVIF_TEXT;
			stCount = itIdx->second.size();
			for (stIndex = 0; stIndex < stCount; stIndex++, stRowIdx++)
			{					
				if (hImageList && pTVMAP->find(_T(IMAGEICONINDEX_NAME)) != pTVMAP->end())
				{
					lvi.mask |= LVIF_IMAGE;
					lvi.iImage = _ttol(pTVMAP->at(_T(IMAGEICONINDEX_NAME)).at(stIndex).c_str());
				}
				lvi.iItem = stRowIdx;
				lvi.pszText = (LPTSTR)itIdx->second.at(stIndex).c_str();
				if (!lvi.iSubItem)
				{
					ListView_InsertItem(hListViewWnd, &lvi);
				}
				else
				{
					ListView_SetItem(hListViewWnd, &lvi);
				}
			}
			lvi.mask = LVIF_PARAM;
			lvi.lParam = stRowIdx;
			ListView_SetItem(hListViewWnd, &lvi);
		}
	}

	typedef struct COLORVALUE
	{
		COLORREF clrText;//字体颜色
		COLORREF clrTextBk;//字体背景颜色
	}ColorValue;
	static const ColorValue GLOBAL_COLORVALUEARRAY[] = {
#define SINGLE_LINE 0
	{ RGB(0, 0, 0), RGB(255, 255, 0) },
#define DOUBLE_LINE 1
	{ RGB(0, 0, 0), RGB(0, 255, 0) },
	};
	//排序结构
	typedef struct SORTDATA
	{
		HWND hwnd;
		int column;
		int sortorder;
	}SortData;

	SortData Global_SortData = { 0 };

	typedef LRESULT(CALLBACK * HEADERWNDPROC)(HWND, UINT, WPARAM, LPARAM);
	typedef struct HEADERREDRAW
	{
		float gradient; // 画立体背景，渐变系数
		float headerheight; //表头高度倍数
		int fontheight; //字体高度
		int fontwidth;//字体宽度
		int bkmode;//背景模式
		COLORREF clrbk;//背景颜色
		COLORREF clrtext;//字体颜色
	}HeaderRedraw;
	HeaderRedraw Global_DlgHeaderRedraw = {
		1.5,
		1,
		12,
		0,
		TRANSPARENT,
		RGB(255, 0, 0),
		RGB(0, 255, 0)
	};
	_TCHAR Global_AlignFormat[] = _T("1111");//对齐格式
	_TCHAR *Global_ColumnContext[] = { _T("名称"), _T("句柄"), _T("状态"), _T("类名") };

	// Message handler for header ctrl.
	__inline static INT_PTR CALLBACK HeaderDlgProc(HWND hHeaderWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		UNREFERENCED_PARAMETER(lParam);
		PAINTSTRUCT ps;
		HDC hdc;
		switch (message)
		{
		case WM_PAINT:
		{
			hdc = BeginPaint(hHeaderWnd, &ps);
			// TODO: Add any drawing code here...
			int nItem;
			nItem = Header_GetItemCount(hHeaderWnd);//得到有几个单元
			float m_R = (float)((Global_DlgHeaderRedraw.clrbk & 0x000000ff) >> 0),
				m_G = (float)((Global_DlgHeaderRedraw.clrbk & 0x0000ff00) >> 8),
				m_B = (float)((Global_DlgHeaderRedraw.clrbk & 0x00ff0000) >> 16);
			for (int i = 0; i<nItem; i++)
			{
				RECT tRect;
				Header_GetItemRect(hHeaderWnd, i, &tRect);//得到Item的尺寸
				float R = m_R, G = m_G, B = m_B;
				RECT nRect;//拷贝尺寸到新的容器中
				memcpy(&nRect, &tRect, sizeof(RECT));
				nRect.left++;//留出分割线的地方
				//绘制立体背景
				for (int j = tRect.top; j <= tRect.bottom; j++)
				{
					nRect.bottom = nRect.top + 1;
					HBRUSH hBrush;
					hBrush = CreateSolidBrush(RGB(R, G, B));//创建画刷
					FillRect(hdc, &nRect, hBrush); //填充背景
					DeleteObject(hBrush); //释放画刷
					R -= Global_DlgHeaderRedraw.gradient;
					G -= Global_DlgHeaderRedraw.gradient;
					B -= Global_DlgHeaderRedraw.gradient;
					if (R<0)R = 0;
					if (G<0)G = 0;
					if (B<0)B = 0;
					nRect.top = nRect.bottom;
				}
				SetBkMode(hdc, Global_DlgHeaderRedraw.bkmode);
				HFONT hFont, hOldFont;
				SetTextColor(hdc, Global_DlgHeaderRedraw.clrtext);
				hFont = CreateFont(
					Global_DlgHeaderRedraw.fontheight,
					Global_DlgHeaderRedraw.fontwidth,
					0,
					0,
					0,
					FALSE,
					FALSE,
					0,
					0,
					0,
					0,
					0,
					0,
					_T("MS Shell Dlg"));//创建字体
				hOldFont = (HFONT)SelectObject(hdc, hFont);

				UINT nFormat = 1;
				if (Global_AlignFormat[i] == _T('0'))
				{
					nFormat = DT_LEFT;
					tRect.left += 3;
				}
				else if (Global_AlignFormat[i] == _T('1'))
				{
					nFormat = DT_CENTER;
				}
				else if (Global_AlignFormat[i] == _T('2'))
				{
					nFormat = DT_RIGHT;
					tRect.right -= 3;
				}
				TEXTMETRIC metric;
				GetTextMetrics(hdc, &metric);
				int offset = 0;
				offset = tRect.bottom - tRect.top - metric.tmHeight;
				OffsetRect(&tRect, 0, offset / 2);
				HD_ITEM hdi = { 0 };
				_TCHAR tHeaderText[MAXBYTE] = { 0 };
				hdi.mask = HDI_TEXT;
				hdi.cchTextMax = MAXBYTE;
				hdi.pszText = tHeaderText;
				Header_GetItem(hHeaderWnd, i, &hdi);
				DrawText(hdc, tHeaderText, lstrlen(tHeaderText), &tRect, nFormat);
				SelectObject(hdc, hOldFont);
				DeleteObject(hFont); //释放字体
			}
			//画头部剩余部分
			RECT rtRect;
			RECT clientRect;
			Header_GetItemRect(hHeaderWnd, nItem - 1, &rtRect);
			GetClientRect(hHeaderWnd, &clientRect);
			rtRect.left = rtRect.right + 1;
			rtRect.right = clientRect.right;
			float R = m_R, G = m_G, B = m_B;
			RECT nRect;
			memcpy(&nRect, &rtRect, sizeof(RECT));
			//绘制立体背景
			for (int j = rtRect.top; j <= rtRect.bottom; j++)
			{
				nRect.bottom = nRect.top + 1;
				HBRUSH hBrush;
				hBrush = CreateSolidBrush(RGB(R, G, B));//创建画刷
				FillRect(hdc, &nRect, hBrush); //填充背景
				DeleteObject(hBrush); //释放画刷
				R -= Global_DlgHeaderRedraw.gradient;
				G -= Global_DlgHeaderRedraw.gradient;
				B -= Global_DlgHeaderRedraw.gradient;
				if (R<0)R = 0;
				if (G<0)G = 0;
				if (B<0)B = 0;
				nRect.top = nRect.bottom;
			}
			EndPaint(hHeaderWnd, &ps);
		}
		break;
		default:
			return ::CallWindowProc((WNDPROC)::GetWindowLongPtr(hHeaderWnd, GWLP_USERDATA), hHeaderWnd, message, wParam, lParam);
		}
		return (INT_PTR)FALSE;
	}
	//重置窗口大小
	int ResizeWindow(HWND hParentWnd, DWORD dwID)
	{
		int result = 0;
		HWND hDataListCtrl = GetDlgItem(hParentWnd, dwID);
		if (hDataListCtrl)
		{
			RECT rc = { 0 };
			int column = 0;
			HWND hHeaderWnd = 0;
			GetClientRect(hParentWnd, &rc);
			SetWindowPos(hDataListCtrl, HWND_TOP, rc.left, rc.top, rc.right, rc.bottom, SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW);
			InvalidateRect(hParentWnd, &rc, FALSE);
			hHeaderWnd = ListView_GetHeader(hDataListCtrl);
			if (hHeaderWnd)
			{
				column = Header_GetItemCount(hHeaderWnd);
				for (int i = 0; i < column; i++)
				{
					ListView_SetColumnWidth(hDataListCtrl, i, rc.right / column);
				}
			}
		}
		return result;
	}
	//对话框初始化ListCtrl控件
	INT_PTR OnInitDataList(HWND hDlg, DWORD dwID, LPTSTR * pHeaderTitles, DWORD dwHeaderNumber)
	{
		INT_PTR iresult = 0;
		HWND hDataListCtrl = GetDlgItem(hDlg, dwID);
		if (hDataListCtrl)
		{
			int colnum = 0;
			LV_COLUMN lvc = { 0 };
			RECT rc = { 0 };
			LONG lstyle = 0;
			DWORD dwstyle = 0;

			lstyle = GetWindowLong(hDataListCtrl, GWL_STYLE);//获取当前窗口style
			lstyle &= ~LVS_TYPEMASK; //清除显示方式位
			lstyle |= LVS_REPORT; //设置style
			//lstyle |= LVS_SORTASCENDING; //设置排序
			lstyle |= LVS_SINGLESEL; //设置单选
			lstyle |= LVS_SHOWSELALWAYS; //设置一直选择
			lstyle |= LVS_AUTOARRANGE; //设置自动浮动
			SetWindowLong(hDataListCtrl, GWL_STYLE, lstyle);//设置style

			dwstyle = ListView_GetExtendedListViewStyle(hDataListCtrl);
			dwstyle |= LVS_EX_FULLROWSELECT;//选中某行使整行高亮（只适用与report风格的listctrl）
			dwstyle |= LVS_EX_GRIDLINES;//网格线（只适用与report风格的listctrl）
			dwstyle |= LVS_EX_CHECKBOXES;//item前生成checkbox控件
			dwstyle |= LVS_EX_TRACKSELECT;//
			dwstyle |= LVS_EX_HEADERDRAGDROP;//列头可拖拽
			dwstyle |= LVS_EX_ONECLICKACTIVATE;//单击激活
			dwstyle |= LVS_EX_FLATSB;//平滑进度条
			dwstyle |= LVS_EX_REGIONAL;//
			dwstyle |= LVS_EX_INFOTIP;//
			dwstyle |= LVS_EX_UNDERLINEHOT;//下划线
			dwstyle |= LVS_EX_LABELTIP;//
			ListView_SetExtendedListViewStyle(hDataListCtrl, dwstyle); //设置扩展风格

			GetClientRect(hDataListCtrl, &rc);

			lvc.mask = LVCF_TEXT | LVCF_WIDTH;
			for (DWORD dwColnum = 0; dwColnum < dwHeaderNumber; dwColnum++)
			{
				lvc.pszText = pHeaderTitles[dwColnum];
				ListView_InsertColumn(hDataListCtrl, dwColnum++, &lvc);
			}

			for (int i = 0; i < ListView_GetItemCount(hDataListCtrl); i++)
			{
				ListView_RedrawItems(hDataListCtrl, i, i);
			}
			HWND hHeaderWnd = ListView_GetHeader(hDataListCtrl);
			if (hHeaderWnd)
			{
				::SetWindowLongPtr(hHeaderWnd, GWLP_USERDATA, (LONG_PTR)(WNDPROC)::GetWindowLongPtr(hHeaderWnd, GWLP_WNDPROC));
				::SetWindowLongPtr(hHeaderWnd, GWLP_WNDPROC, (LONG_PTR)(HeaderDlgProc));
			}

			ResizeWindow(hDlg, dwID);
		}
		return iresult;
	}
	__inline static LONG_PTR ListCtrlSetSortDataInfo(HWND hListCtrlWnd, SORTDATAINFO * pSDI)
	{
		return SetWindowUserData(ListView_GetHeader(hListCtrlWnd), (LONG_PTR)pSDI);
	}
	__inline static SORTDATAINFO * ListCtrlGetLSortDataInfo(HWND hListCtrlWnd)
	{
		return (SORTDATAINFO *)GetWindowUserData(ListView_GetHeader(hListCtrlWnd));
	}
	__inline static UINT ListCtrlGetSelectedRowCount(HWND hListViewWnd)
	{
		return ListView_GetSelectedCount(hListViewWnd);
	}
	__inline static void ListCtrlGetSelectedRow(std::map<UINT, UINT> * pssmap, HWND hListViewWnd)
	{
		UINT nSelectIndex = 0;
		while ((nSelectIndex = ListView_GetNextItem(hListViewWnd, (-1), LVNI_SELECTED)) != (-1))
		{
			pssmap->insert(std::map<UINT, UINT>::value_type(nSelectIndex, nSelectIndex));
		}		
	}
	
	__inline static LRESULT ListCtrlOnNotify(HWND hListCtrlWnd, LPNMHDR lpNMHDR)
	{
		int nItemPos = 0;
		switch (lpNMHDR->code)
		{
		case NM_RCLICK:
		{
			if ((nItemPos = ListView_GetNextItem(hListCtrlWnd, -1, LVNI_SELECTED)) != -1)
			{
				HMENU hMenu = NULL;
				POINT point = { 0 };
				
				GetCursorPos(&point);
				
				//动态创建弹出式菜单对象
				hMenu = CreatePopupMenu();
				if (hMenu)
				{
					AppendMenu(hMenu, MF_STRING, (0), _T("选择"));					
					TrackPopupMenuEx(hMenu, TPM_RIGHTBUTTON | TPM_VERPOSANIMATION | TPM_LEFTALIGN | TPM_VERTICAL, point.x, point.y, hListCtrlWnd, NULL);
					DestroyMenu(hMenu);
					hMenu = NULL;
				}
			}
		}
		break;
		case NM_DBLCLK:
		{
			if ((nItemPos = ListView_GetNextItem(hListCtrlWnd, -1, LVNI_SELECTED)) != -1)
			{
				
			}
		}
		break;
		//case LVN_COLUMNCLICK:
		case HDN_ITEMCLICK:
		{
			_TCHAR tzText[MAXWORD] = { 0 };
			_TCHAR tzValue[MAXWORD] = { 0 };
			tstring::size_type stIndexPos = 0;
			tstring::size_type stStartPos = 0;
			tstring::size_type stFinalPos = 0;

			LPNMHEADER lpNMHEADER = (LPNMHEADER)lpNMHDR;

			SORTDATAINFO * pSDI = (SORTDATAINFO *)GetWindowUserData(ListView_GetHeader(hListCtrlWnd));
			if (pSDI)
			{
				pSDI->hListCtrlWnd = hListCtrlWnd;
				if (pSDI->nColumnItem != lpNMHEADER->iItem)
				{
					pSDI->nColumnItem = lpNMHEADER->iItem;
					pSDI->bSortFlag = true;
				}
				else
				{
					pSDI->bSortFlag = (!pSDI->bSortFlag);
				}

				GetWindowText(ListView_GetHeader(pSDI->hListCtrlWnd), tzText, sizeof(tzText));
				for (stIndexPos = 0;
					stIndexPos < lpNMHEADER->iItem
					&& (stStartPos = tstring(tzText).find(_T("|"), stStartPos + 1));
				stIndexPos++){
					;
				}
				stFinalPos = tstring(tzText).find(_T("|"), stStartPos + 1);
				lstrcpyn(tzValue, (LPCTSTR)tzText + stStartPos + 1, stFinalPos - stStartPos);
				pSDI->cdType = (COLUMN_DATATYPE)_ttol(tzValue);

				ListView_SortItemsEx(pSDI->hListCtrlWnd, &ListCtrlCompareProcess, pSDI);
			}
		}
		break;
		case LVN_ITEMCHANGED:
		{
			if ((nItemPos = ListView_GetNextItem(hListCtrlWnd, -1, LVNI_SELECTED)) != -1)
			{
				//
			}
		}
		break;
		default:
			break;
		}

		return 0;
	}
	//对话框通知消息处理函数
	__inline static INT_PTR OnListCtrlNotify(HWND hDlg, DWORD dwID, LPARAM lParam)
	{
		UNREFERENCED_PARAMETER(lParam);

		INT_PTR iresult = 0;

		if (((NMHDR *)lParam)->idFrom == dwID)
		{
			switch (((NMHDR *)lParam)->code)
			{
			case NM_CLICK:
			{
				LVHITTESTINFO lvinfo = { 0 };
				DWORD dwposition = GetMessagePos();
				HWND hDataListCtrl = GetDlgItem(hDlg, dwID);
				if (hDataListCtrl)
				{
					lvinfo.pt.x = LOWORD(dwposition);
					lvinfo.pt.y = HIWORD(dwposition);
					ScreenToClient(hDataListCtrl, &lvinfo.pt);
					ListView_HitTest(hDataListCtrl, &lvinfo);

					//判断是否点在CheckBox上
					if (lvinfo.flags == LVHT_ONITEMSTATEICON)
					{
						//::MessageBox(hDlg, _T("点击ListCtrl中CheckBox"), _T("提示"), MB_OK);
						if (ListView_GetCheckState(hDataListCtrl, lvinfo.iItem))
						{
							ListView_SetItemState(hDataListCtrl, lvinfo.iItem, 0, LVIS_SELECTED | LVIS_FOCUSED);
						}
						else
						{
							ListView_SetItemState(hDataListCtrl, lvinfo.iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
						}
					}
					else if (lvinfo.flags == LVHT_ONITEMLABEL)
					{

					}
					else if (lvinfo.flags == (LVHT_ONITEMLABEL | LVHT_BELOW))
					{

					}
					else
					{
						/*if (ListView_GetCheckState(hDataListCtrl, lvinfo.iItem))
						{
							ListView_SetCheckState(hDataListCtrl, lvinfo.iItem, FALSE);
							ListView_SetItemState(hDataListCtrl, lvinfo.iItem, 0, LVIS_SELECTED | LVIS_FOCUSED);
						}
						else
						{
							ListView_SetCheckState(hDataListCtrl, lvinfo.iItem, TRUE);
							ListView_SetItemState(hDataListCtrl, lvinfo.iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
						}*/
					}
				}
			}
			break;
			case NM_CUSTOMDRAW:
			{
				HWND hDataListCtrl = GetDlgItem(hDlg, dwID);
				if (hDataListCtrl)
				{
					NMLVCUSTOMDRAW * pnmlcd = (NMLVCUSTOMDRAW *)lParam;
					iresult = CDRF_DODEFAULT;
					switch (pnmlcd->nmcd.dwDrawStage)//判断步骤
					{
					case CDDS_PREPAINT:
					{
						iresult = CDRF_NOTIFYITEMDRAW;
					}
					break;
					case CDDS_ITEMPREPAINT://如果为画ITEM之前就要进行颜色的改变
					{
						/*if(pnmlcd->nmcd.dwItemSpec % 2)
						{
						pnmlcd->clrText = GLOBAL_COLORVALUEARRAY[SINGLE_LINE].clrText;
						pnmlcd->clrTextBk = GLOBAL_COLORVALUEARRAY[SINGLE_LINE].clrTextBk;
						}
						else
						{
						pnmlcd->clrText = GLOBAL_COLORVALUEARRAY[DOUBLE_LINE].clrText;
						pnmlcd->clrTextBk = GLOBAL_COLORVALUEARRAY[DOUBLE_LINE].clrTextBk;
						}
						iresult = CDRF_DODEFAULT;//使用此代码，每行颜色可一致
						*/
						iresult = CDRF_NOTIFYSUBITEMDRAW;//使用此代码，每个单元格颜色可一致
					}
					break;
					case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
					{
						//printf("%d,%d\r\n", pnmlcd->nmcd.dwItemSpec, pnmlcd->iSubItem);
						if (pnmlcd->nmcd.dwItemSpec % 2 && pnmlcd->iSubItem % 2)
						{
							pnmlcd->clrText = GLOBAL_COLORVALUEARRAY[DOUBLE_LINE].clrText;
							pnmlcd->clrTextBk = GLOBAL_COLORVALUEARRAY[DOUBLE_LINE].clrTextBk;
						}
						else if (pnmlcd->nmcd.dwItemSpec % 2 && !(pnmlcd->iSubItem % 2))
						{
							pnmlcd->clrText = GLOBAL_COLORVALUEARRAY[SINGLE_LINE].clrText;
							pnmlcd->clrTextBk = GLOBAL_COLORVALUEARRAY[SINGLE_LINE].clrTextBk;
						}
						else if (!(pnmlcd->nmcd.dwItemSpec % 2) && pnmlcd->iSubItem % 2)
						{
							pnmlcd->clrText = GLOBAL_COLORVALUEARRAY[SINGLE_LINE].clrText;
							pnmlcd->clrTextBk = GLOBAL_COLORVALUEARRAY[SINGLE_LINE].clrTextBk;
						}
						else
						{
							pnmlcd->clrText = GLOBAL_COLORVALUEARRAY[DOUBLE_LINE].clrText;
							pnmlcd->clrTextBk = GLOBAL_COLORVALUEARRAY[DOUBLE_LINE].clrTextBk;
						}
						/*if(pnmlcd->iSubItem % 2)
						{
						pnmlcd->clrTextBk =  GLOBAL_COLORVALUEARRAY[SINGLE_LINE].clrTextBk;
						}
						else
						{
						pnmlcd->clrTextBk =  GLOBAL_COLORVALUEARRAY[DOUBLE_LINE].clrTextBk;
						}*/
						iresult = CDRF_DODEFAULT;
					}
					break;
					default:
					{
						iresult = CDRF_DODEFAULT;
					}
					break;
					}
				}
			}
			//SetWindowLong(hDlg, DWL_MSGRESULT, iresult);
			SetWindowLongPtr(hDlg, DWLP_MSGRESULT, iresult);
			break;
			default:
			{
				switch (((NMLISTVIEW *)lParam)->hdr.code)
				{
				case LVN_COLUMNCLICK:
				{
					HWND hDataListCtrl = GetDlgItem(hDlg, dwID);
					if (hDataListCtrl)
					{
						NM_LISTVIEW * pnmlv = (NM_LISTVIEW *)lParam;
						//设置排序方式
						if (pnmlv->iSubItem == Global_SortData.column)
						{
							Global_SortData.sortorder = !Global_SortData.sortorder;
						}
						else
						{
							Global_SortData.sortorder = 1;
							Global_SortData.column = pnmlv->iSubItem;
						}
						Global_SortData.hwnd = hDataListCtrl;
						//调用排序函数
						ListView_SortItemsEx(hDataListCtrl, ListCtrlCompareProcess, (LPARAM)&Global_SortData);
					}
				}
				break;
				default:
				{
				}
				break;
				}
			}
			break;
			}
		}

		return iresult;
	}

	__inline static void RegisterDropFilesEvent(HWND hWnd)
	{
#ifndef WM_COPYGLOBALDATA
#define WM_COPYGLOBALDAYA	0x0049
#endif

#ifndef MSGFLT_ADD
#define MSGFLT_ADD 1
#endif

#ifndef MSGFLT_REMOVE
#define MSGFLT_REMOVE 2
#endif
		SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_ACCEPTFILES);
		typedef BOOL(WINAPI *LPFN_ChangeWindowMessageFilter)(__in UINT message, __in DWORD dwFlag);
		LPFN_ChangeWindowMessageFilter pfnChangeWindowMessageFilter = (LPFN_ChangeWindowMessageFilter)GetProcAddress(GetModuleHandle(_T("USER32.DLL")), "ChangeWindowMessageFilter");
		if (pfnChangeWindowMessageFilter)
		{
			pfnChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
			pfnChangeWindowMessageFilter(WM_COPYDATA, MSGFLT_ADD);
			pfnChangeWindowMessageFilter(WM_COPYGLOBALDAYA, MSGFLT_ADD);// 0x0049 == WM_COPYGLOBALDATA
		}
	}
	__inline static size_t GetDropFiles(std::map<TSTRING, TSTRING> * pttmap, HDROP hDropInfo)
	{
		UINT nIndex = 0;
		UINT nNumOfFiles = 0;
		_TCHAR tszFilePathName[MAX_PATH + 1] = { 0 };

		//得到文件个数
		nNumOfFiles = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);

		for (nIndex = 0; nIndex < nNumOfFiles; nIndex++)
		{
			//得到文件名
			DragQueryFile(hDropInfo, nIndex, (LPTSTR)tszFilePathName, _MAX_PATH);
			pttmap->insert(std::map<TSTRING, TSTRING>::value_type(tszFilePathName, tszFilePathName));
		}

		DragFinish(hDropInfo);

		return nNumOfFiles;
	}
	__inline static size_t GetDropFiles(std::vector<TSTRING> * pttmap, HDROP hDropInfo)
	{
		UINT nIndex = 0;
		UINT nNumOfFiles = 0;
		_TCHAR tszFilePathName[MAX_PATH + 1] = { 0 };

		//得到文件个数
		nNumOfFiles = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);

		for (nIndex = 0; nIndex < nNumOfFiles; nIndex++)
		{
			//得到文件名
			DragQueryFile(hDropInfo, nIndex, (LPTSTR)tszFilePathName, _MAX_PATH);
			pttmap->push_back(tszFilePathName);
		}

		DragFinish(hDropInfo);

		return nNumOfFiles;
	}

	__inline static int OnRefreshTreelist(HWND hWndTreeList)
	{
		int nResult = 0;

		_TCHAR tzWindowTitle[MAXBYTE] = { 0 };
		_TCHAR tzWindowClass[MAXBYTE] = { 0 };
		_TCHAR tzWindowBuffer[MAXBYTE] = { 0 };

		//Tree控件操作变量
		TVINSERTSTRUCT tvis = { 0 };
		HWND hWndParent = NULL;
		HTREEITEM hTreeItemParent = NULL;
		HWND hWndDesktop = NULL;
		HTREEITEM hTreeItemDesktop = NULL;
		HWND hWndChildren = NULL;
		HTREEITEM hTreeItemChildren = NULL;

		//设置根窗口句柄
		hWndParent = hWndTreeList;

		//根节点数据
		tvis.hParent = NULL;
		tvis.hInsertAfter = NULL;
		tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
		tvis.item.pszText = _T("窗口句柄");
		tvis.item.lParam = 0;//设置项目个数

		//删除全部节点
		TreeView_DeleteAllItems(hWndParent);

		//添加根节点
		hTreeItemParent = TreeView_InsertItem(hWndParent, &tvis);

		//获取桌面窗口句柄
		hWndDesktop = ::GetDesktopWindow();

		//取窗口标题
		::GetWindowText(hWndDesktop, tzWindowTitle, sizeof(tzWindowTitle) / sizeof(_TCHAR));
		_sntprintf(tzWindowTitle, sizeof(tzWindowClass) / sizeof(_TCHAR), _T("桌面"));

		//取窗口类名
		::GetClassName(hWndDesktop, tzWindowClass, sizeof(tzWindowClass) / sizeof(_TCHAR));

		//把信息格式化
		_sntprintf(tzWindowBuffer, sizeof(tzWindowBuffer) / sizeof(_TCHAR), _T("%s | %s | %d(0x%X)"), tzWindowTitle, tzWindowClass, hWndDesktop, hWndDesktop);

		//节点数据
		tvis.hParent = hTreeItemParent;
		tvis.hInsertAfter = TVI_LAST;
		tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
		tvis.item.pszText = tzWindowBuffer;
		tvis.item.lParam = (int)hWndDesktop;
		tvis.item.cChildren = ::GetWindow(hWndDesktop, GW_CHILD) ? 1 : 0;

		//插入节点
		hTreeItemDesktop = TreeView_InsertItem(hWndParent, &tvis);

		//取子级窗口句柄
		hWndChildren = ::GetWindow(hWndDesktop, GW_CHILD);

		while (hWndChildren)
		{
			//窗口是否可视
			if (::IsWindowVisible(hWndChildren))
			{
				memset(tzWindowTitle, 0, sizeof(tzWindowTitle));
				memset(tzWindowClass, 0, sizeof(tzWindowClass));
				memset(tzWindowBuffer, 0, sizeof(tzWindowBuffer));

				//取窗口标题
				::GetWindowText(hWndChildren, tzWindowTitle, sizeof(tzWindowTitle) / sizeof(_TCHAR));

				//取窗口类名
				::GetClassName(hWndChildren, tzWindowClass, sizeof(tzWindowClass) / sizeof(_TCHAR));

				//把信息格式化
				_sntprintf(tzWindowBuffer, sizeof(tzWindowBuffer) / sizeof(_TCHAR), _T("%s | %s | %d(0x%X)"), tzWindowTitle, tzWindowClass, hWndChildren, hWndChildren);

				//节点数据
				tvis.hParent = hTreeItemDesktop;
				tvis.hInsertAfter = TVI_LAST;
				tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
				tvis.item.pszText = tzWindowBuffer;
				tvis.item.lParam = (int)hWndChildren;
				tvis.item.cChildren = ::GetWindow(hWndChildren, GW_CHILD) ? 1 : 0;

				//插入节点
				hTreeItemChildren = TreeView_InsertItem(hWndParent, &tvis);
			}

			//取下一兄弟窗口
			hWndChildren = ::GetWindow(hWndChildren, GW_HWNDNEXT);
		}

		//展开根节点
		TreeView_Expand(hWndTreeList, hTreeItemParent, TVE_EXPAND);

		//展开桌面节点
		TreeView_Expand(hWndTreeList, hTreeItemDesktop, TVE_EXPAND);

		return nResult;
	}

	//树响应函数，选择项目处理
	__inline static void OnSelectedChangedTreeList(NMHDR * pNMHDR, LRESULT * pResult)
	{
		NMTREEVIEW * pNMTV = NULL;

		_TCHAR tzWindowTitle[MAXBYTE] = { 0 };
		_TCHAR tzWindowClass[MAXBYTE] = { 0 };
		_TCHAR tzWindowBuffer[MAXBYTE] = { 0 };

		//Tree控件操作变量
		TVINSERTSTRUCT tvis = { 0 };
		HWND hWndTreeList = NULL;
		HWND hWndParent = NULL;
		HTREEITEM hTreeItemParent = NULL;
		HWND hWndChildren = NULL;
		HTREEITEM hTreeItemChildren = NULL;

		pNMTV = (NMTREEVIEW *)pNMHDR;
		hWndTreeList = pNMTV->hdr.hwndFrom;
		//::GetClassName(hWndTreeList, tzWindowClass, sizeof(tzWindowClass) / sizeof(_TCHAR));
		//_tprintf(_T("%s\n"), tzWindowClass);

		//hTreeItemParent = pNMTV->itemNew.hItem;
		hTreeItemParent = TreeView_GetSelection(hWndTreeList);

		tvis.item.hItem = hTreeItemParent;
		tvis.item.mask = TVIF_PARAM;
		TreeView_GetItem(hWndTreeList, &tvis.item);
		hWndParent = (HWND)tvis.item.lParam;

		//ItemHasChildren     是否有子节点
		//GetChildItem        获取第一个子结点
		//GetNextSiblingItem  获取下一个兄弟结点结点

		//判断是否有子节点，已有子节点则不处理
		if (TreeView_GetChild(hWndTreeList, hTreeItemParent))
		{
			return;
		}

		//遍历子窗口的所有控件
		//取子级窗口句柄
		hWndChildren = ::GetWindow(hWndParent, GW_CHILD);

		while (hWndChildren)
		{
			//窗口是否可视
			//if (::IsWindowVisible(hWndChildren))
			{
				memset(tzWindowTitle, 0, sizeof(tzWindowTitle));
				memset(tzWindowClass, 0, sizeof(tzWindowClass));
				memset(tzWindowBuffer, 0, sizeof(tzWindowBuffer));

				//取窗口标题
				::GetWindowText(hWndChildren, tzWindowTitle, sizeof(tzWindowTitle) / sizeof(_TCHAR));

				//取窗口类名
				::GetClassName(hWndChildren, tzWindowClass, sizeof(tzWindowClass) / sizeof(_TCHAR));

				//把信息格式化
				_sntprintf(tzWindowBuffer, sizeof(tzWindowBuffer) / sizeof(_TCHAR), _T("%s | %s | %d(0x%X)"), tzWindowTitle, tzWindowClass, hWndChildren, hWndChildren);

				//_tprintf(_T("%s\n"), tzWindowBuffer);

				//节点数据
				tvis.hParent = hTreeItemParent;
				tvis.hInsertAfter = TVI_LAST;
				tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
				tvis.item.pszText = tzWindowBuffer;
				tvis.item.lParam = (int)hWndChildren;
				tvis.item.cChildren = ::GetWindow(hWndChildren, GW_CHILD) ? 1 : 0;

				//插入节点
				hTreeItemChildren = TreeView_InsertItem(hWndTreeList, &tvis);
			}

			//取下一兄弟窗口
			hWndChildren = ::GetWindow(hWndChildren, GW_HWNDNEXT);
		}
	}
	__inline static INT_PTR OnTreeNotify(HWND hWnd, DWORD dwID, LPARAM lParam)
	{
		INT_PTR nResult = (0);
		NMHDR * pNMHDR = (NMHDR *)lParam;
	
		switch (pNMHDR->code)
		{
		case NM_DBLCLK://双击操作
		{
			if (pNMHDR->idFrom == dwID)
			{
				OnSelectedChangedTreeList((NMHDR *)lParam, 0);
			}
		}
		break;
		}

		return nResult;
	}
	//nAnimateType = 1 2 3 4 其它动画类型
	__inline static BOOL DisplayAnimateWindows(HWND hWnd, unsigned long ulTime, bool bShow = true, bool bSlide = true, int nAnimateType = 0)
	{
		unsigned long ulFlags = (bShow ? AW_ACTIVATE : AW_HIDE) | (bSlide ? AW_SLIDE : AW_BLEND);

		switch (nAnimateType)
		{
		case 1:
		{
			ulFlags |= AW_HOR_POSITIVE;
		}
		break;
		case 2:
		{
			ulFlags |= AW_VER_POSITIVE;
		}
		break;
		case 3:
		{
			ulFlags |= AW_HOR_NEGATIVE;
		}
		break;
		case 4:
		{
			ulFlags |= AW_VER_NEGATIVE;
		}
		break;
		default:
		{
			ulFlags |= AW_CENTER;
		}
		break;
		}

		return ::AnimateWindow(hWnd, ulTime, ulFlags);
	}
	//显示在屏幕中央
	__inline static void CenterWindowInScreen(HWND hWnd)
	{
		RECT rcWindow = { 0 };
		RECT rcScreen = { 0 };
		SIZE szAppWnd = { 300, 160 };
		POINT ptAppWnd = { 0, 0 };

		// Get workarea rect.
		BOOL fResult = SystemParametersInfo(SPI_GETWORKAREA,   // Get workarea information
			0,              // Not used
			&rcScreen,    // Screen rect information
			0);             // Not used

		GetWindowRect(hWnd, &rcWindow);
		szAppWnd.cx = rcWindow.right - rcWindow.left;
		szAppWnd.cy = rcWindow.bottom - rcWindow.top;

		//居中显示
		ptAppWnd.x = (rcScreen.right - rcScreen.left - szAppWnd.cx) / 2;
		ptAppWnd.y = (rcScreen.bottom - rcScreen.top - szAppWnd.cy) / 2;
		MoveWindow(hWnd, ptAppWnd.x, ptAppWnd.y, szAppWnd.cx, szAppWnd.cy, TRUE);
	}

	//显示在父窗口中央
	__inline static void CenterWindowInParent(HWND hWnd, HWND hParentWnd)
	{
		RECT rcWindow = { 0 };
		RECT rcParent = { 0 };
		SIZE szAppWnd = { 300, 160 };
		POINT ptAppWnd = { 0, 0 };

		GetWindowRect(hParentWnd, &rcParent);
		GetWindowRect(hWnd, &rcWindow);
		szAppWnd.cx = rcWindow.right - rcWindow.left;
		szAppWnd.cy = rcWindow.bottom - rcWindow.top;

		//居中显示
		ptAppWnd.x = (rcParent.right - rcParent.left - szAppWnd.cx) / 2;
		ptAppWnd.y = (rcParent.bottom - rcParent.top - szAppWnd.cy) / 2;
		MoveWindow(hWnd, ptAppWnd.x, ptAppWnd.y, szAppWnd.cx, szAppWnd.cy, TRUE);
	}
	
	__inline static void StaticSetIconImage(HWND hWndStatic, HICON hIcon)
	{
		HICON hLastIcon = NULL;
		hLastIcon = (HICON)SendMessage(hWndStatic, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
		if (hLastIcon)
		{
			DeleteObject(hLastIcon);
			hLastIcon = NULL;
		}
	}
	__inline static void StaticSetBitmapImage(HWND hWndStatic, HBITMAP hBitmap)
	{
		HBITMAP hLastBitmap = NULL;
		hLastBitmap = (HBITMAP)SendMessage(hWndStatic, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
		if (hLastBitmap)
		{
			DeleteObject(hLastBitmap);
			hLastBitmap = NULL;
		}
	}

	__inline static HICON HIconFromFile(LPCTSTR lpFileName, SIZE size = { 0, 0 })
	{
		return (HICON)LoadImage(GetModuleHandle(NULL), 
			lpFileName,
			IMAGE_ICON,
			size.cx,//GetSystemMetrics(SM_CXSMICON),
			size.cy,//GetSystemMetrics(SM_CYSMICON),
			LR_LOADFROMFILE);
	}
	__inline static HBITMAP HBitmapFromFile(LPCTSTR lpFileName, SIZE size = { 0, 0 })
	{
		return (HBITMAP)LoadImage(GetModuleHandle(NULL),
			lpFileName,
			IMAGE_BITMAP,
			size.cx,//GetSystemMetrics(SM_CXSMICON),
			size.cy,//GetSystemMetrics(SM_CYSMICON),
			LR_LOADFROMFILE);
	}
	__inline static HCURSOR HCursorFromFile(LPCTSTR lpFileName, SIZE size = { 0, 0 })
	{
		return (HCURSOR)LoadImage(GetModuleHandle(NULL),
			lpFileName,
			IMAGE_CURSOR,
			size.cx,//GetSystemMetrics(SM_CXSMICON),
			size.cy,//GetSystemMetrics(SM_CYSMICON),
			LR_LOADFROMFILE);
	}
	__inline static
		void NotifyUpdate(HWND hWnd, RECT *pRect = NULL, BOOL bErase = TRUE)
	{
		RECT rcWnd = { 0 };

		::GetClientRect(hWnd, &rcWnd);
		if (pRect)
		{			
			if (memcmp(pRect, &rcWnd, sizeof(RECT)))
			{
				::InvalidateRect(hWnd, &rcWnd, bErase);
				memcpy(pRect, &rcWnd, sizeof(RECT));
			}
		}
		else
		{
			::InvalidateRect(hWnd, &rcWnd, bErase);
		}
	}

	__inline static
		bool SaveBitmapToFile(HDC hDC, HBITMAP hBitmap, LPCTSTR ptFileName)
	{
		//	HDC hDC;
		//设备描述表
		int iBits;
		//当前显示分辨率下每个像素所占字节数
		WORD wBitCount;
		//位图中每个像素所占字节数
		//定义调色板大小， 位图中像素字节大小 ，  位图文件大小 ， 写入文件字节数
		DWORD  dwPaletteSize = 0, dwBmBitsSize, dwDIBSize, dwWritten;
		BITMAP  Bitmap;
		//位图属性结构
		BITMAPFILEHEADER   bmfHdr;
		//位图文件头结构
		BITMAPINFOHEADER   bi;
		//位图信息头结构
		LPBITMAPINFOHEADER lpbi;
		//指向位图信息头结构
		HANDLE  fh, hDib, hPal;
		HPALETTE  hOldPal = NULL;

		//定义文件，分配内存句柄，调色板句柄
		//计算位图文件每个像素所占字节数
		//hDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
		iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
		//DeleteDC(hDC);
		if (iBits <= 1)
		{
			wBitCount = 1;
		}
		else if (iBits <= 4)
		{
			wBitCount = 4;
		}
		else if (iBits <= 8)
		{
			wBitCount = 8;
		}
		else if (iBits <= 24)
		{
			wBitCount = 24;
		}
		else
		{
			wBitCount = 32;
		}

		//计算调色板大小
		if (wBitCount <= 8)
		{
			dwPaletteSize = (1 << wBitCount)*sizeof(RGBQUAD);
		}

		//设置位图信息头结构
		GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);
		bi.biSize = sizeof(BITMAPINFOHEADER);
		bi.biWidth = Bitmap.bmWidth;
		bi.biHeight = Bitmap.bmHeight;
		bi.biPlanes = 1;
		bi.biBitCount = wBitCount;
		bi.biCompression = BI_RGB;
		bi.biSizeImage = 0;
		bi.biXPelsPerMeter = 0;
		bi.biYPelsPerMeter = 0;
		bi.biClrUsed = 0;
		bi.biClrImportant = 0;
		dwBmBitsSize = ((Bitmap.bmWidth*wBitCount + 31) / 32) * 4 * Bitmap.bmHeight;

		//为位图内容分配内存
		hDib = GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
		lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
		*lpbi = bi;
		// 处理调色板
		hPal = GetStockObject(DEFAULT_PALETTE);
		if (hPal)
		{
			hDC = ::GetDC(NULL);
			hOldPal = SelectPalette(hDC, (HPALETTE)hPal, FALSE);
			RealizePalette(hDC);
		}
		// 获取该调色板下新的像素值
		GetDIBits(hDC, hBitmap, 0, (UINT)Bitmap.bmHeight, (LPSTR)lpbi + sizeof(BITMAPINFOHEADER) + dwPaletteSize, (BITMAPINFO *)lpbi, DIB_RGB_COLORS);
		//恢复调色板   
		if (hOldPal)
		{
			SelectPalette(hDC, hOldPal, TRUE);
			RealizePalette(hDC);
			::ReleaseDC(NULL, hDC);
		}

		//创建位图文件    
		fh = CreateFile(ptFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (fh == INVALID_HANDLE_VALUE)
		{
			GlobalUnlock(hDib);
			GlobalFree(hDib);
			return FALSE;
		}
		//设置位图文件头
		bmfHdr.bfType = 0x4D42;  // "BM"
		dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;
		bmfHdr.bfSize = dwDIBSize;
		bmfHdr.bfReserved1 = 0;
		bmfHdr.bfReserved2 = 0;
		bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;

		WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
		WriteFile(fh, (LPSTR)lpbi, sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize, &dwWritten, NULL);

		GlobalUnlock(hDib);
		GlobalFree(hDib);

		CloseHandle(fh);
		return false;
	}
	__inline static
		bool SaveBitmapToFile(HBITMAP hBitmap, LPCTSTR ptFileName)
	{
		HDC hDC;
		//设备描述表
		int iBits;
		//当前显示分辨率下每个像素所占字节数
		WORD wBitCount;
		//位图中每个像素所占字节数
		//定义调色板大小， 位图中像素字节大小 ，  位图文件大小 ， 写入文件字节数
		DWORD  dwPaletteSize = 0, dwBmBitsSize, dwDIBSize, dwWritten;
		BITMAP  Bitmap;
		//位图属性结构
		BITMAPFILEHEADER   bmfHdr;
		//位图文件头结构
		BITMAPINFOHEADER   bi;
		//位图信息头结构
		LPBITMAPINFOHEADER lpbi;
		//指向位图信息头结构
		HANDLE  fh, hDib, hPal;
		HPALETTE  hOldPal = NULL;

		//定义文件，分配内存句柄，调色板句柄
		//计算位图文件每个像素所占字节数
		hDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
		iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
		DeleteDC(hDC);
		if (iBits <= 1)
		{
			wBitCount = 1;
		}
		else if (iBits <= 4)
		{
			wBitCount = 4;
		}
		else if (iBits <= 8)
		{
			wBitCount = 8;
		}
		else if (iBits <= 24)
		{
			wBitCount = 24;
		}
		else
		{
			wBitCount = 32;
		}

		//计算调色板大小
		if (wBitCount <= 8)
		{
			dwPaletteSize = (1 << wBitCount)*sizeof(RGBQUAD);
		}

		//设置位图信息头结构
		GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);
		bi.biSize = sizeof(BITMAPINFOHEADER);
		bi.biWidth = Bitmap.bmWidth;
		bi.biHeight = Bitmap.bmHeight;
		bi.biPlanes = 1;
		bi.biBitCount = wBitCount;
		bi.biCompression = BI_RGB;
		bi.biSizeImage = 0;
		bi.biXPelsPerMeter = 0;
		bi.biYPelsPerMeter = 0;
		bi.biClrUsed = 0;
		bi.biClrImportant = 0;
		dwBmBitsSize = ((Bitmap.bmWidth*wBitCount + 31) / 32) * 4 * Bitmap.bmHeight;

		//为位图内容分配内存
		hDib = GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
		lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
		*lpbi = bi;
		// 处理调色板
		hPal = GetStockObject(DEFAULT_PALETTE);
		if (hPal)
		{
			hDC = ::GetDC(NULL);
			hOldPal = SelectPalette(hDC, (HPALETTE)hPal, FALSE);
			RealizePalette(hDC);
		}
		// 获取该调色板下新的像素值
		GetDIBits(hDC, hBitmap, 0, (UINT)Bitmap.bmHeight, (LPSTR)lpbi + sizeof(BITMAPINFOHEADER) + dwPaletteSize, (BITMAPINFO *)lpbi, DIB_RGB_COLORS);
		//恢复调色板   
		if (hOldPal)
		{
			SelectPalette(hDC, hOldPal, TRUE);
			RealizePalette(hDC);
			::ReleaseDC(NULL, hDC);
		}

		//创建位图文件    
		fh = CreateFile(ptFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		if (fh == INVALID_HANDLE_VALUE)
		{
			GlobalUnlock(hDib);
			GlobalFree(hDib);
			return FALSE;
		}
		//设置位图文件头
		bmfHdr.bfType = 0x4D42;  // "BM"
		dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;
		bmfHdr.bfSize = dwDIBSize;
		bmfHdr.bfReserved1 = 0;
		bmfHdr.bfReserved2 = 0;
		bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;

		WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
		WriteFile(fh, (LPSTR)lpbi, sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize, &dwWritten, NULL);

		GlobalUnlock(hDib);
		GlobalFree(hDib);

		CloseHandle(fh);
		return false;
	}
	__inline static
		HBITMAP GetBitmapFromDC(HDC hDC, RECT rc, RECT rcMemory = { 0 })
	{
		HDC hMemoryDC = NULL;
		HBITMAP hBitmap = NULL;
		HBITMAP hBitmapTemp = NULL;

		//创建设备上下文(HDC)
		hMemoryDC = CreateCompatibleDC(hDC);

		//创建HBITMAP
		hBitmap = CreateCompatibleBitmap(hDC, rc.right - rc.left, rc.bottom - rc.top);
		hBitmapTemp = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

		if (rcMemory.right <= rcMemory.left)
		{
			rcMemory.right = rcMemory.left + rc.right - rc.left;
		}
		if (rcMemory.bottom <= rcMemory.top)
		{
			rcMemory.bottom = rcMemory.top + rc.bottom - rc.top;
		}

		//得到位图缓冲区
		StretchBlt(hMemoryDC, rcMemory.left, rcMemory.top, rcMemory.right - rcMemory.left, rcMemory.bottom - rcMemory.top,
			hDC, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SRCCOPY);

		//得到最终的位图信息
		hBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmapTemp);

		//释放内存
		DeleteObject(hBitmapTemp);
		hBitmapTemp = NULL;
		::DeleteDC(hMemoryDC);
		hMemoryDC = NULL;

		return hBitmap;
	}

	__inline static TSTRING HResultToTString(HRESULT hInResult)
	{
		HRESULT hResult = S_FALSE;
		TSTRING tsDescription(_T(""));
		IErrorInfo * pErrorInfo = NULL;
		BSTR pbstrDescription = NULL;
		hResult = ::GetErrorInfo(NULL, &pErrorInfo);
		if (SUCCEEDED(hResult) && pErrorInfo)
		{
			hResult = pErrorInfo->GetDescription(&pbstrDescription);
			if (SUCCEEDED(hResult) && pbstrDescription)
			{
				tsDescription = Convert::WToT(pbstrDescription);
			}
		}
		return tsDescription;
	}
	///////////////////////////////////////////////////////////
	//如下代码段实现的功能是从指定的路径中读取图片，并显示出来
	//
	__inline static void ImagesRenderDisplay(HDC hDC, RECT * pRect, const _TCHAR * tImagePath)
	{
		HANDLE hFile = NULL;
		HRESULT hResult = S_FALSE;
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
				
				hResult = ::CreateStreamOnHGlobal(hImageMemory, FALSE, &pIStream); //用全局内存初使化IStream接口指针
				if (SUCCEEDED(hResult) && pIStream)
				{
					//用OleLoadPicture获得IPicture接口指针
					hResult = ::OleLoadPicture(pIStream, 0, FALSE, IID_IPicture, (LPVOID*)&(pIPicture));
					//TSTRING tsError = HResultToTString(hResult);
					//得到IPicture COM接口对象后，就可以进行获得图片信息、显示图片等操作
					if (SUCCEEDED(hResult) && pIPicture)
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
	__inline static void ImagesDisplayScreen(SIZE & szImageSize, const _TCHAR * tImagePath)
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
	__inline static
		void DrawMemoryBitmap(HDC &dc, HWND hWnd, LONG lWidth, LONG lHeight, HBITMAP hBitmap)
	{
		RECT rect;
		HBITMAP hOldBitmap;
		int disHeight, disWidth;

		GetClientRect(hWnd, &rect);//获取客户区大小
		disHeight = rect.bottom - rect.top;
		disWidth = rect.right - rect.left;

		HDC mDc = ::CreateCompatibleDC(dc);//创建当前上下文的兼容dc(内存DC)
		hOldBitmap = (HBITMAP)::SelectObject(mDc, hBitmap);//将位图加载到内存DC

		//拷贝内存DC数据块到当前DC，自动拉伸
		::StretchBlt(dc, 0, 0, disWidth, disHeight, mDc, 0, 0, lWidth, lHeight, SRCCOPY);

		//恢复内存原始数据
		::SelectObject(mDc, hOldBitmap);

		//删除资源，防止泄漏
		::DeleteObject(hBitmap);
		::DeleteDC(mDc);
	}

	__inline static
		void DrawImage(HDC &dc, HWND hWnd, LONG lWidth, LONG lHeight, LPCTSTR ptFileName)
	{
		RECT rect;
		HBITMAP hOrgBitmap;
		HBITMAP hOldBitmap;
		int disHeight, disWidth;

		GetClientRect(hWnd, &rect);//获取客户区大小
		disHeight = rect.bottom - rect.top;
		disWidth = rect.right - rect.left;

		//加载图片
		hOrgBitmap = (HBITMAP)::LoadImage(GetModuleHandle(NULL), ptFileName, IMAGE_BITMAP, lWidth, lHeight, LR_LOADFROMFILE);

		HDC mDc = ::CreateCompatibleDC(dc);//创建当前上下文的兼容dc(内存DC)
		hOldBitmap = (HBITMAP)::SelectObject(mDc, hOrgBitmap);//将位图加载到内存DC

		//拷贝内存DC数据块到当前DC，自动拉伸
		::StretchBlt(dc, 0, 0, disWidth, disHeight, mDc, 0, 0, lWidth, lHeight, SRCCOPY);

		//恢复内存原始数据
		::SelectObject(mDc, hOldBitmap);

		//删除资源，防止泄漏
		::DeleteObject(hOrgBitmap);
		::DeleteDC(mDc);
		mDc = NULL;
	}

	__inline static
		HBITMAP DrawAlphaBlend(HWND hWnd, HDC hDCWnd)
	{
		typedef struct _ALPHABLENDRECT {
			HDC HDCDST;
			RECT RCDST;
			HDC HDCSRC;
			RECT RCSRC;
			BLENDFUNCTION BF;// structure for alpha blending 
		}ALPHABLENDRECT, *PALPHABLENDRECT;
		UINT32 x = 0;// stepping variables 
		UINT32 y = 0;// stepping variables 
		HDC hDC = NULL;// handle of the DC we will create 
		UCHAR uA = 0x00;// used for doing transparent gradient 
		UCHAR uR = 0x00;
		UCHAR uG = 0x00;
		UCHAR uB = 0x00;
		float fAF = 0.0f;// used to do premultiply
		VOID * pvBits = 0;// pointer to DIB section
		RECT rcWnd = { 0 };// used for getting window dimensions
		HBITMAP hBitmap = NULL;// bitmap handle
		BITMAPINFO bmi = { 0 };// bitmap header
		ULONG ulWindowWidth = 0;// window width/height
		ULONG ulBitmapWidth = 0;// bitmap width/height
		ULONG ulWindowHeight = 0;// window width/height
		ULONG ulBitmapHeight = 0;// bitmap width/height
		ALPHABLENDRECT abrc = { 0 };

		// get dest dc
		abrc.HDCDST = hDCWnd;

		// get window dimensions 
		::GetClientRect(hWnd, &rcWnd);

		// calculate window width/height 
		ulWindowWidth = rcWnd.right - rcWnd.left;
		ulWindowHeight = rcWnd.bottom - rcWnd.top;

		// make sure we have at least some window size 
		if ((!ulWindowWidth) || (!ulWindowHeight))
		{
			return NULL;
		}

		// divide the window into 3 horizontal areas 
		ulWindowHeight = ulWindowHeight / 3;

		// create a DC for our bitmap -- the source DC for AlphaBlend  
		abrc.HDCSRC = ::CreateCompatibleDC(abrc.HDCDST);

		// zero the memory for the bitmap info 
		::ZeroMemory(&bmi, sizeof(BITMAPINFO));

		// setup bitmap info  
		// set the bitmap width and height to 60% of the width and height of each of the three horizontal areas. Later on, the blending will occur in the center of each of the three areas. 
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = ulBitmapWidth = ulWindowWidth;// -(ulWindowWidth / 5) * 2;
		bmi.bmiHeader.biHeight = ulBitmapHeight = ulWindowHeight - (ulWindowHeight / 5) * 2;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;// four 8-bit components 
		bmi.bmiHeader.biCompression = BI_RGB;
		bmi.bmiHeader.biSizeImage = ulBitmapWidth * ulBitmapHeight * 4;

		// create our DIB section and select the bitmap into the dc 
		hBitmap = ::CreateDIBSection(abrc.HDCSRC, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0x00);
		::SelectObject(abrc.HDCSRC, hBitmap);

		// in top window area, constant alpha = 50%, but no source alpha 
		// the color format for each pixel is 0xaarrggbb  
		// set all pixels to blue and set source alpha to zero 
		for (y = 0; y < ulBitmapHeight; y++)
		{
			for (x = 0; x < ulBitmapWidth; x++)
			{
				((UINT32 *)pvBits)[x + y * ulBitmapWidth] = 0x0000FF00;// 0x000000FF;
			}
		}

		abrc.RCDST.left = 0;// ulWindowWidth / 5;
		abrc.RCDST.top = ulWindowHeight / 5;
		abrc.RCDST.right = ulBitmapWidth + abrc.RCDST.left;
		abrc.RCDST.bottom = ulBitmapHeight + abrc.RCDST.top;

		abrc.RCSRC.left = 0;
		abrc.RCSRC.top = 0;
		abrc.RCSRC.right = ulBitmapWidth + abrc.RCSRC.left;
		abrc.RCSRC.bottom = ulBitmapHeight + abrc.RCSRC.top;

		abrc.BF.BlendOp = AC_SRC_OVER;
		abrc.BF.BlendFlags = 0;
		abrc.BF.AlphaFormat = 0;// ignore source alpha channel 
		abrc.BF.SourceConstantAlpha = 0x7F;// half of 0x7F = 50% transparency 

		if (!AlphaBlend(abrc.HDCDST,
			abrc.RCDST.left, abrc.RCDST.top,
			abrc.RCDST.right - abrc.RCDST.left,
			abrc.RCDST.bottom - abrc.RCDST.top,
			abrc.HDCSRC,
			abrc.RCSRC.left, abrc.RCSRC.top,
			abrc.RCSRC.right - abrc.RCSRC.left,
			abrc.RCSRC.bottom - abrc.RCSRC.top,
			abrc.BF))
		{
			return NULL;// alpha blend failed 
		}
		// in middle window area, constant alpha = 100% (disabled), source  
		// alpha is 0 in middle of bitmap and opaque in rest of bitmap  
		for (y = 0; y < ulBitmapHeight; y++)
		{
			for (x = 0; x < ulBitmapWidth; x++)
			{
				if ((x > (int)(ulBitmapWidth / 5)) && (x < (ulBitmapWidth - ulBitmapWidth / 5)) &&
					(y >(int)(ulBitmapHeight / 5)) && (y < (ulBitmapHeight - ulBitmapHeight / 5)))
				{
					//in middle of bitmap: source alpha = 0 (transparent). 
					// This means multiply each color component by 0x00. 
					// Thus, after AlphaBlend, we have A, 0x00 * R,  
					// 0x00 * G,and 0x00 * B (which is 0x00000000) 
					// for now, set all pixels to red 
					((UINT32 *)pvBits)[x + y * ulBitmapWidth] = 0x00FF0000;
				}
				else
				{
					// in the rest of bitmap, source alpha = 0xFF (opaque)  
					// and set all pixels to blue  
					((UINT32 *)pvBits)[x + y * ulBitmapWidth] = 0xFF00FF00;// 0xFF0000FF;
				}
			}
		}

		abrc.RCDST.left = 0;// ulWindowWidth / 5;
		abrc.RCDST.top = ulWindowHeight / 5 + ulWindowHeight;
		abrc.RCDST.right = ulBitmapWidth + abrc.RCDST.left;
		abrc.RCDST.bottom = ulBitmapHeight + abrc.RCDST.top;

		abrc.RCSRC.left = 0;
		abrc.RCSRC.top = 0;
		abrc.RCSRC.right = ulBitmapWidth + abrc.RCSRC.left;
		abrc.RCSRC.bottom = ulBitmapHeight + abrc.RCSRC.top;

		abrc.BF.BlendOp = AC_SRC_OVER;
		abrc.BF.BlendFlags = 0;
		abrc.BF.AlphaFormat = MY_AC_SRC_ALPHA;// ignore source alpha channel 
		abrc.BF.SourceConstantAlpha = 0xFF;// half of 0xFF = 50% transparency 

		if (!AlphaBlend(abrc.HDCDST,
			abrc.RCDST.left, abrc.RCDST.top,
			abrc.RCDST.right - abrc.RCDST.left,
			abrc.RCDST.bottom - abrc.RCDST.top,
			abrc.HDCSRC,
			abrc.RCSRC.left, abrc.RCSRC.top,
			abrc.RCSRC.right - abrc.RCSRC.left,
			abrc.RCSRC.bottom - abrc.RCSRC.top,
			abrc.BF))
		{
			return NULL;// alpha blend failed 
		}

		// bottom window area, use constant alpha = 75% and a changing 
		// source alpha. Create a gradient effect using source alpha, and  
		// then fade it even more with constant alpha 
		uR = 0x00;
		uG = 0xFF;// 0x00;
		uB = 0x00;// 0xFF;

		for (y = 0; y < ulBitmapHeight; y++)
		{
			for (x = 0; x < ulBitmapWidth; x++)
			{
				// for a simple gradient, base the alpha value on the x  
				// value of the pixel  
				uA = (UCHAR)((float)x / (float)ulBitmapWidth * 0xFF);
				//calculate the factor by which we multiply each component 
				fAF = (float)uA / (float)0xFF;
				// multiply each pixel by fAlphaFactor, so each component  
				// is less than or equal to the alpha value. 
				((UINT32 *)pvBits)[x + y * ulBitmapWidth] =
					((UCHAR)(uA * 0x1) << 0x18) | //0xAA000000 
					((UCHAR)(uR * fAF) << 0x10) | //0x00RR0000 
					((UCHAR)(uG * fAF) << 0x08) | //0x0000GG00 
					((UCHAR)(uB * fAF) << 0x00); //0x000000BB 
			}
		}

		abrc.RCDST.left = 0;// ulWindowWidth / 5;
		abrc.RCDST.top = ulWindowHeight / 5 + 2 * ulWindowHeight;
		abrc.RCDST.right = ulBitmapWidth + abrc.RCDST.left;
		abrc.RCDST.bottom = ulBitmapHeight + abrc.RCDST.top;

		abrc.RCSRC.left = 0;
		abrc.RCSRC.top = 0;
		abrc.RCSRC.right = ulBitmapWidth + abrc.RCSRC.left;
		abrc.RCSRC.bottom = ulBitmapHeight + abrc.RCSRC.top;

		abrc.BF.BlendOp = AC_SRC_OVER;
		abrc.BF.BlendFlags = 0;
		abrc.BF.AlphaFormat = MY_AC_SRC_ALPHA;// ignore source alpha channel 
		abrc.BF.SourceConstantAlpha = 0xBF;// use constant alpha, with 75% opaqueness  

		if (!AlphaBlend(abrc.HDCDST,
			abrc.RCDST.left, abrc.RCDST.top,
			abrc.RCDST.right - abrc.RCDST.left,
			abrc.RCDST.bottom - abrc.RCDST.top,
			abrc.HDCSRC,
			abrc.RCSRC.left, abrc.RCSRC.top,
			abrc.RCSRC.right - abrc.RCSRC.left,
			abrc.RCSRC.bottom - abrc.RCSRC.top,
			abrc.BF))
		{
			return NULL;// alpha blend failed 
		}

		// do cleanup 
		DeleteObject(hBitmap);
		DeleteDC(hDC);
		hDC = NULL;

		return hBitmap;
	}

	__inline static
		HBITMAP DrawAlphaBlendRect(HWND hWnd, HDC hDCWnd, RECT * pRect)
	{
		typedef struct _ALPHABLENDRECT {
			HDC HDCDST;
			RECT RCDST;
			HDC HDCSRC;
			RECT RCSRC;
			BLENDFUNCTION BF;// structure for alpha blending 
		}ALPHABLENDRECT, *PALPHABLENDRECT;
		UINT32 x = 0;// stepping variables
		UINT32 y = 0;// stepping variables 
		UCHAR uA = 0x00;// used for doing transparent gradient 
		UCHAR uR = 0x00;
		UCHAR uG = 0x00;
		UCHAR uB = 0x00;
		float fAF = 0.0f;// used to do premultiply
		VOID * pvBits = 0;// pointer to DIB section
		RECT rcWnd = { 0 };// used for getting window dimensions
		HBITMAP hBitmap = NULL;// bitmap handle
		BITMAPINFO bmi = { 0 };// bitmap header
		ULONG ulWindowWidth = 0;// window width/height
		ULONG ulBitmapWidth = 0;// bitmap width/height
		ULONG ulWindowHeight = 0;// window width/height
		ULONG ulBitmapHeight = 0;// bitmap width/height
		ALPHABLENDRECT abrc = { 0 };

		// get dest dc
		abrc.HDCDST = hDCWnd;

		// get window dimensions 
		::GetClientRect(hWnd, &rcWnd);

		// calculate window width/height 
		ulWindowWidth = rcWnd.right - rcWnd.left;
		ulWindowHeight = rcWnd.bottom - rcWnd.top;

		// make sure we have at least some window size 
		if ((!ulWindowWidth) || (!ulWindowHeight))
		{
			return NULL;
		}

		// create a DC for our bitmap -- the source DC for AlphaBlend  
		abrc.HDCSRC = ::CreateCompatibleDC(abrc.HDCDST);

		// zero the memory for the bitmap info 
		::ZeroMemory(&bmi, sizeof(BITMAPINFO));

		// setup bitmap info  
		// set the bitmap width and height to 60% of the width and height of each of the three horizontal areas. Later on, the blending will occur in the center of each of the three areas. 
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = ulBitmapWidth = ulWindowWidth;
		bmi.bmiHeader.biHeight = ulBitmapHeight = ulWindowHeight;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;// four 8-bit components 
		bmi.bmiHeader.biCompression = BI_RGB;
		bmi.bmiHeader.biSizeImage = ulBitmapWidth * ulBitmapHeight * 4;

		// create our DIB section and select the bitmap into the dc 
		hBitmap = ::CreateDIBSection(abrc.HDCSRC, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0x00);
		::SelectObject(abrc.HDCSRC, hBitmap);

		// in middle window area, constant alpha = 100% (disabled), source  
		// alpha is 0 in middle of bitmap and opaque in rest of bitmap  
		for (y = 0; y < ulBitmapHeight; y++)
		{
			for (x = 0; x < ulBitmapWidth; x++)
			{
				if ((x > (int)(pRect->left)) && (x < (ulBitmapWidth - pRect->right - 1)) &&
					(y >(int)(pRect->top)) && (y < (ulBitmapHeight - pRect->bottom - 1)))
				{
					//in middle of bitmap: source alpha = 0 (transparent). 
					// This means multiply each color component by 0x00. 
					// Thus, after AlphaBlend, we have A, 0x00 * R,  
					// 0x00 * G,and 0x00 * B (which is 0x00000000) 
					// for now, set all pixels to red 
					((UINT32 *)pvBits)[x + y * ulBitmapWidth] = 0x7F0000FF;// 0x00FF0000;
				}
				else
				{
					// in the rest of bitmap, source alpha = 0xFF (opaque)  
					// and set all pixels to blue  
					((UINT32 *)pvBits)[x + y * ulBitmapWidth] = 0x7F00FF00;// 0xFF0000FF;
				}
			}
		}

		abrc.RCDST.left = 0;
		abrc.RCDST.top = 0;
		abrc.RCDST.right = ulBitmapWidth + abrc.RCDST.left;
		abrc.RCDST.bottom = ulBitmapHeight + abrc.RCDST.top;

		abrc.RCSRC.left = 0;
		abrc.RCSRC.top = 0;
		abrc.RCSRC.right = ulBitmapWidth + abrc.RCSRC.left;
		abrc.RCSRC.bottom = ulBitmapHeight + abrc.RCSRC.top;

		abrc.BF.BlendOp = AC_SRC_OVER;
		abrc.BF.BlendFlags = 0;
		abrc.BF.AlphaFormat = MY_AC_SRC_ALPHA;// ignore source alpha channel 
		abrc.BF.SourceConstantAlpha = 0xFF;// use constant alpha, with 75% opaqueness  

		if (!AlphaBlend(abrc.HDCDST,
			abrc.RCDST.left, abrc.RCDST.top,
			abrc.RCDST.right - abrc.RCDST.left,
			abrc.RCDST.bottom - abrc.RCDST.top,
			abrc.HDCSRC,
			abrc.RCSRC.left, abrc.RCSRC.top,
			abrc.RCSRC.right - abrc.RCSRC.left,
			abrc.RCSRC.bottom - abrc.RCSRC.top,
			abrc.BF))
		{
			return NULL;// alpha blend failed 
		}

		SaveBitmapToFile(abrc.HDCDST, hBitmap, _T("d:\\test.bmp"));

		// do cleanup 
		DeleteObject(hBitmap);
		DeleteDC(abrc.HDCSRC);
		abrc.HDCSRC = NULL;

		return hBitmap;
	}
	
#define ARGB(uA,uR,uG,uB) ((UCHAR)(uA) << 0x18) | ((UCHAR)(uR) << 0x10) | ((UCHAR)(uG) << 0x08) | ((UCHAR)(uB) << 0x00)

	__inline static
		HBITMAP DrawAlphaBlendRect(HWND hWnd, ULONG(&uARGB)[2], HDC hDCWnd, RECT * pRect)
	{
		typedef struct _ALPHABLENDRECT {
			HDC HDCDST;
			RECT RCDST;
			HDC HDCSRC;
			RECT RCSRC;
			BLENDFUNCTION BF;// structure for alpha blending 
		}ALPHABLENDRECT, *PALPHABLENDRECT;
		UINT32 x = 0;// stepping variables
		UINT32 y = 0;// stepping variables 
		UCHAR uA = 0x00;// used for doing transparent gradient 
		UCHAR uR = 0x00;
		UCHAR uG = 0x00;
		UCHAR uB = 0x00;
		float fAF = 0.0f;// used to do premultiply
		VOID * pvBits = 0;// pointer to DIB section
		RECT rcWnd = { 0 };// used for getting window dimensions
		HBITMAP hBitmap = NULL;// bitmap handle
		BITMAPINFO bmi = { 0 };// bitmap header
		ULONG ulWindowWidth = 0;// window width/height
		ULONG ulBitmapWidth = 0;// bitmap width/height
		ULONG ulWindowHeight = 0;// window width/height
		ULONG ulBitmapHeight = 0;// bitmap width/height
		ALPHABLENDRECT abrc = { 0 };

		// get dest dc
		abrc.HDCDST = hDCWnd;

		// get window dimensions 
		::GetClientRect(hWnd, &rcWnd);

		// calculate window width/height 
		ulWindowWidth = rcWnd.right - rcWnd.left;
		ulWindowHeight = rcWnd.bottom - rcWnd.top;

		// make sure we have at least some window size 
		if ((!ulWindowWidth) || (!ulWindowHeight))
		{
			return NULL;
		}

		// create a DC for our bitmap -- the source DC for AlphaBlend  
		abrc.HDCSRC = ::CreateCompatibleDC(abrc.HDCDST);

		// zero the memory for the bitmap info 
		::ZeroMemory(&bmi, sizeof(BITMAPINFO));

		// setup bitmap info  
		// set the bitmap width and height to 60% of the width and height of each of the three horizontal areas. Later on, the blending will occur in the center of each of the three areas. 
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = ulBitmapWidth = ulWindowWidth;
		bmi.bmiHeader.biHeight = ulBitmapHeight = ulWindowHeight;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;// four 8-bit components 
		bmi.bmiHeader.biCompression = BI_RGB;
		bmi.bmiHeader.biSizeImage = ulBitmapWidth * ulBitmapHeight * 4;

		// create our DIB section and select the bitmap into the dc 
		hBitmap = ::CreateDIBSection(abrc.HDCSRC, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0x00);
		::SelectObject(abrc.HDCSRC, hBitmap);

		// in middle window area, constant alpha = 100% (disabled), source  
		// alpha is 0 in middle of bitmap and opaque in rest of bitmap  
		for (x = 0; x < ulBitmapWidth; x++)
		{
			for (y = 0; y < ulBitmapHeight; y++)
			{
				if ((x >(int)(pRect->right)) && (x < (ulBitmapWidth - pRect->left - 1)) &&
					(y >(int)(pRect->bottom)) && (y < (ulBitmapHeight - pRect->top - 1)))
				{
					//in middle of bitmap: source alpha = 0 (transparent). 
					// This means multiply each color component by 0x00. 
					// Thus, after AlphaBlend, we have A, 0x00 * R,  
					// 0x00 * G,and 0x00 * B (which is 0x00000000) 
					// for now, set all pixels to red 
					((UINT32 *)pvBits)[x + y * ulBitmapWidth] = uARGB[0];//0x7F0000FF;// 0x00FF0000;
				}
				else
				{
					// in the rest of bitmap, source alpha = 0xFF (opaque)  
					// and set all pixels to blue  
					((UINT32 *)pvBits)[x + y * ulBitmapWidth] = uARGB[1];//0x7F00FF00;// 0xFF0000FF;
				}
			}
		}

		abrc.RCDST.left = 0;
		abrc.RCDST.top = 0;
		abrc.RCDST.right = ulBitmapWidth + abrc.RCDST.left;
		abrc.RCDST.bottom = ulBitmapHeight + abrc.RCDST.top;

		abrc.RCSRC.left = 0;
		abrc.RCSRC.top = 0;
		abrc.RCSRC.right = ulBitmapWidth + abrc.RCSRC.left;
		abrc.RCSRC.bottom = ulBitmapHeight + abrc.RCSRC.top;

		abrc.BF.BlendOp = AC_SRC_OVER;
		abrc.BF.BlendFlags = 0;
		abrc.BF.AlphaFormat = AC_SRC_ALPHA;// ignore source alpha channel 
		abrc.BF.SourceConstantAlpha = 0xFF;// use constant alpha, with 75% opaqueness  

		if (!AlphaBlend(abrc.HDCDST,
			abrc.RCDST.left, abrc.RCDST.top,
			abrc.RCDST.right - abrc.RCDST.left,
			abrc.RCDST.bottom - abrc.RCDST.top,
			abrc.HDCSRC,
			abrc.RCSRC.left, abrc.RCSRC.top,
			abrc.RCSRC.right - abrc.RCSRC.left,
			abrc.RCSRC.bottom - abrc.RCSRC.top,
			abrc.BF))
		{
			return NULL;// alpha blend failed 
		}

		//SaveBitmapToFile(abrc.HDCDST, hBitmap, _T("d:\\test.bmp"));

		// do cleanup 
		DeleteObject(hBitmap);
		DeleteDC(abrc.HDCSRC);
		abrc.HDCSRC = NULL;

		return hBitmap;
	}

	__inline static
		HFONT CreatePaintFont(LPCTSTR ptszFaceName = _T("宋体"),
		LONG lfHeight = 12,
		LONG lfWidth = 0,
		LONG lfEscapement = 0,
		LONG lfOrientation = 0,
		LONG lfWeight = FW_NORMAL,
		BYTE lfItalic = FALSE,
		BYTE lfUnderline = FALSE,
		BYTE lfStrikeOut = FALSE,
		BYTE lfCharSet = ANSI_CHARSET,
		BYTE lfOutPrecision = OUT_DEFAULT_PRECIS,
		BYTE lfClipPrecision = CLIP_DEFAULT_PRECIS,
		BYTE lfQuality = DEFAULT_QUALITY,
		BYTE lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS)
	{
		return CreateFont(lfHeight, lfWidth,
			lfEscapement,
			lfOrientation,
			lfWeight,
			lfItalic,
			lfUnderline,
			lfStrikeOut,
			lfCharSet,
			lfOutPrecision,
			lfClipPrecision,
			lfQuality,
			lfPitchAndFamily,
			ptszFaceName);
	}
	__inline static
		void RectDrawText(HDC hDC, RECT * pRect, LPCTSTR ptszText, COLORREF clrTextColor = COLOR_WINDOWTEXT, HFONT hFont = NULL, UINT uFormat = DT_LEFT | DT_WORDBREAK)
	{
		HFONT hFontOld = NULL;

		if (hFont)
		{
			hFontOld = (HFONT)SelectObject(hDC, hFont);

			SetBkMode(hDC, TRANSPARENT);
			SetTextColor(hDC, clrTextColor);

			DrawText(hDC, ptszText, lstrlen(ptszText), pRect, uFormat);

			(HFONT)SelectObject(hDC, hFontOld);
		}
	}

	__inline static
		HGLOBAL OpenResource(LPVOID & lpData, DWORD & dwSize, LPCTSTR lpName, LPCTSTR lpType, HMODULE hModule = ::GetModuleHandle(NULL))
	{
		BOOL bResult = FALSE;
		HRSRC hRsrcRes = NULL;// handle/ptr. to res. info. in hSource 
		HGLOBAL hGLOBAL = NULL;// handle to loaded resource

		// Locate the resource in the source image file. 
		hRsrcRes = ::FindResource(hModule, lpName, lpType);
		if (hRsrcRes == NULL)
		{
			goto __LEAVE_CLEAN__;
		}
		
		////////////////////////////////////////////////////////////////////////////////////////////
		// See:https://msdn.microsoft.com/en-us/library/windows/desktop/ms648047(v=vs.85).aspx
		// A handle to the resource to be accessed. The LoadResource function returns this handle. 
		//Note that this parameter is listed as an HGLOBAL variable only for backward compatibility. 
		//Do not pass any value as a parameter other than a successful return value 
		//from the LoadResource function.

		// Load the resource into global memory. 
		hGLOBAL = ::LoadResource(hModule, hRsrcRes);
		if (hGLOBAL == NULL)
		{
			goto __LEAVE_CLEAN__;
		}

		// Lock the resource into global memory. 
		lpData = ::LockResource(hGLOBAL);
		if (lpData == NULL)
		{
			FreeResource(hGLOBAL);
			hGLOBAL = NULL;
			goto __LEAVE_CLEAN__;
		}

		dwSize = ::SizeofResource(hModule, hRsrcRes);

	__LEAVE_CLEAN__:

		return hGLOBAL;
	}

	__inline static
		BOOL ParseResrc(LPCTSTR ptszFileName, UINT uResourceID, LPCTSTR ptszTypeName, HMODULE hModule = ::GetModuleHandle(NULL))
	{
		DWORD dwSize = 0;
		HANDLE hFile = NULL;
		BOOL bResult = FALSE;
		LPVOID lpData = NULL;
		HGLOBAL hGlobal = NULL;
		DWORD dwNumberOfBytesWritten = 0;

		hGlobal = OpenResource(lpData, dwSize, MAKEINTRESOURCE(uResourceID), ptszTypeName, hModule);

		//我们用刚才得到的pBuffer和dwSize来做一些需要的事情。可以直接在内存中使
		//用，也可以写入到硬盘文件。这里我们简单的写入到硬盘文件，如果我们的自定
		//义资源是作为嵌入DLL来应用，情况可能要复杂一些。
		hFile = CreateFile(ptszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile && hGlobal && dwSize > 0)
		{
			WriteFile(hFile, hGlobal, dwSize, &dwNumberOfBytesWritten, NULL);
			CloseHandle(hFile);

			bResult = TRUE;
		}
		
		return bResult;
	}

	__inline static
		BOOL DragMove(HWND hWnd, RECT * pRect)
	{
		POINT pt = { 0 };
		BOOL bResult = FALSE;
		RECT rcWndOuter = { 0 };
		RECT rcWndInner = { 0 };
		GetCursorPos(&pt);
		::GetWindowRect(hWnd, &rcWndOuter);

		rcWndInner.left = rcWndOuter.left + pRect->left;
		rcWndInner.top = rcWndOuter.top + pRect->top;
		rcWndInner.right = rcWndOuter.right - pRect->right;
		rcWndInner.bottom = rcWndOuter.bottom - pRect->bottom;

		if ((PtInRect(&rcWndOuter, pt) && !PtInRect(&rcWndInner, pt)))
		{
#ifndef _SYSCOMMAND_SC_DRAGMOVE
#define _SYSCOMMAND_SC_DRAGMOVE  0xF012
#endif // !_SYSCOMMAND_SC_DRAGMOVE
			::SystemParametersInfo(SPI_SETDRAGFULLWINDOWS, TRUE, NULL, 0);
			::SendMessage(hWnd, WM_SYSCOMMAND, _SYSCOMMAND_SC_DRAGMOVE, 0);
			bResult = TRUE;
#ifdef _SYSCOMMAND_SC_DRAGMOVE
#undef _SYSCOMMAND_SC_DRAGMOVE  
#endif // !_SYSCOMMAND_SC_DRAGMOVE
		}

		return bResult;
	}
	__inline static
		BOOL DragMoveFull(HWND hWnd)
	{
#ifndef _SYSCOMMAND_SC_DRAGMOVE
#define _SYSCOMMAND_SC_DRAGMOVE  0xF012
#endif // !_SYSCOMMAND_SC_DRAGMOVE
		::SystemParametersInfo(SPI_SETDRAGFULLWINDOWS, TRUE, NULL, 0);
		::SendMessage(hWnd, WM_SYSCOMMAND, _SYSCOMMAND_SC_DRAGMOVE, 0);
		//RECT rc = { 0 };
		//::GetClientRect(hWnd, &rc);
		//::SystemParametersInfo(SPI_SETDRAGFULLWINDOWS, FALSE, NULL, 0);
		//::PostMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(rc.left, rc.top));
#ifdef _SYSCOMMAND_SC_DRAGMOVE
#undef _SYSCOMMAND_SC_DRAGMOVE  
#endif // !_SYSCOMMAND_SC_DRAGMOVE

		return TRUE;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	//函数功能：设置圆角窗口
	//函数参数：
	//	hWnd		要设置的窗口句柄
	//	pstEllipse	要设置圆角的横向半径和纵向半径
	//	prcExcepted	要排除圆角的左上右下侧大小
	//返回值：无返回
	__inline static void SetWindowEllispeFrame(HWND hWnd, SIZE * pszEllipse = 0, RECT * prcExcepted = 0, BOOL bErase = TRUE)
	{
		HRGN hRgnWindow = 0;
		POINT ptPosition = { 0, 0 };
		RECT rcWindow = { 0, 0, 0, 0 };

		::GetWindowRect(hWnd, &rcWindow);
		if (prcExcepted)
		{
			ptPosition.x = prcExcepted->left;
			ptPosition.y = prcExcepted->top;
			rcWindow.left += prcExcepted->left;
			rcWindow.top += prcExcepted->top;
			rcWindow.right -= prcExcepted->right;
			rcWindow.bottom -= prcExcepted->bottom;
		}

		hRgnWindow = ::CreateRoundRectRgn(ptPosition.x, ptPosition.y, \
			rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, pszEllipse->cx, pszEllipse->cy);
		if (hRgnWindow)
		{
			::SetWindowRgn(hWnd, hRgnWindow, bErase);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// 函数说明：不失真模式获取HWND的HBITMAP
	// 参    数：窗口句柄
	// 返 回 值：返回HBITMAP
	// 编 写 者: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static HBITMAP HBITMAPFromHWND(HWND hWnd)
	{
		INT nX = 0;
		INT nY = 0;
		INT nWidth = 0;
		INT nHeight = 0;
		RECT rect = { 0 };
		INT nMode = 0;
		HDC hWndDC = NULL;
		HDC hMemDC = NULL;
		HBITMAP hOldBitmap = NULL;
		HBITMAP hNewBitmap = NULL;

		GetClientRect(hWnd, &rect);

		nX = rect.left;
		nY = rect.top;
		nWidth = rect.right - rect.left;
		nHeight = rect.bottom - rect.top;

		hWndDC = GetDC(hWnd);

		// 为屏幕设备描述表创建兼容的内存设备描述表
		hMemDC = CreateCompatibleDC(hWndDC);

		// 创建一个与屏幕设备描述表兼容的位图
		hNewBitmap = CreateCompatibleBitmap(hWndDC, nWidth, nHeight);

		// 把新位图选到内存设备描述表中
		hOldBitmap = (HBITMAP)SelectObject(hMemDC, hNewBitmap);

		// 设置不失真模式
		nMode = SetStretchBltMode(hWndDC, COLORONCOLOR);

		// 把屏幕设备描述表拷贝到内存设备描述表中
		//BitBlt(hMemDC, 0, 0, nWidth, nHeight, hWndDC, nX, nY, SRCCOPY);
		StretchBlt(hMemDC, 0, 0, nWidth, nHeight, hWndDC, nX, nY, nWidth, nHeight, SRCCOPY);

		// 设置不失真模式
		SetStretchBltMode(hMemDC, nMode);

		// 得到屏幕位图的句柄
		hNewBitmap = (HBITMAP)SelectObject(hMemDC, hOldBitmap);

		// 清除DC
		DeleteDC(hMemDC);
		hMemDC = NULL;
		DeleteDC(hWndDC);
		hWndDC = NULL;

		// 释放DC
		ReleaseDC(hWnd, hWndDC);

		return hNewBitmap;
	}
	//////////////////////////////////////////////////////////////////////////
	// 函数说明：不失真模式获取HWND的HBITMAP
	// 参    数：窗口句柄，指定坐标位置及大小
	// 返 回 值：返回HBITMAP
	// 编 写 者: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static HBITMAP HBITMAPFromHWND(HWND hWnd, int nX, int nY,
		int nWidth, int nHeight)
	{
		INT nMode = 0;
		HDC hWndDC = NULL;
		HDC hMemDC = NULL;
		HBITMAP hOldBitmap = NULL;
		HBITMAP hNewBitmap = NULL;

		hWndDC = GetDC(hWnd);

		// 为屏幕设备描述表创建兼容的内存设备描述表
		hMemDC = CreateCompatibleDC(hWndDC);

		// 创建一个与屏幕设备描述表兼容的位图
		hNewBitmap = CreateCompatibleBitmap(hWndDC, nWidth, nHeight);

		// 把新位图选到内存设备描述表中
		hOldBitmap = (HBITMAP)SelectObject(hMemDC, hNewBitmap);

		// 设置不失真模式
		nMode = SetStretchBltMode(hWndDC, COLORONCOLOR);

		// 把屏幕设备描述表拷贝到内存设备描述表中
		//BitBlt(hMemDC, 0, 0, nWidth, nHeight, hWndDC, nX, nY, SRCCOPY);
		StretchBlt(hMemDC, 0, 0, nWidth, nHeight, hWndDC, nX, nY, nWidth, nHeight, SRCCOPY);

		// 设置不失真模式
		SetStretchBltMode(hMemDC, nMode);

		// 得到屏幕位图的句柄
		hNewBitmap = (HBITMAP)SelectObject(hMemDC, hOldBitmap);

		// 清除DC
		DeleteDC(hMemDC);
		hMemDC = NULL;
		DeleteDC(hWndDC);
		hWndDC = NULL;

		// 释放DC
		ReleaseDC(hWnd, hWndDC);

		return hNewBitmap;
	}
	//////////////////////////////////////////////////////////////////////////
	// 函数说明：不失真模式获取HWND的HBITMAP
	// 参    数：窗口句柄，指定坐标位置及大小
	// 返 回 值：返回HBITMAP
	// 编 写 者: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static HBITMAP HBITMAPFromHWND(HWND hWnd, int nX, int nY,
		int nWidth, int nHeight,
		HGDIOBJ * phGdiObj)
	{
		INT nMode = 0;
		HDC hWndDC = NULL;
		HDC hMemDC = NULL;
		HBITMAP hOldBitmap = NULL;
		HBITMAP hNewBitmap = NULL;

		hWndDC = GetDC(hWnd);

		// 为屏幕设备描述表创建兼容的内存设备描述表
		hMemDC = CreateCompatibleDC(hWndDC);

		// 创建一个与屏幕设备描述表兼容的位图
		hNewBitmap = CreateCompatibleBitmap(hWndDC, nWidth, nHeight);

		// 把新位图选到内存设备描述表中
		hOldBitmap = (HBITMAP)SelectObject(hMemDC, hNewBitmap);

		// 设置不失真模式
		nMode = SetStretchBltMode(hWndDC, COLORONCOLOR);

		// 把屏幕设备描述表拷贝到内存设备描述表中
		//BitBlt(hMemDC, 0, 0, nWidth, nHeight, hWndDC, nX, nY, SRCCOPY);
		StretchBlt(hMemDC, 0, 0, nWidth, nHeight, hWndDC, nX, nY, nWidth, nHeight, SRCCOPY);

		// 设置不失真模式
		SetStretchBltMode(hMemDC, nMode);

		// 得到屏幕位图的句柄
		hNewBitmap = (HBITMAP)SelectObject(hMemDC, hOldBitmap);

		// 清除DC
		DeleteDC(hMemDC);
		hMemDC = NULL;
		DeleteDC(hWndDC);
		hWndDC = NULL;

		// 释放DC
		ReleaseDC(hWnd, hWndDC);

		if (phGdiObj)
		{
			if ((*phGdiObj))
			{
				DeleteObject((*phGdiObj));
				(*phGdiObj) = NULL;
			}
			(*phGdiObj) = hNewBitmap;
		}

		return hNewBitmap;
	}
	//////////////////////////////////////////////////////////////////////////
	// 函数说明：不失真模式获取HDC的HBITMAP
	// 参    数：设备DC，大小
	// 返 回 值：返回HBITMAP
	// 编 写 者: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static HBITMAP HBitmapFromHdc(HDC hDC, DWORD BitWidth, DWORD BitHeight)
	{
		HDC hMemDC;
		INT nMode = 0;
		HBITMAP hBitmap, hBitTemp;
		//创建设备上下文(HDC)
		hMemDC = CreateCompatibleDC(hDC);
		//创建HBITMAP
		hBitmap = CreateCompatibleBitmap(hDC, BitWidth, BitHeight);
		hBitTemp = (HBITMAP)SelectObject(hMemDC, hBitmap);

		nMode = SetStretchBltMode(hDC, COLORONCOLOR);//设置不失真模式
		//得到位图缓冲区
		StretchBlt(hMemDC, 0, 0, BitWidth, BitHeight, hDC, 0, 0, BitWidth, BitHeight, SRCCOPY);
		SetStretchBltMode(hDC, nMode);
		//得到最终的位图信息
		hBitmap = (HBITMAP)SelectObject(hMemDC, hBitTemp);
		//释放内存
		DeleteObject(hBitTemp);
		DeleteDC(hMemDC);

		return hBitmap;
	}
	//////////////////////////////////////////////////////////////////////////
	// 函数说明：获取HWND到IStream数据流,返回内存句柄(使用完毕需手动释放ReleaseGlobalHandle)
	// 参    数：窗口句柄,输出IStream流
	// 返 回 值：返回HANDLE.(使用完毕需手动释放ReleaseGlobalHandle)
	// 编 写 者: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static HANDLE HWNDToIStream(HWND hWnd, IStream ** ppStream)
	{
		BITMAP bmp = { 0 };
		HANDLE hFile = NULL;
		DWORD dwDataSize = 0;
		DWORD dwFileSize = 0;
		BITMAPFILEHEADER bfh;
		BITMAPINFOHEADER bih;
		HANDLE hMemoryDIB = 0;
		BYTE *lpBitmap = NULL;
		DWORD dwBytesWritten = 0;
		IStream * pStream = NULL;
		HDC hWndDC = GetDC(hWnd);
		HBITMAP hWndBitmap = HBITMAPFromHWND(hWnd);

		// Get the BITMAP from the HBITMAP
		GetObject(hWndBitmap, sizeof(BITMAP), &bmp);

		bih.biSize = sizeof(BITMAPINFOHEADER);
		bih.biWidth = bmp.bmWidth;
		bih.biHeight = bmp.bmHeight;
		bih.biPlanes = 1;
		bih.biBitCount = 32;
		bih.biCompression = BI_RGB;
		bih.biSizeImage = 0;
		bih.biXPelsPerMeter = 0;
		bih.biYPelsPerMeter = 0;
		bih.biClrUsed = 0;
		bih.biClrImportant = 0;

		dwDataSize = ((bmp.bmWidth * bih.biBitCount + 31) / 32) * 4 * bmp.bmHeight;

		//Offset to where the actual bitmap bits start.
		bfh.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

		//Size of the file
		bfh.bfSize = dwDataSize;

		//bfType must always be BM for Bitmaps
		bfh.bfType = 0x4D42; //BM

		// Add the size of the headers to the size of the bitmap to get the total file size
		dwFileSize = bfh.bfOffBits + dwDataSize;

		// Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that
		// call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc
		// have greater overhead than HeapAlloc.
		hMemoryDIB = GlobalAlloc(GHND, dwFileSize);
		if (hMemoryDIB)
		{
			lpBitmap = (BYTE *)GlobalLock(hMemoryDIB);

			memcpy(lpBitmap, &bfh, sizeof(BITMAPFILEHEADER));
			memcpy(lpBitmap + sizeof(BITMAPFILEHEADER), &bih, sizeof(BITMAPINFOHEADER));

			// Gets the "bits" from the bitmap and copies them into a buffer
			// which is pointed to by lpBitmap.
			GetDIBits(hWndDC,
				hWndBitmap,
				0,
				(UINT)bmp.bmHeight,
				lpBitmap + bfh.bfOffBits,
				(BITMAPINFO *)&bih, DIB_RGB_COLORS);

			CreateStreamOnHGlobal(lpBitmap, FALSE, ppStream);
		}

		return hMemoryDIB;
	}

	//////////////////////////////////////////////////////////////////////////
	// 函数说明：获取HWND到IStream数据流,返回内存句柄(使用完毕需手动释放ReleaseGlobalHandle)
	// 参    数：窗口句柄,指定坐标及大小，输出IStream流
	// 返 回 值：返回HANDLE.(使用完毕需手动释放ReleaseGlobalHandle)
	// 编 写 者: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static HANDLE HWNDToIStream(HWND hWnd, int nX, int nY,
		int nWidth, int nHeight, IStream ** ppStream)
	{
		BITMAP bmp = { 0 };
		HANDLE hFile = NULL;
		DWORD dwDataSize = 0;
		DWORD dwFileSize = 0;
		BITMAPFILEHEADER bfh;
		BITMAPINFOHEADER bih;
		HANDLE hMemoryDIB = 0;
		BYTE *lpBitmap = NULL;
		DWORD dwBytesWritten = 0;
		IStream * pStream = NULL;
		HDC hWndDC = GetDC(hWnd);
		HBITMAP hWndBitmap = HBITMAPFromHWND(hWnd, nX, nY, nWidth, nHeight);

		// Get the BITMAP from the HBITMAP
		GetObject(hWndBitmap, sizeof(BITMAP), &bmp);

		bih.biSize = sizeof(BITMAPINFOHEADER);
		bih.biWidth = bmp.bmWidth;
		bih.biHeight = bmp.bmHeight;
		bih.biPlanes = 1;
		bih.biBitCount = 32;
		bih.biCompression = BI_RGB;
		bih.biSizeImage = 0;
		bih.biXPelsPerMeter = 0;
		bih.biYPelsPerMeter = 0;
		bih.biClrUsed = 0;
		bih.biClrImportant = 0;

		dwDataSize = ((bmp.bmWidth * bih.biBitCount + 31) / 32) * 4 * bmp.bmHeight;

		//Offset to where the actual bitmap bits start.
		bfh.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

		//Size of the file
		bfh.bfSize = dwDataSize;

		//bfType must always be BM for Bitmaps
		bfh.bfType = 0x4D42; //BM

		// Add the size of the headers to the size of the bitmap to get the total file size
		dwFileSize = bfh.bfOffBits + dwDataSize;

		// Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that
		// call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc
		// have greater overhead than HeapAlloc.
		hMemoryDIB = GlobalAlloc(GHND, dwFileSize);
		if (hMemoryDIB)
		{
			lpBitmap = (BYTE *)GlobalLock(hMemoryDIB);

			memcpy(lpBitmap, &bfh, sizeof(BITMAPFILEHEADER));
			memcpy(lpBitmap + sizeof(BITMAPFILEHEADER), &bih, sizeof(BITMAPINFOHEADER));

			// Gets the "bits" from the bitmap and copies them into a buffer
			// which is pointed to by lpBitmap.
			GetDIBits(hWndDC,
				hWndBitmap,
				0,
				(UINT)bmp.bmHeight,
				lpBitmap + bfh.bfOffBits,
				(BITMAPINFO *)&bih, DIB_RGB_COLORS);

			CreateStreamOnHGlobal(lpBitmap, FALSE, ppStream);
		}

		return hMemoryDIB;
	}
	//////////////////////////////////////////////////////////////////////////
	// 函数说明：释放内存句柄
	// 参    数：窗口句柄,输出IStream流
	// 返 回 值：无返回值
	// 编 写 者: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static void ReleaseGlobalHandle(HANDLE * hMemoryHandle)
	{
		if (hMemoryHandle && (*hMemoryHandle))
		{
			GlobalUnlock((*hMemoryHandle));
			(*hMemoryHandle) = NULL;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// 函数说明：保存HBITMAP到BMP图片文件
	// 参    数：窗口句柄,要保存文件名称
	// 返 回 值：bool类型。成功返回true;失败返回false;
	// 编 写 者: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static bool HBitmapToFile(HBITMAP hBitmap, const TCHAR *pFileName)
	{
		BITMAP bmp = { 0 };
		WORD wBitCount = 0;
		HANDLE hFile = NULL;
		DWORD dwDataSize = 0;
		DWORD dwFileSize = 0;
		BYTE *lpBitmap = NULL;
		HANDLE hDIBData = NULL;
		DWORD dwBytesWritten = 0;
		BITMAPFILEHEADER bfh = { 0 };
		BITMAPINFOHEADER bih = { 0 };

		HDC hDC = GetWindowDC(NULL);

		//计算位图文件每个像素所占字节数
		wBitCount = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);

		if (wBitCount <= 1)
		{
			wBitCount = 1;
		}
		else if (wBitCount <= 4)
		{
			wBitCount = 4;
		}
		else if (wBitCount <= 8)
		{
			wBitCount = 8;
		}
		else
		{
			wBitCount = 24;
		}

		// Get the BITMAP from the HBITMAP
		GetObject(hBitmap, sizeof(BITMAP), &bmp);

		bih.biSize = sizeof(BITMAPINFOHEADER);
		bih.biWidth = bmp.bmWidth;
		bih.biHeight = bmp.bmHeight;
		bih.biPlanes = 1;
		bih.biBitCount = wBitCount;
		bih.biCompression = BI_RGB;
		bih.biSizeImage = 0;
		bih.biXPelsPerMeter = 0;
		bih.biYPelsPerMeter = 0;
		bih.biClrUsed = 0;
		bih.biClrImportant = 0;

		dwDataSize = ((bmp.bmWidth * bih.biBitCount + 31) / 32) * 4 * bmp.bmHeight;

		//Offset to where the actual bitmap bits start.
		bfh.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

		//Size of the file
		bfh.bfSize = dwDataSize;

		//bfType must always be BM for Bitmaps
		bfh.bfType = 0x4D42; //BM

		// Add the size of the headers to the size of the bitmap to get the total file size
		dwFileSize = bfh.bfOffBits + dwDataSize;

		// Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that
		// call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc
		// have greater overhead than HeapAlloc.
		hDIBData = GlobalAlloc(GHND, dwFileSize);
		if (!hDIBData)
		{
			return false;
		}
		lpBitmap = (BYTE *)GlobalLock(hDIBData);

		memcpy(lpBitmap, &bfh, sizeof(BITMAPFILEHEADER));
		memcpy(lpBitmap + sizeof(BITMAPFILEHEADER), &bih, sizeof(BITMAPINFOHEADER));

		// Gets the "bits" from the bitmap and copies them into a buffer
		// which is pointed to by lpBitmap.
		GetDIBits(hDC,
			hBitmap,
			0,
			(UINT)bmp.bmHeight,
			lpBitmap + bfh.bfOffBits,
			(BITMAPINFO *)&bih, DIB_RGB_COLORS);

		// A file is created, this is where we will save the screen capture.
		hFile = CreateFile(pFileName,
			GENERIC_WRITE,
			0,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			return false;
		}

		WriteFile(hFile, (LPSTR)lpBitmap, dwFileSize, &dwBytesWritten, NULL);

		//Unlock and Free the DIB from the heap
		GlobalUnlock(hDIBData);
		GlobalFree(hDIBData);
		hDIBData = NULL;

		//Close the handle for the file that was created
		CloseHandle(hFile);
		hFile = NULL;

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// 函数说明：保存HBITMAP到BMP图片文件
	// 参    数：位图句柄,要保存文件名称
	// 返 回 值：bool类型。成功返回true;失败返回false;
	// 编 写 者: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static bool HBitmapToFileEx(HBITMAP hBitmap, const TCHAR * ptFileName)
	{
		HDC hDC = NULL;
		BITMAP bmp = { 0 };
		WORD wBitsCount = 0;
		bool result = false;
		HANDLE hFile = NULL;
		HANDLE hDIBData = 0;
		DWORD dwDataSize = 0;
		DWORD dwFileSize = 0;
		BITMAPFILEHEADER bfh;
		BITMAPINFOHEADER bih;
		BYTE *lpByteData = NULL;
		DWORD dwBytesWritten = 0;
		HPALETTE hPalette = NULL;
		HPALETTE hPaletteBackup = NULL;

		//计算位图文件每个像素所占字节数
		hDC = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
		wBitsCount = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);

		if (wBitsCount <= 1)
		{
			wBitsCount = 1;
		}
		else if (wBitsCount <= 4)
		{
			wBitsCount = 4;
		}
		else if (wBitsCount <= 8)
		{
			wBitsCount = 8;
		}
		else
		{
			wBitsCount = 24;
		}
		// Get the BITMAP from the HBITMAP
		GetObject(hBitmap, sizeof(BITMAP), &bmp);

		bih.biSize = sizeof(BITMAPINFOHEADER);
		bih.biWidth = bmp.bmWidth;
		bih.biHeight = bmp.bmHeight;
		bih.biPlanes = 1;
		bih.biBitCount = wBitsCount;
		bih.biCompression = BI_RGB;
		bih.biSizeImage = 0;
		bih.biXPelsPerMeter = 0;
		bih.biYPelsPerMeter = 0;
		bih.biClrUsed = 0;
		bih.biClrImportant = 0;

		dwDataSize = ((bmp.bmWidth * bih.biBitCount + 31) / 32) * 4 * bmp.bmHeight;

		//Offset to where the actual bitmap bits start.
		bfh.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

		//Size of the file
		bfh.bfSize = dwDataSize;

		//bfType must always be BM for Bitmaps
		bfh.bfType = 0x4D42; //BM

		// Add the size of the headers to the size of the bitmap to get the total file size
		dwFileSize = bfh.bfOffBits + dwDataSize;

		// Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that
		// call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc
		// have greater overhead than HeapAlloc.
		hDIBData = GlobalAlloc(GHND, dwFileSize);
		if (hDIBData)
		{
			lpByteData = (BYTE *)GlobalLock(hDIBData);

			memcpy(lpByteData, &bfh, sizeof(BITMAPFILEHEADER));
			memcpy(lpByteData + sizeof(BITMAPFILEHEADER), &bih, sizeof(BITMAPINFOHEADER));

			// Gets default palette
			hPalette = (HPALETTE)GetStockObject(DEFAULT_PALETTE);
			if (hPalette)
			{
				hDC = ::GetDC(NULL);
				hPaletteBackup = ::SelectPalette(hDC, (HPALETTE)hPalette, FALSE);
				::RealizePalette(hDC);
			}

			// Gets the "bits" from the bitmap and copies them into a buffer
			// which is pointed to by lpByteData.
			GetDIBits(hDC,
				hBitmap,
				0,
				(UINT)bmp.bmHeight,
				lpByteData + bfh.bfOffBits,
				(BITMAPINFO *)&bih, DIB_RGB_COLORS);
			// Recover original palette
			if (hPaletteBackup)
			{
				::SelectPalette(hDC, (HPALETTE)hPaletteBackup, TRUE);
				::RealizePalette(hDC);
				::ReleaseDC(NULL, hDC);
			}

			// A file is created, this is where we will save the screen capture.
			hFile = CreateFile(ptFileName,
				GENERIC_WRITE,
				0,
				NULL,
				CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				WriteFile(hFile, (void *)lpByteData, dwFileSize, &dwBytesWritten, NULL);

				//Close the handle for the file that was created
				CloseHandle(hFile);
				hFile = NULL;

				result = true;
			}

			//Unlock and Free the DIB from the heap
			GlobalUnlock(hDIBData);
			GlobalFree(hDIBData);
			hDIBData = NULL;
		}

		return result;
	}
	//////////////////////////////////////////////////////////////////////////
	// 函数说明：保存HWND到BMP图片文件
	// 参    数：窗口句柄,要保存文件名称
	// 返 回 值：bool类型。成功返回true;失败返回false;
	// 编 写 者: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static bool HWNDToFile(HWND hWnd, TCHAR * ptFileName)
	{
		BITMAP bmp = { 0 };
		bool result = false;
		HANDLE hFile = NULL;
		HANDLE hDIBData = 0;
		DWORD dwDataSize = 0;
		DWORD dwFileSize = 0;
		BITMAPFILEHEADER bfh;
		BITMAPINFOHEADER bih;
		BYTE *lpByteData = NULL;
		DWORD dwBytesWritten = 0;
		HDC hWndDC = GetDC(hWnd);
		HBITMAP hBitmap = HBITMAPFromHWND(hWnd);

		// Get the BITMAP from the HBITMAP
		GetObject(hBitmap, sizeof(BITMAP), &bmp);

		bih.biSize = sizeof(BITMAPINFOHEADER);
		bih.biWidth = bmp.bmWidth;
		bih.biHeight = bmp.bmHeight;
		bih.biPlanes = 1;
		bih.biBitCount = 32;
		bih.biCompression = BI_RGB;
		bih.biSizeImage = 0;
		bih.biXPelsPerMeter = 0;
		bih.biYPelsPerMeter = 0;
		bih.biClrUsed = 0;
		bih.biClrImportant = 0;

		dwDataSize = ((bmp.bmWidth * bih.biBitCount + 31) / 32) * 4 * bmp.bmHeight;

		//Offset to where the actual bitmap bits start.
		bfh.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

		//Size of the file
		bfh.bfSize = dwDataSize;

		//bfType must always be BM for Bitmaps
		bfh.bfType = 0x4D42; //BM

		// Add the size of the headers to the size of the bitmap to get the total file size
		dwFileSize = bfh.bfOffBits + dwDataSize;

		// Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that
		// call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc
		// have greater overhead than HeapAlloc.
		hDIBData = GlobalAlloc(GHND, dwFileSize);
		if (hDIBData)
		{
			lpByteData = (BYTE *)GlobalLock(hDIBData);

			memcpy(lpByteData, &bfh, sizeof(BITMAPFILEHEADER));
			memcpy(lpByteData + sizeof(BITMAPFILEHEADER), &bih, sizeof(BITMAPINFOHEADER));

			// Gets the "bits" from the bitmap and copies them into a buffer
			// which is pointed to by lpByteData.
			GetDIBits(hWndDC,
				hBitmap,
				0,
				(UINT)bmp.bmHeight,
				lpByteData + bfh.bfOffBits,
				(BITMAPINFO *)&bih, DIB_RGB_COLORS);

			// A file is created, this is where we will save the screen capture.
			hFile = CreateFile(ptFileName,
				GENERIC_WRITE,
				0,
				NULL,
				CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile != INVALID_HANDLE_VALUE)
			{
				WriteFile(hFile, (void *)lpByteData, dwFileSize, &dwBytesWritten, NULL);

				//Close the handle for the file that was created
				CloseHandle(hFile);
				hFile = NULL;

				result = true;
			}

			//Unlock and Free the DIB from the heap
			GlobalUnlock(hDIBData);
			GlobalFree(hDIBData);
			hDIBData = NULL;
		}

		return result;
	}
	//////////////////////////////////////////////////////////////////////////
	// 函数说明：显示HBITMAP到HWND上
	// 参    数：窗口句柄,HBITMAP位图句柄
	// 返 回 值：bool类型。成功返回true;失败返回false;
	// 编 写 者: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static HBITMAP showBitmap(HWND hStaticWnd, HBITMAP hStaticBitmap)
	{
		INT nMode = 0;
		RECT rect = { 0 };
		BITMAP bmp = { 0 };
		HDC hParentDC = NULL;
		HDC hStaticDC = NULL;
		HDC hMemoryDC = NULL;
		HWND hParentWnd = NULL;
		HBITMAP hOldBitmap = NULL;
		HBITMAP hNewBitmap = NULL;

		hParentWnd = GetParent(hStaticWnd);

		//获取图片显示框的大小
		GetClientRect(hStaticWnd, &rect);

		//获取位图的大小信息
		GetObject(hStaticBitmap, sizeof(bmp), &bmp);

		if ((rect.right - rect.left) >= bmp.bmWidth &&
			(rect.bottom - rect.top) >= bmp.bmHeight)
		{
			return  hNewBitmap;
		}

		hParentDC = GetDC(hParentWnd);
		hStaticDC = GetDC(hStaticWnd);

		hMemoryDC = CreateCompatibleDC(hParentDC);

		hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hStaticBitmap);

		//设置不失真模式
		nMode = SetStretchBltMode(hStaticDC, COLORONCOLOR);

		StretchBlt(hStaticDC, rect.left, rect.top, rect.right - rect.left,
			rect.bottom - rect.top, hMemoryDC, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
		SetStretchBltMode(hStaticDC, nMode);

		DeleteDC(hMemoryDC);
		ReleaseDC(hStaticWnd, hStaticDC);
		ReleaseDC(hParentWnd, hParentDC);

		return hNewBitmap;
	}
	//////////////////////////////////////////////////////////////////////////
	// 函数说明：显示HBITMAP到HWND上
	// 参    数：主窗口句柄,源子控件ID，目标子控件ID
	// 返 回 值：无返回值;
	// 编 写 者: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static void showPicture(HWND hWnd, INT nSrcID, INT nDstID)
	{
		RECT rect = { 0 };
		HGDIOBJ hOldBitmap = NULL;
		HGDIOBJ hNewBitmap = NULL;
		hNewBitmap = HBITMAPFromHWND(GetDlgItem(hWnd, nSrcID));
		if (hNewBitmap)
		{
			hOldBitmap = (HGDIOBJ)SendDlgItemMessage(hWnd, nDstID, STM_SETIMAGE,
				(WPARAM)IMAGE_BITMAP, (LPARAM)hNewBitmap);
			if (hOldBitmap)
			{
				DeleteObject(hOldBitmap);
				hOldBitmap = NULL;
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////
	// 函数说明：显示HBITMAP到HWND上
	// 参    数：主窗口句柄,HBITMAP句柄，目标子控件ID
	// 返 回 值：无返回值;
	// 编 写 者: ppshuai 20141126
	//////////////////////////////////////////////////////////////////////////
	__inline static void showPicture(HWND hWnd, HGDIOBJ hBitmap, INT nDstID)
	{
		RECT rect = { 0 };
		HGDIOBJ hOldBitmap = NULL;

		if (hBitmap)
		{
			hOldBitmap = (HGDIOBJ)SendDlgItemMessage(hWnd, nDstID, STM_SETIMAGE,
				(WPARAM)IMAGE_BITMAP, (LPARAM)hBitmap);
			if (hOldBitmap)
			{
				DeleteObject(hOldBitmap);
				hOldBitmap = NULL;
			}
		}
	}

	// Helper to calculate the alpha-premultiled value for a pixel
	__inline static DWORD PreMultiply(COLORREF cl, unsigned char nAlpha)
	{
		// It's strange that the byte order of RGB in 32b BMP is reverse to in COLORREF
		return (GetRValue(cl) * (DWORD)nAlpha / 255) << 16 |
			(GetGValue(cl) * (DWORD)nAlpha / 255) << 8 |
			(GetBValue(cl) * (DWORD)nAlpha / 255);
	}

	__inline static void MakeShadow(UINT32 *pShadBits, HWND hParent, RECT *rcParent,
		INT nDarkness = 150,
		INT nSharpness = 5,
		INT nSize = 12,
		INT nxOffset = 5,
		INT nyOffset = 5,
		COLORREF Color = RGB(255, 0, 0))
	{
		// The shadow algorithm:
		// Get the region of parent window,
		// Apply morphologic erosion to shrink it into the size (ShadowWndSize - Sharpness)
		// Apply modified (with blur effect) morphologic dilation to make the blurred border
		// The algorithm is optimized by assuming parent window is just "one piece" and without "wholes" on it

		RECT rcRegion = { 0, 0, rcParent->right - rcParent->left, rcParent->bottom - rcParent->top };

		// Get the region of parent window,
		HRGN hParentRgn = CreateRectRgn(rcRegion.left, rcRegion.top, rcRegion.right, rcRegion.bottom);
		GetWindowRgn(hParent, hParentRgn);

		// Determine the Start and end point of each horizontal scan line
		SIZE szParent = { rcParent->right - rcParent->left, rcParent->bottom - rcParent->top };
		SIZE szShadow = { szParent.cx + 2 * nSize, szParent.cy + 2 * nSize };
		// Extra 2 lines (set to be empty) in ptAnchors are used in dilation
		int nAnchors = max(szParent.cy, szShadow.cy);    // # of anchor points pares
		int(*ptAnchors)[2] = new int[nAnchors + 2][2];
		int(*ptAnchorsOri)[2] = new int[szParent.cy][2];    // anchor points, will not modify during erosion
		ptAnchors[0][0] = szParent.cx;
		ptAnchors[0][1] = 0;
		ptAnchors[nAnchors + 1][0] = szParent.cx;
		ptAnchors[nAnchors + 1][1] = 0;
		if (nSize > 0)
		{
			// Put the parent window anchors at the center
			for (int i = 0; i < nSize; i++)
			{
				ptAnchors[i + 1][0] = szParent.cx;
				ptAnchors[i + 1][1] = 0;
				ptAnchors[szShadow.cy - i][0] = szParent.cx;
				ptAnchors[szShadow.cy - i][1] = 0;
			}
			ptAnchors += nSize;
		}
		for (int i = 0; i < szParent.cy; i++)
		{
			// find start point
			int j;
			for (j = 0; j < szParent.cx; j++)
			{
				if (PtInRegion(hParentRgn, j, i))
				{
					ptAnchors[i + 1][0] = j + nSize;
					ptAnchorsOri[i][0] = j;
					break;
				}
			}

			if (j >= szParent.cx)    // Start point not found
			{
				ptAnchors[i + 1][0] = szParent.cx;
				ptAnchorsOri[i][1] = 0;
				ptAnchors[i + 1][0] = szParent.cx;
				ptAnchorsOri[i][1] = 0;
			}
			else
			{
				// find end point
				for (j = szParent.cx - 1; j >= ptAnchors[i + 1][0]; j--)
				{
					if (PtInRegion(hParentRgn, j, i))
					{
						ptAnchors[i + 1][1] = j + nSize;
						ptAnchorsOri[i][1] = j + 1;
						break;
					}
				}
			}
		}

		if (nSize > 0)
		{
			ptAnchors -= nSize;    // Restore pos of ptAnchors for erosion
		}
		int(*ptAnchorsTmp)[2] = new int[nAnchors + 2][2];    // Store the result of erosion
		// First and last line should be empty
		ptAnchorsTmp[0][0] = szParent.cx;
		ptAnchorsTmp[0][1] = 0;
		ptAnchorsTmp[nAnchors + 1][0] = szParent.cx;
		ptAnchorsTmp[nAnchors + 1][1] = 0;
		int nEroTimes = 0;
		// morphologic erosion
		for (int i = 0; i < nSharpness - nSize; i++)
		{
			nEroTimes++;
			//ptAnchorsTmp[1][0] = szParent.cx;
			//ptAnchorsTmp[1][1] = 0;
			//ptAnchorsTmp[szParent.cy + 1][0] = szParent.cx;
			//ptAnchorsTmp[szParent.cy + 1][1] = 0;
			for (int j = 1; j < nAnchors + 1; j++)
			{
				ptAnchorsTmp[j][0] = max(ptAnchors[j - 1][0], max(ptAnchors[j][0], ptAnchors[j + 1][0])) + 1;
				ptAnchorsTmp[j][1] = min(ptAnchors[j - 1][1], min(ptAnchors[j][1], ptAnchors[j + 1][1])) - 1;
			}
			// Exchange ptAnchors and ptAnchorsTmp;
			int(*ptAnchorsXange)[2] = ptAnchorsTmp;
			ptAnchorsTmp = ptAnchors;
			ptAnchors = ptAnchorsXange;
		}

		// morphologic dilation
		ptAnchors += (nSize < 0 ? -nSize : 0) + 1;    // now coordinates in ptAnchors are same as in shadow window
		// Generate the kernel
		int nKernelSize = nSize > nSharpness ? nSize : nSharpness;
		int nCenterSize = nSize > nSharpness ? (nSize - nSharpness) : 0;
		UINT32 *pKernel = new UINT32[(2 * nKernelSize + 1) * (2 * nKernelSize + 1)];
		UINT32 *pKernelIter = pKernel;
		for (int i = 0; i <= 2 * nKernelSize; i++)
		{
			for (int j = 0; j <= 2 * nKernelSize; j++)
			{
				double dLength = sqrt((i - nKernelSize) * (i - nKernelSize) + (j - nKernelSize) * (double)(j - nKernelSize));
				if (dLength < nCenterSize)
				{
					*pKernelIter = nDarkness << 24 | PreMultiply(Color, nDarkness);
				}
				else if (dLength <= nKernelSize)
				{
					UINT32 nFactor = ((UINT32)((1 - (dLength - nCenterSize) / (nSharpness + 1)) * nDarkness));
					*pKernelIter = nFactor << 24 | PreMultiply(Color, nFactor);
				}
				else
				{
					*pKernelIter = 0;
				}
				pKernelIter++;
			}
		}
		// Generate blurred border
		for (int i = nKernelSize; i < szShadow.cy - nKernelSize; i++)
		{
			int j = 0;
			if (ptAnchors[i][0] < ptAnchors[i][1])
			{
				// Start of line
				for (j = ptAnchors[i][0];
					j < min(max(ptAnchors[i - 1][0], ptAnchors[i + 1][0]) + 1, ptAnchors[i][1]);
					j++)
				{
					for (int k = 0; k <= 2 * nKernelSize; k++)
					{
						UINT32 *pPixel = pShadBits +
							(szShadow.cy - i - 1 + nKernelSize - k) * szShadow.cx + j - nKernelSize;
						UINT32 *pKernelPixel = pKernel + k * (2 * nKernelSize + 1);
						for (int l = 0; l <= 2 * nKernelSize; l++)
						{
							if (*pPixel < *pKernelPixel)
							{
								*pPixel = *pKernelPixel;
							}
							pPixel++;
							pKernelPixel++;
						}
					}
				}    // for() start of line

				// End of line
				for (j = max(j, min(ptAnchors[i - 1][1], ptAnchors[i + 1][1]) - 1);
					j < ptAnchors[i][1];
					j++)
				{
					for (int k = 0; k <= 2 * nKernelSize; k++)
					{
						UINT32 *pPixel = pShadBits +
							(szShadow.cy - i - 1 + nKernelSize - k) * szShadow.cx + j - nKernelSize;
						UINT32 *pKernelPixel = pKernel + k * (2 * nKernelSize + 1);
						for (int l = 0; l <= 2 * nKernelSize; l++)
						{
							if (*pPixel < *pKernelPixel)
							{
								*pPixel = *pKernelPixel;
							}
							pPixel++;
							pKernelPixel++;
						}
					}
				}    // for() end of line
			}
		}    // for() Generate blurred border

		// Erase unwanted parts and complement missing
		UINT32 clCenter = nDarkness << 24 | PreMultiply(Color, nDarkness);
		for (int i = min(nKernelSize, max(nSize - nyOffset, 0));
			i < max(szShadow.cy - nKernelSize, min(szParent.cy + nSize - nyOffset, szParent.cy + 2 * nSize));
			i++)
		{
			UINT32 *pLine = pShadBits + (szShadow.cy - i - 1) * szShadow.cx;
			if (i - nSize + nyOffset < 0 || i - nSize + nyOffset >= szParent.cy)    // Line is not covered by parent window
			{
				for (int j = ptAnchors[i][0]; j < ptAnchors[i][1]; j++)
				{
					*(pLine + j) = clCenter;
				}
			}
			else
			{
				for (int j = ptAnchors[i][0];
					j < min(ptAnchorsOri[i - nSize + nyOffset][0] + nSize - nxOffset, ptAnchors[i][1]);
					j++)
					*(pLine + j) = clCenter;
				for (int j = max(ptAnchorsOri[i - nSize + nyOffset][0] + nSize - nxOffset, 0);
					j < min((long)ptAnchorsOri[i - nSize + nyOffset][1] + nSize - nxOffset, szShadow.cx);
					j++)
					*(pLine + j) = 0;
				for (int j = max(ptAnchorsOri[i - nSize + nyOffset][1] + nSize - nxOffset, ptAnchors[i][0]);
					j < ptAnchors[i][1];
					j++)
					*(pLine + j) = clCenter;
			}
		}

		// Delete used resources
		delete[](ptAnchors - (nSize < 0 ? -nSize : 0) - 1);
		delete[] ptAnchorsTmp;
		delete[] ptAnchorsOri;
		delete[] pKernel;
		DeleteObject(hParentRgn);
	}

	__inline static HBITMAP CreateShadowBitmap(HWND hParent,
		INT nDarkness = 150,
		INT nSharpness = 5,
		INT nSize = 12,
		INT nxOffset = 5,
		INT nyOffset = 5,
		COLORREF Color = RGB(255, 0, 0))
	{
		RECT rc = { 0 };
		BYTE *pvBits = NULL;    // pointer to DIB section
		HBITMAP hBitmap = NULL;

		int nShadowWindowWidth = 0;
		int nShadowWindowHeight = 0;

		// Create the alpha blending bitmap
		BITMAPINFO bmi = { 0 };    // bitmap header
		HDC hDC = GetDC(hParent);
		WORD wBitsCount = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);

		if (wBitsCount <= 1)
		{
			wBitsCount = 1;
		}
		else if (wBitsCount <= 4)
		{
			wBitsCount = 4;
		}
		else if (wBitsCount <= 8)
		{
			wBitsCount = 8;
		}
		else if (wBitsCount <= 24)
		{
			wBitsCount = 24;
		}
		else
		{
			wBitsCount = 32;
		}

		GetWindowRect(hParent, &rc);

		nShadowWindowWidth = rc.right - rc.left + nSize * 2;
		nShadowWindowHeight = rc.bottom - rc.top + nSize * 2;

		ZeroMemory(&bmi, sizeof(BITMAPINFO));
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = nShadowWindowWidth;
		bmi.bmiHeader.biHeight = nShadowWindowHeight;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = wBitsCount;   // four 8-bit components
		bmi.bmiHeader.biCompression = BI_RGB;
		bmi.bmiHeader.biSizeImage = nShadowWindowWidth * nShadowWindowHeight * 4;

		hBitmap = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void **)&pvBits, NULL, 0);

		ZeroMemory(pvBits, bmi.bmiHeader.biSizeImage);
		MakeShadow((UINT32 *)pvBits, hParent, &rc,
			nDarkness, nSharpness, nSize, nxOffset,
			nyOffset, Color);
		return hBitmap;
	}
	__inline static BOOL SaveHBitmapToFile(HBITMAP hBitmap, const TCHAR *pFileName)
	{
		HDC hDC = NULL;
		//当前分辨率下每象素所占字节数
		WORD wDPIBits = 0;
		//位图中每象素所占字节数
		WORD wBitCount = 0;
		//定义调色板大小
		DWORD dwPaletteSize = 0;
		//位图中像素字节大小
		DWORD dwBmBitsSize = 0;
		//位图文件大小
		DWORD dwDibBitSize = 0;
		//写入文件字节数
		DWORD dwWritten = 0;
		//位图属性结构
		BITMAP bmp = { 0 };
		//位图文件头结构
		BITMAPFILEHEADER bmpfh = { 0 };
		//位图信息头结构
		BITMAPINFOHEADER bmpih = { 0 };
		//指向位图信息头结构
		BITMAPINFOHEADER * pbmpih = NULL;
		//定义文件
		HANDLE hFile = NULL;
		//分配内存句柄
		HGLOBAL hDibBit = NULL;
		//当前调色板句柄
		HPALETTE hPaltte = NULL;
		//备份调色板句柄
		HPALETTE hPaltteBackup = NULL;

		//计算位图文件每个像素所占字节数
		hDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
		wDPIBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);

		if (wDPIBits <= 1)
		{
			wBitCount = 1;
		}
		else if (wDPIBits <= 4)
		{
			wBitCount = 4;
		}
		else if (wDPIBits <= 8)
		{
			wBitCount = 8;
		}
		else if (wDPIBits <= 24)
		{
			wBitCount = 24;
		}
		else
		{
			wBitCount = 32;
		}

		GetObject(hBitmap, sizeof(bmp), (LPSTR)&bmp);
		bmpih.biSize = sizeof(BITMAPINFOHEADER);
		bmpih.biWidth = bmp.bmWidth;
		bmpih.biHeight = bmp.bmHeight;
		bmpih.biPlanes = 1;
		bmpih.biBitCount = wBitCount;
		bmpih.biCompression = BI_RGB;
		bmpih.biSizeImage = 0;
		bmpih.biXPelsPerMeter = 0;
		bmpih.biYPelsPerMeter = 0;
		bmpih.biClrImportant = 0;
		bmpih.biClrUsed = 0;

		dwBmBitsSize = ((bmp.bmWidth * bmpih.biBitCount + 31) / 32) * 4 * bmp.bmHeight;

		//为位图内容分配内存
		hDibBit = GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
		pbmpih = (LPBITMAPINFOHEADER)GlobalLock(hDibBit);
		*pbmpih = bmpih;

		//处理调色板
		hPaltte = (HPALETTE)GetStockObject(DEFAULT_PALETTE);
		if (hPaltte)
		{
			hDC = ::GetDC(NULL);
			hPaltteBackup = ::SelectPalette(hDC, (HPALETTE)hPaltte, FALSE);
			::RealizePalette(hDC);
		}

		//获取该调色板下新的像素值
		GetDIBits(hDC,
			hBitmap,
			0,
			(UINT)bmp.bmHeight,
			((BYTE *)pbmpih + sizeof(BITMAPINFOHEADER) + dwPaletteSize),
			(BITMAPINFO *)pbmpih,
			DIB_RGB_COLORS);

		//恢复调色板
		if (hPaltteBackup)
		{
			::SelectPalette(hDC, (HPALETTE)hPaltteBackup, TRUE);
			::RealizePalette(hDC);
			::ReleaseDC(NULL, hDC);
		}

		//创建位图文件
		hFile = CreateFile((LPCTSTR)pFileName,
			GENERIC_WRITE,
			0,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			//设置位图文件头
			bmpfh.bfType = 0x4D42;   //"BM"
			bmpfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;
			bmpfh.bfReserved1 = 0;
			bmpfh.bfReserved2 = 0;
			bmpfh.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;
			dwDibBitSize = bmpfh.bfSize;

			//写入位图文件头
			WriteFile(hFile, (void *)&bmpfh, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);

			//写入位图文件其余内容
			WriteFile(hFile, (void *)pbmpih, dwDibBitSize, &dwWritten, NULL);

			//清除
			GlobalUnlock(hDibBit);
			GlobalFree(hDibBit);
			hDibBit = NULL;

			//关闭文件
			CloseHandle(hFile);
			hFile = NULL;
		}

		DeleteDC(hDC);
		hDC = NULL;

		return TRUE;
	}
	
	///////////////////////////////////////////////////////////////////////////////

	__inline static std::wstring GetName(__in CComPtr<IAccessible> pAcc, __in CComVariant varChild)
	{
		if (!pAcc)
			return L"";
		CComBSTR bstrName;
		HRESULT hr = pAcc->get_accName(varChild, &bstrName);
		if (FAILED(hr))
			return L"";
		if (!bstrName.m_str)
			return L"<NULL>";
		return bstrName.m_str;
	}

	__inline static std::wstring GetRole(__in CComPtr<IAccessible> pAcc, __in CComVariant varChild)
	{
		if (!pAcc)
			return L"";
		CComVariant varRoleID;
		HRESULT hr = pAcc->get_accRole(varChild, &varRoleID);
		if (FAILED(hr))
			return L"";
		WCHAR sRoleBuff[1024] = { 0 };
		hr = ::GetRoleTextW(varRoleID.lVal, sRoleBuff, 1024);
		if (FAILED(hr))
			return L"";
		return sRoleBuff;
	}
	__inline static std::wstring GetValue(__in CComPtr<IAccessible> pAcc, __in CComVariant varChild)
	{
		if (!pAcc)
			return L"";
		CComBSTR bstrName;
		HRESULT hr = pAcc->get_accValue(varChild, &bstrName);
		if (FAILED(hr))
			return L"";
		if (!bstrName.m_str)
			return L"<NULL>";
		return bstrName.m_str;
	}
	__inline static std::wstring GetDescription(__in CComPtr<IAccessible> pAcc, __in CComVariant varChild)
	{
		if (!pAcc)
			return L"";
		CComBSTR bstrName;
		HRESULT hr = pAcc->get_accDescription(varChild, &bstrName);
		if (FAILED(hr))
			return L"";
		if (!bstrName.m_str)
			return L"<NULL>";
		return bstrName.m_str;
	}

	__inline static HRESULT WalkTreeWithAccessibleChildren(__in CComPtr<IAccessible> pAcc, __in int depth)
	{
		long childCount = 0;
		long returnCount = 0;

		HRESULT hr = pAcc->get_accChildCount(&childCount);

		if (childCount == 0)
			return S_FALSE;

		CComVariant* pArray = new CComVariant[childCount];
		hr = ::AccessibleChildren(pAcc, 0L, childCount, pArray, &returnCount);
		if (FAILED(hr))
			return hr;

		// Iterate through children.
		for (int x = 0; x < returnCount; x++)
		{
			CComVariant vtChild = pArray[x];
			// If it's an accessible object, get the IAccessible, and recurse.
			if (vtChild.vt == VT_DISPATCH)
			{
				CComPtr<IDispatch> pDisp = vtChild.pdispVal;
				CComQIPtr<IAccessible> pAccChild = pDisp;
				if (!pAccChild)
					continue;

				// Print current accessible element
				std::wcout << std::endl;
				for (int y = 0; y < depth; y++)
				{
					std::wcout << L"    ";
				}
				std::wcout << L"* " << GetName(pAccChild, CHILDID_SELF).data() << L"  |  " << GetRole(pAccChild, CHILDID_SELF).data() << L" (Object) | " << GetValue(pAccChild, CHILDID_SELF).data() << L" | " << GetDescription(pAccChild, CHILDID_SELF).data();

				WalkTreeWithAccessibleChildren(pAccChild, depth + 1);
			}
			// Else it's a child element so we have to call accNavigate on the parent,
			//   and we do not recurse because child elements can't have children.
			else
			{
				// Print current accessible element
				std::wcout << std::endl;
				for (int y = 0; y < depth; y++)
				{
					std::wcout << L"    ";
				}
				std::wcout << L"* " << GetName(pAcc, vtChild.lVal).data() << L"  |  " << GetRole(pAcc, vtChild.lVal).data() << " (Child) | " << GetValue(pAcc, vtChild.lVal).data() << L" | " << GetDescription(pAcc, vtChild.lVal).data();
			}
		}
		delete[] pArray;
		return S_OK;
	}

	__inline static int Explorer(HWND hWnd)
	{
		int result = 0;

		CComPtr<IAccessible> pAccMain;
		if (hWnd)
		{
			_tsetlocale(LC_ALL, _T("chs"));

			CoInitializeEx(NULL, COINIT_MULTITHREADED);

			::AccessibleObjectFromWindow(hWnd, OBJID_CLIENT, IID_IAccessible, (void**)(&pAccMain));

			WalkTreeWithAccessibleChildren(pAccMain, 0);

			CoUninitialize();
		}

		return result;
	}

	///////////////////////////////////////////////////////////////////////////////
	__inline static INT_PTR CALLBACK DlgWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_INITDIALOG:
		{

		}
		break;
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDOK:
			{
				EndDialog(hWnd, LOWORD(wParam));
			}
			break;
			case IDCANCEL:
			{
				EndDialog(hWnd, LOWORD(wParam));
			}
			break;
			default:
				break;
			}
		}
		break;
		default:
			break;
		}
		return FALSE;
	}

	class CDlgItemTemplate
	{
	public:

		CDlgItemTemplate(DWORD _dwStyle, DWORD _dwExStyle, WORD _wX, WORD _wY, WORD _wCX, WORD _wCY, WORD _wID, LPCTSTR _tCN, LPCTSTR _tTN, WORD _wCDAIT)
		{
			this->Initialize(_dwStyle, _dwExStyle, _wX, _wY, _wCX, _wCY, _wID, _tCN, _tTN, _wCDAIT);
		}

		void Initialize(DWORD _dwStyle, DWORD _dwExStyle, WORD _wX, WORD _wY, WORD _wCX, WORD _wCY, WORD _wID, LPCTSTR _tCN, LPCTSTR _tTN, WORD _wCDAIT)
		{
			this->dit.style = _dwStyle;
			this->dit.dwExtendedStyle = _dwExStyle;
			this->dit.x = _wX;
			this->dit.y = _wY;
			this->dit.cx = _wCX;
			this->dit.cy = _wCY;
			this->dit.id = _wID;
			this->tCN = _tCN;
			this->tTN = _tTN;
			this->wCDAIT = _wCDAIT;
			this->dwSize = sizeof(DLGITEMTEMPLATE) + sizeof(wCDAIT) + (this->tCN.length() + this->tTN.length() + 2) * sizeof(WCHAR);
		}

	public:
		DWORD dwSize;
		
		DLGITEMTEMPLATE dit;
		TSTRING tCN;
		TSTRING tTN;
		WORD wCDAIT;		
	};

	class CDlgTemplate
	{
	public:

		CDlgTemplate(DWORD _dwStyle, DWORD _dwExStyle, WORD _wCDIT, WORD _wX, WORD _wY, WORD _wCX, WORD _wCY, WORD _wMENU, LPCTSTR _tCN, LPCTSTR _tTN, WORD _wFS, LPCTSTR _tFN, std::vector<CDlgItemTemplate> & _SDITVECTOR)
		{
			this->Initialize(_dwStyle, _dwExStyle, _wCDIT, _wX, _wY, _wCX, _wCY, _wMENU, _tCN, _tTN, _wFS, _tFN, _SDITVECTOR);
		}

		void Initialize(DWORD _dwStyle, DWORD _dwExStyle, WORD _wCDIT, WORD _wX, WORD _wY, WORD _wCX, WORD _wCY, WORD _wMENU, LPCTSTR _tCN, LPCTSTR _tTN, WORD _wFS, LPCTSTR _tFN, std::vector<CDlgItemTemplate> & _SDITVECTOR)
		{
			this->dt.style = _dwStyle;
			this->dt.dwExtendedStyle = _dwExStyle;
			this->dt.cdit = _wCDIT;
			this->dt.x = _wX;
			this->dt.y = _wY;
			this->dt.cx = _wCX;
			this->dt.cy = _wCY;
			this->wMENU = _wMENU;
			this->tCN = _tCN;
			this->tTN = _tTN;
			this->wFS = _wFS;
			this->tFN = _tFN;
			this->dwSize = sizeof(DLGTEMPLATE) + sizeof(wMENU) + sizeof(wFS) + (tCN.length() + tTN.length() + tFN.length() + 3) * sizeof(WCHAR);

			this->dt.cdit = 0;
			std::for_each(_SDITVECTOR.begin(), _SDITVECTOR.end(), [&, this](const std::vector<CDlgItemTemplate>::value_type & it){
				this->dwSize += it.dwSize;
				this->dt.cdit++;
				SDITVECTOR.push_back(it);
			});
		}

	public:
		DWORD dwSize;

		DLGTEMPLATE dt;
		WORD wMENU;
		TSTRING tCN;
		TSTRING tTN;
		WORD wFS;
		TSTRING tFN;

		std::vector<CDlgItemTemplate> SDITVECTOR;
	};
	
	__inline static BYTE * AllocDlgData(CDlgTemplate * pCDT)
	{
		BYTE * pbData = NULL;
		DWORD  dwNext = (0L);

		pbData = (BYTE *)malloc(pCDT->dwSize * sizeof(BYTE)); memset(pbData, 0, pCDT->dwSize);

		memcpy(pbData + dwNext, &pCDT->dt, sizeof(pCDT->dt)); dwNext += sizeof(pCDT->dt);
		memcpy(pbData + dwNext, &pCDT->wMENU, sizeof(pCDT->wMENU));	dwNext += sizeof(pCDT->wMENU);
		memcpy(pbData + dwNext, Convert::TToW(pCDT->tCN).c_str(), (pCDT->tCN.length() + 1) * sizeof(WCHAR)); dwNext += (pCDT->tCN.length() + 1) * sizeof(WCHAR);
		memcpy(pbData + dwNext, Convert::TToW(pCDT->tTN).c_str(), (pCDT->tTN.length() + 1) * sizeof(WCHAR)); dwNext += (pCDT->tTN.length() + 1) * sizeof(WCHAR);
		memcpy(pbData + dwNext, &pCDT->wFS, sizeof(pCDT->wFS)); dwNext += sizeof(pCDT->wFS);
		memcpy(pbData + dwNext, Convert::TToW(pCDT->tFN).c_str(), (pCDT->tFN.length() + 1) * sizeof(WCHAR)); dwNext += (pCDT->tFN.length() + 1) * sizeof(WCHAR);

		for (auto it : pCDT->SDITVECTOR)
		{
			memcpy(pbData + dwNext, &it.dit, sizeof(it.dit)); dwNext += sizeof(it.dit);
			memcpy(pbData + dwNext, Convert::TToW(it.tCN).c_str(), (it.tCN.length() + 1) * sizeof(WCHAR)); dwNext += (it.tCN.length() + 1) * sizeof(WCHAR);
			memcpy(pbData + dwNext, Convert::TToW(it.tTN).c_str(), (it.tTN.length() + 1) * sizeof(WCHAR)); dwNext += (it.tTN.length() + 1) * sizeof(WCHAR);
			memcpy(pbData + dwNext, &it.wCDAIT, sizeof(it.wCDAIT)); dwNext += sizeof(it.wCDAIT);
		}

		return pbData;
	}
	__inline static VOID FreeDlgData(BYTE ** pData)
	{
		if (pData && *pData)
		{
			free((*pData));
			(*pData) = NULL;
		}
	}
	LPWORD lpwAlign(LPWORD lpIn)
	{
		ULONG ul;

		ul = (ULONG)lpIn;
		ul++;
		ul >>= 1;
		ul <<= 1;
		return (LPWORD)ul;
	}
	__inline static std::string InitDlgData(CDlgTemplate & cdt)
	{
		std::string strData((""));
			
		strData.append((const char *)&cdt.dt, sizeof(cdt.dt));
		strData.append((const char *)&cdt.wMENU, sizeof(cdt.wMENU));
		strData.append((const char *)Convert::TToW(cdt.tCN).c_str(), (cdt.tCN.length() + 1) * sizeof(WCHAR));
		strData.append((const char *)Convert::TToW(cdt.tTN).c_str(), (cdt.tTN.length() + 1) * sizeof(WCHAR));
		strData.append((const char *)&cdt.wFS, sizeof(cdt.wFS));
		strData.append((const char *)Convert::TToW(cdt.tFN).c_str(), (cdt.tFN.length() + 1) * sizeof(WCHAR));

		for (auto it : cdt.SDITVECTOR)
		{
			strData.append((const char *)&it.dit, sizeof(it.dit));
			strData.append((const char *)Convert::TToW(it.tCN).c_str(), (it.tCN.length() + 1) * sizeof(WCHAR));
			strData.append((const char *)Convert::TToW(it.tTN).c_str(), (it.tTN.length() + 1) * sizeof(WCHAR));
			strData.append((const char *)&it.wCDAIT, sizeof(it.wCDAIT));
		}

		return strData;
	}
	__inline static void * InitParams()
	{		
		DLGTEMPLATE dt = { 0 };
		DLGITEMTEMPLATE dit = { 0 };
		BYTE * pbData = NULL;
		SIZE_T stSize = 0L;
		SIZE_T stPlusSize = 0L;
		WORD wMenu = 0;
		WORD wFontSize = 0;
		WORD wCdit = 0;
		WCHAR wClassName[MAX_PATH] = { 0 };
		WCHAR wTitleName[MAX_PATH] = { 0 };
		WCHAR wFontName[MAX_PATH] = { 0 };
		WORD stChildControlsNum = 2;
		
		stSize = 0;
		stPlusSize = sizeof(DLGTEMPLATE);
		pbData = (BYTE *)realloc(pbData, (stSize + stPlusSize) * sizeof(BYTE));
		dt.style = DS_MODALFRAME | DS_3DLOOK | DS_SETFONT | DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
		dt.dwExtendedStyle = 0;
		dt.cdit = stChildControlsNum;
		dt.x = 0;
		dt.y = 0;
		dt.cx = 278;
		dt.cy = 54;
		memcpy(pbData + stSize, &dt, stPlusSize);
		stSize += stPlusSize;

		wMenu = 0;
		stPlusSize = sizeof(wMenu);
		pbData = (BYTE *)realloc(pbData, (stSize + stPlusSize) * sizeof(BYTE));
		memcpy(pbData + stSize, &wMenu, stPlusSize);
		stSize += stPlusSize;

		wcscpy(wClassName, L"");
		stPlusSize = (wcslen(wClassName) + 1) * sizeof(WCHAR);
		pbData = (BYTE *)realloc(pbData, (stSize + stPlusSize) * sizeof(BYTE));
		memcpy(pbData + stSize, wClassName, stPlusSize);
		stSize += stPlusSize;
		
		wcscpy(wTitleName, L"Zipping");
		stPlusSize = (wcslen(wTitleName) + 1) * sizeof(WCHAR);
		pbData = (BYTE *)realloc(pbData, (stSize + stPlusSize) * sizeof(BYTE));
		memcpy(pbData + stSize, wTitleName, stPlusSize);
		stSize += stPlusSize;

		wFontSize = 8;
		stPlusSize = sizeof(wFontSize);
		pbData = (BYTE *)realloc(pbData, (stSize + stPlusSize) * sizeof(BYTE));
		memcpy(pbData + stSize, &wFontSize, stPlusSize);
		stSize += stPlusSize;

		wcscpy(wFontName, L"MS Sans Serif");
		stPlusSize = (wcslen(wFontName) + 1) * sizeof(WCHAR);
		pbData = (BYTE *)realloc(pbData, (stSize + stPlusSize) * sizeof(BYTE));
		memcpy(pbData + stSize, wFontName, stPlusSize);
		stSize += stPlusSize;

		dit.style = BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE;
		dit.dwExtendedStyle = 0;
		dit.x = 113;
		dit.y = 32;
		dit.cx = 50;
		dit.cy = 14;
		dit.id = IDCANCEL;
		stPlusSize = sizeof(dit);
		pbData = (BYTE *)realloc(pbData, (stSize + stPlusSize) * sizeof(BYTE));
		memcpy(pbData + stSize, &dit, stPlusSize);
		stSize += stPlusSize;

		wcscpy(wClassName, L"Button");
		stPlusSize = (wcslen(wClassName) + 1) * sizeof(WCHAR);
		pbData = (BYTE *)realloc(pbData, (stSize + stPlusSize) * sizeof(BYTE));
		memcpy(pbData + stSize, wClassName, stPlusSize);
		stSize += stPlusSize;

		wcscpy(wTitleName, L"Cancel");
		stPlusSize = (wcslen(wTitleName) + 1) * sizeof(WCHAR);
		pbData = (BYTE *)realloc(pbData, (stSize + stPlusSize) * sizeof(BYTE));
		memcpy(pbData + stSize, wTitleName, stPlusSize);
		stSize += stPlusSize;

		wCdit = 0;
		stPlusSize = sizeof(wCdit);
		pbData = (BYTE *)realloc(pbData, (stSize + stPlusSize) * sizeof(BYTE));
		memcpy(pbData + stSize, &wCdit, stPlusSize);
		stSize += stPlusSize;

		dit.style = WS_CHILD | WS_VISIBLE;
		dit.dwExtendedStyle = 0;
		dit.x = 7;
		dit.y = 7;
		dit.cx = 264;
		dit.cy = 18;
		dit.id = 1;
		stPlusSize = sizeof(dit);
		pbData = (BYTE *)realloc(pbData, (stSize + stPlusSize) * sizeof(BYTE));
		memcpy(pbData + stSize, &dit, stPlusSize);
		stSize += stPlusSize;

		wcscpy(wClassName, L"msctls_progress32");
		stPlusSize = (wcslen(wClassName) + 1) * sizeof(WCHAR);
		pbData = (BYTE *)realloc(pbData, (stSize + stPlusSize) * sizeof(BYTE));
		memcpy(pbData + stSize, wClassName, stPlusSize);
		stSize += stPlusSize;

		wcscpy(wTitleName, L"");
		stPlusSize = (wcslen(wTitleName) + 1) * sizeof(WCHAR);
		pbData = (BYTE *)realloc(pbData, (stSize + stPlusSize) * sizeof(BYTE));
		memcpy(pbData + stSize, wTitleName, stPlusSize);
		stSize += stPlusSize;

		wCdit = 0;
		stPlusSize = sizeof(wCdit);
		pbData = (BYTE *)realloc(pbData, (stSize + stPlusSize) * sizeof(BYTE));
		memcpy(pbData + stSize, &wCdit, stPlusSize);
		stSize += stPlusSize;

		return pbData;
	}

	__inline static INT_PTR ShowDialogBoxSample1()
	{
		HINSTANCE hInstance = NULL;
#pragma pack(push,1)
		struct TDlgItemTemplate { DWORD s, ex; short x, y, cx, cy; WORD id; };
		struct TDlgTemplate { DWORD s, ex; WORD cdit; short x, y, cx, cy; };
		struct TDlgItem1 { TDlgItemTemplate dli; WCHAR wclass[7]; WCHAR title[7]; WORD cdat; };
		struct TDlgItem2 { TDlgItemTemplate dli; WCHAR wclass[18]; WCHAR title[25]; WORD cdat; };
		struct TDlgData  { TDlgTemplate dlt; WORD menu; WCHAR wclass[1]; WCHAR title[8]; WORD fontsize; WCHAR font[14]; TDlgItem1 i1; TDlgItem2 i2; };
		TDlgData dtp = {
			{ DS_MODALFRAME | DS_3DLOOK | DS_SETFONT | DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE, 0, 2, 0, 0, 278, 54 },
			0, L"", L"Zipping", 8, L"MS Sans Serif",
			{ { BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 0, 113, 32, 50, 14, IDCANCEL }, L"BUTTON", L"Cancel", 0 },
			{ { WS_CHILD | WS_VISIBLE, 0, 7, 7, 264, 18, 1 }, L"msctls_progress32", L"WeChatMmopen,Version1.01", 0 } };
#pragma pack(pop)

		hInstance = GetModuleHandle(NULL);

		InitCommonControls();
		int nsize = sizeof(dtp);
		int res = DialogBoxIndirectParam(hInstance, (DLGTEMPLATE*)&dtp, 0, (DLGPROC)DlgWindowProc, (LPARAM)NULL);
		if (res == IDCANCEL) return 0;
		return DialogBoxIndirectParam(hInstance, (DLGTEMPLATE*)&dtp, 0, (DLGPROC)DlgWindowProc, (LPARAM)NULL);
	}

	__inline static INT_PTR ShowDialogBoxSample2()
	{
		INT_PTR nRet = 0;
		SIZE_T stSize = 0;
		BYTE * pbData = NULL;
		HINSTANCE hInstance = NULL;

		CDlgTemplate cdt(DS_MODALFRAME | DS_3DLOOK | DS_SETFONT | DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_VISIBLE, 0, 
			2, 0, 0, 278, 54, 0, _T(""), _T("Zipping"), 8, _T("MS Sans Serif"), 
			std::vector<CDlgItemTemplate>{
			CDlgItemTemplate(BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 0,
			113, 32, 50, 14, IDCANCEL, WC_BUTTON, _T("Cancel"), 0),
			CDlgItemTemplate(WS_CHILD | WS_VISIBLE, 0,
			7, 7, 264, 18, 1, PROGRESS_CLASS, _T("ProgressBar"), 0),
		});
		
		pbData = AllocDlgData(&cdt);

		hInstance = GetModuleHandle(NULL);

		InitCommonControls();

		nRet = DialogBoxIndirectParam(hInstance, (DLGTEMPLATE*)pbData, 0, (DLGPROC)DlgWindowProc, (LPARAM)NULL);

		FreeDlgData(&pbData);

		return nRet;
	}

	__inline static INT_PTR ShowDialogBoxSample3()
	{
		INT_PTR nResult = 0;
		HINSTANCE hInstance = NULL;
		std::string strDlgData((""));

		strDlgData = InitDlgData(
			CDlgTemplate(DS_MODALFRAME | DS_3DLOOK | DS_SETFONT | DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE, 0,
			2, 0, 0, 278, 54, 0, _T(""), _T("Zipping"), 8, _T("MS Sans Serif"), 
			std::vector<CDlgItemTemplate>({
			CDlgItemTemplate(BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 0,
			63, 32, 50, 14, IDOK, WC_BUTTON, _T("Ok"), 0),
			CDlgItemTemplate(BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 0,
			113, 32, 50, 14, IDCANCEL, WC_BUTTON, _T("Cancel"), 0),
			CDlgItemTemplate(WS_CHILD | WS_VISIBLE, 0,
			7, 7, 264, 18, 1, PROGRESS_CLASS, _T("ProgressBar"), 0)
		})));

		hInstance = GetModuleHandle(NULL);

		InitCommonControls();

		nResult = DialogBoxIndirectParam(hInstance, (DLGTEMPLATE*)strDlgData.c_str(), 0, (DLGPROC)DlgWindowProc, (LPARAM)NULL);
		if (nResult != IDCANCEL)
		{
			//nResult = DialogBoxIndirectParam(hInstance, (DLGTEMPLATE*)strDlgData.c_str(), 0, (DLGPROC)DlgWindowProc, (LPARAM)NULL);
		}

		return nResult;
	}
	__inline static INT_PTR ShowDialogBoxSample4(DLGPROC dlgproc = DlgWindowProc)
	{
		INT_PTR nRet = 0;
		SIZE_T stSize = 0;
		BYTE * pbData = NULL;
		HINSTANCE hInstance = NULL;

		CDlgTemplate cdt(DS_MODALFRAME | DS_3DLOOK | DS_SETFONT | DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_VISIBLE, 0,
			2, 0, 0, 278, 54, 0, _T(""), _T("Zipping"), 8, _T("MS Sans Serif"), 
			std::vector<CDlgItemTemplate>({
				CDlgItemTemplate(BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 0,
				113, 32, 50, 14, IDCANCEL, WC_BUTTON, _T("Cancel"), 0),
				CDlgItemTemplate(WS_CHILD | WS_VISIBLE, 0,
				7, 7, 264, 18, 1, PROGRESS_CLASS, _T("ProgressBar"), 0),
		}));


		InitCommonControls();

		pbData = AllocDlgData(&cdt);

		hInstance = GetModuleHandle(NULL);

		InitCommonControls();

		nRet = DialogBoxIndirectParam(hInstance, (DLGTEMPLATE*)pbData, 0, (DLGPROC)dlgproc, (LPARAM)NULL);

		FreeDlgData(&pbData);

		return nRet;
	}

	__inline static INT_PTR ShowDialogBoxSample5(DLGPROC dlgproc = DlgWindowProc)
	{
		INT_PTR nResult = 0;
		HINSTANCE hInstance = NULL;
		std::string strDlgData((""));

		InitCommonControls();

		strDlgData = InitDlgData(
			CDlgTemplate(DS_MODALFRAME | DS_3DLOOK | DS_SETFONT | DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE, 0,
			2, 0, 0, 278, 54, 0, _T(""), _T("Zipping"), 8, _T("MS Sans Serif"),
			std::vector<CDlgItemTemplate>({
			CDlgItemTemplate(BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 0,
			63, 32, 50, 14, IDOK, WC_BUTTON, _T("Ok"), 0),
			CDlgItemTemplate(BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE, 0,
			113, 32, 50, 14, IDCANCEL, WC_BUTTON, _T("Cancel"), 0),
			CDlgItemTemplate(WS_CHILD | WS_VISIBLE, 0,
			7, 7, 264, 18, 1, PROGRESS_CLASS, _T("ProgressBar"), 0)
		})));

		hInstance = GetModuleHandle(NULL);

		InitCommonControls();

		nResult = DialogBoxIndirectParam(hInstance, (DLGTEMPLATE*)strDlgData.c_str(), 0, (DLGPROC)dlgproc, (LPARAM)NULL);
		if (nResult != IDCANCEL)
		{
			//nResult = DialogBoxIndirectParam(hInstance, (DLGTEMPLATE*)strDlgData.c_str(), 0, (DLGPROC)dlgproc, (LPARAM)NULL);
		}

		return nResult;
	}

	__inline static INT_PTR DisplayDialogBox(DLGTEMPLATE * pdlgtemplate, DLGPROC dlgproc = DlgWindowProc, HWND hWndParent = NULL, LPARAM lparam = NULL, HINSTANCE hInstance = GetModuleHandle(NULL))
	{
		return ::DialogBoxIndirectParam((HINSTANCE)hInstance, (DLGTEMPLATE*)pdlgtemplate, (HWND)hWndParent, (DLGPROC)dlgproc, (LPARAM)lparam);
	}

	__inline static INT_PTR DisplayDialogBox(CDlgTemplate & cdt, DLGPROC dlgproc = DlgWindowProc, HWND hWndParent = NULL, LPARAM lparam = NULL, HINSTANCE hInstance = GetModuleHandle(NULL))
	{
		return DisplayDialogBox((DLGTEMPLATE*)InitDlgData(cdt).c_str(), (DLGPROC)dlgproc, (HWND)hWndParent, (LPARAM)lparam, (HINSTANCE)hInstance);
	}

	__inline static HWND CreateDialogBox(DLGTEMPLATE * pdlgtemplate, DLGPROC dlgproc = DlgWindowProc, HWND hWndParent = NULL, LPARAM lparam = NULL, HINSTANCE hInstance = GetModuleHandle(NULL))
	{
		return ::CreateDialogIndirectParam((HINSTANCE)hInstance, (DLGTEMPLATE*)pdlgtemplate, (HWND)hWndParent, (DLGPROC)dlgproc, (LPARAM)lparam);
	}

	__inline static HWND CreateDialogBox(CDlgTemplate & cdt, DLGPROC dlgproc = DlgWindowProc, HWND hWndParent = NULL, LPARAM lparam = NULL, HINSTANCE hInstance = GetModuleHandle(NULL))
	{
		return CreateDialogBox((DLGTEMPLATE*)InitDlgData(cdt).c_str(), (DLGPROC)dlgproc, (HWND)hWndParent, (LPARAM)lparam, (HINSTANCE)hInstance);
	}

	__inline static BOOL WindowClassesRegister(HINSTANCE hInstance,
		LPCTSTR lpClassName,
		WNDPROC lpfnWndProc,
		LPCTSTR lpszMenuName = NULL,
		HBRUSH hbrBackground = (HBRUSH)COLOR_BACKGROUND,
		HICON hIcon = LoadIcon(NULL, IDI_APPLICATION),
		HICON hIconSm = LoadIcon(NULL, IDI_APPLICATION),
		HICON hCursor = LoadCursor(NULL, IDC_ARROW),
		UINT uStyle = CS_DBLCLKS,
		INT cbClsExtra = 0,
		INT cbWndExtra = 0
		)
	{
		//Data structure for the windowclass
		WNDCLASSEX wcex = { 0 };

		wcex.cbSize = sizeof(WNDCLASSEX);

		// The Window structure
		wcex.hInstance = hInstance;
		wcex.lpszClassName = lpClassName;
		//This function is called by windows
		wcex.lpfnWndProc = lpfnWndProc;
		//Catch double-clicks
		wcex.style = uStyle;


		// Use default icon and mouse-pointer
		wcex.hIcon = hIcon;
		wcex.hIconSm = hIconSm;
		wcex.hCursor = hCursor;
		//No menu
		wcex.lpszMenuName = lpszMenuName;
		//No extra bytes after the window class
		wcex.cbClsExtra = cbClsExtra;
		//structure or the window instance
		wcex.cbWndExtra = cbWndExtra;
		// Use Windows's default colour as the background of the window
		wcex.hbrBackground = hbrBackground;

		// Register the window class, and if it fails quit the program
		return RegisterClassEx(&wcex);
	}

	__inline static HWND CreateCustomWindow(HINSTANCE hInstance,
		LPCTSTR lpszClassName,
		LPCTSTR lpszTitleName = _T("TemplateWindowsApplication"),
		DWORD dwStyle = WS_OVERLAPPEDWINDOW,
		DWORD dwExtendStyle = 0,
		RECT rcRect = { CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT + 300, CW_USEDEFAULT + 200 },
		HWND hWndParent = HWND_DESKTOP,
		HMENU hMenu = NULL,
		LPVOID lpCreationData = NULL)
	{
		// The class is registered, let's create the program
		return CreateWindowEx(
			dwExtendStyle,// Extended possibilites for variation
			lpszClassName,// Classname
			lpszTitleName,//Title Text
			dwStyle,//default window
			rcRect.left,//Windows decides the position
			rcRect.top,//where the window ends up on the screen
			rcRect.right - rcRect.left,//The programs width
			rcRect.bottom - rcRect.top,//and height in pixels
			hWndParent,//The window is a child-window to desktop
			hMenu,//No menu */
			hInstance,//Program Instance handler
			lpCreationData//No Window Creation data
			);
	}

	__inline static UINT_PTR StartupWindows(HWND hWnd, INT nCmdShow)
	{
		MSG msg = { 0 };

		// Make the window visible on the screen
		::ShowWindow(hWnd, nCmdShow);

		// Update the window visible on the screen
		::UpdateWindow(hWnd);

		// Run the message loop. It will run until GetMessage() returns 0
		while (::GetMessage(&msg, NULL, 0, 0))
		{
			// Translate virtual-key messages into character messages
			::TranslateMessage(&msg);
			// Send message to WindowProcedure
			::DispatchMessage(&msg);
		}

		// The program return-value is 0 - The value that PostQuitMessage() gave
		return msg.wParam;
	}
	
	__inline static UINT_PTR StartupWindowsAsynchromous(HWND hWnd, INT nCmdShow)
	{
		MSG msg = { 0 };

		// Make the window visible on the screen
		::ShowWindow(hWnd, nCmdShow);

		// Update the window visible on the screen
		::UpdateWindow(hWnd);

		// Run the message loop. It will run until PeekMessage() returns 0
		while (1)
		{
			if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				// Translate virtual-key messages into character messages
				::TranslateMessage(&msg);
				// Send message to WindowProcedure
				::DispatchMessage(&msg);
			}
			else
			{
				// Do something
			}
		}
		// The program return-value is 0 - The value that PostQuitMessage() gave
		return msg.wParam;
	}
	
	typedef struct tagEnumWindowInfo
	{
		HWND hWnd;
		DWORD dwPid;
	}ENUMWINDOWINFO, *PENUMWINDOWINFO;

	__inline static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
	{
		ENUMWINDOWINFO* pInfo = (ENUMWINDOWINFO*)lParam;
		DWORD dwProcessId = 0;
		GetWindowThreadProcessId(hWnd, &dwProcessId);

		if (dwProcessId == pInfo->dwPid)
		{
			pInfo->hWnd = hWnd;
			return FALSE;
		}
		return TRUE;
	}

	__inline static void ShowAbout(HWND hWnd = NULL, WORD wIconResId = (0L), LPCTSTR lpWindowInfo = _T("About"), LPCTSTR lpVersionInfo = _T("Version1.0"), LPCTSTR lpCopyrightInfo = _T("Copyright (C) 2018"), LPCTSTR lpContactInfo = _T("Contact:xingyun86"), LPCTSTR lpButtonInfo = _T("确定"))
	{
#define TTMAP_KEY_ICONID			"ICON_ID"
#define TTMAP_KEY_WINDOWINFO		"WINDOW_INFO"
#define TTMAP_KEY_VERSIONINFO		"VERSION_INFO"
#define TTMAP_KEY_COPYRIGHTINFO		"COPYRIGHT_INFO"
#define TTMAP_KEY_CONTACTINFO		"CONTACT_INFO"
#define TTMAP_KEY_OKBUTTON_INFO		"OKBUTTON_INFO"
		TSTRINGTSTRINGMAP ttmap = {
			{ _T(TTMAP_KEY_ICONID), STRING_FORMAT(_T("0x%08X"), wIconResId) },
			{ _T(TTMAP_KEY_WINDOWINFO), lpWindowInfo },
			{ _T(TTMAP_KEY_VERSIONINFO), lpVersionInfo },
			{ _T(TTMAP_KEY_COPYRIGHTINFO), lpCopyrightInfo },
			{ _T(TTMAP_KEY_CONTACTINFO), lpContactInfo },
			{ _T(TTMAP_KEY_OKBUTTON_INFO), lpButtonInfo },
		};
		// 通用控件 初始化 
		InitCommonControls();
		DisplayDialogBox(
			CDlgTemplate(DS_MODALFRAME | DS_FIXEDSYS | DS_3DLOOK | DS_SETFONT | DS_CENTER | WS_BORDER | WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, \
			2, 0, 0, 200, 100, 0, _T(""), _T("ABOUT"), 8, _T("MS Sans Serif"), \
			std::vector<CDlgItemTemplate>{ \
			CDlgItemTemplate(WS_CHILD | WS_VISIBLE | SS_ICON | SS_CENTERIMAGE, 0, \
			14, 14, 21, 20, 100, WC_STATIC, WC_STATIC, 0), \
			CDlgItemTemplate(WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX, 0, \
			42, 14, 198, 8, 101, WC_STATIC, WC_STATIC, 0), \
			CDlgItemTemplate(WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | ES_READONLY, 0, \
			42, 26, 198, 24, 102, WC_EDIT, WC_EDIT, 0), \
			CDlgItemTemplate(WS_CHILD | WS_VISIBLE | SS_LEFT, 0, \
			41, 51, 198, 8, 103, WC_STATIC, WC_STATIC, 0), \
			CDlgItemTemplate(WS_CHILD | WS_VISIBLE | WS_GROUP | BS_DEFPUSHBUTTON, 0, \
			114, 72, 50, 14, IDOK, WC_BUTTON, WC_BUTTON, 0), \
		}),
		(DLGPROC)[](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)->INT_PTR
		{
			switch (uMsg)
			{
			case WM_INITDIALOG:
			{	
				HICON hIcon = NULL;
				TSTRINGTSTRINGMAP * pTTMAP = (TSTRINGTSTRINGMAP*)lParam;
				WORD wIconId = _tcstoul(pTTMAP->at(_T(TTMAP_KEY_ICONID)).c_str(), 0, 16);
				if (IsWindow(GetParent(hWnd)))
				{
					hIcon = (HICON)SendMessage(GetParent(hWnd), WM_GETICON, ICON_BIG, (LPARAM)0L);
					if (!hIcon)
					{
						hIcon = (HICON)SendMessage(GetParent(hWnd), WM_GETICON, ICON_SMALL, (LPARAM)0L);
						if (!hIcon)
						{
							hIcon = (HICON)SendMessage(GetParent(hWnd), WM_GETICON, ICON_SMALL2, (LPARAM)0L);
						}
					}
				}
				else if (wIconId)
				{
					hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(wIconId));
				}
				else
				{

				}
				if (hIcon)
				{
					SNDMSG(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
					SNDMSG(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
					SNDMSG(hWnd, WM_SETICON, ICON_SMALL2, (LPARAM)hIcon);
					SNDMSG(GetDlgItem(hWnd, 100), STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
				}
				SetWindowText(hWnd, pTTMAP->at(_T(TTMAP_KEY_WINDOWINFO)).c_str());
				SetDlgItemText(hWnd, 101, pTTMAP->at(_T(TTMAP_KEY_VERSIONINFO)).c_str());
				SetDlgItemText(hWnd, 102, pTTMAP->at(_T(TTMAP_KEY_COPYRIGHTINFO)).c_str());
				SetDlgItemText(hWnd, 103, pTTMAP->at(_T(TTMAP_KEY_CONTACTINFO)).c_str());
				SetDlgItemText(hWnd, IDOK, pTTMAP->at(_T(TTMAP_KEY_OKBUTTON_INFO)).c_str());
			}
			break;
			case WM_SETFONT:
			{
			}
			break;
			case WM_COMMAND:
			{
				switch (LOWORD(wParam))
				{
				case IDOK:
				case IDCANCEL:
				{
					EndDialog(hWnd, LOWORD(wParam));
				}
				break;
				default:
					break;
				}
			}
			break;
			//case WM_CLOSE:
			//{
			//	PostQuitMessage(LOWORD(wParam));
			//}
			//break;
			default:
				break;
			}
			return FALSE;
		}, hWnd, (LPARAM)&ttmap);
	}
	__inline static void SelectColor()
	{
		CHOOSECOLOR cc;                 // common dialog box structure 
		static COLORREF acrCustClr[16]; // array of custom colors 
		HWND hwnd;                      // owner window
		HBRUSH hbrush;                  // brush handle
		static DWORD rgbCurrent;        // initial color selection

		// Initialize CHOOSECOLOR 
		ZeroMemory(&cc, sizeof(cc));
		cc.lStructSize = sizeof(cc);
		cc.hwndOwner = hwnd;
		cc.lpCustColors = (LPDWORD)acrCustClr;
		cc.rgbResult = rgbCurrent;
		cc.Flags = CC_FULLOPEN | CC_RGBINIT;

		if (ChooseColor(&cc) == TRUE)
		{
			hbrush = CreateSolidBrush(cc.rgbResult);
			rgbCurrent = cc.rgbResult;
		}
	}
	__inline static void SelectFont()
	{
		HWND hwnd;                // owner window
		HDC hdc;                  // display device context of owner window

		CHOOSEFONT cf;            // common dialog box structure
		static LOGFONT lf;        // logical font structure
		static DWORD rgbCurrent;  // current text color
		HFONT hfont, hfontPrev;
		DWORD rgbPrev;

		// Initialize CHOOSEFONT
		ZeroMemory(&cf, sizeof(cf));
		cf.lStructSize = sizeof(cf);
		cf.hwndOwner = hwnd;
		cf.lpLogFont = &lf;
		cf.rgbColors = rgbCurrent;
		cf.Flags = CF_SCREENFONTS | CF_EFFECTS;

		if (ChooseFont(&cf) == TRUE)
		{
			hfont = CreateFontIndirect(cf.lpLogFont);
			hfontPrev = (HFONT)SelectObject(hdc, hfont);
			rgbCurrent = cf.rgbColors;
			rgbPrev = SetTextColor(hdc, rgbCurrent);
		}
	}
	//显示打印对话框
	__inline static void ShowPrintDlg(HWND hwnd = NULL)
	{
		PRINTDLG pd;
		//HWND hwnd = NULL;

		// Initialize PRINTDLG
		ZeroMemory(&pd, sizeof(pd));
		pd.lStructSize = sizeof(pd);
		pd.hwndOwner = hwnd;
		pd.hDevMode = NULL;     // Don't forget to free or store hDevMode.
		pd.hDevNames = NULL;     // Don't forget to free or store hDevNames.
		pd.Flags = PD_USEDEVMODECOPIESANDCOLLATE | PD_RETURNDC;
		pd.nCopies = 1;
		pd.nFromPage = 0xFFFF;
		pd.nToPage = 0xFFFF;
		pd.nMinPage = 1;
		pd.nMaxPage = 0xFFFF;

		if (PrintDlg(&pd) == TRUE)
		{
			// GDI calls to render output. 

			// Delete DC when done.
			DeleteDC(pd.hDC);
		}
	}
	// hWnd is the window that owns the property sheet.
	__inline static HRESULT DisplayPrintPropertySheet(HWND hWnd)
	{
		HRESULT hResult;
		PRINTDLGEX pdx = { 0 };
		LPPRINTPAGERANGE pPageRanges = NULL;

		// Allocate an array of PRINTPAGERANGE structures.
		pPageRanges = (LPPRINTPAGERANGE)GlobalAlloc(GPTR, 10 * sizeof(PRINTPAGERANGE));
		if (!pPageRanges)
			return E_OUTOFMEMORY;

		//  Initialize the PRINTDLGEX structure.
		pdx.lStructSize = sizeof(PRINTDLGEX);
		pdx.hwndOwner = hWnd;
		pdx.hDevMode = NULL;
		pdx.hDevNames = NULL;
		pdx.hDC = NULL;
		pdx.Flags = PD_RETURNDC | PD_COLLATE;
		pdx.Flags2 = 0;
		pdx.ExclusionFlags = 0;
		pdx.nPageRanges = 0;
		pdx.nMaxPageRanges = 10;
		pdx.lpPageRanges = pPageRanges;
		pdx.nMinPage = 1;
		pdx.nMaxPage = 1000;
		pdx.nCopies = 1;
		pdx.hInstance = 0;
		pdx.lpPrintTemplateName = NULL;
		pdx.lpCallback = NULL;
		pdx.nPropertyPages = 0;
		pdx.lphPropertyPages = NULL;
		pdx.nStartPage = START_PAGE_GENERAL;
		pdx.dwResultAction = 0;

		//  Invoke the Print property sheet.

		hResult = PrintDlgEx(&pdx);

		if ((hResult == S_OK) && pdx.dwResultAction == PD_RESULT_PRINT)
		{
			// User clicked the Print button, so use the DC and other information returned in the 
			// PRINTDLGEX structure to print the document.
		}

		if (pdx.hDevMode != NULL)
			GlobalFree(pdx.hDevMode);
		if (pdx.hDevNames != NULL)
			GlobalFree(pdx.hDevNames);
		if (pdx.lpPageRanges != NULL)
			GlobalFree(pPageRanges);

		if (pdx.hDC != NULL)
			DeleteDC(pdx.hDC);

		return hResult;
	}

	__inline static UINT_PTR CALLBACK PaintHook(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LPRECT lprc;
		COLORREF crMargRect;
		HDC hdc, hdcOld;

		switch (uMsg)
		{
			// Draw the margin rectangle. 
		case WM_PSD_MARGINRECT:
			hdc = (HDC)wParam;
			lprc = (LPRECT)lParam;

			// Get the system highlight color. 
			crMargRect = GetSysColor(COLOR_HIGHLIGHT);

			// Create a dash-dot pen of the system highlight color and 
			// select it into the DC of the sample page. 
			hdcOld = (HDC)SelectObject(hdc, CreatePen(PS_DASHDOT, .5, crMargRect));

			// Draw the margin rectangle. 
			Rectangle(hdc, lprc->left, lprc->top, lprc->right, lprc->bottom);

			// Restore the previous pen to the DC. 
			SelectObject(hdc, hdcOld);
			return TRUE;

		default:
			return FALSE;
		}
		return TRUE;
	}

	__inline static void SettingUpPaintPage(LPPAGESETUPHOOK lpPageSetupHook = NULL, LPPAGEPAINTHOOK lpPagePaintHook = &PaintHook)
	{
		PAGESETUPDLG psd;    // common dialog box structure
		HWND hwnd;           // owner window

		// Initialize PAGESETUPDLG
		ZeroMemory(&psd, sizeof(psd));
		psd.lStructSize = sizeof(psd);
		psd.hwndOwner = hwnd;
		psd.hDevMode = NULL; // Don't forget to free or store hDevMode.
		psd.hDevNames = NULL; // Don't forget to free or store hDevNames.
		psd.Flags = PSD_INTHOUSANDTHSOFINCHES | PSD_MARGINS |
			PSD_ENABLEPAGEPAINTHOOK;
		psd.rtMargin.top = 1000;
		psd.rtMargin.left = 1250;
		psd.rtMargin.right = 1250;
		psd.rtMargin.bottom = 1000;
		psd.lpfnPageSetupHook = lpPageSetupHook;
		psd.lpfnPagePaintHook = lpPagePaintHook;

		if (PageSetupDlg(&psd) == TRUE)
		{
			// check paper size and margin values here.
		}
	}
	__inline static void ShowFindTextDlg(HWND hWnd = NULL, LPCTSTR lpWindowInfo = _T("About"))
	{
#define TTMAP_KEY_WINDOWINFO		"WINDOW_INFO"
		static UINT uFindReplaceMsg;  // message identifier for FINDMSGSTRING 
		static HWND hdlg = NULL;     // handle to Find dialog box
		SORTDATAINFO m_sdi;
		static SORTDATAINFO * pSDI = &m_sdi;
		TSTRINGTSTRINGMAP ttmap = {
			{ _T(TTMAP_KEY_WINDOWINFO), lpWindowInfo },
		};
		// 通用控件 初始化 
		InitCommonControls();
		INITCOMMONCONTROLSEX iccex = { 0 };
		iccex.dwSize = sizeof(iccex);
		// 将它设置为包括所有要在应用程序中使用的公共控件类。
		iccex.dwICC = ICC_WIN95_CLASSES;
		::InitCommonControlsEx(&iccex);
		
		DisplayDialogBox(
			CDlgTemplate(DS_MODALFRAME | DS_FIXEDSYS | DS_3DLOOK | DS_SETFONT | DS_CENTER | WS_BORDER | WS_POPUP | WS_MAXIMIZEBOX | WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, \
			2, 0, 0, 200, 100, 0, _T(""), _T("About"), 8, _T("MS Sans Serif"), \
			std::vector<CDlgItemTemplate>{ \
			//CDlgItemTemplate(WS_CHILD | WS_VISIBLE | SS_ICON | SS_CENTERIMAGE, 0, \
			//14, 14, 21, 20, 100, WC_STATIC, _T(""), 0), 
			CDlgItemTemplate(WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | LVS_REPORT | LVS_SINGLESEL, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT, \
			42, 14, 198, 8, 101, WC_LISTVIEW, _T("ListVi"), 0), 
			//CDlgItemTemplate(WS_CHILD | WS_VISIBLE | WS_GROUP | BS_DEFPUSHBUTTON, 0, \
			//114, 72, 50, 14, IDOK, WC_BUTTON, _T("确定"), 0), 
		}),
		(DLGPROC)[](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)->INT_PTR
		{
			switch (uMsg)
			{
			case WM_INITDIALOG:
			{
				//启动查找时打开
				//uFindReplaceMsg = RegisterWindowMessage(FINDMSGSTRING);
				FINDREPLACE fr;       // common dialog box structure
				//HWND hwnd;            // owner window
				_TCHAR szFindWhat[MAXBYTE] = { 0 };  // buffer receiving string
				//HWND hdlg = NULL;     // handle to Find dialog box

				// Initialize FINDREPLACE
				ZeroMemory(&fr, sizeof(fr));
				fr.lStructSize = sizeof(fr);
				fr.hwndOwner = GetDlgItem(hWnd, 101);
				fr.lpstrFindWhat = szFindWhat;
				fr.wFindWhatLen = MAXBYTE;
				fr.Flags = 0;

				//hdlg = FindText(&fr);

				HICON hIcon = NULL;
				TSTRINGTSTRINGMAP * pTTMAP = (TSTRINGTSTRINGMAP*)lParam;
				if (IsWindow(GetParent(hWnd)))
				{
					hIcon = (HICON)SendMessage(GetParent(hWnd), WM_GETICON, ICON_BIG, (LPARAM)0L);
					if (!hIcon)
					{
						hIcon = (HICON)SendMessage(GetParent(hWnd), WM_GETICON, ICON_SMALL, (LPARAM)0L);
						if (!hIcon)
						{
							hIcon = (HICON)SendMessage(GetParent(hWnd), WM_GETICON, ICON_SMALL2, (LPARAM)0L);
						}
					}
				}
				else
				{

				}
				if (hIcon)
				{
					SNDMSG(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
					SNDMSG(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
					SNDMSG(hWnd, WM_SETICON, ICON_SMALL2, (LPARAM)hIcon);
					SNDMSG(GetDlgItem(hWnd, 100), STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
				}
				SetWindowText(hWnd, pTTMAP->at(_T(TTMAP_KEY_WINDOWINFO)).c_str());
				//RECT rc = { 0 };
				//HWND hListWnd = CreateCustomWindow(GetModuleHandle(NULL), WC_LISTVIEW, _T(""), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | LVS_REPORT | LVS_SINGLESEL, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT, rc, hWnd, (HMENU)101);
				TSTRINGVECTOR tv;

				TSTRINGVECTORMAP tvmap = {
					{ _T("进程名称"), tv },
					{ _T("进程ID"), tv },
					{ _T("图标文件"), tv },
				};
				std::map<DWORD, PROCESSENTRY32> pemap;
				std::map<DWORD, PROCESSENTRY32>::iterator itEnd;
				std::map<DWORD, PROCESSENTRY32>::iterator itIdx;
				SystemKernel::EnumProcess_R3(&pemap);
				itEnd = pemap.end();
				itIdx = pemap.begin();

				HIMAGELIST hImageList = ImageList_Create(32, 32, ILC_COLOR8 | ILC_MASK, 3, 1);

				for (; itIdx != itEnd; itIdx++)
				{
					/*TSTRING tsName = _T("");
					_TCHAR tF[MAX_PATH] = { 0 };
					GetProcessFullPath(itIdx->second.th32ProcessID, tF);
					HANDLE h_Process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, itIdx->second.th32ProcessID);

					if (h_Process)
					{
					_TCHAR tFilePath[MAX_PATH] = { 0 };
					::GetModuleFileName((HMODULE)h_Process, tFilePath, MAX_PATH + 1);
					CloseHandle(h_Process);
					}*/
					TSTRING tsFileName = _T("");
					std::map<DWORD, MODULEENTRY32> memap;
					SystemKernel::EnumModules_R3(&memap, itIdx->second.th32ProcessID);
					if (memap.size() && lstrlen(memap.begin()->second.szExePath))
					{
						tsFileName = memap.begin()->second.szExePath;
					}
					else
					{
						if (FilePath::IsFileExistEx((PPSHUAI::FilePath::GetSystemPath() + itIdx->second.szExeFile).c_str()))
						{
							tsFileName = (PPSHUAI::FilePath::GetSystemPath() + itIdx->second.szExeFile);
						}
						else
						{
							tsFileName = itIdx->second.szExeFile;
						}
					}
					tvmap.at(_T("进程名称")).push_back(itIdx->second.szExeFile);
					tvmap.at(_T("进程ID")).push_back(PPSHUAI::STRING_FORMAT(_T("%ld"), itIdx->second.th32ProcessID));
					tvmap.at(_T("图标文件")).push_back(tsFileName);
				}
				GUI::ImageListInit(hImageList, &tvmap, _T("图标文件"));
				ListView_SetImageList(GetDlgItem(hWnd, 101), hImageList, LVSIL_NORMAL);
				ListView_SetImageList(GetDlgItem(hWnd, 101), hImageList, LVSIL_SMALL);
				
				ListCtrlSetSortDataInfo(GetDlgItem(hWnd, 101), pSDI);
				ListCtrlDeleteAllRows(GetDlgItem(hWnd, 101));
				ListCtrlDeleteAllColumns(GetDlgItem(hWnd, 101));
				ListCtrlInsertData(&tvmap,GetDlgItem(hWnd, 101), hImageList);

				MoveWindow(GetDlgItem(hWnd, 101), 0, 0, 200, 100, FALSE);
			}
			break;
			case WM_SETFONT:
			{
			}
			break;
			case WM_SIZE:
			{
				RECT rcWin = { 0 };
				GetClientRect(hWnd, &rcWin);
				HWND hListViewWnd = GetDlgItem(hWnd, 101);
				if (hListViewWnd)
				{
					
					MoveWindow(hListViewWnd, rcWin.left, rcWin.top, rcWin.right - rcWin.left, rcWin.bottom - rcWin.top, FALSE);
				}
				RedrawWindow(hWnd, &rcWin, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_ERASE);
			}
			break;
			case WM_COMMAND:
			{
				switch (LOWORD(wParam))
				{
				case IDOK:
				case IDCANCEL:
				{
					EndDialog(hWnd, LOWORD(wParam));
				}
				break;
				default:
					break;
				}
			}
			break;
			//case WM_CLOSE:
			//{
			//	PostQuitMessage(LOWORD(wParam));
			//}
			//break;
			default:
			{
				LPFINDREPLACE lpfr;

				if (uMsg == uFindReplaceMsg)
				{
					// Get pointer to FINDREPLACE structure from lParam.
					lpfr = (LPFINDREPLACE)lParam;

					// If the FR_DIALOGTERM flag is set, 
					// invalidate the handle that identifies the dialog box. 
					if (lpfr->Flags & FR_DIALOGTERM)
					{
						hdlg = NULL;
						return 0;
					}

					// If the FR_FINDNEXT flag is set, 
					// call the application-defined search routine
					// to search for the requested string. 
					if (lpfr->Flags & FR_FINDNEXT)
					{
						//SearchFile(lpfr->lpstrFindWhat, \
							(BOOL)(lpfr->Flags & FR_DOWN), \
							(BOOL)(lpfr->Flags & FR_MATCHCASE));
					}

					return 0;
				}
			}
				break;
			}
			return FALSE;
		}, hWnd, (LPARAM)&ttmap);
	}
	/*******************************************************
	*函数功能:按照进程ID获取主窗口句柄
	*函数参数:参数1：进程ID
	*函数返回:HWND
	*注意事项:无
	*最后修改时间:2017/5/13
	*******************************************************/
	__inline static HWND GetHwndByProcessId(DWORD dwProcessId)
	{
		ENUMWINDOWINFO info = { 0 };
		info.hWnd = NULL;
		info.dwPid = dwProcessId;
		EnumWindows(EnumWindowsProc, (LPARAM)&info);
		return info.hWnd;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////
	//
	__inline static LONG GetDWORDRegKey(HKEY hKey, const TSTRING &strValueName, DWORD &nValue, DWORD nDefaultValue) {
		nValue = nDefaultValue;
		DWORD dwBufferSize(sizeof(DWORD));
		DWORD nResult(0);
		LONG nError = ::RegQueryValueEx(hKey, strValueName.c_str(), 0, NULL, reinterpret_cast<LPBYTE>(&nResult), &dwBufferSize);
		if (ERROR_SUCCESS == nError) {
			nValue = nResult;
		}

		return nError;
	}
	__inline static LONG GetStringRegKey(HKEY hKey, const TSTRING &strValueName, TSTRING &strValue, TSTRING &strDefaultValue) {
		strValue = strDefaultValue;
		_TCHAR szBuffer[512];
		DWORD dwBufferSize = sizeof(szBuffer);
		ULONG nError;
		nError = RegQueryValueEx(hKey, strValueName.c_str(), 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
		if (ERROR_SUCCESS == nError) {
			strValue = szBuffer;
		}
		return nError;
	}

	__inline static bool SetBrowserFeatureControlKey(TSTRING feature, _TCHAR *appName, DWORD value) {
		HKEY key;
		bool success = true;
		TSTRING featuresPath(_T("Software\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\"));
		TSTRING path(featuresPath + feature);

		LONG nError = RegCreateKeyEx(HKEY_CURRENT_USER, path.c_str(), 0, NULL, REG_OPTION_VOLATILE, KEY_WRITE, NULL, &key, NULL);
		if (nError != ERROR_SUCCESS) {
			success = false;

		}
		else {
			nError = RegSetValueEx(key, appName, 0, REG_DWORD, (const BYTE*)&value, sizeof(value));
			if (nError != ERROR_SUCCESS) {
				success = false;
			}

			nError = RegCloseKey(key);
			if (nError != ERROR_SUCCESS) {
				success = false;
			}
		}

		return success;
	}
	__inline static DWORD GetBrowserEmulationMode() {
		long browserVersion = 8;
		TSTRING sBrowserVersion;
		HKEY key;
		bool success = true;
		TSTRING path(_T("SOFTWARE\\Microsoft\\Internet Explorer"));
		LONG nError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_QUERY_VALUE, &key);
		DWORD mode = 11000; // Internet Explorer 11. Webpages containing standards-based !DOCTYPE directives are displayed in IE11 Standards mode. Default value for Internet Explorer 11.

		if (nError != ERROR_SUCCESS) {
			success = false;
		}
		else {
			nError = GetStringRegKey(key, _T("svcVersion"), sBrowserVersion, TSTRING(_T("8")));
			if (nError != ERROR_SUCCESS) {
				nError = GetStringRegKey(key, _T("version"), sBrowserVersion, TSTRING(_T("8")));
				if (nError != ERROR_SUCCESS) {
					success = false;
				}
			}

			if (RegCloseKey(key) != ERROR_SUCCESS) {
				success = false;
			}
		}

		if (success) {
			TSTRINGVECTOR splittedBrowserVersion;
			PPSHUAI::String::TSTRING_SPLIT_TO_VECTOR(splittedBrowserVersion, sBrowserVersion, _T("."));
			browserVersion = _ttol(splittedBrowserVersion.at(0).c_str()); // convert base 16 number in s to int

			/////////////////////////////////////////////////////////
			//	11001(0×2af9)    IE11 忽略html5
			//	11000(0×2af8)    IE11
			//	10001(0×2711)    IE10 忽略html5
			//	10000(0×2710)    IE10
			//	9999 (0x270F)    IE9  忽略html5
			//	9000 (0×2328)    IE9
			//	8888 (0x22B8)    IE8  忽略html5
			//	8000 (0x1F40)    IE8
			//	7000 (0x1B58)    IE7
			switch (browserVersion) {
			case 7:
				mode = 7000; // Webpages containing standards-based !DOCTYPE directives are displayed in IE7 Standards mode. Default value for applications hosting the WebBrowser Control.
				break;
			case 8:
				mode = 8000; // Webpages containing standards-based !DOCTYPE directives are displayed in IE8 mode. Default value for Internet Explorer 8
				break;
			case 9:
				mode = 9000; // Internet Explorer 9. Webpages containing standards-based !DOCTYPE directives are displayed in IE9 mode. Default value for Internet Explorer 9.
				break;
			case 10:
				mode = 10000; // Internet Explorer 10. Webpages containing standards-based !DOCTYPE directives are displayed in IE10 mode. Default value for Internet Explorer 10.
				break;
			case 11:
				mode = 11000; // Internet Explorer 11. Webpages containing standards-based !DOCTYPE directives are displayed in IE11 mode. Default value for Internet Explorer 11.
				break;
			default:
				// use IE8 mode by default
				mode = 8000; // Internet Explorer 8. Webpages containing standards-based !DOCTYPE directives are displayed in IE8 mode. Default value for Internet Explorer 8.
				break;
			}

		}
		else {
			mode = -1;
		}

		return mode;
	}
	__inline static void SetBrowserFeatureControl() {
		// http://msdn.microsoft.com/en-us/library/ee330720(v=vs.85).aspx
		DWORD emulationMode = GetBrowserEmulationMode();

		if (emulationMode > 0) {
			// FeatureControl settings are per-process
			_TCHAR fileName[MAX_PATH + 1] = { 0 };
			ZeroMemory(fileName, (MAX_PATH + 1) * sizeof(_TCHAR));
			GetModuleFileName(NULL, fileName, 256);
			TSTRINGVECTOR splittedFileName;
			PPSHUAI::String::TSTRING_SPLIT_TO_VECTOR(splittedFileName, fileName, _T("\\"));
			ZeroMemory(fileName, (MAX_PATH + 1) * sizeof(_TCHAR));
			TSTRING exeName = splittedFileName.at(splittedFileName.size() - 1);
			memcpy(fileName, exeName.c_str(), sizeof(_TCHAR) * exeName.length());

			// make the control is not running inside Visual Studio Designer
			//if (String.Compare(fileName, "devenv.exe", true) == 0 || String.Compare(fileName, "XDesProc.exe", true) == 0) {
			//  return;
			//}

			// Windows Internet Explorer 8 and later. The FEATURE_BROWSER_EMULATION feature defines the default emulation mode for Internet
			// Explorer and supports the following values.
			// Webpages containing standards-based !DOCTYPE directives are displayed in IE10 Standards mode.
			SetBrowserFeatureControlKey(_T("FEATURE_BROWSER_EMULATION"), fileName, emulationMode);

			// Internet Explorer 8 or later. The FEATURE_AJAX_CONNECTIONEVENTS feature enables events that occur when the value of the online
			// property of the navigator object changes, such as when the user chooses to work offline. For more information, see the ononline
			// and onoffline events.
			// Default: DISABLED
			SetBrowserFeatureControlKey(_T("FEATURE_AJAX_CONNECTIONEVENTS"), fileName, 1);

			// Internet Explorer 9. Internet Explorer 9 optimized the performance of window-drawing routines that involve clipping regions associated
			// with child windows. This helped improve the performance of certain window drawing operations. However, certain applications hosting the
			// WebBrowser Control rely on the previous behavior and do not function correctly when these optimizations are enabled. The
			// FEATURE_ENABLE_CLIPCHILDREN_OPTIMIZATION feature can disable these optimizations.
			// Default: ENABLED
			// SetBrowserFeatureControlKey(_T("FEATURE_ENABLE_CLIPCHILDREN_OPTIMIZATION"), fileName, 1);

			// Internet Explorer 8 and later. By default, Internet Explorer reduces memory leaks caused by circular references between Internet Explorer
			// and the Microsoft JScript engine, especially in scenarios where a webpage defines an expando and the page is refreshed. If a legacy
			// application no longer functions with these changes, the FEATURE_MANAGE_SCRIPT_CIRCULAR_REFS feature can disable these improvements.
			// Default: ENABLED
			// SetBrowserFeatureControlKey(_T("FEATURE_MANAGE_SCRIPT_CIRCULAR_REFS"), fileName, 1);

			// Windows Internet Explorer 8. When enabled, the FEATURE_DOMSTORAGE feature allows Internet Explorer and applications hosting the WebBrowser
			// Control to use the Web Storage API. For more information, see Introduction to Web Storage.
			// Default: ENABLED
			// SetBrowserFeatureControlKey(_T("FEATURE_DOMSTORAGE"), fileName, 1);

			// Internet Explorer 9. The FEATURE_GPU_RENDERING feature enables Internet Explorer to use a graphics processing unit (GPU) to render content.
			// This dramatically improves performance for webpages that are rich in graphics.
			// Default: DISABLED
			SetBrowserFeatureControlKey(_T("FEATURE_GPU_RENDERING"), fileName, 1);

			// Internet Explorer 9. By default, the WebBrowser Control uses Microsoft DirectX to render webpages, which might cause problems for
			// applications that use the Draw method to create bitmaps from certain webpages. In Internet Explorer 9, this method returns a bitmap
			// (in a Windows Graphics Device Interface (GDI) wrapper) instead of a GDI metafile representation of the webpage. When the
			// FEATURE_IVIEWOBJECTDRAW_DMLT9_WITH_GDI feature is enabled, the following conditions cause the Draw method to use GDI instead of DirectX
			// to create the resulting representation. The GDI representation will contain text records and vector data, but is not guaranteed to be
			// similar to the same represenation returned in earlier versions of the browser:
			//    The device context passed to the Draw method points to an enhanced metafile.
			//    The webpage is not displayed in IE9 Standards mode.
			// By default, this feature is ENABLED for applications hosting the WebBrowser Control. This feature is ignored by Internet Explorer and
			// Windows Explorer. To enable this feature by using the registry, add the name of your executable file to the following setting.
			SetBrowserFeatureControlKey(_T("FEATURE_IVIEWOBJECTDRAW_DMLT9_WITH_GDI"), fileName, 0);

			// Windows 8 introduces a new input model that is different from the Windows 7 input model. In order to provide the broadest compatibility
			// for legacy applications, the WebBrowser Control for Windows 8 emulates the Windows 7 mouse, touch, and pen input model (also known as the
			// legacy input model). When the legacy input model is in effect, the following conditions are true:
			//    Windows pointer messages are not processed by the Trident rendering engine (mshtml.dll).
			//    Document Object Model (DOM) pointer and gesture events do not fire.
			//    Mouse and touch messages are dispatched according to the Windows 7 input model.
			//    Touch selection follows the Windows 7 model ("drag to select") instead of the Windows 8 model ("tap to select").
			//    Hardware accelerated panning and zooming is disabled.
			//    The Zoom and Pan Cascading Style Sheets (CSS) properties are ignored.
			// The FEATURE_NINPUT_LEGACYMODE feature control determines whether the legacy input model is enabled
			// Default: ENABLED
			SetBrowserFeatureControlKey(_T("FEATURE_NINPUT_LEGACYMODE"), fileName, 0);

			// Internet Explorer 7 consolidated HTTP compression and data manipulation into a centralized component in order to improve performance and
			// to provide greater consistency between transfer encodings (such as HTTP no-cache headers). For compatibility reasons, the original
			// implementation was left in place. When the FEATURE_DISABLE_LEGACY_COMPRESSION feature is disabled, the original compression implementation
			// is used.
			// Default: ENABLED
			// SetBrowserFeatureControlKey(_T("FEATURE_DISABLE_LEGACY_COMPRESSION"), fileName, 1);

			// When the FEATURE_LOCALMACHINE_LOCKDOWN feature is enabled, Internet Explorer applies security restrictions on content loaded from the
			// user's local machine, which helps prevent malicious behavior involving local files:
			//    Scripts, Microsoft ActiveX controls, and binary behaviors are not allowed to run.
			//    Object safety settings cannot be overridden.
			//    Cross-domain data actions require confirmation from the user.
			// Default: DISABLED
			// SetBrowserFeatureControlKey(_T("FEATURE_LOCALMACHINE_LOCKDOWN"), fileName, 0);

			// Internet Explorer 7 and later. When enabled, the FEATURE_BLOCK_LMZ_??? feature allows ??? stored in the Local Machine zone to be
			// loaded only by webpages loaded from the Local Machine zone or by webpages hosted by sites in the Trusted Sites list. For more information,
			// see Security and Compatibility in Internet Explorer 7.
			// Default: DISABLED
			//    FEATURE_BLOCK_LMZ_IMG can block images that try to load from the user's local file system. To opt in, add your process name and set 
			//                          the value to 0x00000001.
			//    FEATURE_BLOCK_LMZ_OBJECT can block objects that try to load from the user's local file system. To opt in, add your process name and
			//                          set the value to 0x00000001.
			//    FEATURE_BLOCK_LMZ_SCRIPT can block script access from the user's local file system. To opt in, add your process name and set the value
			//                          to 0x00000001.
			// SetBrowserFeatureControlKey(_T("FEATURE_BLOCK_LMZ_OBJECT"), fileName, 0);
			// SetBrowserFeatureControlKey(_T("FEATURE_BLOCK_LMZ_OBJECT"), fileName, 0);
			// SetBrowserFeatureControlKey(_T("FEATURE_BLOCK_LMZ_SCRIPT"), fileName, 0);

			// Internet Explorer 8 and later. When enabled, the FEATURE_DISABLE_NAVIGATION_SOUNDS feature disables the sounds played when you open a
			// link in a webpage.
			// Default: DISABLED
			SetBrowserFeatureControlKey(_T("FEATURE_DISABLE_NAVIGATION_SOUNDS"), fileName, 1);

			// Windows Internet Explorer 7 and later. Prior to Internet Explorer 7, href attributes of a objects supported the javascript prototcol;
			// this allowed webpages to execute script when the user clicked a link. For security reasons, this support was disabled in Internet
			// Explorer 7. For more information, see Event 1034 - Cross-Domain Barrier and Script URL Mitigation.
			// When enabled, the FEATURE_SCRIPTURL_MITIGATION feature allows the href attribute of a objects to support the javascript prototcol. 
			// Default: DISABLED
			SetBrowserFeatureControlKey(_T("FEATURE_SCRIPTURL_MITIGATION"), fileName, 1);

			// For Windows 8 and later, the FEATURE_SPELLCHECKING feature controls this behavior for Internet Explorer and for applications hosting
			// the web browser control (WebOC). When fully enabled, this feature automatically corrects grammar issues and identifies misspelled words
			// for the conditions described earlier.
			//    (DWORD) 00000000 - Features are disabled.
			//    (DWORD) 00000001 - Features are enabled for the conditions described earlier. (This is the default value.)
			//    (DWORD) 00000002 - Features are enabled, but only for elements that specifically set the spellcheck attribute to true.
			SetBrowserFeatureControlKey(_T("FEATURE_SPELLCHECKING"), fileName, 0);

			// When enabled, the FEATURE_STATUS_BAR_THROTTLING feature limits the frequency of status bar updates to one update every 200 milliseconds.
			// Default: DISABLED
			SetBrowserFeatureControlKey(_T("FEATURE_STATUS_BAR_THROTTLING"), fileName, 1);

			// Internet Explorer 7 or later. When enabled, the FEATURE_TABBED_BROWSING feature enables tabbed browsing navigation shortcuts and
			// notifications. For more information, see Tabbed Browsing for Developers.
			// Default: DISABLED
			// SetBrowserFeatureControlKey(_T("FEATURE_TABBED_BROWSING"), fileName, 1);

			// When enabled, the FEATURE_VALIDATE_NAVIGATE_URL feature control prevents Windows Internet Explorer from navigating to a badly formed URL.
			// Default: DISABLED
			SetBrowserFeatureControlKey(_T("FEATURE_VALIDATE_NAVIGATE_URL"), fileName, 1);

			// When enabled,the FEATURE_WEBOC_DOCUMENT_ZOOM feature allows HTML dialog boxes to inherit the zoom state of the parent window.
			// Default: DISABLED
			SetBrowserFeatureControlKey(_T("FEATURE_WEBOC_DOCUMENT_ZOOM"), fileName, 1);

			// The FEATURE_WEBOC_POPUPMANAGEMENT feature allows applications hosting the WebBrowser Control to receive the default Internet Explorer
			// pop-up window management behavior.
			// Default: ENABLED
			SetBrowserFeatureControlKey(_T("FEATURE_WEBOC_POPUPMANAGEMENT"), fileName, 0);

			// Applications hosting the WebBrowser Control should ensure that window resizing and movement events are handled appropriately for the
			// needs of the application. By default, these events are ignored if the WebBrowser Control is not hosted in a proper container. When enabled,
			// the FEATURE_WEBOC_MOVESIZECHILD feature allows these events to affect the parent window of the application hosting the WebBrowser Control.
			// Because this can lead to unpredictable results, it is not considered desirable behavior.
			// Default: DISABLED
			// SetBrowserFeatureControlKey(_T("FEATURE_WEBOC_MOVESIZECHILD"), fileName, 0);

			// The FEATURE_ADDON_MANAGEMENT feature enables applications hosting the WebBrowser Control
			// to respect add-on management selections made using the Add-on Manager feature of Internet Explorer.
			// Add-ons disabled by the user or by administrative group policy will also be disabled in applications that enable this feature.
			SetBrowserFeatureControlKey(_T("FEATURE_ADDON_MANAGEMENT"), fileName, 0);

			// Internet Explorer 10. When enabled, the FEATURE_WEBSOCKET feature allows script to create and use WebSocket objects.
			// The WebSocketobject allows websites to request data across domains from your browser by using the WebSocket protocol.
			// Default: ENABLED
			SetBrowserFeatureControlKey(_T("FEATURE_WEBSOCKET"), fileName, 1);

			// When enabled, the FEATURE_WINDOW_RESTRICTIONS feature adds several restrictions to the size and behavior of popup windows:
			//    Popup windows must appear in the visible display area.
			//    Popup windows are forced to have status and address bars.
			//    Popup windows must have minimum sizes.
			//    Popup windows cannot cover important areas of the parent window.
			// When enabled, this feature can be configured differently for each security zone by using the URLACTION_FEATURE_WINDOW_RESTRICTIONS URL
			// action flag. 
			// Default: ENABLED
			SetBrowserFeatureControlKey(_T("FEATURE_WINDOW_RESTRICTIONS"), fileName, 0);

			// Internet Explorer 7 and later. The FEATURE_XMLHTTP feature enables or disables the native XMLHttpRequest object.
			// Default: ENABLED
			// SetBrowserFeatureControlKey(_T("FEATURE_XMLHTTP"), fileName, 1);
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	__inline static CThreadHelper * StartHttpServer(DWORD & dwHttpServPort, LPCSTR lpHttpServPath = ("."), INT nDelayMilliSeconds = 10000 , LPCSTR lpHttp404Pages = ("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nNo data found!\r\n"))
	{
#define THREAD_HTTP_SERV_PATH "THREAD_HTTP_SERV_PATH"
#define THREAD_HTTP_SERV_PORT "THREAD_HTTP_SERV_PORT"
#define THREAD_HTTP_404_ERROR "THREAD_HTTP_404_ERROR"
#define THREAD_HTTP_TIMEDELAY "THREAD_HTTP_TIMEDELAY"

		static std::map<std::string, std::string> ssmap = {
			{ (THREAD_HTTP_SERV_PATH), lpHttpServPath },
			{ (THREAD_HTTP_SERV_PORT), STRING_FORMAT_A("%ld", (dwHttpServPort = GenerateRandom(10000, 60000))).c_str() }, //(rand() % ((60000 - 10000) + 1) + 10000);
		};
		
		putenv(std::string(std::string(THREAD_HTTP_404_ERROR) + "=" + lpHttp404Pages).c_str());
		putenv(std::string(std::string(THREAD_HTTP_TIMEDELAY) + "=" + PPSHUAI::STRING_FORMAT_A(("%d"), nDelayMilliSeconds)).c_str());
		static CThreadHelper threadhelper_httpserver((LPTHREAD_START_ROUTINE)[](LPVOID lpParams)->DWORD
		{
			CThreadHelper * pTH = (CThreadHelper *)lpParams;
			std::map<std::string, std::string> * pSSMAP = (std::map<std::string, std::string> *)pTH->GetThreadParameters();
			struct shttpd_ctx * ctx = 0;
			char cRoots[USHRT_MAX] = { 0 };
			char cPorts[8] = { 0 };
			strcpy(cRoots, (PPSHUAI::Convert::TToA(PPSHUAI::FilePath::GetTempPath()) + pSSMAP->at(THREAD_HTTP_SERV_PATH)).c_str());
			strcpy(cPorts, pSSMAP->at(THREAD_HTTP_SERV_PORT).c_str());
			char *argv[] = { "", 
				"-root", cRoots,
				"-ports", cPorts,
				"-dir_list", "no", 
				"-index_files", "index.html,index.htm,index.php,index.cgi",
				"-cgi_ext", "cgi,pl,php", 
				"-ssi_ext", "shtml,shtm", "-auth_realm", "mydomain.com",
				"-systray", "no", 
				"-threads", "1", 
				NULL };
			int	argc = sizeof(argv) / sizeof(argv[0]) - 1;
			if ((ctx = shttpd_init(argc, argv)) != NULL)
			{
				/////////////////////////////////////////////////////////////////
				// This callback function is used to show how to handle 404 error
				//
				shttpd_handle_error(ctx, 404, 
					[](struct shttpd_arg *arg)->void
				{
					shttpd_printf(arg, "%s", getenv(THREAD_HTTP_404_ERROR));
					arg->flags |= SHTTPD_END_OF_OUTPUT;
				}, NULL);
				while (pTH->IsThreadRunning())
				{
					shttpd_poll(ctx, atoi(getenv(THREAD_HTTP_TIMEDELAY)));
				}

				shttpd_fini(ctx);
				ctx = 0;
			}

			return (EXIT_SUCCESS);
		}, (LPVOID)&ssmap);
		threadhelper_httpserver.Start();
		return &threadhelper_httpserver;
	}
	__inline static void StopHttpServer(CThreadHelper * pTH)
	{
		pTH->Close();
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//

	//////////////////////////////////////////////////////////////////////////
	//    全局宏定义声明区域
	//////////////////////////////////////////////////////////////////////////

#define AX_WINDOWS_CONTAINER_NAME OLESTR("SHELL.EXPLORER.2")

	//////////////////////////////////////////////////////////////////////////
	//    全局变量声明区域
	//////////////////////////////////////////////////////////////////////////
	CComModule _Module;  //使用CComDispatchDriver ATL的智能指针，此处必须声明

	class CWebBrowser{
	public:
		CWebBrowser()
		{
			
		}
		~CWebBrowser()
		{

		}
		__inline static void ATL_INIT()
		{
			//    初始化
			AtlAxWinInit();
		}
		__inline static void ATL_TERM()
		{
			//    释放
			AtlAxWinTerm();			
		}
		__inline HRESULT Create(HWND hParentWnd, LPRECT lpRect, LPCTSTR lpWindowName = _T(""), DWORD dwStyles = 0L, DWORD dwExStyles = 0L, HMENU hMenu = (0L), HINSTANCE hInstance = GetModuleHandle(NULL), LPVOID lpCreateParams = NULL)
		{
			HRESULT hResult = S_FALSE;
			
			//    创建容器			
			m_hWnd = CreateWindowEx(dwExStyles, _T(ATLAXWIN_CLASS), lpWindowName, dwStyles, lpRect->left, lpRect->top, lpRect->right, lpRect->bottom, hParentWnd, hMenu, hInstance, lpCreateParams);
			
			//    创建容器控件
			hResult = CreateControlEx(m_hWnd, AX_WINDOWS_CONTAINER_NAME);
			if (hResult != 0)
			{
				MessageBox(NULL, __TEXT("创建容器控件失败!"), __TEXT("错误"), MB_ICONERROR);
				return hResult;
			}

			//    获取容器控件可用句柄
			hResult = QueryControl(m_hWnd, __uuidof(IWebBrowser2), (LPVOID*)&m_IWebBrowser2);
			if (hResult != 0)
			{
				MessageBox(NULL, __TEXT("获取容器控件可用句柄失败!"), __TEXT("错误"), MB_ICONERROR);
				return hResult;
			}

			//m_IWebBrowser2->put_Silent(TRUE);

			//DisplayWebpage(_T("http://localhost:8888/"));
			return hResult;
		}
		__inline HRESULT SetWebBrowserProperty(DWORD dwFlags = (DOCHOSTUIFLAG_NO3DBORDER | DOCHOSTUIFLAG_ENABLE_FORMS_AUTOCOMPLETE | DOCHOSTUIFLAG_THEME | DOCHOSTUIFLAG_SCROLL_NO | DOCHOSTUIFLAG_DIALOG), VARIANT_BOOL bAllowContextMenu = VARIANT_FALSE)
		{
			CComPtr<IAxWinAmbientDispatch> axAmbientDispatch;
			HRESULT hResult = QueryHost(m_hWnd, IID_IAxWinAmbientDispatch, (LPVOID*)&axAmbientDispatch);
			if (SUCCEEDED(hResult))
			{
				//////////////////////////////////////////////////////////////////////////////////////////
				//禁止右键菜单
				axAmbientDispatch->put_AllowContextMenu(bAllowContextMenu);
				//////////////////////////////////////////////////////////////////////////////////////////
				//DOCHOSTUIFLAG_SCROLL_NO   没有滚动条
				//DOCHOSTUIFLAG_DIALOG		像对话框一样，网页上的东西不可选择
				//DOCHOSTUIFLAG_THEME		XP风格
				axAmbientDispatch->put_DocHostFlags(dwFlags);
				axAmbientDispatch.Release();
			}
			return hResult;
		}
		__inline HRESULT GetWebBrowserProperty(DWORD & dwFlags, VARIANT_BOOL * pbAllowContextMenu = NULL)
		{
			CComPtr<IAxWinAmbientDispatch> axAmbientDispatch;
			HRESULT hResult = QueryHost(m_hWnd, IID_IAxWinAmbientDispatch, (LPVOID*)&axAmbientDispatch);
			if (SUCCEEDED(hResult))
			{
				if (pbAllowContextMenu)
				{
					//////////////////////////////////////////////////////////////////////////////////////////
					//获取右键菜单可用状态
					axAmbientDispatch->get_AllowContextMenu(pbAllowContextMenu);
				}
				
				//////////////////////////////////////////////////////////////////////////////////////////
				//DOCHOSTUIFLAG_SCROLL_NO   没有滚动条
				//DOCHOSTUIFLAG_DIALOG		像对话框一样，网页上的东西不可选择
				//DOCHOSTUIFLAG_THEME		XP风格
				axAmbientDispatch->get_DocHostFlags(&dwFlags);
				axAmbientDispatch.Release();
			}
			return hResult;
		}
		//显示网页
		__inline HRESULT DisplayWebpage(LPCTSTR lpUrl)
		{
			HRESULT hResult = S_FALSE;
			VARIANT l_varMyURL = { 0 };
			VariantInit(&l_varMyURL);
			l_varMyURL.vt = VT_BSTR;
			l_varMyURL.bstrVal = SysAllocString(PPSHUAI::Convert::TToW(lpUrl).c_str());
			if (l_varMyURL.bstrVal)
			{
				hResult = m_IWebBrowser2->Navigate2(&l_varMyURL, 0, 0, 0, 0);
				SysFreeString(l_varMyURL.bstrVal);
			}
			hResult = VariantClear(&l_varMyURL);
			return hResult;
		}
		__inline HWND GetHwnd()
		{
			return m_hWnd;
		}
		__inline CComPtr<IWebBrowser2> GetIWebBrowser2Ptr()
		{
			return m_IWebBrowser2;
		}
	private:
		__inline HRESULT QueryHost(_In_ HWND hWnd,
			_In_ REFIID iid,
			_Outptr_ void** ppUnk)
		{
			ATLASSERT(ppUnk != NULL);
			if (ppUnk == NULL)
				return E_POINTER;
			HRESULT hr;
			*ppUnk = NULL;
			CComPtr<IUnknown> spUnk;
			hr = AtlAxGetHost(hWnd, &spUnk);
			if (SUCCEEDED(hr))
				hr = spUnk->QueryInterface(iid, ppUnk);
			return hr;
		}
		template <class Q>
		__inline HRESULT QueryHost(HWND hWnd, _Outptr_ Q** ppUnk)
		{
			return QueryHost(hWnd, __uuidof(Q), (void**)ppUnk);
		}
		__inline HRESULT QueryControl(
			HWND hWnd,
			_In_ REFIID iid,
			_Outptr_ void** ppUnk)
		{
			ATLASSERT(ppUnk != NULL);
			if (ppUnk == NULL)
				return E_POINTER;
			HRESULT hr;
			*ppUnk = NULL;
			CComPtr<IUnknown> spUnk;
			hr = AtlAxGetControl(hWnd, &spUnk);
			if (SUCCEEDED(hr))
				hr = spUnk->QueryInterface(iid, ppUnk);
			return hr;
		}
		template <class Q>
		__inline HRESULT QueryControl(HWND hWnd, _Outptr_ Q** ppUnk)
		{
			return QueryControl(hWnd, __uuidof(Q), (void**)ppUnk);
		}
		__inline HRESULT SetExternalDispatch(HWND hWnd, _Inout_ IDispatch* pDisp)
		{
			HRESULT hr;
			CComPtr<IAxWinHostWindow> spHost;
			hr = QueryHost(hWnd, __uuidof(IAxWinHostWindow), (void**)&spHost);
			if (SUCCEEDED(hr))
				hr = spHost->SetExternalDispatch(pDisp);
			return hr;
		}
		__inline HRESULT SetExternalUIHandler(HWND hWnd, _Inout_ IDocHostUIHandlerDispatch* pUIHandler)
		{
			HRESULT hr;
			CComPtr<IAxWinHostWindow> spHost;
			hr = QueryHost(hWnd, __uuidof(IAxWinHostWindow), (void**)&spHost);
			if (SUCCEEDED(hr))
				hr = spHost->SetExternalUIHandler(pUIHandler);
			return hr;
		}
		__inline HRESULT CreateControlEx(
			HWND hWnd,
			_In_z_ LPCOLESTR lpszName,
			_Inout_opt_ IStream* pStream = NULL,
			_Outptr_opt_ IUnknown** ppUnkContainer = NULL,
			_Outptr_opt_ IUnknown** ppUnkControl = NULL,
			_In_ REFIID iidSink = IID_NULL,
			_Inout_opt_ IUnknown* punkSink = NULL)
		{
			//ATLASSERT(::IsWindow(m_hWnd));
			// We must have a valid window!

			// Get a pointer to the container object connected to this window
			CComPtr<IAxWinHostWindow> spWinHost;
			HRESULT hr = QueryHost(hWnd, &spWinHost);

			// If QueryHost failed, there is no host attached to this window
			// We assume that the user wants to create a new host and subclass the current window
			if (FAILED(hr))
				return AtlAxCreateControlEx(lpszName, hWnd, pStream, ppUnkContainer, ppUnkControl, iidSink, punkSink);

			// Create the control requested by the caller
			CComPtr<IUnknown> pControl;
			if (SUCCEEDED(hr))
				hr = spWinHost->CreateControlEx(lpszName, hWnd, pStream, &pControl, iidSink, punkSink);

			// Send back the necessary interface pointers
			if (SUCCEEDED(hr))
			{
				if (ppUnkControl)
					*ppUnkControl = pControl.Detach();

				if (ppUnkContainer)
				{
					hr = spWinHost.QueryInterface(ppUnkContainer);
					ATLASSERT(SUCCEEDED(hr)); // This should not fail!
				}
			}

			return hr;
		}

	private:
		HWND m_hWnd;//IWebBrowser2句柄
		CComPtr<IWebBrowser2> m_IWebBrowser2;//IWebBrowser2句柄
	};
}
}

#endif //#ifndef __WINDOWHEADER_H_