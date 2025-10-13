#pragma once
#include <afxwin.h>
#include "pch.h"
#include "framework.h"
#include<opencv2/opencv.hpp>
namespace CMyOpenCV {
    struct mat_operate {                        // ��װһ�θ��ϱ任�Ĳ���
        int tx = 0;                             // ˮƽƽ���������أ�
        int ty = 0;                             // ��ֱƽ���������أ�
        double tr = 0;                          // ��ת�Ƕȣ��ȣ���ֵ˳ʱ�룩
        double tsx = 0;                         // ˮƽ����ϵ��
        double tsy = 0;                         // ��ֱ����ϵ��
        mat_operate(int _tx, int _ty, double _tr, double _tsx, double _tsy) {
            tx = _tx;                           // ��ʼ��ƽ����
            ty = _ty;
            tr = _tr;                           // ��ʼ����ת��
            tsx = _tsx;                         // ��ʼ������ϵ��
            tsy = _tsy;
        }
    };

    cv::Mat mat_shift(cv::Mat img, double x = 0, double y = 0, int flags = cv::INTER_LINEAR, cv::Scalar fillColor = cv::Scalar(0, 0, 0));

    cv::Mat mat_rotate(cv::Mat img, double rotate_theta, int flags = cv::INTER_LINEAR, cv::Scalar fillColor = cv::Scalar(0, 0, 0));

    cv::Mat mat_scale(cv::Mat img, double scalex, double scaley, int flags = cv::INTER_LINEAR, cv::Scalar fillColor = cv::Scalar(0, 0, 0));

    cv::Mat mat_complex_transitions(cv::Mat img, mat_operate op, int flags = cv::INTER_LINEAR, cv::Scalar fillColor = cv::Scalar(0, 0, 0));

}