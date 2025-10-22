#include "pch.h"
#include "framework.h"
#include "afxdialogex.h"
#include "MyTransforms.h"
#include <opencv2/opencv.hpp>
#include <stack>
#include <cmath>
#include <complex>
#include <vector>
using namespace std;
using namespace cv;
using std::complex;
using std::vector;

int DEFAULT_FLAGMODE = cv::INTER_LINEAR;
Scalar DEFAULT_FILLCOLOR = cv::Scalar(39, 34, 30);
namespace Transforms_Assist {

	uchar gray_binaryize(float x) {
		return (x < 128) ? 0 : 255;
	}

}
namespace Transforms {
	Mat Transform(Mat input, UINT x, UINT y ,UINT MODULE = Modules::BASIC::SHIFT) {
		if (input.channels() == 1)cvtColor(input, input, COLOR_GRAY2BGR);
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
		if (input.channels() == 1)cvtColor(input, input, COLOR_GRAY2BGR);
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
		else if (MODULE == Modules::DEVIDE::REGIONGROW) {
			Mat gray(input.rows, input.cols, CV_8UC1);
			return gray;
		}
	}
	Mat Transform(Mat input, float scalex, float scaley, UINT MODULE = Modules::BASIC::SCALE) {
		if (input.channels() == 1)cvtColor(input, input, COLOR_GRAY2BGR);
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
					binary.at<uchar>(h, w) = Transforms_Assist::gray_binaryize(0.30 * input.at<Vec3b>(h, w)[2] + 0.59 * input.at<Vec3b>(h, w)[1] + 0.11 * input.at<Vec3b>(h, w)[0]);
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
		else if (MODULE == Modules::DEVIDE::OTSU) {
			Mat _input;
			input.copyTo(_input);
			if (_input.channels() == 3)cvtColor(_input, _input, COLOR_BGR2GRAY);
			Mat gray = Mat::zeros(_input.size(), CV_8UC1);
			int n[256] = { 0 };
			long gn[256] = { 0 };
			double mk[256] = { 0.0 };
			double pa[256] = { 0.0 };
			long N = _input.rows * _input.cols;
			double sigma2 = 0.0;  // 改为初始化为0
			double k = 128;       // 设置默认阈值
			double mg = 0.0;
			double eps = 1e-10;
			for (int h = 0; h < _input.rows; ++h) {
				for (int w = 0; w < _input.cols; ++w) {
					n[_input.at<uchar>(h, w)] += 1;
					mg += _input.at<uchar>(h, w);
				}
			}
			mg /= N;
			// 计算累积分布
			gn[0] = n[0];
			pa[0] = (double)n[0] / N;
			mk[0] = 0;
			for (int i = 1; i < 256; i++) {
				gn[i] = gn[i - 1] + n[i];
				pa[i] = pa[i - 1] + (double)n[i] / N;
				mk[i] = mk[i - 1] + i * (double)n[i] / N;
			}
			// 寻找最佳阈值
			bool found = false;
			for (int i = 1; i < 255; i++) {  // 避免边界值
				if (pa[i] <= eps || pa[i] >= 1.0 - eps)continue;
				double denominator = pa[i] * (1 - pa[i]);
				if (denominator < eps)continue;
				double tmp = pow(mg * pa[i] - mk[i], 2) / denominator;
				if (tmp > sigma2) {
					sigma2 = tmp;
					k = i;
					found = true;
				}
			}
			if (!found)k = 128;
			for (int h = 0; h < _input.rows; h++) {
				for (int w = 0; w < _input.cols; w++) {
					gray.at<uchar>(h, w) = (_input.at<uchar>(h, w) <= k) ? 0 : 255;
				}
			}
			return gray;
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
			// 如果输入是彩色图，先转灰度；如果是灰度图，直接用
			Mat gray;
			if (input.channels() == 3)cvtColor(input, gray, COLOR_BGR2GRAY);
			else if (input.channels() == 1)gray = input;
			else return Mat(); // 不支持的格式
			const int histSize = 256;
			int hist[histSize] = { 0 };
			int total = gray.rows * gray.cols;
			// 统计直方图
			for (int i = 0; i < gray.rows; ++i) {
				for (int j = 0; j < gray.cols; ++j) {
					uchar p = gray.at<uchar>(i, j);
					hist[p]++;
				}
			}
			// 计算概率和累积分布
			double cdf[histSize] = { 0.0 };
			cdf[0] = (double)hist[0] / total;
			for (int i = 1; i < histSize; ++i)cdf[i] = cdf[i - 1] + (double)hist[i] / total;
			// 映射新灰度值
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
	Mat Enhance(Mat input, double argument, UINT MODULE) {
		if (MODULE == Modules::EDGE::ROBERT) {
			Mat _input;
			input.copyTo(_input);
			if (_input.channels() == 3)cvtColor(_input, _input, COLOR_BGR2GRAY);
			cv::Mat dst = cv::Mat::zeros(_input.size(), CV_8UC1);

			const int rows = _input.rows;
			const int cols = _input.cols;

			for (int i = 0; i < rows - 1; ++i)
			{
				const uchar* curr = _input.ptr<uchar>(i);
				const uchar* next = _input.ptr<uchar>(i + 1);
				uchar* out = dst.ptr<uchar>(i);

				for (int j = 0; j < cols - 1; ++j)
				{
					int gx = next[j + 1] - curr[j];
					int gy = next[j] - curr[j + 1];

					float mag = std::sqrt(float(gx * gx + gy * gy));
					out[j] = mag >= argument ? 255 : 0;
				}
			}
			return dst;
		}
		else if (MODULE == Modules::EDGE::PREWITT) {
			Mat _input;
			input.copyTo(_input);
			if (_input.channels() == 3)cvtColor(_input, _input, COLOR_BGR2GRAY);
			cv::Mat dst = cv::Mat::zeros(_input.size(), CV_8UC1);

			const int rows = _input.rows;
			const int cols = _input.cols;
			double gx, gy;
			for (int i = 1; i < rows - 1; ++i)
			{
				for (int j = 1; j < cols - 1; ++j)
				{
					gx = _input.at<uchar>(i + 1, j - 1) - _input.at<uchar>(i - 1, j - 1)
						+ _input.at<uchar>(i + 1, j) - _input.at<uchar>(i - 1, j)
						+ _input.at<uchar>(i + 1, j + 1) - _input.at<uchar>(i - 1, j + 1);
					gy = _input.at<uchar>(i - 1, j + 1) - _input.at<uchar>(i - 1, j - 1)
						+ _input.at<uchar>(i, j + 1) - _input.at<uchar>(i, j - 1)
						+ _input.at<uchar>(i + 1, j + 1) - _input.at<uchar>(i + 1, j - 1);
					if (abs(sqrt(gx * gx + gy * gy)) > argument)dst.at<uchar>(i, j) = 255;
				}
			}
			return dst;
		}
		else if (MODULE == Modules::EDGE::SOBEL) {
			Mat _input;
			input.copyTo(_input);
			if (_input.channels() == 3)cvtColor(_input, _input, COLOR_BGR2GRAY);
			cv::Mat dst = cv::Mat::zeros(_input.size(), CV_8UC1);

			const int rows = _input.rows;
			const int cols = _input.cols;
			double gx, gy;
			for (int i = 1; i < rows - 1; ++i)
			{
				for (int j = 1; j < cols - 1; ++j)
				{
					gx = _input.at<uchar>(i + 1, j - 1) - _input.at<uchar>(i - 1, j - 1)
						+ 2*_input.at<uchar>(i + 1, j) - 2*_input.at<uchar>(i - 1, j)
						+ _input.at<uchar>(i + 1, j + 1) - _input.at<uchar>(i - 1, j + 1);
					gy = _input.at<uchar>(i - 1, j + 1) - _input.at<uchar>(i - 1, j - 1)
						+ 2*_input.at<uchar>(i, j + 1) - 2*_input.at<uchar>(i, j - 1)
						+ _input.at<uchar>(i + 1, j + 1) - _input.at<uchar>(i + 1, j - 1);
					if (abs(sqrt(gx * gx + gy * gy)) > argument)dst.at<uchar>(i, j) = 255;
				}
			}
			return dst;
		}
		else if (MODULE == Modules::EDGE::FRIE_CHEN) {
			Mat _input;
			input.copyTo(_input);
			if (_input.channels() == 3)cvtColor(_input, _input, COLOR_BGR2GRAY);
			cv::Mat dst = cv::Mat::zeros(_input.size(), CV_8UC1);

			const int rows = _input.rows;
			const int cols = _input.cols;
			double gx, gy;
			double sqrt2 = sqrt(2);
			for (int i = 1; i < rows - 1; ++i)
			{
				for (int j = 1; j < cols - 1; ++j)
				{
					gx = _input.at<uchar>(i + 1, j - 1) - _input.at<uchar>(i - 1, j - 1)
						+  sqrt(2)* _input.at<uchar>(i + 1, j) - sqrt(2) * _input.at<uchar>(i - 1, j)
						+ _input.at<uchar>(i + 1, j + 1) - _input.at<uchar>(i - 1, j + 1);
					gy = _input.at<uchar>(i - 1, j + 1) - _input.at<uchar>(i - 1, j - 1)
						+ sqrt(2) * _input.at<uchar>(i, j + 1) - sqrt(2) * _input.at<uchar>(i, j - 1)
						+ _input.at<uchar>(i + 1, j + 1) - _input.at<uchar>(i + 1, j - 1);
					if (abs(sqrt(gx * gx + gy * gy)) > argument)dst.at<uchar>(i, j) = 255;
				}
			}
			return dst;
		}
	};
	Mat Enhance(Mat input, double argument1, double argument2, UINT MODULE){
		if (MODULE == Modules::EDGE::LAPLACIAN) {
			Mat _input;
			input.copyTo(_input);
			if (_input.channels() == 3)cvtColor(_input, _input, COLOR_BGR2GRAY);
			cv::Mat dst = cv::Mat::zeros(_input.size(), CV_8UC1);

			const int rows = _input.rows;
			const int cols = _input.cols;
			double Lij, gx, gy, th;
			for (int i = 1; i < rows - 1; ++i)
			{
				for (int j = 1; j < cols - 1; ++j)
				{
					Lij = _input.at<uchar>(i + 1, j)
						+ _input.at<uchar>(i - 1, j)
						+ _input.at<uchar>(i, j + 1)
						+ _input.at<uchar>(i, j - 1)
						- 4 * _input.at<uchar>(i, j);
					gx = _input.at<uchar>(i + 1, j - 1) - _input.at<uchar>(i - 1, j - 1)
						+ 2 * _input.at<uchar>(i + 1, j) - 2 * _input.at<uchar>(i - 1, j)
						+ _input.at<uchar>(i + 1, j + 1) - _input.at<uchar>(i - 1, j + 1);
					gy = _input.at<uchar>(i - 1, j + 1) - _input.at<uchar>(i - 1, j - 1)
						+ 2 * _input.at<uchar>(i, j + 1) - 2 * _input.at<uchar>(i, j - 1)
						+ _input.at<uchar>(i + 1, j + 1) - _input.at<uchar>(i + 1, j - 1);
					th = abs(sqrt(gx * gx + gy * gy));
					if(th>argument1 && Lij < argument2)dst.at<uchar>(i, j) = 255;
				}
			}
			return dst;
		}
	}
	namespace Fourier {
		Mat spectrumView(Mat freq)
		{
			CV_Assert(freq.type() == CV_32FC2);
			cv::Mat planes[2];
			cv::split(freq, planes);                 // planes[0]=Re, planes[1]=Im

			cv::Mat mag;
			cv::magnitude(planes[0], planes[1], mag);   // mag = sqrt(Re^2+Im^2)

			// 中心化（低频移中间）
			int cx = mag.cols / 2, cy = mag.rows / 2;
			cv::Mat tmp;
			cv::Mat q0(mag, cv::Rect(0, 0, cx, cy));
			cv::Mat q1(mag, cv::Rect(cx, 0, cx, cy));
			cv::Mat q2(mag, cv::Rect(0, cy, cx, cy));
			cv::Mat q3(mag, cv::Rect(cx, cy, cx, cy));
			q0.copyTo(tmp); q3.copyTo(q0); tmp.copyTo(q3);
			q1.copyTo(tmp); q2.copyTo(q1); tmp.copyTo(q2);

			cv::log(mag + 1.f, mag);                 // 取 log 压缩动态范围
			cv::normalize(mag, mag, 0, 255, cv::NORM_MINMAX);
			mag.convertTo(mag, CV_8U);
			return mag;
		}
		Mat FFT(Mat gray)
		{
			if (gray.channels() != 1)cvtColor(gray, gray, COLOR_BGR2GRAY);
			int optH = cv::getOptimalDFTSize(gray.rows);
			int optW = cv::getOptimalDFTSize(gray.cols);
			cv::Mat padded;
			cv::copyMakeBorder(gray, padded, 0, optH - gray.rows, 0, optW - gray.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));
			padded.convertTo(padded, CV_32F);
			cv::Mat planes[] = { padded, cv::Mat::zeros(padded.size(), CV_32F) };
			cv::Mat complexI;
			cv::merge(planes, 2, complexI);
			cv::dft(complexI, complexI);                // 原地 FFT
			return complexI;                            // CV_32FC2
		}
		cv::Mat IFFT(Mat freq)
		{
			CV_Assert(freq.type() == CV_32FC2);
			cv::Mat complexI = freq.clone();           
			cv::idft(complexI, complexI);              
			// 取实部并归一化到 0-255
			cv::Mat planes[2];
			cv::split(complexI, planes);
			cv::Mat result;
			cv::normalize(planes[0], result, 0, 255, cv::NORM_MINMAX);
			result.convertTo(result, CV_8U);
			return result;                              
		}
	}
}