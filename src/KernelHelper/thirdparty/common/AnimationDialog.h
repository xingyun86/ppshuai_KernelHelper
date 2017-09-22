#pragma once


// CAnimationDialog dialog

class CAnimationDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CAnimationDialog)

public:
	CAnimationDialog(CWnd* pParent = NULL, LPCTSTR pImagesName = _T(""), LPCTSTR pShowTips = _T(""));   // standard constructor
	virtual ~CAnimationDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ANIMATION_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
private:
	HDC m_hRenderDC;//设备环境
	int m_nIndex;//图片动画索引
	int m_nCount;//图片动画个数
	int m_nElapse;//倒计时时间
	RECT m_rect;//窗口大小
	std::wstring m_wstrImagesName;//图片路径前缀
	std::wstring m_wstrShowTips;//显示提示信息

public:
	void StartAnimation();
	void StartAnimation(int nElapse, RECT * pRect);
	void CloseAnimation();
};
