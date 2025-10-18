#pragma once
#include "afxdialogex.h"
#include "MyButton.h"
#include "MyStatic.h"
// MFC_PicViewer_OptionDlg 对话框

class MFC_PicViewer_OptionDlg : public CDialogEx
{
	DECLARE_DYNAMIC(MFC_PicViewer_OptionDlg)

public:
	MFC_PicViewer_OptionDlg(CWnd* pParent = nullptr);   // 标准构造函数
	//virtual ~MFC_PicViewer_OptionDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	HICON m_hIcon;
public:
	CMyButton m_btnExit;
	CMyButton m_btnMax;
	CMyButton m_btnMin;
	CMyStatic m_myText;
	CMyStatic m_myText_Analyse_Title;
	BOOL m_isrestore;
	CRect m_wndRect;//保存窗口尺寸
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedButton101();
	afx_msg void OnBnClickedButton102();
	afx_msg void OnBnClickedButton103();
	DECLARE_MESSAGE_MAP()
};
