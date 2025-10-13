#include <afxwin.h>
#include "pch.h"
#include "framework.h"
#include "MyOpenCV.h"
#include<opencv2/opencv.hpp>

cv::Mat CMyOpenCV::mat_shift(cv::Mat img, double x, double y, int flags, cv::Scalar fillColor) {
        cv::Mat res;                                // 结果图像
        cv::Mat transform = (cv::Mat_<double>(2, 3) << 1, 0, x, 0, 1, y); // 平移矩阵

        cv::Size res_size = img.size();             // 先复制原尺寸
        res_size.width += x;                    // 宽度扩展平移量
        res_size.height += y;                   // 高度扩展平移量
        warpAffine(img, res, transform, res_size, flags, cv::BORDER_CONSTANT, fillColor); // 执行平移
        return res;                             // 返回平移结果
    }

cv::Mat CMyOpenCV::mat_rotate(cv::Mat img, double rotate_theta, int flags, cv::Scalar fillColor) {
        cv::Mat res;                                // 结果图像
        int w = img.size().width;               // 原图宽
        int h = img.size().height;              // 原图高
        int new_w = ceil(abs(w * cos(rotate_theta)) + abs(h * sin(rotate_theta))); // 计算旋转后所需宽
        int new_h = ceil(abs(w * sin(rotate_theta)) + abs(h * cos(rotate_theta))); // 计算旋转后所需高
        cv::Point2f rotate_center = cv::Point2f(w / 2.0, h / 2.0); // 旋转中心取图像中心
        cv::Mat rotMat = cv::getRotationMatrix2D(rotate_center, rotate_theta, 1.0); // 获取旋转矩阵
        rotMat.at<double>(0, 2) += (new_w - w) / 2.0; // 补平移量使图像居中
        rotMat.at<double>(1, 2) += (new_h - h) / 2.0;
        cv::warpAffine(img, res, rotMat, cv::Size(new_w, new_h), flags, cv::BORDER_CONSTANT, fillColor); // 执行旋转
        return res;                             // 返回旋转结果
    }

cv::Mat CMyOpenCV::mat_scale(cv::Mat img, double scalex, double scaley, int flags, cv::Scalar fillColor) {
        cv::Mat res;                                // 结果图像
        cv::Mat transform = (cv::Mat_<double>(2, 3) << scalex, 0, 0, 0, scaley, 0); // 缩放矩阵

        cv::Size res_size = img.size();             // 先复制原尺寸
        res_size.width = ceil(res_size.width * scalex); // 计算缩放后宽
        res_size.height = ceil(res_size.height * scaley); // 计算缩放后高
        warpAffine(img, res, transform, res_size, flags, cv::BORDER_CONSTANT, fillColor); // 执行缩放
        return res;                             // 返回缩放结果
    }

cv::Mat CMyOpenCV::mat_complex_transitions(cv::Mat img, mat_operate op, int flags, cv::Scalar fillColor) {
        cv::Mat res = mat_shift(img, op.tx, op.ty, flags, fillColor); // 先平移
        res = mat_rotate(res, op.tr, flags, fillColor); // 再旋转
        res = mat_scale(res, op.tsx, op.tsy, flags, fillColor); // 最后缩放
        return res;                             // 返回复合变换结果
    }