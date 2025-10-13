#pragma once
#include <afxwin.h>
#include "pch.h"
#include "framework.h"

class CMyStatic : public CStatic
{
    DECLARE_DYNAMIC(CMyStatic)

public:
    CMyStatic();
    virtual ~CMyStatic();

protected:
    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnPaint();
};