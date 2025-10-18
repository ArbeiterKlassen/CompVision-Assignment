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
        Mat dst(src.size(), src.type());      // 创建与 src 同尺寸、同类型的目标矩阵
        int cx = src.cols / 2;                // 频谱中心列坐标
        int cy = src.rows / 2;                // 频谱中心行坐标
        for (int i = 0; i < src.rows; ++i)    // 遍历每一行
        {
            for (int j = 0; j < src.cols; ++j) // 遍历每一列
            {
                int u = j - cx;               // 水平频率：相对于中心的偏移
                int v = i - cy;               // 垂直频率：相对于中心的偏移
                float D_uv = std::sqrt(u * u + v * v); // 当前点到频谱中心的距离
                // 巴特沃斯高通滤波器传递函数：H(u,v) = 1 - 1/(1+(D/D0)^2n)
                float h = 1.0f - 1.0f / (1 + std::pow(D_uv / D0, 2 * n));
                // 将复数频谱的实部、虚部同时乘以滤波器增益
                dst.at<Vec2f>(i, j)[0] = src.at<Vec2f>(i, j)[0] * h;
                dst.at<Vec2f>(i, j)[1] = src.at<Vec2f>(i, j)[1] * h;
            }
        }
        return dst;                           // 返回滤波后的频域矩阵
    }
    static Mat butterworth_lowFilter(const Mat& src, UINT n, double D0)
    {
        Mat dst(src.size(), src.type());      // 创建与 src 同尺寸、同类型的目标矩阵

        int cx = src.cols / 2;                // 频谱中心列坐标
        int cy = src.rows / 2;                // 频谱中心行坐标
        for (int i = 0; i < src.rows; ++i)    // 遍历每一行
        {
            for (int j = 0; j < src.cols; ++j) // 遍历每一列
            {
                int u = j - cx;               // 水平频率：相对于中心的偏移
                int v = i - cy;               // 垂直频率：相对于中心的偏移
                float D_uv = std::sqrt(u * u + v * v); // 当前点到频谱中心的距离
                // 巴特沃斯低通滤波器传递函数：H(u,v) = 1/(1+(D/D0)^2n)
                float h = 1.0f / (1 + std::pow(D_uv / D0, 2 * n));
                // 将复数频谱的实部、虚部同时乘以滤波器增益
                dst.at<Vec2f>(i, j)[0] = src.at<Vec2f>(i, j)[0] * h;
                dst.at<Vec2f>(i, j)[1] = src.at<Vec2f>(i, j)[1] * h;
            }
        }
        return dst;                           // 返回滤波后的频域矩阵
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