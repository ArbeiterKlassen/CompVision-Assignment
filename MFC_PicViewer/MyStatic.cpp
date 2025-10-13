#include <afxwin.h>
#include "pch.h"
#include "framework.h"
#include "MyStatic.h"

IMPLEMENT_DYNAMIC(CMyStatic, CStatic)

CMyStatic::CMyStatic() {}
CMyStatic::~CMyStatic() {}

BEGIN_MESSAGE_MAP(CMyStatic, CStatic)
    ON_WM_PAINT()
END_MESSAGE_MAP()

void CMyStatic::OnPaint()
{
    CPaintDC dc(this); // 设备上下文

    CRect rect;
    GetClientRect(&rect);

    // 设置背景色（与你界面一致）
    dc.FillSolidRect(&rect, RGB(30, 34, 39)); // 深灰色背景

    // 设置文字颜色为白色
    dc.SetTextColor(RGB(255, 255, 255));
    dc.SetBkMode(TRANSPARENT);

    // 获取控件文字
    CString strText;
    GetWindowText(strText);

    // 绘制文字（居中）
    dc.DrawText(strText, &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
}