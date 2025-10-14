
// MFC_PicViewerDlg.h: 头文件
//

#pragma once
#include "MyButton.h"
#include "MyStatic.h"
#include "MyOpenCV.h"
#include "MyImageStatic.h"
#include "MyFilter.h"
#include "MyNoise.h"
#include "MyTransforms.h"
#include<opencv2/opencv.hpp>
using namespace cv;
// CMFCPicViewerDlg 对话框
#define MSG_MENU_ID 1100
#define MSG_MENU_OPEN 1111
#define MSG_MENU_SAVETO 1112
#define MSG_MENU_SAVEASTO 1113
#define MSG_MENU_GRAYIZE 1114
#define MSG_MENU_BINARYIZE 1115
#define MSG_MENU_UNDO 1116
#define MSG_MENU_RELOAD 1117
#define MSG_MENU_ABOUT 1118
#define MSG_MENU_SHIFT 1119
#define MSG_MENU_SCALE 1120
#define MSG_MENU_ROTATE 1121
#define MSG_MENU_HISTOGRAM 1122
#define MSG_MENU_HISTOGRAM_GRAY 1123
#define MSG_MENU_TRANSFORM 1124
#define MSG_MENU_NEGATIVE 1125
#define MSG_MENU_NEGATIVE_COLORFUL 1126
#define MSG_MENU_LINEAR_GRAYSCALE 1127
#define MSG_MENU_LINEAR_GRAYSCALE_MULTIRANGES 1128
#define MSG_MENU_NONLINEAR_LOG 1129
#define MSG_MENU_NONLINEAR_EXP 1130
#define MSG_MENU_DHE 1131
#define MSG_MENU_DHS 1132
#define MSG_MENU_FILTER_MAX 1133
#define MSG_MENU_FILTER_MIN 1134
#define MSG_MENU_FILTER_MEDIAN 1135
#define MSG_MENU_FILTER_MEAN 1136
#define MSG_MENU_NOISE_AWGN 1137
#define MSG_MENU_NOISE_POISSON_GAUSSIAN 1138
#define MSG_MENU_NOISE_ELD_SFRN 1139

class CMFCPicViewerDlg : public CDialogEx
{
// 构造
public:
	CMFCPicViewerDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFC_PICVIEWER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
public:
	CMyButton m_btnExit;
	CMyButton m_btnMax;
	CMyButton m_btnMin;
	CRect m_rtBtnfile;
	CRect m_rtBtnEdit;
	CRect m_rtBtnEnhance;
	CRect m_rtBtnFilterate;
	CRect m_rtBtnNoise;
	CRect m_rtBtnAnalyze;
	CRect m_rtBtnHelp;
	// 生成的消息映射函数
	CMyStatic m_myText;
	CMyStatic m_myText_Analyse_Title;
	// 重写字体颜色
	BOOL m_isrestore;
	CRect m_wndRect;//保存窗口尺寸
	CImageStatic m_picStatic_Analyze; // 自定义图片
	CImageStatic m_picStatic;//自定义图片
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void CVMat_to_Pic(cv::Mat& img, UINT nID);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg cv::String Cstring_to_cvString(CString cstr);

	afx_msg void OnMenuOpen();
	afx_msg void OnMenuSaveTo();
	afx_msg void OnMenuSaveAsTo();
	afx_msg void OnMenuGrayize();
	afx_msg void OnMenuBinaryize();
	afx_msg void OnMenuUndo();
	afx_msg void OnMenuReload();
	afx_msg void OnMenuAbout();
	afx_msg void OnMenuShift();
	afx_msg void OnMenuScale();
	afx_msg void OnMenuRotate();
	afx_msg void OnMenuHistogram();
	afx_msg void OnMenuHistogramGray();
	afx_msg void OnMenuTransform(Mat (*func)(Mat),Mat input);
	afx_msg void OnMenuNegative();
	afx_msg void OnMenuNegativeColorful();
	afx_msg void OnMenuLinearGrayScale();
	afx_msg void OnMenuLinearGrayScaleMultiranges();
	afx_msg void OnMenuNonlinearLog();
	afx_msg void OnMenuNonlinearExp();
	afx_msg void OnMenuDHE();
	afx_msg void OnMenuDHS();
	afx_msg void OnMenuSpatialFilterMax();
	afx_msg void OnMenuSpatialFilterMin();
	afx_msg void OnMenuSpatialFilterMedian();
	afx_msg void OnMenuSpatialFilterMean();
	afx_msg void OnMenuNoiseAWGN();
	afx_msg void OnMenuNoisePoissonGaussian();
	afx_msg void OnMenuNoiseELDSFRN();
};
