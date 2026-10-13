#include "pch.h"
#include "framework.h"
#include "MyFeatureMatch.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>
using namespace std;
using namespace cv;

namespace FeatureMatch_Assist {
	// 按模块创建特征检测器
	Ptr<Feature2D> createDetector(UINT MODULE, int max_features)
	{
		switch (MODULE)
		{
		case FeatureMatch::Modules::ORB_MATCH:   return ORB::create(max_features);
		case FeatureMatch::Modules::SIFT_MATCH:  return SIFT::create(max_features);
		case FeatureMatch::Modules::AKAZE_MATCH: return AKAZE::create();
		default: return Ptr<Feature2D>();
		}
	}

	// ORB、AKAZE 的描述子是二进制串，用汉明距离暴力匹配；SIFT 描述子是浮点向量，用 FLANN 匹配
	Ptr<DescriptorMatcher> createMatcher(UINT MODULE)
	{
		if (MODULE == FeatureMatch::Modules::SIFT_MATCH)
			return makePtr<FlannBasedMatcher>();
		return makePtr<BFMatcher>(NORM_HAMMING);
	}

	// 统一转成三通道 BGR，便于并排绘制
	Mat toBgr(Mat img)
	{
		Mat res;
		if (img.channels() == 1)cvtColor(img, res, COLOR_GRAY2BGR);
		else img.copyTo(res);
		return res;
	}
}

namespace FeatureMatch {
	MatchResult match(Mat img1, Mat img2, UINT MODULE, double ratio, int max_features, double ransac_threshold, int max_draw)
	{
		MatchResult res;
		// 只支持 8 位图像（傅里叶频谱等特殊格式无法提取特征点）
		if (img1.empty() || img2.empty() || img1.depth() != CV_8U || img2.depth() != CV_8U)return res;

		// 图像过大时先等比缩小，避免特征提取过慢、可视化画布过大
		double max_side = 1600.0;
		double scale1 = max_side / std::max(img1.cols, img1.rows);
		double scale2 = max_side / std::max(img2.cols, img2.rows);
		res.scale = std::min(1.0, std::min(scale1, scale2));
		if (res.scale < 1.0)
		{
			resize(img1, img1, Size(), res.scale, res.scale, INTER_AREA);
			resize(img2, img2, Size(), res.scale, res.scale, INTER_AREA);
		}

		Mat gray1, gray2;
		if (img1.channels() == 3)cvtColor(img1, gray1, COLOR_BGR2GRAY);
		else gray1 = img1;
		if (img2.channels() == 3)cvtColor(img2, gray2, COLOR_BGR2GRAY);
		else gray2 = img2;

		// 提取特征点与描述子
		Ptr<Feature2D> detector = FeatureMatch_Assist::createDetector(MODULE, max_features);
		if (detector.empty())return res;
		vector<KeyPoint> keypoints1, keypoints2;
		Mat descriptors1, descriptors2;
		detector->detectAndCompute(gray1, noArray(), keypoints1, descriptors1);
		detector->detectAndCompute(gray2, noArray(), keypoints2, descriptors2);
		res.keypoints1 = (int)keypoints1.size();
		res.keypoints2 = (int)keypoints2.size();
		if (descriptors1.empty() || descriptors2.empty() || keypoints1.size() < 2 || keypoints2.size() < 2)return res;

		// 最近邻匹配：每个特征点取 2 个最近邻
		Ptr<DescriptorMatcher> matcher = FeatureMatch_Assist::createMatcher(MODULE);
		vector<vector<DMatch>> knn_matches;
		matcher->knnMatch(descriptors1, descriptors2, knn_matches, 2);

		// Lowe 比值检验：最近邻距离明显小于次近邻时才认为是可靠匹配
		vector<DMatch> good_matches;
		for (int i = 0; i < (int)knn_matches.size(); ++i)
		{
			if (knn_matches[i].size() >= 2 && knn_matches[i][0].distance < ratio * knn_matches[i][1].distance)
				good_matches.push_back(knn_matches[i][0]);
		}
		res.good_matches = (int)good_matches.size();

		// RANSAC 估计单应矩阵并统计内点：不同角度拍摄时用几何一致性衡量匹配质量
		if (good_matches.size() >= 4)
		{
			vector<Point2f> points1, points2;
			for (int i = 0; i < (int)good_matches.size(); ++i)
			{
				points1.push_back(keypoints1[good_matches[i].queryIdx].pt);
				points2.push_back(keypoints2[good_matches[i].trainIdx].pt);
			}
			vector<char> inlier_mask;
			findHomography(points1, points2, RANSAC, ransac_threshold, inlier_mask);
			for (int i = 0; i < (int)inlier_mask.size(); ++i)
			{
				if (inlier_mask[i])res.inliers++;
			}
		}

		// 可视化：连线太多会糊成一片，只绘制描述子距离最小的若干条
		vector<DMatch> draw_matches = good_matches;
		if (max_draw > 0 && (int)draw_matches.size() > max_draw)
		{
			sort(draw_matches.begin(), draw_matches.end(), [](const DMatch& a, const DMatch& b) { return a.distance < b.distance; });
			draw_matches.resize(max_draw);
		}

		// 两张图像并排，用连线标出匹配的特征点对
		drawMatches(FeatureMatch_Assist::toBgr(img1), keypoints1,
			FeatureMatch_Assist::toBgr(img2), keypoints2,
			draw_matches, res.visualization,
			Scalar::all(-1), Scalar::all(-1), vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
		res.success = !res.visualization.empty();
		return res;
	}
}
