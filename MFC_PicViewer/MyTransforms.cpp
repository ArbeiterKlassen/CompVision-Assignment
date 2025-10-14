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
		if (input.channels() == 1)cvtColor(input, input, COLOR_GRAY2BGR);
		Mat transform = (cv::Mat_<double>(2, 3) << 1, 0, x, 0, 1, y); // ƽ�ƾ���
		Size res_size = input.size();             // �ȸ���ԭ�ߴ�
		res_size.width += x;                    // �����չƽ����
		res_size.height += y;                   // �߶���չƽ����
		Mat res;
		input.copyTo(res);
		warpAffine(res, res, transform, res_size, DEFAULT_FLAGMODE, cv::BORDER_CONSTANT, DEFAULT_FILLCOLOR); // ִ��ƽ��
		return res;
	}
	Mat Transform(Mat input, float argument, UINT MODULE) {
		if (input.channels() == 1)cvtColor(input, input, COLOR_GRAY2BGR);
		if (MODULE == Modules::BASIC::ROTATE) {
			int w = input.size().width;               // ԭͼ��
			int h = input.size().height;              // ԭͼ��
			int new_w = ceil(abs(w * cos(argument)) + abs(h * sin(argument))); // ������ת�������
			int new_h = ceil(abs(w * sin(argument)) + abs(h * cos(argument))); // ������ת�������
			Point2f rotate_center = Point2f(w / 2.0, h / 2.0); // ��ת����ȡͼ������
			Mat rotMat = cv::getRotationMatrix2D(rotate_center, argument, 1.0); // ��ȡ��ת����
			rotMat.at<double>(0, 2) += (new_w - w) / 2.0; // ��ƽ����ʹͼ�����
			rotMat.at<double>(1, 2) += (new_h - h) / 2.0;
			Mat res;
			input.copyTo(res);
			cv::warpAffine(res, res, rotMat, cv::Size(new_w, new_h), DEFAULT_FLAGMODE, cv::BORDER_CONSTANT, DEFAULT_FILLCOLOR);
			return res;
		}
		else if (MODULE == Modules::NONLINEAR::LOG) {
			Mat gray(input.rows, input.cols, CV_32FC1);  // ���� float ��
			float maxVal = 0;
			for (int h = 0; h < input.rows; ++h) {
				for (int w = 0; w < input.cols; ++w) {
					uchar p = (uchar)(0.30 * input.at<Vec3b>(h, w)[2] + 0.59 * input.at<Vec3b>(h, w)[1] + 0.11 * input.at<Vec3b>(h, w)[0]);
					float logVal = log(1.0f + p) / log(argument);  // �� 1 �� log(0)
					gray.at<float>(h, w) = logVal;
					maxVal = max(maxVal, logVal);
				}
			}
			gray /= maxVal;  // ��һ���� 0~1
			gray *= 255;     // ���ŵ� 0~255
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
					float expVal = exp(p * log(argument) / 255.0f);  // ���� p �� 0~1
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
		if (input.channels() == 1)cvtColor(input, input, COLOR_GRAY2BGR);
		Mat transform = (cv::Mat_<double>(2, 3) << scalex, 0, 0, 0, scaley, 0); // ���ž���
		Size res_size = input.size();             // �ȸ���ԭ�ߴ�
		res_size.width = ceil(res_size.width * scalex); // �������ź��
		res_size.height = ceil(res_size.height * scaley); // �������ź��
		Mat res;
		input.copyTo(res);
		warpAffine(res, res, transform, res_size, DEFAULT_FLAGMODE, cv::BORDER_CONSTANT, DEFAULT_FILLCOLOR); // ִ������
		return res;
	}
	Mat Transform(Mat input, UINT MODULE) {
		if (input.channels() == 1)cvtColor(input, input, COLOR_GRAY2BGR);
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
		if (input.channels() == 1)cvtColor(input, input, COLOR_GRAY2BGR);
		Mat gray(input.rows, input.cols, CV_8UC1);
		for (int h = 0; h < input.rows; ++h) {
			for (int w = 0; w < input.cols; ++w) {
				gray.at<uchar>(h, w) = (uchar)(((0.30 * input.at<Vec3b>(h, w)[2] + 0.59 * input.at<Vec3b>(h, w)[1] + 0.11 * input.at<Vec3b>(h, w)[0]))/255.0*(output_gray_scale.second-output_gray_scale.first)+output_gray_scale.first);
			}
		}
		return gray;
	}//0-255 to x-y -> q = p/255*(y-x)+x
	Mat Transform(Mat input, std::vector<std::pair<int, int>> operate_ranges, std::vector<std::pair<int, int>> output_gray_scales,UINT MODULE = Modules::LINEAR::SCALE_LINEAR_MULTIRANGES) {
		if (input.channels() == 1)cvtColor(input, input, COLOR_GRAY2BGR);
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
	Mat Enhance(Mat input, UINT MODULE) {
		if (MODULE == Modules::ENHANCE::DHE) {
			// ��������ǲ�ɫͼ����ת�Ҷȣ�����ǻҶ�ͼ��ֱ����
			Mat gray;
			if (input.channels() == 3)cvtColor(input, gray, COLOR_BGR2GRAY);
			else if (input.channels() == 1)gray = input;
			else return Mat(); // ��֧�ֵĸ�ʽ
			const int histSize = 256;
			int hist[histSize] = { 0 };
			int total = gray.rows * gray.cols;
			// ͳ��ֱ��ͼ
			for (int i = 0; i < gray.rows; ++i) {
				for (int j = 0; j < gray.cols; ++j) {
					uchar p = gray.at<uchar>(i, j);
					hist[p]++;
				}
			}
			// ������ʺ��ۻ��ֲ�
			double cdf[histSize] = { 0.0 };
			cdf[0] = (double)hist[0] / total;
			for (int i = 1; i < histSize; ++i)cdf[i] = cdf[i - 1] + (double)hist[i] / total;
			// ӳ���»Ҷ�ֵ
			Mat result(gray.size(), CV_8UC1);
			for (int i = 0; i < gray.rows; ++i) {
				for (int j = 0; j < gray.cols; ++j) {
					uchar p = gray.at<uchar>(i, j);
					result.at<uchar>(i, j) = saturate_cast<uchar>(cdf[p] * 255);
				}
			}
			return result;
		}
	}
	Mat Enhance(Mat input, Mat ref, UINT MODULE) {
		if (MODULE == Modules::ENHANCE::DHS) {
			CV_Assert(input.type() == CV_8UC1 || input.type() == CV_8UC3);
			CV_Assert(ref.type() == CV_8UC1 || ref.type() == CV_8UC3);
			Mat gSrc, gRef;
			if (input.channels() == 3) cvtColor(input, gSrc, COLOR_BGR2GRAY);
			else gSrc = input.clone();
			if (ref.channels() == 3) cvtColor(ref, gRef, COLOR_BGR2GRAY);
			else gRef = ref.clone();
			auto makeCDF = [](const Mat& gray, double cdf[256]) {
				int hist[256] = { 0 };
				for (int i = 0; i < gray.rows; ++i)for (int j = 0; j < gray.cols; ++j)hist[gray.at<uchar>(i, j)]++;
				const int total = gray.total();
				cdf[0] = double(hist[0]) / total;
				for (int i = 1; i < 256; ++i)cdf[i] = cdf[i - 1] + double(hist[i]) / total;
			};

			double cdfSrc[256], cdfRef[256];
			makeCDF(gSrc, cdfSrc);
			makeCDF(gRef, cdfRef);
			uchar map[256];
			for (int s = 0; s < 256; ++s) {
				double minDiff = 1e9;
				int bestZ = 0;
				for (int z = 0; z < 256; ++z) {
					double diff = std::abs(cdfSrc[s] - cdfRef[z]);
					if (diff < minDiff) { minDiff = diff; bestZ = z; }
				}
				map[s] = static_cast<uchar>(bestZ);
			}
			Mat dst(gSrc.size(), CV_8UC1);
			for (int i = 0; i < gSrc.rows; ++i)for (int j = 0; j < gSrc.cols; ++j)dst.at<uchar>(i, j) = map[gSrc.at<uchar>(i, j)];
			return dst;
		}
	}
}