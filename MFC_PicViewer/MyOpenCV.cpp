#include <afxwin.h>
#include "pch.h"
#include "framework.h"
#include "MyOpenCV.h"
#include<opencv2/opencv.hpp>

cv::Mat CMyOpenCV::mat_shift(cv::Mat img, double x, double y, int flags, cv::Scalar fillColor) {
        cv::Mat res;                                // ���ͼ��
        cv::Mat transform = (cv::Mat_<double>(2, 3) << 1, 0, x, 0, 1, y); // ƽ�ƾ���

        cv::Size res_size = img.size();             // �ȸ���ԭ�ߴ�
        res_size.width += x;                    // �����չƽ����
        res_size.height += y;                   // �߶���չƽ����
        warpAffine(img, res, transform, res_size, flags, cv::BORDER_CONSTANT, fillColor); // ִ��ƽ��
        return res;                             // ����ƽ�ƽ��
    }

cv::Mat CMyOpenCV::mat_rotate(cv::Mat img, double rotate_theta, int flags, cv::Scalar fillColor) {
        cv::Mat res;                                // ���ͼ��
        int w = img.size().width;               // ԭͼ��
        int h = img.size().height;              // ԭͼ��
        int new_w = ceil(abs(w * cos(rotate_theta)) + abs(h * sin(rotate_theta))); // ������ת�������
        int new_h = ceil(abs(w * sin(rotate_theta)) + abs(h * cos(rotate_theta))); // ������ת�������
        cv::Point2f rotate_center = cv::Point2f(w / 2.0, h / 2.0); // ��ת����ȡͼ������
        cv::Mat rotMat = cv::getRotationMatrix2D(rotate_center, rotate_theta, 1.0); // ��ȡ��ת����
        rotMat.at<double>(0, 2) += (new_w - w) / 2.0; // ��ƽ����ʹͼ�����
        rotMat.at<double>(1, 2) += (new_h - h) / 2.0;
        cv::warpAffine(img, res, rotMat, cv::Size(new_w, new_h), flags, cv::BORDER_CONSTANT, fillColor); // ִ����ת
        return res;                             // ������ת���
    }

cv::Mat CMyOpenCV::mat_scale(cv::Mat img, double scalex, double scaley, int flags, cv::Scalar fillColor) {
        cv::Mat res;                                // ���ͼ��
        cv::Mat transform = (cv::Mat_<double>(2, 3) << scalex, 0, 0, 0, scaley, 0); // ���ž���

        cv::Size res_size = img.size();             // �ȸ���ԭ�ߴ�
        res_size.width = ceil(res_size.width * scalex); // �������ź��
        res_size.height = ceil(res_size.height * scaley); // �������ź��
        warpAffine(img, res, transform, res_size, flags, cv::BORDER_CONSTANT, fillColor); // ִ������
        return res;                             // �������Ž��
    }

cv::Mat CMyOpenCV::mat_complex_transitions(cv::Mat img, mat_operate op, int flags, cv::Scalar fillColor) {
        cv::Mat res = mat_shift(img, op.tx, op.ty, flags, fillColor); // ��ƽ��
        res = mat_rotate(res, op.tr, flags, fillColor); // ����ת
        res = mat_scale(res, op.tsx, op.tsy, flags, fillColor); // �������
        return res;                             // ���ظ��ϱ任���
    }