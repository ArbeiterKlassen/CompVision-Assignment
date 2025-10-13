#pragma once
#include <afxwin.h>
#include "pch.h"
#include "framework.h"
#include <opencv2/opencv.hpp>

class CImageStatic : public CStatic
{
    DECLARE_DYNAMIC(CImageStatic)

public:
    CImageStatic();
    void SetImage(const cv::Mat& img); // 设置原始图像
    void UpdateDisplay();              // 刷新显示

protected:
    DECLARE_MESSAGE_MAP()

    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);

private:
    cv::Mat m_img;          // 原始图像
    cv::Mat m_display;      // 当前显示的图像
    double  m_scale;        // 当前缩放比例
    CPoint  m_offset;       // 图像偏移（用于拖动）
    BOOL    m_bDragging;
    CPoint  m_ptLast;
};