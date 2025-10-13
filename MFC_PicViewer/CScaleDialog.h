#pragma once
#include "afxdialogex.h"


// CScaleDialog 对话框

class CScaleDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CScaleDialog)

public:
	CScaleDialog(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CScaleDialog();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SCALE };
	double m_dScaleX = 1.0;
	double m_dScaleY = 1.0;
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
