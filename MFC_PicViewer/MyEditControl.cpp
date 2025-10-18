#include <afxwin.h>
#include "pch.h"
#include "framework.h"
#include "MyEditControl.h"

IMPLEMENT_DYNAMIC(CMyEditControl, CEdit)

CMyEditControl::CMyEditControl() {}
CMyEditControl::~CMyEditControl() {}

BEGIN_MESSAGE_MAP(CMyEditControl, CEdit)
    ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()



HBRUSH CMyEditControl::CtlColor(CDC* pDC, UINT nCtlColor)
{
    pDC->SetTextColor(RGB(255, 255, 255));       // 文字白色
    pDC->SetBkColor(RGB(30, 34, 39));            // 背景深色
    if (!m_brush.m_hObject)
        m_brush.CreateSolidBrush(RGB(30, 34, 39));
    return m_brush;
}