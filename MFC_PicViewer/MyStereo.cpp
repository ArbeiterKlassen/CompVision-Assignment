#include "pch.h"
#include "framework.h"
#include "MyStereo.h"
#include <opencv2/opencv.hpp>
#include <algorithm>
#include <cmath>
using namespace std;
using namespace cv;

namespace Stereo_Assist {
	// 立体匹配要求单通道灰度图
	Mat toGray(const Mat& src)
	{
		Mat gray;
		if (src.channels() == 3)cvtColor(src, gray, COLOR_BGR2GRAY);
		else gray = src.clone();
		return gray;
	}

	// 把 16 位定点视差（低 4 位为小数）转成浮点视差图，并生成有效掩膜
	// 约定：SGBM/BM 匹配失败的位置取值为 (minDisparity-1)，转成浮点后小于 minDisparity，
	// 因此凡是落在 [minDisparity, minDisparity+numDisparities) 之外的都判为无效。
	Stereo::DisparityResult fromFixedPoint(const Mat& disp16, int min_disparity, int num_disparities)
	{
		Stereo::DisparityResult res;
		if (disp16.empty())return res;

		Mat disp;
		disp16.convertTo(disp, CV_32F, 1.0 / 16.0);
		res.valid_mask = Mat::zeros(disp.size(), CV_8U);

		const float lo = float(min_disparity);
		const float hi = float(min_disparity + num_disparities);
		long valid = 0;
		for (int y = 0; y < disp.rows; ++y)
		{
			float* p = disp.ptr<float>(y);
			uchar* m = res.valid_mask.ptr<uchar>(y);
			for (int x = 0; x < disp.cols; ++x)
			{
				if (p[x] > lo && p[x] < hi) { m[x] = 255; ++valid; }
				else p[x] = -1.0f;
			}
		}
		res.disparity = disp;
		res.valid_ratio = double(valid) / double(disp.total());
		res.success = true;
		return res;
	}

	// 视差图的合法范围（与 fromFixedPoint 一致）
	bool inRange(float d, int min_disparity, int num_disparities)
	{
		return d > float(min_disparity) && d < float(min_disparity + num_disparities);
	}
}

namespace Stereo {

	DisparityResult computeDisparity(const Mat& left, const Mat& right, const Params& params)
	{
		DisparityResult res;
		if (left.empty() || right.empty())return res;

		Mat gray_left = Stereo_Assist::toGray(left);
		Mat gray_right = Stereo_Assist::toGray(right);
		if (gray_left.size() != gray_right.size())return res;

		// 参数合法性：视差范围必须是 16 的倍数，匹配窗口必须是奇数
		int num_disparities = params.num_disparities;
		if (num_disparities < 16)num_disparities = 16;
		if (num_disparities % 16 != 0)num_disparities = ((num_disparities / 16) + 1) * 16;
		int block_size = params.block_size;
		if (block_size < 5)block_size = 5;
		if (block_size % 2 == 0)block_size += 1;

		Mat disp16;
		if (params.mode == Modules::BM)
		{
			// 块匹配：速度快，遮挡与弱纹理处空洞多
			Ptr<StereoBM> bm = StereoBM::create(num_disparities, block_size);
			bm->setMinDisparity(params.min_disparity);
			bm->setPreFilterCap(params.prefilter_cap);
			bm->setUniquenessRatio(params.uniqueness_ratio);
			bm->setSpeckleWindowSize(params.speckle_window_size);
			bm->setSpeckleRange(params.speckle_range);
			bm->compute(gray_left, gray_right, disp16);
		}
		else
		{
			// 半全局块匹配：沿 8 个方向做动态规划，弱纹理与倾斜面明显更稳
			// P1、P2 为视差平滑惩罚项，取 0 时按 OpenCV 的推荐式 8*bs^2 与 32*bs^2 自动取值
			int p1 = params.p1 > 0 ? params.p1 : 8 * block_size * block_size;
			int p2 = params.p2 > 0 ? params.p2 : 32 * block_size * block_size;
			Ptr<StereoSGBM> sgbm = StereoSGBM::create(params.min_disparity, num_disparities, block_size,
				p1, p2, params.disp12_max_diff, params.prefilter_cap,
				params.uniqueness_ratio, params.speckle_window_size, params.speckle_range,
				StereoSGBM::MODE_SGBM);
			sgbm->compute(gray_left, gray_right, disp16);
		}

		return Stereo_Assist::fromFixedPoint(disp16, params.min_disparity, num_disparities);
	}

	DisparityResult postProcess(const DisparityResult& raw, const Params& params, int median_size)
	{
		DisparityResult res = raw;
		if (!raw.success || raw.disparity.empty())return res;

		Mat disp = raw.disparity.clone();
		Mat mask = raw.valid_mask.clone();

		// 无效点先置 0，避免把"匹配失败"的大负值当成真实视差参与滤波
		disp.setTo(0.0f, mask == 0);

		// 中值滤波：压制匹配噪声与细碎误匹配
		if (median_size >= 3)
		{
			int k = (median_size % 2 == 0) ? median_size + 1 : median_size;
			medianBlur(disp, disp, k);
		}

		// 重新生成掩膜：滤波会改变数值，超出合法范围的位置重新判为无效
		res.valid_mask = Mat::zeros(disp.size(), CV_8U);
		long valid = 0;
		for (int y = 0; y < disp.rows; ++y)
		{
			float* p = disp.ptr<float>(y);
			uchar* m = res.valid_mask.ptr<uchar>(y);
			for (int x = 0; x < disp.cols; ++x)
			{
				if (Stereo_Assist::inRange(p[x], params.min_disparity, params.num_disparities))
				{
					m[x] = 255;
					++valid;
				}
				else
				{
					p[x] = -1.0f;
				}
			}
		}
		res.disparity = disp;
		res.valid_ratio = double(valid) / double(disp.total());
		return res;
	}

	Mat disparityToDepth(const Mat& disparity, double focal, double baseline, double& valid_ratio,
		double min_depth, double max_depth)
	{
		Mat depth = Mat::zeros(disparity.size(), CV_32F);
		valid_ratio = 0.0;
		if (disparity.empty() || focal <= 0.0 || baseline <= 0.0)return depth;

		long valid = 0;
		for (int y = 0; y < disparity.rows; ++y)
		{
			const float* d = disparity.ptr<float>(y);
			float* z = depth.ptr<float>(y);
			for (int x = 0; x < disparity.cols; ++x)
			{
				// Z = f B / d：视差越大越近
				if (d[x] <= 0.0f)continue;
				double depth_value = focal * baseline / double(d[x]);
				if (min_depth > 0.0 && depth_value < min_depth)continue;
				if (max_depth > 0.0 && depth_value > max_depth)continue;
				z[x] = float(depth_value);
				++valid;
			}
		}
		valid_ratio = double(valid) / double(disparity.total());
		return depth;
	}

	Mat colorize(const Mat& disparity, double max_disparity)
	{
		Mat gray = Mat::zeros(disparity.size(), CV_8U);
		if (disparity.empty())return gray;
		if (max_disparity <= 0.0)max_disparity = 64.0;

		const double scale = 255.0 / max_disparity;
		for (int y = 0; y < disparity.rows; ++y)
		{
			const float* p = disparity.ptr<float>(y);
			uchar* g = gray.ptr<uchar>(y);
			for (int x = 0; x < disparity.cols; ++x)
			{
				if (p[x] <= 0.0f)continue;                     // 无效点留黑
				double v = double(p[x]) * scale;
				g[x] = saturate_cast<uchar>(v);
			}
		}
		Mat color;
		applyColorMap(gray, color, COLORMAP_JET);
		return color;
	}

	GTMetrics compareWithGT(const Mat& disparity, const Mat& gt, double gt_scale)
	{
		GTMetrics metrics;
		if (disparity.empty() || gt.empty() || gt_scale <= 0.0)return metrics;

		// 真值图尺寸与视差图不一致时先按最近邻对齐
		Mat gt_aligned;
		if (gt.size() != disparity.size())resize(gt, gt_aligned, disparity.size(), 0, 0, INTER_NEAREST);
		else gt_aligned = gt;
		Mat gt_gray;
		if (gt_aligned.channels() >= 3)cvtColor(gt_aligned, gt_gray, COLOR_BGR2GRAY);
		else gt_gray = gt_aligned;

		long gt_valid = 0, common_valid = 0, bad1 = 0, bad2 = 0;
		double sum_sq = 0.0;
		vector<float> errors;
		for (int y = 0; y < disparity.rows; ++y)
		{
			const float* d = disparity.ptr<float>(y);
			const uchar* g = gt_gray.ptr<uchar>(y);
			for (int x = 0; x < disparity.cols; ++x)
			{
				double gt_disp = double(g[x]) / gt_scale;      // 真值视差（像素）
				if (gt_disp <= 0.0)continue;                    // 真值无效（遮挡或空洞）
				++gt_valid;

				if (d[x] <= 0.0f)                               // 本方无效：按 Middlebury 惯例计入坏点
				{
					++bad1;
					++bad2;
					continue;
				}
				double err = fabs(double(d[x]) - gt_disp);
				sum_sq += err * err;
				if (err > 1.0)++bad1;
				if (err > 2.0)++bad2;
				errors.push_back(float(err));
				++common_valid;
			}
		}

		if (gt_valid > 0)
		{
			metrics.bad_1 = double(bad1) / double(gt_valid);
			metrics.bad_2 = double(bad2) / double(gt_valid);
			metrics.valid_ratio = double(common_valid) / double(gt_valid);
		}
		if (common_valid > 0)
		{
			metrics.rmse = sqrt(sum_sq / double(common_valid));
			size_t mid = errors.size() / 2;
			nth_element(errors.begin(), errors.begin() + mid, errors.end());
			metrics.median_error = double(errors[mid]);
		}
		metrics.valid_count = (int)common_valid;
		return metrics;
	}

	double medianDisparity(const Mat& disparity, const Mat& mask)
	{
		if (disparity.empty())return 0.0;
		vector<float> values;
		for (int y = 0; y < disparity.rows; ++y)
		{
			const float* p = disparity.ptr<float>(y);
			const uchar* m = mask.empty() ? nullptr : mask.ptr<uchar>(y);
			for (int x = 0; x < disparity.cols; ++x)
			{
				if (p[x] <= 0.0f)continue;
				if (m != nullptr && m[x] == 0)continue;
				values.push_back(p[x]);
			}
		}
		if (values.empty())return 0.0;
		size_t mid = values.size() / 2;
		nth_element(values.begin(), values.begin() + mid, values.end());
		return double(values[mid]);
	}

	double depthRelativeError(const Mat& depth, const Mat& depth_gt, int& count,
		double min_depth, double max_depth)
	{
		count = 0;
		if (depth.empty() || depth_gt.empty() || depth.size() != depth_gt.size())return 0.0;

		double sum = 0.0;
		for (int y = 0; y < depth.rows; ++y)
		{
			const float* z = depth.ptr<float>(y);
			const float* zg = depth_gt.ptr<float>(y);
			for (int x = 0; x < depth.cols; ++x)
			{
				if (z[x] <= 0.0f || zg[x] <= 0.0f)continue;
				if (min_depth > 0.0 && zg[x] < min_depth)continue;
				if (max_depth > 0.0 && zg[x] > max_depth)continue;
				sum += fabs(double(z[x]) - double(zg[x])) / double(zg[x]);
				++count;
			}
		}
		return count > 0 ? sum / double(count) : 0.0;
	}
}
