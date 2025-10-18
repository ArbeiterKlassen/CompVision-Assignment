#include "pch.h"
#include "framework.h"
#include "afxdialogex.h"
#include "MyFilter.h"
#include <opencv2/opencv.hpp>
#include <stack>
#include <algorithm>
using namespace std;
using namespace cv;
namespace Filters {
    static Mat replicatePad(const Mat& src, int r) {
        Mat dst;
        copyMakeBorder(src, dst, r, r, r, r, BORDER_REPLICATE);
        return dst;
    }
    static Mat maxFilter(const Mat& src, int k) {
        int r = k / 2;
        Mat padded = replicatePad(src, r);
        Mat dst(src.size(), src.type());

        for (int i = 0; i < src.rows; ++i) {
            for (int j = 0; j < src.cols; ++j) {
                Rect roi(j, i, k, k);
                Mat win = padded(roi);
                double mx;
                minMaxLoc(win, nullptr, &mx);
                dst.at<uchar>(i, j) = saturate_cast<uchar>(mx);
            }
        }
        return dst;
    }
    static Mat minFilter(const Mat& src, int k) {
        int r = k / 2;
        Mat padded = replicatePad(src, r);
        Mat dst(src.size(), src.type());
        for (int i = 0; i < src.rows; ++i) {
            for (int j = 0; j < src.cols; ++j) {
                Rect roi(j, i, k, k);
                Mat win = padded(roi);
                double mn;
                minMaxLoc(win, &mn, nullptr);
                dst.at<uchar>(i, j) = saturate_cast<uchar>(mn);
            }
        }
        return dst;
    }
    static Mat medianFilter(const Mat& src, int k) {
        int r = k / 2;
        Mat padded = replicatePad(src, r);
        Mat dst(src.size(), src.type());
        vector<uchar> buf(k * k);
        for (int i = 0; i < src.rows; ++i) {
            for (int j = 0; j < src.cols; ++j) {
                Rect roi(j, i, k, k);
                Mat win = padded(roi);
                int idx = 0;
                for (int y = 0; y < k; ++y)for (int x = 0; x < k; ++x)buf[idx++] = win.at<uchar>(y, x);
                nth_element(buf.begin(), buf.begin() + idx / 2, buf.end());
                dst.at<uchar>(i, j) = buf[idx / 2];
            }
        }
        return dst;
    }
    static Mat meanFilter(const Mat& src, int k) {
        int r = k / 2;
        Mat padded = replicatePad(src, r);
        Mat dst(src.size(), src.type());
        int area = k * k;
        for (int i = 0; i < src.rows; ++i) {
            for (int j = 0; j < src.cols; ++j) {
                Rect roi(j, i, k, k);
                Mat win = padded(roi);
                int sum = 0;
                for (int y = 0; y < k; ++y)for (int x = 0; x < k; ++x)sum += win.at<uchar>(y, x);
                dst.at<uchar>(i, j) = saturate_cast<uchar>(sum / area);
            }
        }
        return dst;
    }
    static Mat butterworth_highFilter(const Mat& src, UINT n, double D0)
    {
        Mat dst(src.size(), src.type());      // ������ src ͬ�ߴ硢ͬ���͵�Ŀ�����
        int cx = src.cols / 2;                // Ƶ������������
        int cy = src.rows / 2;                // Ƶ������������
        for (int i = 0; i < src.rows; ++i)    // ����ÿһ��
        {
            for (int j = 0; j < src.cols; ++j) // ����ÿһ��
            {
                int u = j - cx;               // ˮƽƵ�ʣ���������ĵ�ƫ��
                int v = i - cy;               // ��ֱƵ�ʣ���������ĵ�ƫ��
                float D_uv = std::sqrt(u * u + v * v); // ��ǰ�㵽Ƶ�����ĵľ���
                // ������˹��ͨ�˲������ݺ�����H(u,v) = 1 - 1/(1+(D/D0)^2n)
                float h = 1.0f - 1.0f / (1 + std::pow(D_uv / D0, 2 * n));
                // ������Ƶ�׵�ʵ�����鲿ͬʱ�����˲�������
                dst.at<Vec2f>(i, j)[0] = src.at<Vec2f>(i, j)[0] * h;
                dst.at<Vec2f>(i, j)[1] = src.at<Vec2f>(i, j)[1] * h;
            }
        }
        return dst;                           // �����˲����Ƶ�����
    }
    static Mat butterworth_lowFilter(const Mat& src, UINT n, double D0)
    {
        Mat dst(src.size(), src.type());      // ������ src ͬ�ߴ硢ͬ���͵�Ŀ�����

        int cx = src.cols / 2;                // Ƶ������������
        int cy = src.rows / 2;                // Ƶ������������
        for (int i = 0; i < src.rows; ++i)    // ����ÿһ��
        {
            for (int j = 0; j < src.cols; ++j) // ����ÿһ��
            {
                int u = j - cx;               // ˮƽƵ�ʣ���������ĵ�ƫ��
                int v = i - cy;               // ��ֱƵ�ʣ���������ĵ�ƫ��
                float D_uv = std::sqrt(u * u + v * v); // ��ǰ�㵽Ƶ�����ĵľ���
                // ������˹��ͨ�˲������ݺ�����H(u,v) = 1/(1+(D/D0)^2n)
                float h = 1.0f / (1 + std::pow(D_uv / D0, 2 * n));
                // ������Ƶ�׵�ʵ�����鲿ͬʱ�����˲�������
                dst.at<Vec2f>(i, j)[0] = src.at<Vec2f>(i, j)[0] * h;
                dst.at<Vec2f>(i, j)[1] = src.at<Vec2f>(i, j)[1] * h;
            }
        }
        return dst;                           // �����˲����Ƶ�����
    }
    Mat filterate(Mat input, UINT kernel_size, UINT MODULE) {
        if (kernel_size < 3 || kernel_size % 2 == 0) return Mat();
        Mat gray;
        if (input.channels() == 3) cvtColor(input, gray, COLOR_BGR2GRAY);
        else if (input.channels() == 1) gray = input;
        else return Mat();

        switch (MODULE) {
        case Modules::SPATIAL::MAX:    return maxFilter(gray, kernel_size);
        case Modules::SPATIAL::MIN:    return minFilter(gray, kernel_size);
        case Modules::SPATIAL::MEDIAN: return medianFilter(gray, kernel_size);
        case Modules::SPATIAL::MEAN:   return meanFilter(gray, kernel_size);
        default:                       return Mat();
        }
    }
    Mat filterate(Mat input, UINT butterworth_level, double butterworth_D0, UINT MODULE) {
        switch (MODULE) {
        case Modules::BUTTERWORTH::HIGH: return butterworth_highFilter(input,butterworth_level,butterworth_D0);
        case Modules::BUTTERWORTH::LOW: return butterworth_lowFilter(input,butterworth_level, butterworth_D0);
        default: return Mat();
        }
    };
}