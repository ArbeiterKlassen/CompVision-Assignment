#pragma once

#include "pch.h"
#include "framework.h"
#include <opencv2/opencv.hpp>

// 特征匹配模块：对两张同一物体的不同角度照片提取特征点并进行匹配
namespace FeatureMatch {
	namespace Modules {
		enum {
			ORB_MATCH = 1, SIFT_MATCH, AKAZE_MATCH
		};
	}

	// 一次特征匹配的结果：可视化图像与统计信息
	struct MatchResult {
		cv::Mat visualization;   // 两张图像并排 + 匹配连线
		int keypoints1 = 0;      // 图像 1 提取到的特征点数
		int keypoints2 = 0;      // 图像 2 提取到的特征点数
		int good_matches = 0;    // 通过比值检验的匹配对数
		int inliers = 0;         // RANSAC 单应性内点数
		double scale = 1.0;      // 预处理时的缩放系数（1.0 表示未缩放）
		bool success = false;
	};

	// 对 img1、img2 做特征匹配，返回可视化结果与统计信息
	// max_draw 只限制可视化中绘制的连线数量（按描述子距离取最优），不影响统计结果
	MatchResult match(cv::Mat img1, cv::Mat img2, UINT MODULE, double ratio, int max_features, double ransac_threshold, int max_draw);
}
