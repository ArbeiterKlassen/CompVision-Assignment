#pragma once

#include "pch.h"
#include "framework.h"
#include <opencv2/opencv.hpp>
#include <vector>

// 立体匹配模块：SGBM / BM 两路视差计算、无效值掩膜与中值后处理、视差转深度、
// 以及与真值视差图的量化比较（RMSE、bad-1.0 / bad-2.0）。
// 深度换算沿用讲义式 Z = f B / d，深度分辨率 ΔZ = Z^2 Δd / (f B)。
namespace Stereo {

	namespace Modules {
		enum {
			SGBM = 1, BM
		};
	}

	// 匹配参数（与 properties.ini 的 [stereo] 段一一对应）
	struct Params {
		int num_disparities = 64;        // 视差搜索范围，必须为 16 的倍数
		int block_size = 9;              // 匹配窗口大小，奇数
		int min_disparity = 0;           // 最小视差
		int p1 = 0;                      // SGBM 平滑项（0 表示按 blockSize 自动取 8*cn*bs^2）
		int p2 = 0;                      // SGBM 平滑项（0 表示按 blockSize 自动取 32*cn*bs^2）
		int disp12_max_diff = -1;        // 左右一致性检查的最大允许差（SGBM 内置，-1 表示关闭）
		int prefilter_cap = 31;          // 预处理截断值
		int uniqueness_ratio = 10;       // 唯一性比率（百分数）
		int speckle_window_size = 100;   // 斑点滤除窗口
		int speckle_range = 2;           // 斑点滤除视差范围
		int median_size = 5;             // 视差中值滤波窗口（0 表示不做）
		int mode = Modules::SGBM;        // 匹配方法
	};

	// 视差计算结果
	struct DisparityResult {
		cv::Mat disparity;               // CV_32F 视差图，无效点置为无效标记（-1）
		cv::Mat valid_mask;              // CV_8U，255 表示有效
		double valid_ratio = 0.0;         // 有效像素比例
		bool success = false;
	};

	// 与真值比较的量化指标
	struct GTMetrics {
		double rmse = 0.0;               // 有效区域上的 RMSE（像素）
		double bad_1 = 0.0;              // 坏点比例：|d - d_gt| > 1.0
		double bad_2 = 0.0;              // 坏点比例：|d - d_gt| > 2.0
		double valid_ratio = 0.0;        // 本方法有效像素占真值有效像素的比例
		int valid_count = 0;             // 参与统计的像素数
		double median_error = 0.0;       // 误差中位数（抗少量离群点）
	};

	// 视差计算：内部转灰度，按 params.mode 选择 SGBM 或 BM
	DisparityResult computeDisparity(const cv::Mat& left, const cv::Mat& right, const Params& params);

	// 后处理：斑点滤除 + 中值滤波，并重新生成有效掩膜
	DisparityResult postProcess(const DisparityResult& raw, const Params& params, int median_size);

	// 视差转深度：Z = f B / d，返回 CV_32F 深度图（无效点为 0）
	cv::Mat disparityToDepth(const cv::Mat& disparity, double focal, double baseline,
		double& valid_ratio, double min_depth = 0.0, double max_depth = 0.0);

	// 视差图着色（便于出图）
	cv::Mat colorize(const cv::Mat& disparity, double max_disparity);

	// 与真值视差图比较：gt 为真值图，gt_scale 为真值缩放系数（真值 = 像素值 / gt_scale 才等于像素视差）
	GTMetrics compareWithGT(const cv::Mat& disparity, const cv::Mat& gt, double gt_scale);

	// 有效视差的中位数（用于估计真值缩放系数、比较整体量级）
	double medianDisparity(const cv::Mat& disparity, const cv::Mat& mask = cv::Mat());

	// 深度相对误差统计：返回平均相对误差，同时给出参与统计的像素数
	double depthRelativeError(const cv::Mat& depth, const cv::Mat& depth_gt, int& count,
		double min_depth = 0.0, double max_depth = 0.0);
}
