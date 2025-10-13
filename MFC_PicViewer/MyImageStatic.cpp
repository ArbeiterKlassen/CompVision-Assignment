#include <afxwin.h>
#include "pch.h"
#include "framework.h"
#include "MyImageStatic.h"
#include<cmath>
IMPLEMENT_DYNAMIC(CImageStatic, CStatic)

BEGIN_MESSAGE_MAP(CImageStatic, CStatic)
    ON_WM_MOUSEWHEEL()
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

CImageStatic::CImageStatic() : m_scale(1.0), m_bDragging(FALSE) {}

void CImageStatic::SetImage(const cv::Mat& img)
{
    if (!img.empty())
    {
        m_img = img.clone();
        m_scale = 1.0;
        m_offset = CPoint(0, 0);
        UpdateDisplay();
    }
}

void CImageStatic::UpdateDisplay()
{
    if (m_img.empty()) return;

    CRect rc;
    GetClientRect(&rc);
    int ctrlW = rc.Width();
    int ctrlH = rc.Height();

    int newW = cvRound(m_img.cols * m_scale);
    int newH = cvRound(m_img.rows * m_scale);

    cv::resize(m_img, m_display, cv::Size(newW, newH));

    Invalidate();
}

BOOL CImageStatic::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    if (m_img.empty()) return FALSE;

    double oldScale = m_scale;
    if (zDelta > 0)
        m_scale *= 1.1; // 放大
    else
        m_scale /= 1.1; // 缩小

    // 限制最小 0.1，最大 10 倍
    m_scale = std::max(0.1, std::min(10.0, m_scale));

    UpdateDisplay();
    return TRUE;
}

void CImageStatic::OnPaint()
{
    CPaintDC dc(this);
    CRect rc;
    GetClientRect(&rc);

    if (m_display.empty())
    {
        dc.FillSolidRect(&rc, RGB(30, 34, 39)); // 背景色
        return;
    }

    // 绘制到 DC
    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = m_display.cols;
    bmi.bmiHeader.biHeight = -m_display.rows;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;

    ::StretchDIBits(dc.m_hDC,
        0, 0, m_display.cols, m_display.rows,
        0, 0, m_display.cols, m_display.rows,
        m_display.data, &bmi, DIB_RGB_COLORS, SRCCOPY);
}

void CImageStatic::OnLButtonDown(UINT nFlags, CPoint point)
{
    m_bDragging = TRUE;
    m_ptLast = point;
    SetCapture();
}

void CImageStatic::OnMouseMove(UINT nFlags, CPoint point)
{
    if (m_bDragging)
    {
        int dx = point.x - m_ptLast.x;
        int dy = point.y - m_ptLast.y;
        m_ptLast = point;

        // 可扩展：拖动偏移逻辑
    }
}