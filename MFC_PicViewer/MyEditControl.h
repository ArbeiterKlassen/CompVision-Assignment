#pragma once
#include <afxwin.h>
#include "pch.h"
#include "framework.h"

class CMyEditControl : public CEdit
{
    DECLARE_DYNAMIC(CMyEditControl)
public:
    CMyEditControl();
    virtual ~CMyEditControl();
    CBrush m_brush;

protected:
    DECLARE_MESSAGE_MAP()
    afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
};