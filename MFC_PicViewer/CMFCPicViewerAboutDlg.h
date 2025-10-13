#pragma once
#include "afxdialogex.h"
#include"MyButton.h"
#include"MyStatic.h"

// CMFCPicViewerAboutDlg 对话框

class CMFCPicViewerAboutDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMFCPicViewerAboutDlg)

public:
	CMFCPicViewerAboutDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CMFCPicViewerAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTDIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	HICON m_hIcon;

public:
	CMyButton m_btnExit;
	CMyButton m_btnMax;
	CMyButton m_btnMin;
	// 生成的消息映射函数
	CMyStatic m_myText;
	// 重写字体颜色
	BOOL m_isrestore;
	CRect m_wndRect;//保存窗口尺寸
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	DECLARE_MESSAGE_MAP()
};
