#pragma once
#include "pch.h"
#include "framework.h"
#include "afxdialogex.h"
#include <opencv2/opencv.hpp>
#include <stack>
using namespace std;
using namespace cv;
namespace Filters {
	namespace Modules {
		namespace SPATIAL {
			enum {
				MAX = 1,MIN,MEDIAN,MEAN
			};
		}
	}
	static Mat maxFilter(const Mat& src, int k);
	static Mat minFilter(const Mat& src, int k);
	static Mat medianFilter(const Mat& src, int k);
	static Mat meanFilter(const Mat& src, int k);
	Mat filterate(Mat input, UINT kernel_size, UINT MODULE);
}