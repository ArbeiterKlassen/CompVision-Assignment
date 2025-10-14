#pragma once
#include "pch.h"
#include "framework.h"
#include "afxdialogex.h"
#include <opencv2/opencv.hpp>
#include <stack>
using namespace std;
using namespace cv;
namespace Transforms {
	namespace Modules {
		namespace LINEAR {
			enum {
				NEGATIVE = 1, NEGATIVE_COLORFUL, SCALE_LINEAR, SCALE_LINEAR_MULTIRANGES
			};
		}
		namespace NONLINEAR {
			enum {
				LOG = Transforms::Modules::LINEAR::SCALE_LINEAR_MULTIRANGES + 1, EXP
			};
		}
		namespace BASIC {
			enum {
				SHIFT = Transforms::Modules::NONLINEAR::EXP + 1, ROTATE, SCALE, BINARYIZE, GRAYIZE
			};
		}
		namespace ENHANCE {
			enum {
				DHE = Transforms::Modules::BASIC::GRAYIZE + 1,DHS
			};
		}
	}
	Mat Transform(Mat input, UINT x, UINT y, UINT MODULE);
	Mat Transform(Mat input, float theta, UINT MODULE);
	Mat Transform(Mat input, float scalex, float scaley, UINT MODULE);
	Mat Transform(Mat input, UINT MODULE);
	Mat Transform(Mat input, std::pair<int, int> output_gray_scale, UINT MODULE);
	Mat Transform(Mat input, std::vector<std::pair<int, int>> operate_ranges, std::vector<std::pair<int, int>> output_gray_scales, UINT MODULE);
	Mat Enhance(Mat input, UINT MODULE);
	Mat Enhance(Mat input, Mat ref, UINT MODULE);
}