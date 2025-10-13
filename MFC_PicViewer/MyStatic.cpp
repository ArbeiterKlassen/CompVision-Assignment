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
    CPaintDC dc(this); // �豸������

    CRect rect;
    GetClientRect(&rect);

    // ���ñ���ɫ���������һ�£�
    dc.FillSolidRect(&rect, RGB(30, 34, 39)); // ���ɫ����

    // ����������ɫΪ��ɫ
    dc.SetTextColor(RGB(255, 255, 255));
    dc.SetBkMode(TRANSPARENT);

    // ��ȡ�ؼ�����
    CString strText;
    GetWindowText(strText);

    // �������֣����У�
    dc.DrawText(strText, &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
}