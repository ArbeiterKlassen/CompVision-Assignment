
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
#include "MyEditControl.h"
#include "MyINIReader.h"
#include "ImageDocument.h"
#include <opencv2/opencv.hpp>
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
#define MSG_MENU_OPTION 1118
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
#define MSG_MENU_SAVE_ANALYZE_TO 1140
#define MSG_MENU_DFT 1141
#define MSG_MENU_IDFT 1142
#define MSG_MENU_FILTER_BUTTERWORTH_LOW 1143
#define MSG_MENU_FILTER_BUTTERWORTH_HIGH 1144
#define MSG_MENU_BINARYIZE_OTSU 1145
#define MSG_MENU_BINARYIZE_REGIONGROW 1146
#define MSG_MENU_EDGE_ROBERT 1147
#define MSG_MENU_EDGE_PREWITT 1148
#define MSG_MENU_EDGE_SOBEL 1149
#define MSG_MENU_EDGE_FRIECHEN 1150
#define MSG_MENU_EDGE_LAPLACIAN 1151
#define MSG_MENU_EDGE_LOG 1152
#define MSG_MENU_EDGE_CANNY 1153
#define MSG_MENU_LOAD_PROPERTIES 1154
#define MSG_MENU_EDGE_WATERSHED 1155
#define MSG_MENU_REDO 1156
#define MSG_MENU_GAMMA 1157
#define MSG_MENU_CLAHE 1158
#define MSG_MENU_MORPH_ERODE 1159
#define MSG_MENU_MORPH_DILATE 1160
#define MSG_MENU_MORPH_OPEN 1161
#define MSG_MENU_MORPH_CLOSE 1162
#define MSG_MENU_FILTER_BILATERAL 1163
#define MSG_MENU_HOUGH 1164
#define MSG_MENU_HARRIS 1165
#define MSG_MENU_MATCH_OPEN_SECOND 1166
#define MSG_MENU_MATCH_ORB 1167
#define MSG_MENU_MATCH_SIFT 1168
#define MSG_MENU_MATCH_AKAZE 1169


struct ImageOpDef;   // 图像操作注册表条目（定义见 MFC_PicViewerDlg.cpp）

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
	CRect m_rBtnOption;
	CRect m_rtBtnMatch;
	// 生成的消息映射函数
	CMyStatic m_myText;
	CMyStatic m_myText_Analyse_Title;
	// 重写字体颜色
	BOOL m_isrestore;
	CRect m_wndRect;//保存窗口尺寸
	CImageStatic m_picStatic_Analyze; // 自定义图片
	CImageStatic m_picStatic;//自定义图片
	CMyEditControl Info_Bar_Cedit;
	ImageDocument m_doc;   // 图像状态（原始图/当前图/分析图/参考图 + 撤销重做栈）
	UINT m_infoLine = 0;   // 信息栏行号
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
	afx_msg void CVMat_to_Pic(const cv::Mat& img, UINT nID);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg cv::String Cstring_to_cvString(CString cstr);

	afx_msg void OnMenuOpen();
	afx_msg void OnMenuSaveTo();
	afx_msg void OnMenuSaveAsTo();
	afx_msg void OnMenuUndo();
	afx_msg void OnMenuRedo();
	afx_msg void OnMenuReload();
	afx_msg void OnMenuOption();
	afx_msg void OnMenuHistogram();
	afx_msg void OnMenuHistogramGray();
	afx_msg void OnMenuDHS();
	afx_msg void OnMenuSaveAnalyzeTo();
	afx_msg void OnMenuLoadProperties();
	afx_msg void OnMenuOperationRange(UINT nID);
	afx_msg void OnMenuMatchOpenSecond();
	afx_msg void OnMenuMatchORB();
	afx_msg void OnMenuMatchSIFT();
	afx_msg void OnMenuMatchAKAZE();

	void Info_append(std::string str);
	void Info_clear();
	void RunFeatureMatch(UINT MODULE, CString method_name);   // 特征匹配统一流程

private:
	const ImageOpDef* FindOp(UINT nID);
	void RunOperation(UINT nID);    // 注册表统一执行流程
	void ShowPopupMenu(int index);  // 按声明式菜单表弹出菜单
	void ApplyRoundedCorners();     // 窗口圆角：Win11 用 DWM 圆角，其余系统回退到区域裁剪
	void ApplyRoundedRegion();      // 回退方案：用窗口区域裁出圆角
	bool m_rounded_by_region = false;   // 是否走了区域裁剪方案
};
