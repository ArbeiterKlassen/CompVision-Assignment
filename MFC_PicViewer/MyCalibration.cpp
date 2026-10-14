#include "pch.h"
#include "framework.h"
#include "MyCalibration.h"
#include <opencv2/opencv.hpp>
#include <algorithm>
#include <cmath>
using namespace std;
using namespace cv;

namespace Calibration_Assist {
	// 单幅视图的重投影误差（RMS，像素）：把棋盘格点按标定结果投回图像，与检测点比较
	double viewReprojectionError(const vector<Point3f>& object_points, const vector<Point2f>& image_points,
		const Mat& K, const Mat& D, const Mat& rvec, const Mat& tvec)
	{
		vector<Point2f> projected;
		projectPoints(object_points, rvec, tvec, K, D, projected);
		double sum = 0.0;
		for (size_t i = 0; i < projected.size() && i < image_points.size(); ++i)
		{
			double dx = double(projected[i].x) - double(image_points[i].x);
			double dy = double(projected[i].y) - double(image_points[i].y);
			sum += dx * dx + dy * dy;
		}
		return projected.empty() ? 0.0 : sqrt(sum / double(std::min(projected.size(), image_points.size())));
	}
}

namespace Calibration {

	void buildBoardPoints(Size board_size, double square_size, vector<Point3f>& points)
	{
		points.clear();
		for (int i = 0; i < board_size.height; ++i)
			for (int j = 0; j < board_size.width; ++j)
				points.push_back(Point3f(float(j * square_size), float(i * square_size), 0.0f));
	}

	bool findChessboard(const Mat& gray, Size board_size, vector<Point2f>& corners,
		int subpix_win, bool use_subpix)
	{
		corners.clear();
		if (gray.empty())return false;

		// 自适应阈值 + 归一化：应对棋盘格在图像中的光照不均
		int flags = CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE;
		if (!findChessboardCorners(gray, board_size, corners, flags))return false;

		if (use_subpix)
		{
			// 亚像素细化：角点定位精度直接决定重投影误差，细化后可到 0.1 像素量级
			cornerSubPix(gray, corners, Size(subpix_win, subpix_win), Size(-1, -1),
				TermCriteria(TermCriteria::EPS + TermCriteria::MAX_ITER, 30, 0.001));
		}
		return true;
	}

	MonoResult calibrateMono(const vector<vector<Point2f>>& image_points, Size image_size,
		Size board_size, double square_size, int flags)
	{
		MonoResult res;
		if (image_points.empty())return res;

		vector<Point3f> board;
		buildBoardPoints(board_size, square_size, board);
		vector<vector<Point3f>> object_points(image_points.size(), board);

		// 张正友平面棋盘格法：先线性解初值，再用最大似然（LM）迭代优化
		vector<Mat> rvecs, tvecs;
		res.rms = calibrateCamera(object_points, image_points, image_size,
			res.camera_matrix, res.dist_coeffs, rvecs, tvecs, flags);

		res.view_count = (int)image_points.size();
		res.per_view_rms.resize(image_points.size());
		for (size_t i = 0; i < image_points.size(); ++i)
			res.per_view_rms[i] = Calibration_Assist::viewReprojectionError(board, image_points[i],
				res.camera_matrix, res.dist_coeffs, rvecs[i], tvecs[i]);
		res.success = true;
		return res;
	}

	StereoResult calibrateStereo(const vector<vector<Point2f>>& left_points,
		const vector<vector<Point2f>>& right_points, Size image_size, Size board_size,
		double square_size, const MonoResult& left, const MonoResult& right, int flags)
	{
		StereoResult res;
		if (left_points.empty() || left_points.size() != right_points.size())return res;
		if (!left.success || !right.success)return res;

		vector<Point3f> board;
		buildBoardPoints(board_size, square_size, board);
		vector<vector<Point3f>> object_points(left_points.size(), board);

		// 以内参为初值求左右相机之间的相对位姿；flags 传 CALIB_FIX_INTRINSIC 时不再优化内参
		Mat K1 = left.camera_matrix.clone(), D1 = left.dist_coeffs.clone();
		Mat K2 = right.camera_matrix.clone(), D2 = right.dist_coeffs.clone();
		res.rms = stereoCalibrate(object_points, left_points, right_points,
			K1, D1, K2, D2, image_size, res.R, res.T, res.E, res.F, flags,
			TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 100, 1e-6));

		res.baseline = baselineLength(res.T);
		res.success = true;
		return res;
	}

	RectifyResult rectify(const MonoResult& left, const MonoResult& right,
		const StereoResult& stereo, Size image_size, double alpha)
	{
		RectifyResult res;
		res.image_size = image_size;
		if (!left.success || !right.success || !stereo.success)return res;

		// Bouguet 校正：让两幅图像的极线平行于 u 轴，同名点落在同一行上
		stereoRectify(left.camera_matrix, left.dist_coeffs, right.camera_matrix, right.dist_coeffs,
			image_size, stereo.R, stereo.T, res.R1, res.R2, res.P1, res.P2, res.Q,
			CALIB_ZERO_DISPARITY, alpha, image_size);
		res.success = true;
		return res;
	}

	Mat undistortImage(const Mat& src, const MonoResult& calib)
	{
		Mat dst;
		if (src.empty() || !calib.success)return dst;
		undistort(src, dst, calib.camera_matrix, calib.dist_coeffs);
		return dst;
	}

	void getRectifyMaps(const MonoResult& left, const MonoResult& right, const RectifyResult& rect,
		Mat& map1x, Mat& map1y, Mat& map2x, Mat& map2y)
	{
		if (!rect.success)return;
		initUndistortRectifyMap(left.camera_matrix, left.dist_coeffs, rect.R1, rect.P1,
			rect.image_size, CV_32FC1, map1x, map1y);
		initUndistortRectifyMap(right.camera_matrix, right.dist_coeffs, rect.R2, rect.P2,
			rect.image_size, CV_32FC1, map2x, map2y);
	}

	Mat remapImage(const Mat& src, const Mat& mapx, const Mat& mapy)
	{
		Mat dst;
		if (src.empty() || mapx.empty() || mapy.empty())return dst;
		remap(src, dst, mapx, mapy, INTER_LINEAR);
		return dst;
	}

	double baselineLength(const Mat& T)
	{
		if (T.empty())return 0.0;
		return norm(T);
	}

	double rotationAngleDeg(const Mat& R1, const Mat& R2)
	{
		if (R1.empty() || R2.empty())return 0.0;
		Mat rel = R1 * R2.t();          // 两个旋转之间的相对旋转
		Mat rvec;
		Rodrigues(rel, rvec);           // 相对旋转的轴角表示
		return norm(rvec) * 180.0 / CV_PI;
	}

	void intrinsicsError(const Mat& K, const Mat& K_ref,
		double& fx_err, double& fy_err, double& cx_err, double& cy_err)
	{
		fx_err = fy_err = cx_err = cy_err = 0.0;
		if (K.empty() || K_ref.empty())return;
		fx_err = fabs(K.at<double>(0, 0) - K_ref.at<double>(0, 0)) / K_ref.at<double>(0, 0);
		fy_err = fabs(K.at<double>(1, 1) - K_ref.at<double>(1, 1)) / K_ref.at<double>(1, 1);
		cx_err = fabs(K.at<double>(0, 2) - K_ref.at<double>(0, 2)) / K_ref.at<double>(0, 2);
		cy_err = fabs(K.at<double>(1, 2) - K_ref.at<double>(1, 2)) / K_ref.at<double>(1, 2);
	}

	void epipolarError(const vector<vector<Point2f>>& left_points,
		const vector<vector<Point2f>>& right_points,
		const Mat& K1, const Mat& D1, const Mat& R1, const Mat& P1,
		const Mat& K2, const Mat& D2, const Mat& R2, const Mat& P2,
		double& mean_abs_dv, double& max_abs_dv)
	{
		mean_abs_dv = 0.0;
		max_abs_dv = 0.0;
		double sum = 0.0;
		int count = 0;
		size_t views = std::min(left_points.size(), right_points.size());
		for (size_t v = 0; v < views; ++v)
		{
			vector<Point2f> lr, rr;
			// 用校正旋转与校正投影矩阵把角点投到校正图像的像素坐标
			undistortPoints(left_points[v], lr, K1, D1, R1, P1);
			undistortPoints(right_points[v], rr, K2, D2, R2, P2);
			size_t n = std::min(lr.size(), rr.size());
			for (size_t i = 0; i < n; ++i)
			{
				double dv = fabs(double(lr[i].y) - double(rr[i].y));
				sum += dv;
				if (dv > max_abs_dv)max_abs_dv = dv;
				++count;
			}
		}
		mean_abs_dv = count > 0 ? sum / double(count) : 0.0;
	}
}
