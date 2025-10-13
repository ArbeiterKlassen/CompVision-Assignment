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
    void SetImage(const cv::Mat& img); // ����ԭʼͼ��
    void UpdateDisplay();              // ˢ����ʾ

protected:
    DECLARE_MESSAGE_MAP()

    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);

private:
    cv::Mat m_img;          // ԭʼͼ��
    cv::Mat m_display;      // ��ǰ��ʾ��ͼ��
    double  m_scale;        // ��ǰ���ű���
    CPoint  m_offset;       // ͼ��ƫ�ƣ������϶���
    BOOL    m_bDragging;
    CPoint  m_ptLast;
};