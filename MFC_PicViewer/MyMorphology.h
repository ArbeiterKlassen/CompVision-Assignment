#pragma once

#include "pch.h"
#include "framework.h"
#include <opencv2/opencv.hpp>

// 形态学操作模块：腐蚀 / 膨胀 / 开运算 / 闭运算
namespace Morphology {
	namespace Modules {
		enum {
			ERODE = 1, DILATE, OPEN, CLOSE
		};
	}
	// 对灰度图执行指定形态学操作；结果保持单通道
	cv::Mat morphology(cv::Mat input, UINT kernel_size, UINT MODULE);
}
