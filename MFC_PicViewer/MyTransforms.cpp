#include "pch.h"
#include "framework.h"
#include "afxdialogex.h"
#include "MyTransforms.h"
#include <opencv2/opencv.hpp>
#include <stack>
using namespace std;
using namespace cv;

int DEFAULT_FLAGMODE = cv::INTER_LINEAR;
Scalar DEFAULT_FILLCOLOR = cv::Scalar(39, 34, 30);

namespace Transforms {
	uchar gray_binaryize(float x) {
		return (x < 128) ? 0 : 255;
	}

	Mat Transform(Mat input, UINT x, UINT y ,UINT MODULE = Modules::BASIC::SHIFT) {
		Mat transform = (cv::Mat_<double>(2, 3) << 1, 0, x, 0, 1, y); // 平移矩阵
		Size res_size = input.size();             // 先复制原尺寸
		res_size.width += x;                    // 宽度扩展平移量
		res_size.height += y;                   // 高度扩展平移量
		Mat res;
		input.copyTo(res);
		warpAffine(res, res, transform, res_size, DEFAULT_FLAGMODE, cv::BORDER_CONSTANT, DEFAULT_FILLCOLOR); // 执行平移
		return res;
	}
	Mat Transform(Mat input, float argument, UINT MODULE) {
		if (MODULE == Modules::BASIC::ROTATE) {
			int w = input.size().width;               // 原图宽
			int h = input.size().height;              // 原图高
			int new_w = ceil(abs(w * cos(argument)) + abs(h * sin(argument))); // 计算旋转后所需宽
			int new_h = ceil(abs(w * sin(argument)) + abs(h * cos(argument))); // 计算旋转后所需高
			Point2f rotate_center = Point2f(w / 2.0, h / 2.0); // 旋转中心取图像中心
			Mat rotMat = cv::getRotationMatrix2D(rotate_center, argument, 1.0); // 获取旋转矩阵
			rotMat.at<double>(0, 2) += (new_w - w) / 2.0; // 补平移量使图像居中
			rotMat.at<double>(1, 2) += (new_h - h) / 2.0;
			Mat res;
			input.copyTo(res);
			cv::warpAffine(res, res, rotMat, cv::Size(new_w, new_h), DEFAULT_FLAGMODE, cv::BORDER_CONSTANT, DEFAULT_FILLCOLOR);
			return res;
		}
		else if (MODULE == Modules::NONLINEAR::LOG) {
			Mat gray(input.rows, input.cols, CV_32FC1);  // 先用 float 算
			float maxVal = 0;
			for (int h = 0; h < input.rows; ++h) {
				for (int w = 0; w < input.cols; ++w) {
					uchar p = (uchar)(0.30 * input.at<Vec3b>(h, w)[2] + 0.59 * input.at<Vec3b>(h, w)[1] + 0.11 * input.at<Vec3b>(h, w)[0]);
					float logVal = log(1.0f + p) / log(argument);  // 加 1 防 log(0)
					gray.at<float>(h, w) = logVal;
					maxVal = max(maxVal, logVal);
				}
			}
			gray /= maxVal;  // 归一化到 0~1
			gray *= 255;     // 缩放到 0~255
			Mat out;
			gray.convertTo(out, CV_8UC1);
			return out;
		}
		else if (MODULE == Modules::NONLINEAR::EXP) {
			Mat gray(input.rows, input.cols, CV_32FC1);
			float maxVal = 0;
			for (int h = 0; h < input.rows; ++h) {
				for (int w = 0; w < input.cols; ++w) {
					uchar p = (uchar)(0.30 * input.at<Vec3b>(h, w)[2] + 0.59 * input.at<Vec3b>(h, w)[1] + 0.11 * input.at<Vec3b>(h, w)[0]);
					float expVal = exp(p * log(argument) / 255.0f);  // 缩放 p 到 0~1
					gray.at<float>(h, w) = expVal;
					maxVal = max(maxVal, expVal);
				}
			}
			gray /= maxVal;
			gray *= 255;
			Mat out;
			gray.convertTo(out, CV_8UC1);
			return out;
		}

	}
	Mat Transform(Mat input, float scalex, float scaley, UINT MODULE = Modules::BASIC::SCALE) {
		Mat transform = (cv::Mat_<double>(2, 3) << scalex, 0, 0, 0, scaley, 0); // 缩放矩阵
		Size res_size = input.size();             // 先复制原尺寸
		res_size.width = ceil(res_size.width * scalex); // 计算缩放后宽
		res_size.height = ceil(res_size.height * scaley); // 计算缩放后高
		Mat res;
		input.copyTo(res);
		warpAffine(res, res, transform, res_size, DEFAULT_FLAGMODE, cv::BORDER_CONSTANT, DEFAULT_FILLCOLOR); // 执行缩放
		return res;
	}
	Mat Transform(Mat input, UINT MODULE) {
		if (MODULE == Modules::BASIC::GRAYIZE) {
			Mat gray(input.rows, input.cols, CV_8UC1);
			for (int h = 0; h < input.rows; ++h) {
				for (int w = 0; w < input.cols; ++w) {
					gray.at<uchar>(h, w) = (uchar)(0.30 * input.at<Vec3b>(h, w)[2] + 0.59 * input.at<Vec3b>(h, w)[1] + 0.11 * input.at<Vec3b>(h, w)[0]);
				}
			}
			return gray;
		}
		else if(MODULE == Modules::BASIC::BINARYIZE){
			Mat binary(input.rows, input.cols, CV_8UC1);
			for (int h = 0; h < input.rows; ++h) {
				for (int w = 0; w < input.cols; ++w) {
					binary.at<uchar>(h, w) = gray_binaryize(0.30 * input.at<Vec3b>(h, w)[2] + 0.59 * input.at<Vec3b>(h, w)[1] + 0.11 * input.at<Vec3b>(h, w)[0]);
				}
			}
			return binary;
		}
		else if (MODULE == Modules::LINEAR::NEGATIVE) {
			Mat gray(input.rows, input.cols, CV_8UC1);
			for (int h = 0; h < input.rows; ++h) {
				for (int w = 0; w < input.cols; ++w) {
					gray.at<uchar>(h, w) = 255 - (uchar)(0.30 * input.at<Vec3b>(h, w)[2] + 0.59 * input.at<Vec3b>(h, w)[1] + 0.11 * input.at<Vec3b>(h, w)[0]);
				}
			}
			return gray;
		}
		else if (MODULE == Modules::LINEAR::NEGATIVE_COLORFUL) {
			Mat res(input.rows, input.cols, CV_8UC3);
			for (int h = 0; h < input.rows; ++h) {
				for (int w = 0; w < input.cols; ++w) {
					res.at<Vec3b>(h, w)[0] = 255 - input.at<Vec3b>(h, w)[0];
					res.at<Vec3b>(h, w)[1] = 255 - input.at<Vec3b>(h, w)[1];
					res.at<Vec3b>(h, w)[2] = 255 - input.at<Vec3b>(h, w)[2];
				}
			}
			return res;
		}
	}
	Mat Transform(Mat input, std::pair<int, int> output_gray_scale, UINT MODULE = Modules::LINEAR::SCALE_LINEAR) {
		Mat gray(input.rows, input.cols, CV_8UC1);
		for (int h = 0; h < input.rows; ++h) {
			for (int w = 0; w < input.cols; ++w) {
				gray.at<uchar>(h, w) = (uchar)(((0.30 * input.at<Vec3b>(h, w)[2] + 0.59 * input.at<Vec3b>(h, w)[1] + 0.11 * input.at<Vec3b>(h, w)[0]))/255.0*(output_gray_scale.second-output_gray_scale.first)+output_gray_scale.first);
			}
		}
		return gray;
	}//0-255 to x-y -> q = p/255*(y-x)+x
	Mat Transform(Mat input, std::vector<std::pair<int, int>> operate_ranges, std::vector<std::pair<int, int>> output_gray_scales,UINT MODULE = Modules::LINEAR::SCALE_LINEAR_MULTIRANGES) {
		Mat gray(input.rows, input.cols, CV_8UC1);
		for (int h = 0; h < input.rows; ++h) {
			for (int w = 0; w < input.cols; ++w) {
				uchar p = (uchar)((0.30 * input.at<Vec3b>(h, w)[2] + 0.59 * input.at<Vec3b>(h, w)[1] + 0.11 * input.at<Vec3b>(h, w)[0]));
				for (int i = 0; i < operate_ranges.size(); i++) {
					if (p >= operate_ranges[i].first && p <= operate_ranges[i].second) {
						int gray_scalex = output_gray_scales[i].first;
						int gray_scaley = output_gray_scales[1].second;
						gray.at<uchar>(h, w) = (uchar)(p/255.0*(gray_scaley - gray_scalex) + gray_scalex);
						break;
					}
				}

			}
		}
		return gray;
	}//0-255 to x-y -> q = p/255*(y-x)+x
}