#pragma once
#include <afxwin.h>
#include "pch.h"
#include "framework.h"
#include<opencv2/opencv.hpp>
namespace CMyOpenCV {
    struct mat_operate {                        // 封装一次复合变换的参数
        int tx = 0;                             // 水平平移量（像素）
        int ty = 0;                             // 垂直平移量（像素）
        double tr = 0;                          // 旋转角度（度，正值顺时针）
        double tsx = 0;                         // 水平缩放系数
        double tsy = 0;                         // 垂直缩放系数
        mat_operate(int _tx, int _ty, double _tr, double _tsx, double _tsy) {
            tx = _tx;                           // 初始化平移量
            ty = _ty;
            tr = _tr;                           // 初始化旋转角
            tsx = _tsx;                         // 初始化缩放系数
            tsy = _tsy;
        }
    };

    cv::Mat mat_shift(cv::Mat img, double x = 0, double y = 0, int flags = cv::INTER_LINEAR, cv::Scalar fillColor = cv::Scalar(0, 0, 0));

    cv::Mat mat_rotate(cv::Mat img, double rotate_theta, int flags = cv::INTER_LINEAR, cv::Scalar fillColor = cv::Scalar(0, 0, 0));

    cv::Mat mat_scale(cv::Mat img, double scalex, double scaley, int flags = cv::INTER_LINEAR, cv::Scalar fillColor = cv::Scalar(0, 0, 0));

    cv::Mat mat_complex_transitions(cv::Mat img, mat_operate op, int flags = cv::INTER_LINEAR, cv::Scalar fillColor = cv::Scalar(0, 0, 0));

}