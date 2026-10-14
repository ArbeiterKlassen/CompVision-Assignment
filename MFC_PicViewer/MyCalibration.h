#pragma once

#include "pch.h"
#include "framework.h"
#include <opencv2/opencv.hpp>
#include <vector>

// 摄像机标定模块：棋盘格角点检测、单目标定、双目标定、畸变校正与立体校正。
// 记号沿用讲义第五章：内参矩阵 K、畸变系数 D、外参 (R, T)、完整投影 p = M P = K[R T] P，
// 双目标定的 (R, T) 指右相机相对左相机，基线长度即 |T|。
namespace Calibration {

	// 单目标定结果
	struct MonoResult {
		cv::Mat camera_matrix;               // 内参矩阵 K
		cv::Mat dist_coeffs;                 // 畸变系数 (k1,k2,p1,p2,k3)
		double rms = 0.0;                    // 整体 RMS 重投影误差（像素）
		std::vector<double> per_view_rms;    // 每幅图的重投影误差（像素）
		int view_count = 0;                  // 参与标定的视图数
		bool success = false;
	};

	// 双目标定结果
	struct StereoResult {
		cv::Mat R;                           // 右相机相对左相机的旋转矩阵
		cv::Mat T;                           // 右相机相对左相机的平移向量
		cv::Mat E;                           // 本质矩阵
		cv::Mat F;                           // 基础矩阵
		double baseline = 0.0;               // 基线长度 |T|（与 square_size 同单位）
		double rms = 0.0;                    // 整体 RMS 重投影误差（像素）
		bool success = false;
	};

	// 立体校正结果（Bouguet 方法：使两幅校正图的行完全对齐、极线平行于 u 轴）
	struct RectifyResult {
		cv::Mat R1, R2;                      // 左右相机的校正旋转
		cv::Mat P1, P2;                      // 校正后的投影矩阵（含新的内参）
		cv::Mat Q;                           // 视差到深度的重投影矩阵
		cv::Size image_size;                 // 图像尺寸
		bool success = false;
	};

	// 棋盘格世界坐标：z = 0 平面上的内角点，行优先排列
	void buildBoardPoints(cv::Size board_size, double square_size, std::vector<cv::Point3f>& points);

	// 棋盘格角点检测：findChessboardCorners + cornerSubPix 亚像素细化
	bool findChessboard(const cv::Mat& gray, cv::Size board_size, std::vector<cv::Point2f>& corners,
		int subpix_win = 11, bool use_subpix = true);

	// 单目标定：返回 K、畸变系数、整体与逐幅图的重投影误差
	MonoResult calibrateMono(const std::vector<std::vector<cv::Point2f>>& image_points,
		cv::Size image_size, cv::Size board_size, double square_size, int flags = 0);

	// 双目标定：返回 R、T、E、F 与基线长度
	// flags 传 cv::CALIB_FIX_INTRINSIC 时固定各自单目标定得到的内参，只优化相对位姿
	StereoResult calibrateStereo(const std::vector<std::vector<cv::Point2f>>& left_points,
		const std::vector<std::vector<cv::Point2f>>& right_points,
		cv::Size image_size, cv::Size board_size, double square_size,
		const MonoResult& left, const MonoResult& right, int flags = 0);

	// 立体校正
	RectifyResult rectify(const MonoResult& left, const MonoResult& right,
		const StereoResult& stereo, cv::Size image_size, double alpha = -1.0);

	// 畸变校正：按单目标定结果做反向映射
	cv::Mat undistortImage(const cv::Mat& src, const MonoResult& calib);

	// 立体校正映射表与重映射
	void getRectifyMaps(const MonoResult& left, const MonoResult& right, const RectifyResult& rect,
		cv::Mat& map1x, cv::Mat& map1y, cv::Mat& map2x, cv::Mat& map2y);
	cv::Mat remapImage(const cv::Mat& src, const cv::Mat& mapx, const cv::Mat& mapy);

	// 质量度量
	double rotationAngleDeg(const cv::Mat& R1, const cv::Mat& R2);   // 两个旋转矩阵之间的夹角（度）
	double baselineLength(const cv::Mat& T);                          // 基线长度 |T|
	void intrinsicsError(const cv::Mat& K, const cv::Mat& K_ref,
		double& fx_err, double& fy_err, double& cx_err, double& cy_err);   // 相对误差
	// 校正后同名点的极线对齐误差：|v_left - v_right| 的均值与最大值（像素）
	void epipolarError(const std::vector<std::vector<cv::Point2f>>& left_points,
		const std::vector<std::vector<cv::Point2f>>& right_points,
		const cv::Mat& K1, const cv::Mat& D1, const cv::Mat& R1, const cv::Mat& P1,
		const cv::Mat& K2, const cv::Mat& D2, const cv::Mat& R2, const cv::Mat& P2,
		double& mean_abs_dv, double& max_abs_dv);
}
