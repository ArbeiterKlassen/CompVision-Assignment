
// MFC_PicViewerDlg.cpp: 实现文件
//
//程序头
#include "pch.h"
#include "framework.h"
#include "MFC_PicViewer.h"
#include "MFC_PicViewerDlg.h"
#include "afxdialogex.h"
//自定义头
#include "MyMenu.h"
#include "MyButton.h"
#include "MyStatic.h"
#include "MyOpenCV.h"
#include "MyImageStatic.h"
#include "MyTransforms.h"
#include "MyFilter.h"
#include "MyNoise.h"
#include "MyMorphology.h"
#include "MyFeatureMatch.h"
#include "MyEditControl.h"
#include "MyINIReader.h"
#include "ImageDocument.h"
//STL头
#include <opencv2/opencv.hpp>
#include <stack>
#include <cmath>
#include <algorithm>
#include <complex>
#include <vector>
#include <map>
#include <functional>
#include <string>

//窗口头
#include "MFC_PicViewer_OptionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
using namespace cv;

// ===================== 算法参数区 =====================
// 参数统一收敛到 AppParams 结构体，由 properties.ini 驱动。
// 后续可在此基础上增加序列化/保存到配置文件的能力。
struct AppParams {
    // ---- 变换 ----
    float scale_x = 1.1f;
    float scale_y = 1.3f;
    float rotate_theta = 90.0f;
    UINT shift_x = 50;
    UINT shift_y = 50;
    float logK = 2.0f;
    float expK = 0.5f;
    float gamma = 1.0f;               // Gamma 校正指数
    std::pair<int, int> output_grayscale{ 100, 155 };
    std::vector<std::pair<int, int>> oper_ranges{ {0,49},{50,199},{200,255} };
    std::vector<std::pair<int, int>> gray_scales{ {0,24},{50,224},{227,255} };

    // ---- 滤波 ----
    UINT filter_kernel = 7;
    double butterworth_D0 = 500.0;
    UINT butterworth_level = 1;
    int bilateral_d = 9;              // 双边滤波直径
    double bilateral_sigma_color = 75.0;
    double bilateral_sigma_space = 75.0;

    // ---- 形态学 ----
    UINT morphology_kernel = 3;

    // ---- 噪声 ----
    float noise_AWGN_sigma = 25.0f;
    double noise_Poisson_Gaussian_gain = 4.0;
    double noise_Poisson_Gaussian_sigma = 5.0;
    double noise_ELD_SFRN_alpha = 4.0;
    double noise_ELD_SFRN_sigmaT = 3.0;
    double noise_ELD_SFRN_sigmaG = 2.0;
    double noise_ELD_SFRN_q = 1.0;

    // ---- 直方图增强 ----
    double clahe_clip_limit = 2.0;
    UINT clahe_tile_size = 8;

    // ---- 检测 ----
    double hough_threshold = 100;
    double hough_min_line_length = 50;
    double hough_max_line_gap = 10;
    double harris_threshold = 0.01;
    double harris_k = 0.04;

    // ---- 特征匹配 ----
    double match_ratio = 0.75;
    int match_max_features = 1000;
    double match_ransac_threshold = 3.0;
    int match_max_draw_matches = 100;

    // ---- 边缘检测 ----
    double default_edge_threshold = 100;
    double default_edge_laplacian_level1_threshold = 100;
    double default_edge_laplacian_level2_threshold = 10;
    double default_edge_canny_low_threshold = 5;
    double default_edge_canny_high_threshold = 150;
    int default_edge_watershed_mode = 1;

    double edge_threshold = 100;
    double edge_laplacian_level1_threshold = 100;
    double edge_laplacian_level2_threshold = 10;
    double edge_canny_low_threshold = 5;
    double edge_canny_high_threshold = 150;
    int edge_watershed_mode = 1;

    bool auto_estimate_threshold = true;
};

static AppParams g_params;

// ===================== 声明式菜单定义 =====================
// 菜单栏矩形（m_rtBtnfile 等）与 kMenuBars 按相同顺序一一对应，
// 具体对应关系见 OnLButtonDown()。
struct MenuItemDef {
    UINT id;
    LPCTSTR text;
};
struct MenuBarDef {
    int popupX;                 // 弹出菜单位置：窗口左边距
    const MenuItemDef* items;
    int itemCount;
};

static const MenuItemDef kFileMenuItems[] = {
    { MSG_MENU_OPEN, _T("打开") },
    { MSG_MENU_SAVETO, _T("保存图像到...") },
    { MSG_MENU_SAVE_ANALYZE_TO, _T("保存分析结果到...") },
    { MSG_MENU_SAVEASTO, _T("另存为...") },
    { MSG_MENU_UNDO, _T("撤销") },
    { MSG_MENU_REDO, _T("重做") },
    { MSG_MENU_RELOAD, _T("恢复") },
};

static const MenuItemDef kEditMenuItems[] = {
    { MSG_MENU_GRAYIZE, _T("转换为灰度值") },
    { MSG_MENU_BINARYIZE, _T("转换为二值图") },
    { MSG_MENU_BINARYIZE_OTSU, _T("转换为二值图(Otsu)") },
    { MSG_MENU_BINARYIZE_REGIONGROW, _T("转换为二值图(区域增长)") },
    { MSG_MENU_SHIFT, _T("平移操作") },
    { MSG_MENU_SCALE, _T("放缩操作") },
    { MSG_MENU_ROTATE, _T("旋转操作") },
    { MSG_MENU_NEGATIVE_COLORFUL, _T("色彩反转操作") },
    { MSG_MENU_NEGATIVE, _T("灰度反转操作") },
    { MSG_MENU_LINEAR_GRAYSCALE, _T("线性灰度变换") },
    { MSG_MENU_LINEAR_GRAYSCALE_MULTIRANGES, _T("分段线性灰度变换") },
    { MSG_MENU_NONLINEAR_LOG, _T("对数灰度变换") },
    { MSG_MENU_NONLINEAR_EXP, _T("指数灰度变换") },
    { MSG_MENU_GAMMA, _T("Gamma校正") },
    { MSG_MENU_DFT, _T("离散傅里叶变换") },
    { MSG_MENU_IDFT, _T("逆离散傅里叶变换") },
};

static const MenuItemDef kEnhanceMenuItems[] = {
    { MSG_MENU_DHE, _T("离散直方图均衡化") },
    { MSG_MENU_DHS, _T("离散直方图规定化") },
    { MSG_MENU_CLAHE, _T("CLAHE局部直方图均衡化") },
    { MSG_MENU_EDGE_ROBERT, _T("边缘提取（Robert算子）") },
    { MSG_MENU_EDGE_PREWITT, _T("边缘提取（Prewitt算子）") },
    { MSG_MENU_EDGE_SOBEL, _T("边缘提取（Sobel算子）") },
    { MSG_MENU_EDGE_FRIECHEN, _T("边缘提取（Frie-Chen算子）") },
    { MSG_MENU_EDGE_LAPLACIAN, _T("边缘提取（Laplacian算子）") },
    { MSG_MENU_EDGE_LOG, _T("边缘提取（LOG算子）") },
    { MSG_MENU_EDGE_CANNY, _T("边缘提取（Canny算子）") },
    { MSG_MENU_EDGE_WATERSHED, _T("边缘提取（分水岭）") },
    { MSG_MENU_HOUGH, _T("Hough直线检测") },
    { MSG_MENU_HARRIS, _T("Harris角点检测") },
};

static const MenuItemDef kFilterMenuItems[] = {
    { MSG_MENU_FILTER_MAX, _T("邻域最大值滤波") },
    { MSG_MENU_FILTER_MIN, _T("邻域最小值滤波") },
    { MSG_MENU_FILTER_MEDIAN, _T("邻域中值滤波") },
    { MSG_MENU_FILTER_MEAN, _T("邻域均值滤波") },
    { MSG_MENU_FILTER_BUTTERWORTH_LOW, _T("巴特沃斯低通滤波") },
    { MSG_MENU_FILTER_BUTTERWORTH_HIGH, _T("巴特沃斯高通滤波") },
    { MSG_MENU_FILTER_BILATERAL, _T("双边滤波") },
    { MSG_MENU_MORPH_ERODE, _T("形态学-腐蚀") },
    { MSG_MENU_MORPH_DILATE, _T("形态学-膨胀") },
    { MSG_MENU_MORPH_OPEN, _T("形态学-开运算") },
    { MSG_MENU_MORPH_CLOSE, _T("形态学-闭运算") },
};

static const MenuItemDef kNoiseMenuItems[] = {
    { MSG_MENU_NOISE_AWGN, _T("加性高斯白噪声") },
    { MSG_MENU_NOISE_POISSON_GAUSSIAN, _T("泊松-高斯很合噪声") },
    { MSG_MENU_NOISE_ELD_SFRN, _T("信号相关联合噪声") },
};

static const MenuItemDef kAnalyzeMenuItems[] = {
    { MSG_MENU_HISTOGRAM, _T("色彩直方图分析") },
    { MSG_MENU_HISTOGRAM_GRAY, _T("灰度直方图分析") },
};

static const MenuItemDef kOptionMenuItems[] = {
    { MSG_MENU_LOAD_PROPERTIES, _T("参数载入") },
    { MSG_MENU_OPTION, _T("参数配置") },
};

static const MenuItemDef kMatchMenuItems[] = {
    { MSG_MENU_MATCH_OPEN_SECOND, _T("打开第二张图像") },
    { MSG_MENU_MATCH_ORB, _T("ORB 特征匹配") },
    { MSG_MENU_MATCH_SIFT, _T("SIFT 特征匹配") },
    { MSG_MENU_MATCH_AKAZE, _T("AKAZE 特征匹配") },
};

static const MenuBarDef kMenuBars[] = {
    { 10,  kFileMenuItems,    _countof(kFileMenuItems) },
    { 70,  kEditMenuItems,    _countof(kEditMenuItems) },
    { 130, kEnhanceMenuItems, _countof(kEnhanceMenuItems) },
    { 190, kFilterMenuItems,  _countof(kFilterMenuItems) },
    { 250, kNoiseMenuItems,   _countof(kNoiseMenuItems) },
    { 310, kAnalyzeMenuItems, _countof(kAnalyzeMenuItems) },
    { 370, kOptionMenuItems,  _countof(kOptionMenuItems) },
    { 430, kMatchMenuItems,   _countof(kMatchMenuItems) },
};

// ===================== 图像操作注册表 =====================
// 每个图像操作（灰度化、滤波、边缘检测等）声明为一个条目：
//   - run        : 纯计算函数（输入当前图，输出结果图）
//   - log        : 完成后的信息栏日志
//   - needsFreqDomain : 要求当前图为傅里叶频域 (CV_32FC2)
//   - toBgr      : 单通道结果转回 BGR 再存储（与重构前各 handler 行为一致）
struct ImageOpDef {
    UINT id;
    LPCTSTR menuText;                             // 说明性字段（菜单文本由菜单表维护）
    bool needsFreqDomain = false;
    bool toBgr = true;
    std::function<cv::Mat(const cv::Mat&)> run;
    std::function<std::string()> log;
};

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMFCPicViewerDlg 对话框

CMFCPicViewerDlg::CMFCPicViewerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFC_PICVIEWER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCPicViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON1, m_btnMax);
	DDX_Control(pDX, IDC_BUTTON2, m_btnMin);
	DDX_Control(pDX, IDC_BUTTON3, m_btnExit);
	DDX_Control(pDX, IDC_STATIC_TITLE, m_myText);
	DDX_Control(pDX, IDC_STATIC_TITLE_ANALYZE, m_myText_Analyse_Title);
	DDX_Control(pDX, IDC_STATIC_PIC, m_picStatic);
	DDX_Control(pDX, IDC_STATIC_PIC_ANALYZE, m_picStatic_Analyze);
	DDX_Control(pDX, IDC_INFOBAR, Info_Bar_Cedit);
}

BEGIN_MESSAGE_MAP(CMFCPicViewerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_NCHITTEST()
	ON_WM_QUERYDRAGICON()
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(IDC_BUTTON1, &CMFCPicViewerDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CMFCPicViewerDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CMFCPicViewerDlg::OnBnClickedButton3)

	ON_COMMAND(MSG_MENU_OPEN, &CMFCPicViewerDlg::OnMenuOpen)
	ON_COMMAND(MSG_MENU_SAVETO, &CMFCPicViewerDlg::OnMenuSaveTo)
	ON_COMMAND(MSG_MENU_SAVEASTO, &CMFCPicViewerDlg::OnMenuSaveAsTo)

	// 图像操作（灰度化 ~ 分水岭、撤销/重做、特征匹配等）统一分发
	ON_COMMAND_RANGE(MSG_MENU_GRAYIZE, MSG_MENU_MATCH_AKAZE, &CMFCPicViewerDlg::OnMenuOperationRange)
END_MESSAGE_MAP()


// CMFCPicViewerDlg 消息处理程序

BOOL CMFCPicViewerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	//退出按钮绘制
	CRect rtBtnClo;
	GetClientRect(&rtBtnClo);
	rtBtnClo.left = rtBtnClo.right - 20;
	m_btnExit.SetImagePath(_T("res\\icon_popup_off.png"), _T(".\\res\\icon_popup_off.png"), _T(".\\res\\icon_popup_off.png"));
	m_btnExit.InitMyButton(rtBtnClo.left, 5, 16, 16, true);
	//最大化按钮绘制
	GetClientRect(&rtBtnClo);
	rtBtnClo.left = rtBtnClo.right - 80;
	m_btnMax.SetImagePath(_T(".\\res\\icon_square.png"), _T(".\\res\\icon_square.png"), _T(".\\res\\icon_square.png"));
	m_btnMax.InitMyButton(rtBtnClo.left, 5, 16, 16, true);
	//最小化按钮
	// GetClientRect(&rtBtnClo);
	rtBtnClo.left = rtBtnClo.right - 50;
	m_btnMin.SetImagePath(_T(".\\res\\icon_minimiz.png"), _T(".\\res\\icon_minimiz.png"), _T(".\\res\\icon_minimiz.png"));
	m_btnMin.InitMyButton(rtBtnClo.left, 5, 16, 16, true);
	//dwStyle = (dwStyle | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	ApplyRoundedCorners();
	OnMenuLoadProperties();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMFCPicViewerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMFCPicViewerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CPaintDC dc(this);
		CRect rect;
		//------------------
		//修改字体
		HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);  //获取系统默认GUI字体
		LOGFONT logfont;
		GetObject(hFont, sizeof(LOGFONT), &logfont);
		wcscpy_s(logfont.lfFaceName, L"黑体");//改变为宋体
		//logfont.lfWeight = 60;
		logfont.lfHeight = 15;
		HFONT hNewFont = NULL;
		hNewFont = CreateFontIndirect(&logfont); //改变系统默认的字体，设为宋体，创建了一个新的HFONT 
		HFONT loldfont = (HFONT)(SelectObject(dc, hNewFont));//选上新创建的这个，返回的是旧的
		CRect tmprect;
		GetClientRect(&rect);
		//rect.bottom = 60;
		rect.bottom = static_cast<LONG>(rect.bottom * 0.1);
		/*int nCount = 165 - 115 + 186 - 158 + 190 - 115;
		int nIncrecs = (rect.right - rect.left) / nCount;*/
		dc.FillSolidRect(rect, RGB(45, 51, 60));
		dc.SetBkMode(TRANSPARENT);
		CRect rtBtnClo;
		//绘制文件菜单栏
		GetClientRect(&rtBtnClo);
		rtBtnClo.left = rtBtnClo.left + 10;
		rtBtnClo.right = rtBtnClo.left + 30;
		rtBtnClo.bottom = static_cast<LONG>(rtBtnClo.bottom * 0.1);
		rtBtnClo.top = rtBtnClo.bottom - 20;
		m_rtBtnfile = rtBtnClo;
		dc.Rectangle(rtBtnClo);
		dc.FillSolidRect(m_rtBtnfile, RGB(45, 51, 59));
		dc.SetBkMode(TRANSPARENT);
		CString str1 = TEXT("文件");
		dc.SetTextColor(RGB(185, 192, 197));
		dc.DrawText(str1, m_rtBtnfile, DT_LEFT | DT_VCENTER);
		//CDialogEx::OnPaint();
		//绘制选项菜单栏
		GetClientRect(&rtBtnClo);
		rtBtnClo.left = rtBtnClo.left + 70;
		rtBtnClo.right = rtBtnClo.left + 30;
		rtBtnClo.bottom = static_cast<LONG>(rtBtnClo.bottom * 0.1);
		rtBtnClo.top = rtBtnClo.bottom - 20;
		m_rtBtnEdit = rtBtnClo;
		dc.Rectangle(rtBtnClo);
		dc.FillSolidRect(m_rtBtnEdit, RGB(45, 51, 59));
		dc.SetBkMode(TRANSPARENT);
		str1 = TEXT("编辑");
		dc.SetTextColor(RGB(185, 192, 197));
		dc.DrawText(str1, m_rtBtnEdit, DT_LEFT | DT_VCENTER);
		//绘制增强菜单栏
		GetClientRect(&rtBtnClo);
		rtBtnClo.left = rtBtnClo.left + 130;
		rtBtnClo.right = rtBtnClo.left + 30;
		rtBtnClo.bottom = static_cast<LONG>(rtBtnClo.bottom * 0.1);
		rtBtnClo.top = rtBtnClo.bottom - 20;
		m_rtBtnEnhance = rtBtnClo;
		dc.Rectangle(rtBtnClo);
		dc.FillSolidRect(m_rtBtnEnhance, RGB(45, 51, 59));
		dc.SetBkMode(TRANSPARENT);
		str1 = TEXT("增强");
		dc.SetTextColor(RGB(185, 192, 197));
		dc.DrawText(str1, m_rtBtnEnhance, DT_LEFT | DT_VCENTER);
		//绘制滤波菜单栏
		GetClientRect(&rtBtnClo);
		rtBtnClo.left = rtBtnClo.left + 190;
		rtBtnClo.right = rtBtnClo.left + 30;
		rtBtnClo.bottom = static_cast<LONG>(rtBtnClo.bottom * 0.1);
		rtBtnClo.top = rtBtnClo.bottom - 20;
		m_rtBtnFilterate = rtBtnClo;
		dc.Rectangle(rtBtnClo);
		dc.FillSolidRect(m_rtBtnFilterate, RGB(45, 51, 59));
		dc.SetBkMode(TRANSPARENT);
		str1 = TEXT("滤波");
		dc.SetTextColor(RGB(185, 192, 197));
		dc.DrawText(str1, m_rtBtnFilterate, DT_LEFT | DT_VCENTER);
		//绘制噪声菜单栏
		GetClientRect(&rtBtnClo);
		rtBtnClo.left = rtBtnClo.left + 250;
		rtBtnClo.right = rtBtnClo.left + 30;
		rtBtnClo.bottom = static_cast<LONG>(rtBtnClo.bottom * 0.1);
		rtBtnClo.top = rtBtnClo.bottom - 20;
		m_rtBtnNoise = rtBtnClo;
		dc.Rectangle(rtBtnClo);
		dc.FillSolidRect(m_rtBtnNoise, RGB(45, 51, 59));
		dc.SetBkMode(TRANSPARENT);
		str1 = TEXT("噪声");
		dc.SetTextColor(RGB(185, 192, 197));
		dc.DrawText(str1, m_rtBtnNoise, DT_LEFT | DT_VCENTER);
		//绘制分析菜单栏
		GetClientRect(&rtBtnClo);
		rtBtnClo.left = rtBtnClo.left + 310;
		rtBtnClo.right = rtBtnClo.left + 30;
		rtBtnClo.bottom = static_cast<LONG>(rtBtnClo.bottom * 0.1);
		rtBtnClo.top = rtBtnClo.bottom - 20;
		m_rtBtnAnalyze = rtBtnClo;
		dc.Rectangle(rtBtnClo);
		dc.FillSolidRect(m_rtBtnAnalyze, RGB(45, 51, 59));
		dc.SetBkMode(TRANSPARENT);
		str1 = TEXT("分析");
		dc.SetTextColor(RGB(185, 192, 197));
		dc.DrawText(str1, m_rtBtnAnalyze, DT_LEFT | DT_VCENTER);
		//绘制配置菜单栏
		GetClientRect(&rtBtnClo);
		rtBtnClo.left = rtBtnClo.left + 370;
		rtBtnClo.right = rtBtnClo.left + 30;
		rtBtnClo.bottom = static_cast<LONG>(rtBtnClo.bottom * 0.1);
		rtBtnClo.top = rtBtnClo.bottom - 20;
		m_rBtnOption = rtBtnClo;
		dc.Rectangle(rtBtnClo);
		dc.FillSolidRect(m_rBtnOption, RGB(45, 51, 59));
		dc.SetBkMode(TRANSPARENT);
		str1 = TEXT("配置");
		dc.SetTextColor(RGB(185, 192, 197));
		dc.DrawText(str1, m_rBtnOption, DT_LEFT | DT_VCENTER);
		//绘制特征匹配菜单栏
		GetClientRect(&rtBtnClo);
		rtBtnClo.left = rtBtnClo.left + 430;
		rtBtnClo.right = rtBtnClo.left + 30;
		rtBtnClo.bottom = static_cast<LONG>(rtBtnClo.bottom * 0.1);
		rtBtnClo.top = rtBtnClo.bottom - 20;
		m_rtBtnMatch = rtBtnClo;
		dc.Rectangle(rtBtnClo);
		dc.FillSolidRect(m_rtBtnMatch, RGB(45, 51, 59));
		dc.SetBkMode(TRANSPARENT);
		str1 = TEXT("匹配");
		dc.SetTextColor(RGB(185, 192, 197));
		dc.DrawText(str1, m_rtBtnMatch, DT_LEFT | DT_VCENTER);

		SelectObject(dc, loldfont);
		DeleteObject(hNewFont);
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMFCPicViewerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CMFCPicViewerDlg::OnNcHitTest(CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	UINT nHitTest = static_cast<UINT>(CDialogEx::OnNcHitTest(point));
	CRect rect;
	GetClientRect(&rect);
	rect.bottom = 70;
	//函数参数point是相对于屏幕坐标的，需要将其转换为
	//客户区坐标才能使用PtInRect()，否则会因为坐标的不同使判断失误
	//rect.left = rect.left + 200;
	ScreenToClient(&point);
	if (rect.PtInRect(point))
	{
		if (HTCLIENT == nHitTest)
			nHitTest = HTCAPTION;
		//如果鼠标点中的是关闭按钮的位置，需要将上一步的设置还原，
		if (m_rtBtnfile.PtInRect(point) ||
			m_rtBtnEdit.PtInRect(point) ||
			m_rtBtnEnhance.PtInRect(point) ||
			m_rtBtnFilterate.PtInRect(point) ||
			m_rtBtnNoise.PtInRect(point) ||
			m_rtBtnAnalyze.PtInRect(point) ||
			m_rtBtnMatch.PtInRect(point) ||
			m_rBtnOption.PtInRect(point))
		{
			nHitTest = HTCLIENT;
		}
	}
	return nHitTest;
}

void CMFCPicViewerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	//重新获得窗口尺寸
	GetClientRect(&m_wndRect);

	//退出按钮重新绘制
	CWnd* pWnd = GetDlgItem(IDC_BUTTON3);
	if (pWnd && nType != 1 && m_wndRect.Width() && m_wndRect.Height())
	{
		m_btnExit.ReleaseImg();
		CRect rect;   //获取控件变化前的大小 
		pWnd->GetWindowRect(&rect);
		ScreenToClient(&rect);//将控件大小转换为在对话框中的区域坐标
		rect.left = m_wndRect.right - 20;
		rect.top = 5;
		rect.right = rect.left + 16;
		rect.bottom = rect.top + 16;
		m_btnExit.SetImagePath(_T(".\\res\\icon_popup_off.png"), _T(".\\res\\icon_popup_off.png"), _T(".\\res\\icon_popup_off.png"));
		m_btnExit.InitMyButton(rect.left, rect.top, 16, 16, true);
	}
	//最大化按钮重新绘制


	pWnd = GetDlgItem(IDC_BUTTON1);
	if (pWnd && nType != 1 && m_wndRect.Width() && m_wndRect.Height())
	{
		m_btnMax.ReleaseImg();
		CRect rect;   //获取控件变化前的大小 
		pWnd->GetWindowRect(&rect);
		ScreenToClient(&rect);//将控件大小转换为在对话框中的区域坐标
		rect.left = m_wndRect.right - 50;
		rect.top = 5;
		rect.right = rect.left + 16;
		rect.bottom = rect.top + 16;

		m_btnMax.SetImagePath(_T(".\\res\\icon_square.png"), _T(".\\res\\icon_square.png"), _T(".\\res\\icon_square.png"));

		m_btnMax.InitMyButton(rect.left, rect.top, 16, 16, true);
	}



	//最小化按钮重新绘制

	pWnd = GetDlgItem(IDC_BUTTON2);
	if (pWnd && nType != 1 && m_wndRect.Width() && m_wndRect.Height())
	{
		m_btnMin.ReleaseImg();
		CRect rect;   //获取控件变化前的大小 
		pWnd->GetWindowRect(&rect);
		ScreenToClient(&rect);//将控件大小转换为在对话框中的区域坐标
		rect.left = m_wndRect.right - 80;
		rect.top = 5;
		rect.right = rect.left + 16;
		rect.bottom = rect.top + 16;
		m_btnMin.SetImagePath(_T(".\\res\\icon_minimiz.png"), _T(".\\res\\icon_minimiz.png"), _T(".\\res\\icon_minimiz.png"));
		m_btnMin.InitMyButton(rect.left, rect.top, 16, 16, true);
	}
	// 区域裁剪方案需要在窗口尺寸变化后重新计算圆角区域
	if (m_rounded_by_region)ApplyRoundedRegion();
	Invalidate();
	// TODO: 在此处添加消息处理程序代码
}

// 窗口圆角：优先请求 Windows 11 的 DWM 圆角（抗锯齿、跟随系统视觉设置），
// 系统不支持时回退为窗口区域裁剪。
// 注意：本对话框是无边框窗口（WS_POPUP，没有 WS_CAPTION/WS_THICKFRAME），
// DWM 只对"带边框"的窗口应用圆角，因此实际生效的通常是区域裁剪方案。
void CMFCPicViewerDlg::ApplyRoundedCorners()
{
	HMODULE hDwmApi = ::LoadLibraryW(L"dwmapi.dll");
	if (hDwmApi != nullptr)
	{
		typedef HRESULT(WINAPI* PFN_DwmSetWindowAttribute)(HWND, DWORD, LPCVOID, DWORD);
		PFN_DwmSetWindowAttribute pDwmSetWindowAttribute =
			reinterpret_cast<PFN_DwmSetWindowAttribute>(::GetProcAddress(hDwmApi, "DwmSetWindowAttribute"));
		if (pDwmSetWindowAttribute != nullptr)
		{
			const DWORD DWMWA_WINDOW_CORNER_PREFERENCE_LOCAL = 33;   // Windows 11 SDK 中的 DWMWA_WINDOW_CORNER_PREFERENCE
			const int DWMWCP_ROUND_LOCAL = 2;                        // DWM_WINDOW_CORNER_PREFERENCE::DWMWCP_ROUND
			int corner_preference = DWMWCP_ROUND_LOCAL;
			HRESULT hr = pDwmSetWindowAttribute(m_hWnd, DWMWA_WINDOW_CORNER_PREFERENCE_LOCAL,
				&corner_preference, sizeof(corner_preference));
			if (SUCCEEDED(hr))
			{
				::FreeLibrary(hDwmApi);
				Info_append("窗口圆角：DWM 圆角请求成功");
				return;
			}
		}
		::FreeLibrary(hDwmApi);
	}

	m_rounded_by_region = true;
	ApplyRoundedRegion();
	Info_append("窗口圆角：使用区域裁剪方案");
}

void CMFCPicViewerDlg::ApplyRoundedRegion()
{
	// 最大化/最小化时不需要圆角，否则屏幕边缘会出现缺口
	if (IsZoomed() || IsIconic())
	{
		::SetWindowRgn(m_hWnd, nullptr, TRUE);
		return;
	}
	CRect rc;
	GetWindowRect(&rc);
	const int radius = 12;   // 圆角半径（像素）
	HRGN hRgn = ::CreateRoundRectRgn(0, 0, rc.Width() + 1, rc.Height() + 1, radius * 2, radius * 2);
	::SetWindowRgn(m_hWnd, hRgn, TRUE);   // 设置成功后由系统负责释放 hRgn
}

void CMFCPicViewerDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_isrestore)
	{
		ShowWindow(SW_RESTORE);
	}
	else
	{
		ShowWindow(SW_MAXIMIZE);
	}
	m_isrestore = !m_isrestore;
}

void CMFCPicViewerDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	ShowWindow(SW_SHOWMINIMIZED);
}

void CMFCPicViewerDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialog::OnCancel();
}

HBRUSH CMFCPicViewerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	switch (nCtlColor)
	{
	case CTLCOLOR_DLG:
		HBRUSH aBrush;
		aBrush = CreateSolidBrush(RGB(30, 34, 39));
		hbr = aBrush;
		break;
	}

	if (nCtlColor == CTLCOLOR_STATIC)
	{
	}
	return hbr;
}

void CMFCPicViewerDlg::CVMat_to_Pic(const cv::Mat& img, UINT nID)
{
	// 主显示区显示的是哪个图像就记录哪个：
	// 特征匹配结果是"两图并排+连线"图，它不替换当前工作图像，
	// 保存图像（保存到/另存为）时以显示内容为准，避免存成原图。
	if (nID == IDC_STATIC_PIC)img.copyTo(m_main_display);
	Mat _img;
	img.copyTo(_img);
	if (_img.type() == CV_32FC2) {
		_img = Transforms::Fourier::spectrumView(_img);
	}
	if (_img.channels() == 1) {
		cvtColor(_img, _img, COLOR_GRAY2BGR);
	}
	CWnd* pWnd = GetDlgItem(nID);
	if (!pWnd) return;

	CRect rect;
	pWnd->GetClientRect(&rect);
	int ctrlW = rect.Width();
	int ctrlH = rect.Height();

	if (_img.empty() || ctrlW <= 0 || ctrlH <= 0)
		return;

	// 计算缩放比例（保持宽高比）
	double scaleX = static_cast<double>(ctrlW) / _img.cols;
	double scaleY = static_cast<double>(ctrlH) / _img.rows;
	double scale = min(scaleX, scaleY);  // 取较小比例，防止溢出

	int dstW = cvRound(_img.cols * scale);
	int dstH = cvRound(_img.rows * scale);

	// 计算居中偏移
	int offsetX = (ctrlW - dstW) / 2;
	int offsetY = (ctrlH - dstH) / 2;

	// 等比例缩放图像
	cv::Mat imgResized;
	cv::resize(_img, imgResized, cv::Size(dstW, dstH));

	// 创建空白画布（填充背景色），注意画布的宽度和高度是控件的尺寸
	// 但我们需要确保画布的每行字节数是4的倍数
	int stride = (ctrlW * 3 + 3) & ~3;  // 计算4的倍数的步长
	cv::Mat canvas(ctrlH, ctrlW, CV_8UC3, cv::Scalar(39, 34, 30));
	// 将图像复制到居中区域
	cv::Mat roi = canvas(cv::Rect(offsetX, offsetY, dstW, dstH));
	imgResized.copyTo(roi);

	// 转换为设备上下文
	CDC* pDC = pWnd->GetDC();
	if (!pDC) return;

	// 构造BITMAPINFO，并设置步长
	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = ctrlW;
	bmi.bmiHeader.biHeight = -ctrlH;  // 负值表示自上而下
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = stride * ctrlH;

	// 由于canvas的步长可能不是4的倍数，我们需要一个步长为stride的缓冲区
	std::vector<uchar> buffer(ctrlH * stride);
	for (int y = 0; y < ctrlH; ++y) {
		memcpy(&buffer[y * stride], canvas.ptr(y), ctrlW * 3);
	}

	// 将缓冲区数据绘制到控件
	::StretchDIBits(
		pDC->GetSafeHdc(),
		0, 0, ctrlW, ctrlH,
		0, 0, ctrlW, ctrlH,
		buffer.data(),
		&bmi,
		DIB_RGB_COLORS,
		SRCCOPY
	);

	ReleaseDC(pDC);
}

void CMFCPicViewerDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// 菜单栏矩形与声明式菜单表 kMenuBars 按相同顺序一一对应：
	// 文件 / 编辑 / 增强 / 滤波 / 噪声 / 分析 / 配置 / 匹配
	const CRect* menuRects[] = {
		&m_rtBtnfile, &m_rtBtnEdit, &m_rtBtnEnhance, &m_rtBtnFilterate,
		&m_rtBtnNoise, &m_rtBtnAnalyze, &m_rBtnOption, &m_rtBtnMatch
	};
	for (int i = 0; i < _countof(menuRects); ++i)
	{
		if (menuRects[i]->PtInRect(point))
		{
			ShowPopupMenu(i);
			break;
		}
	}
}

void CMFCPicViewerDlg::ShowPopupMenu(int index)
{
	if (index < 0 || index >= _countof(kMenuBars))
		return;

	const MenuBarDef& bar = kMenuBars[index];

	CMenuEx menu;
	menu.CreatePopupMenu();
	menu.SetItemHeight(30);
	menu.SetFontInfo(14);
	menu.SetBorderColor(RGB(45, 51, 60));
	menu.SetMenuWidth(130);
	menu.InstallHook(theApp.m_hInstance);

	for (int i = 0; i < bar.itemCount; ++i)
		menu.AppendItem(MF_STRING, bar.items[i].id, bar.items[i].text, _T(""), 0);

	CRect rectDlg;
	GetWindowRect(rectDlg);//获得窗体的大小
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,
		rectDlg.left + bar.popupX,
		rectDlg.top + static_cast<int>((rectDlg.bottom - rectDlg.top) * 0.104),
		this);
	menu.DestroyMenu();
	menu.UnInstallHook();
}

cv::String CMFCPicViewerDlg::Cstring_to_cvString(CString cstr) {
	CT2A ascii(cstr.GetString());
	return cv::String(static_cast<const char*>(ascii));
}

void CMFCPicViewerDlg::OnMenuOpen()
{
	// 打开文件
	CFileDialog dlg(TRUE, _T("jpg"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("图片文件 (*.jpg;*.png;*.bmp;*.jpeg;*.tiff;*.tif)|*.jpg;*.png;*.bmp;*.jpeg;*.tiff;*.tif||"));
	if (dlg.DoModal() == IDOK)
	{
		if (!m_doc.open(dlg.GetPathName()))
		{
			AfxMessageBox(_T("图像加载失败！"), MB_ICONWARNING);
			return;
		}
		CVMat_to_Pic(m_doc.current(), IDC_STATIC_PIC);
	}
}

void CMFCPicViewerDlg::OnMenuSaveTo()
{
	if (!m_doc.hasImage())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}

	// 1. 选择文件夹
	TCHAR szPath[MAX_PATH]{};
	BROWSEINFO bi{};
	bi.hwndOwner = this->m_hWnd;
	bi.lpszTitle = _T("请选择保存目录");
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if (pidl == nullptr)
		return; // 用户取消

	if (!SHGetPathFromIDList(pidl, szPath))
	{
		CoTaskMemFree(pidl);
		return;
	}
	CoTaskMemFree(pidl);

	CString strFolder = szPath;
	if (strFolder.Right(1) != _T("\\"))
		strFolder += _T("\\");

	// 2. 提取原扩展名（含点号）
	CString strExt = PathFindExtension(m_doc.sourcePath()); // .jpg 等
	if (strExt.IsEmpty())
		strExt = _T(".png");

	m_doc.setSavePath(strFolder + _T("result") + strExt);

	// 3. OpenCV 保存
	// 保存主显示区当前显示的图像（特征匹配结果是并排连线图，不替换当前工作图像）
	const cv::Mat& current = m_main_display.empty() ? m_doc.current() : m_main_display;
	bool ok = false;
	if (strExt.CompareNoCase(_T(".jpg")) == 0 || strExt.CompareNoCase(_T(".jpeg")) == 0)
		if (current.type() == CV_32FC2) {
			ok = cv::imwrite(Cstring_to_cvString(m_doc.savePath()), Transforms::Fourier::spectrumView(current),
				std::vector<int>{cv::IMWRITE_JPEG_QUALITY, 95});
		}
		else ok = cv::imwrite(Cstring_to_cvString(m_doc.savePath()), current,
			std::vector<int>{cv::IMWRITE_JPEG_QUALITY, 95});
	else
		ok = cv::imwrite(Cstring_to_cvString(m_doc.savePath()), current);

	// 4. 提示
	if (ok)
		AfxMessageBox(_T("已保存到：\n") + m_doc.savePath(), MB_ICONINFORMATION);
	else
		AfxMessageBox(_T("保存失败！"), MB_ICONERROR);
}

void CMFCPicViewerDlg::OnMenuSaveAsTo()
{
	if (!m_doc.hasImage())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}

	// 获取原扩展名（含点号）
	CString strExt = PathFindExtension(m_doc.sourcePath());
	if (strExt.IsEmpty())
		strExt = _T(".png");

	// 构造默认文件名
	CString strDefName = _T("result") + strExt;

	// 构造过滤器
	CString strFilter =
		_T("PNG 图像 (*.png)|*.png|")
		_T("JPEG 图像 (*.jpg;*.jpeg)|*.jpg;*.jpeg|")
		_T("BMP 图像 (*.bmp)|*.bmp|")
		_T("TIFF 图像 (*.tiff;*.tif)|*.tiff;*.tif|")
		_T("所有文件 (*.*)|*.*||");

	CFileDialog dlg(FALSE,          // FALSE = 保存对话框
		strExt.Mid(1),             // 默认扩展名（去掉点）
		strDefName,                // 默认文件名
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		strFilter);

	if (dlg.DoModal() != IDOK)
		return; // 用户取消

	CString savePath = dlg.GetPathName();
	CString ext = PathFindExtension(savePath);
	ext.MakeLower();

	// 保存参数
	const cv::Mat& current = m_main_display.empty() ? m_doc.current() : m_main_display;
	bool ok = false;
	if (ext == _T(".jpg") || ext == _T(".jpeg"))
	{
		// 注意：与重构前一致，此分支写入的是 m_doc.savePath()（上次"保存到"推导的路径），
		// 而非用户刚选的 savePath。属历史遗留 bug，本次重构保持原行为，仅记录。
		if (current.type() == CV_32FC2) {
			ok = cv::imwrite(Cstring_to_cvString(m_doc.savePath()), Transforms::Fourier::spectrumView(current),
				std::vector<int>{cv::IMWRITE_JPEG_QUALITY, 95});
		}
		else ok = cv::imwrite(Cstring_to_cvString(m_doc.savePath()), current,
			std::vector<int>{cv::IMWRITE_JPEG_QUALITY, 95});
	}
	else if (ext == _T(".png"))
	{
		if (current.type() == CV_32FC2) {
			ok = cv::imwrite(Cstring_to_cvString(savePath), Transforms::Fourier::spectrumView(current),
				std::vector<int>{cv::IMWRITE_PNG_COMPRESSION, 9});
		}
		else ok = cv::imwrite(Cstring_to_cvString(savePath), current,
			std::vector<int>{cv::IMWRITE_PNG_COMPRESSION, 9});
	}
	else
	{
		ok = cv::imwrite(Cstring_to_cvString(savePath), current);
	}

	if (ok)
		AfxMessageBox(_T("图像已保存至：\n") + savePath, MB_ICONINFORMATION);
	else
		AfxMessageBox(_T("保存失败！"), MB_ICONERROR);
}

void CMFCPicViewerDlg::OnMenuUndo()
{
	if (!m_doc.hasImage())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	if (!m_doc.undo())
	{
		AfxMessageBox(_T("已经是最初状态，无法继续撤销！"), MB_ICONINFORMATION);
		return;
	}
	CVMat_to_Pic(m_doc.current(), IDC_STATIC_PIC);
	Info_append("撤销一步！");
}

void CMFCPicViewerDlg::OnMenuRedo()
{
	if (!m_doc.hasImage())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	if (!m_doc.redo())
	{
		AfxMessageBox(_T("没有可以重做的操作！"), MB_ICONINFORMATION);
		return;
	}
	CVMat_to_Pic(m_doc.current(), IDC_STATIC_PIC);
	Info_append("重做一步！");
}

void CMFCPicViewerDlg::OnMenuReload()
{
	if (!m_doc.hasImage())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	m_doc.resetToOrigin();
	CVMat_to_Pic(m_doc.current(), IDC_STATIC_PIC);
	Info_append("重置完成");
}

void CMFCPicViewerDlg::OnMenuOption()
{
	MFC_PicViewer_OptionDlg* OptionDlg = new MFC_PicViewer_OptionDlg();
	OptionDlg->Create(IDD_DIALOG1);
	OptionDlg->ShowWindow(SW_SHOW);
	UpdateData(TRUE);
}

pair<int,int> Count_Gray_MinMax(Mat grayhist, UINT histSize) {
	int gmin = 256, gmax = 1;
	for (int i = 0; i < static_cast<int>(histSize); ++i) {
		gmin = gmin < cvRound(grayhist.at<float>(i)) ? gmin : cvRound(grayhist.at<float>(i));
		gmax = gmax > cvRound(grayhist.at<float>(i)) ? gmax : cvRound(grayhist.at<float>(i));
	}
	return make_pair(gmin, gmax);
}

void CMFCPicViewerDlg::OnMenuHistogram()
{
	if (!m_doc.hasImage())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	std::vector<cv::Mat> bgr;
	cv::split(m_doc.current(), bgr);      // bgr[0]=B, bgr[1]=G, bgr[2]=R

	int histSize = 256;           // 0-255
	float range[] = { 0, 256 };
	const float* histRange = { range };
	bool uniform = true, accumulate = false;

	cv::Mat b_hist, g_hist, r_hist;
	cv::calcHist(&bgr[0], 1, 0, cv::Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate);
	cv::calcHist(&bgr[1], 1, 0, cv::Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate);
	cv::calcHist(&bgr[2], 1, 0, cv::Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate);

	//画成图，窗口显示
	int hist_w = 512, hist_h = 350;
	int bin_w = cvRound((double)hist_w / histSize);
	cv::Mat histImage(hist_h, hist_w, CV_8UC3, cv::Scalar(39, 34, 30));

	// 归一化到 [0, histImage.rows]
	cv::normalize(b_hist, b_hist, 0, histImage.rows, cv::NORM_MINMAX);
	cv::normalize(g_hist, g_hist, 0, histImage.rows, cv::NORM_MINMAX);
	cv::normalize(r_hist, r_hist, 0, histImage.rows, cv::NORM_MINMAX);

	for (int i = 1; i < histSize; ++i) {
		cv::line(histImage,
			cv::Point(bin_w * (i - 1), hist_h - cvRound(b_hist.at<float>(i - 1))),
			cv::Point(bin_w * i, hist_h - cvRound(b_hist.at<float>(i))),
			cv::Scalar(255, 0, 0), 2);
		cv::line(histImage,
			cv::Point(bin_w * (i - 1), hist_h - cvRound(g_hist.at<float>(i - 1))),
			cv::Point(bin_w * i, hist_h - cvRound(g_hist.at<float>(i))),
			cv::Scalar(0, 255, 0), 2);
		cv::line(histImage,
			cv::Point(bin_w * (i - 1), hist_h - cvRound(r_hist.at<float>(i - 1))),
			cv::Point(bin_w * i, hist_h - cvRound(r_hist.at<float>(i))),
			cv::Scalar(0, 0, 255), 2);
	}
	m_doc.setAnalyze(histImage);
	CVMat_to_Pic(histImage, IDC_STATIC_PIC_ANALYZE);
	Info_append("分析完成");
}

void CMFCPicViewerDlg::OnMenuHistogramGray() {
	if (!m_doc.hasImage())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	Mat gray(m_doc.current().rows, m_doc.current().cols, CV_8UC1);
	m_doc.current().copyTo(gray);
	std::vector<cv::Mat> bgr;
	cv::split(gray, bgr);          // bgr[0]=B, bgr[1]=G, bgr[2]=R

	int histSize = 256;           // 0-255
	float range[] = { 0, 256 };
	const float* histRange = { range };
	bool uniform = true, accumulate = false;

	cv::Mat gray_hist, g_hist, r_hist;
	cv::calcHist(&bgr[0], 1, 0, cv::Mat(), gray_hist, 1, &histSize, &histRange, uniform, accumulate);

	//画成图，窗口显示
	int hist_w = 512, hist_h = 350;
	int bin_w = cvRound((double)hist_w / histSize);
	cv::Mat histImage(hist_h, hist_w, CV_8UC3, cv::Scalar(39, 34, 30));

	// 归一化到 [0, histImage.rows]
	cv::normalize(gray_hist, gray_hist, 0, histImage.rows, cv::NORM_MINMAX);

	for (int i = 1; i < histSize; ++i) {
		cv::line(histImage,
			cv::Point(bin_w * (i - 1), hist_h - cvRound(gray_hist.at<float>(i - 1))),
			cv::Point(bin_w * i, hist_h - cvRound(gray_hist.at<float>(i))),
			cv::Scalar(128, 128, 128), 2);
	}
	m_doc.setAnalyze(histImage);
	CVMat_to_Pic(histImage, IDC_STATIC_PIC_ANALYZE);
	pair<int,int> key = Count_Gray_MinMax(gray_hist,histSize);
	Info_append("灰度最小值为：");
	Info_append(std::to_string(key.first));
	Info_append("灰度最大值为：");
	Info_append(std::to_string(key.second));
	/* ---------- 1. 归一化为概率 ---------- */
	double total = static_cast<double>(gray.total());          // 总像素数
	cv::Mat prob;                         // 概率向量
	gray_hist.convertTo(prob, CV_64F);    // 转成 double
	prob /= total;                        // 每个 bin 除以总像素 → 概率

	/* ---------- 2. 计算统计特征 ---------- */
	double mean = 0.0, variance = 0.0, entropy = 0.0;
	const double* p = prob.ptr<double>(0);

	for (int i = 0; i < 256; ++i) {
		mean += i * p[i];                           // 一阶矩
	}
	for (int i = 0; i < 256; ++i) {
		double diff = i - mean;
		variance += diff * diff * p[i];             // 二阶中心矩
	}
	for (int i = 0; i < 256; ++i) {
		if (p[i] > 1e-12)                           // 避免 log(0)
			entropy -= p[i] * log2(p[i]);           // Shannon 熵
	}

	/* ---------- 3. 结果提示 ---------- */
	CString stats;
	stats.Format(_T("均值=%.2f\n 方差=%.2f\n 信息熵=%.4f bit"), mean, variance, entropy);
	Info_append(Cstring_to_cvString(stats));
	Info_append("分析完成");
}

void CMFCPicViewerDlg::OnMenuDHS() {
	if (!m_doc.hasImage())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	// 打开文件
	CFileDialog dlg(TRUE, _T("jpg"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("图片文件 (*.jpg;*.png;*.bmp;*.jpeg;*.tiff;*.tif)|*.jpg;*.png;*.bmp;*.jpeg;*.tiff;*.tif||"));
	if (dlg.DoModal() == IDOK)
	{
		m_doc.setReferencePath(dlg.GetPathName());
		m_doc.setReference(imread(Cstring_to_cvString(m_doc.referencePath())));
	}
	// 注意：重构前选择参考图时会清空撤销历史（疑似复制粘贴残留），
	// 本次重构保留撤销历史，视为有意的轻微行为修正。
	cv::Mat result = Transforms::Enhance(m_doc.current(), m_doc.reference(), Transforms::Modules::ENHANCE::DHS);
	if (result.empty())
		return;
	m_doc.apply(result);
	CVMat_to_Pic(m_doc.current(), IDC_STATIC_PIC);
	Info_append("离散直方图规定化完成");
}

void CMFCPicViewerDlg::OnMenuSaveAnalyzeTo() {
	if (!m_doc.hasImage())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	// 1. 选择文件夹
	TCHAR szPath[MAX_PATH]{};
	BROWSEINFO bi{};
	bi.hwndOwner = this->m_hWnd;
	bi.lpszTitle = _T("请选择保存目录");
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if (pidl == nullptr)
		return; // 用户取消

	if (!SHGetPathFromIDList(pidl, szPath))
	{
		CoTaskMemFree(pidl);
		return;
	}
	CoTaskMemFree(pidl);

	CString strFolder = szPath;
	if (strFolder.Right(1) != _T("\\"))
		strFolder += _T("\\");

	// 2. 提取原扩展名（含点号）
	CString strExt = PathFindExtension(m_doc.sourcePath()); // .jpg 等
	if (strExt.IsEmpty())
		strExt = _T(".png");

	m_doc.setSavePath(strFolder + _T("result") + strExt);

	// 3. OpenCV 保存
	bool ok = false;
	if (strExt.CompareNoCase(_T(".jpg")) == 0 || strExt.CompareNoCase(_T(".jpeg")) == 0)
		ok = cv::imwrite(Cstring_to_cvString(m_doc.savePath()), m_doc.analyze(),
			std::vector<int>{cv::IMWRITE_JPEG_QUALITY, 95});
	else
		ok = cv::imwrite(Cstring_to_cvString(m_doc.savePath()), m_doc.analyze());

	// 4. 提示
	if (ok)
		AfxMessageBox(_T("已保存到：\n") + m_doc.savePath(), MB_ICONINFORMATION);
	else
		AfxMessageBox(_T("保存失败！"), MB_ICONERROR);
}

void CMFCPicViewerDlg::Info_append(std::string str) {
	CString now;
	CString cstr = CA2T(str.c_str());
	Info_Bar_Cedit.GetWindowTextW(now);
	now = now + _T("\r\n") + CA2T(std::to_string(++m_infoLine).c_str()) + _T(" ") + cstr;
	Info_Bar_Cedit.SetWindowTextW(now);
	Info_Bar_Cedit.RedrawWindow();
	Info_Bar_Cedit.SetSel(1, 1, FALSE);
	Info_Bar_Cedit.SetFocus();
}

void CMFCPicViewerDlg::Info_clear() {
	Info_Bar_Cedit.SetWindowTextW(_T(""));
}

void CMFCPicViewerDlg::OnMenuLoadProperties() {
	INIParser parser;
	std::map<std::string, std::map<std::string, std::string>> res;
	if (parser.readFile("properties.ini")) {
		Info_append("Properties read successfully");
		g_params.scale_x = stof(parser.getValue("transform", "scale_x"));
		g_params.scale_y = stof(parser.getValue("transform", "scale_y"));
		g_params.rotate_theta = stof(parser.getValue("transform", "rotate_theta"));
		g_params.shift_x = stoi(parser.getValue("transform", "shift_x"));
		g_params.shift_y = stoi(parser.getValue("transform", "shift_y"));
		g_params.logK = stof(parser.getValue("transform", "logK"));
		g_params.expK = stof(parser.getValue("transform", "expK"));
		g_params.gamma = stof(parser.getValue("transform", "gamma"));
		g_params.output_grayscale.first = stoi(parser.getValue("transform", "grayscale_l"));
		g_params.output_grayscale.second = stoi(parser.getValue("transform", "grayscale_r"));
		g_params.filter_kernel = stoi(parser.getValue("filter", "filter_kernel"));
		g_params.bilateral_d = stoi(parser.getValue("filter", "bilateral_d"));
		g_params.bilateral_sigma_color = stod(parser.getValue("filter", "bilateral_sigma_color"));
		g_params.bilateral_sigma_space = stod(parser.getValue("filter", "bilateral_sigma_space"));
		g_params.morphology_kernel = stoi(parser.getValue("morphology", "morphology_kernel"));
		g_params.clahe_clip_limit = stod(parser.getValue("enhance", "clahe_clip_limit"));
		g_params.clahe_tile_size = stoi(parser.getValue("enhance", "clahe_tile_size"));
		g_params.noise_AWGN_sigma = stof(parser.getValue("noise", "noise_AWGN_sigma"));
		g_params.noise_Poisson_Gaussian_gain = stod(parser.getValue("noise", "noise_Poisson_Gaussian_gain"));
		g_params.noise_Poisson_Gaussian_sigma = stod(parser.getValue("noise", "noise_Poisson_Gaussian_sigma"));
		g_params.noise_ELD_SFRN_alpha = stod(parser.getValue("noise", "noise_ELD_SFRN_alpha"));
		g_params.noise_ELD_SFRN_sigmaT = stod(parser.getValue("noise", "noise_ELD_SFRN_sigmaT"));
		g_params.noise_ELD_SFRN_sigmaG = stod(parser.getValue("noise", "noise_ELD_SFRN_sigmaG"));
		g_params.noise_ELD_SFRN_q = stod(parser.getValue("noise", "noise_ELD_SFRN_q"));
		g_params.butterworth_D0 = stof(parser.getValue("filter", "butterworth_D0"));
		g_params.butterworth_level = stoi(parser.getValue("filter", "butterworth_level"));
		g_params.default_edge_threshold = stod(parser.getValue("edge", "default_edge_threshold"));
		g_params.default_edge_laplacian_level1_threshold = stod(parser.getValue("edge", "default_edge_laplacian_level1_threshold"));
		g_params.default_edge_laplacian_level2_threshold = stod(parser.getValue("edge", "default_edge_laplacian_level2_threshold"));
		g_params.default_edge_canny_low_threshold = stod(parser.getValue("edge", "default_edge_canny_low_threshold"));
		g_params.default_edge_canny_high_threshold = stod(parser.getValue("edge", "default_edge_canny_high_threshold"));
		g_params.default_edge_watershed_mode = stoi(parser.getValue("edge", "default_edge_watershed_mode"));
		g_params.auto_estimate_threshold = (bool)stoi(parser.getValue("edge", "if_auto_estimate_threshold"));
		g_params.edge_threshold = g_params.default_edge_threshold;
		g_params.edge_laplacian_level1_threshold = g_params.default_edge_laplacian_level1_threshold;
		g_params.edge_laplacian_level2_threshold = g_params.default_edge_laplacian_level2_threshold;
		g_params.edge_canny_low_threshold = g_params.default_edge_canny_low_threshold;
		g_params.edge_canny_high_threshold = g_params.default_edge_canny_high_threshold;
		g_params.edge_watershed_mode = g_params.default_edge_watershed_mode;
		g_params.hough_threshold = stod(parser.getValue("detect", "hough_threshold"));
		g_params.hough_min_line_length = stod(parser.getValue("detect", "hough_min_line_length"));
		g_params.hough_max_line_gap = stod(parser.getValue("detect", "hough_max_line_gap"));
		g_params.harris_threshold = stod(parser.getValue("detect", "harris_threshold"));
		g_params.harris_k = stod(parser.getValue("detect", "harris_k"));
		g_params.match_ratio = stod(parser.getValue("feature_match", "match_ratio"));
		g_params.match_max_features = stoi(parser.getValue("feature_match", "match_max_features"));
		g_params.match_ransac_threshold = stod(parser.getValue("feature_match", "match_ransac_threshold"));
		g_params.match_max_draw_matches = stoi(parser.getValue("feature_match", "match_max_draw_matches"));
		res = parser.getAll();
		Info_append("sections: " + std::to_string(res.size()));
		for (std::map<std::string, std::map<std::string, std::string>>::iterator it = res.begin(); it != res.end(); ++it) {
			std::string sec = it->first;
			Info_append("[" + sec + "]");
			std::map<std::string, std::string> comp = it->second;
			for (std::map<std::string, std::string>::iterator jt = comp.begin(); jt != comp.end(); ++jt) {
				Info_append(jt->first + " " + jt->second);
			}
		}
	}
}

// ===================== 命令统一分发 =====================

void CMFCPicViewerDlg::OnMenuOperationRange(UINT nID)
{
	// 非纯图像操作（涉及文件对话框/状态栈/参数载入）走各自实现
	switch (nID)
	{
	case MSG_MENU_UNDO:            OnMenuUndo(); return;
	case MSG_MENU_REDO:            OnMenuRedo(); return;
	case MSG_MENU_RELOAD:          OnMenuReload(); return;
	case MSG_MENU_OPTION:          OnMenuOption(); return;
	case MSG_MENU_HISTOGRAM:       OnMenuHistogram(); return;
	case MSG_MENU_HISTOGRAM_GRAY:  OnMenuHistogramGray(); return;
	case MSG_MENU_DHS:             OnMenuDHS(); return;
	case MSG_MENU_SAVE_ANALYZE_TO: OnMenuSaveAnalyzeTo(); return;
	case MSG_MENU_LOAD_PROPERTIES: OnMenuLoadProperties(); return;
	// 特征匹配结果输出到分析区而非替换当前图像，因此单独分发
	case MSG_MENU_MATCH_OPEN_SECOND: OnMenuMatchOpenSecond(); return;
	case MSG_MENU_MATCH_ORB:       OnMenuMatchORB(); return;
	case MSG_MENU_MATCH_SIFT:      OnMenuMatchSIFT(); return;
	case MSG_MENU_MATCH_AKAZE:     OnMenuMatchAKAZE(); return;
	default: break;
	}

	RunOperation(nID);
}

void CMFCPicViewerDlg::RunOperation(UINT nID)
{
	const ImageOpDef* op = FindOp(nID);
	if (!op)
		return;

	if (!m_doc.hasImage())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}

	if (op->needsFreqDomain && m_doc.current().type() != CV_32FC2)
	{
		AfxMessageBox(_T("还没有转换傅里叶滤波图像！"), MB_ICONWARNING);
		return;
	}

	cv::Mat result = op->run(m_doc.current());
	if (result.empty())
		return;   // 算法未产生有效结果，不提交

	if (op->toBgr && result.channels() == 1)
		cvtColor(result, result, COLOR_GRAY2BGR);

	m_doc.apply(result);
	CVMat_to_Pic(m_doc.current(), IDC_STATIC_PIC);

	if (op->log)
		Info_append(op->log());
}

const ImageOpDef* CMFCPicViewerDlg::FindOp(UINT nID)
{
	static const std::map<UINT, ImageOpDef> s_ops = []()
	{
		std::map<UINT, ImageOpDef> ops;

		// ---- 基本变换 ----
		ops[MSG_MENU_GRAYIZE] = {
			MSG_MENU_GRAYIZE, _T("转换为灰度值"), false, true,
			[](const cv::Mat& in) { return Transforms::Transform(in, Transforms::Modules::BASIC::GRAYIZE); },
			[]() { return std::string("灰度化完成"); } };
		ops[MSG_MENU_BINARYIZE] = {
			MSG_MENU_BINARYIZE, _T("转换为二值图"), false, true,
			[](const cv::Mat& in) { return Transforms::Transform(in, Transforms::Modules::BASIC::BINARYIZE); },
			[]() { return std::string("二值化完成"); } };
		ops[MSG_MENU_SHIFT] = {
			MSG_MENU_SHIFT, _T("平移操作"), false, false,
			[](const cv::Mat& in) { return Transforms::Transform(in, g_params.shift_x, g_params.shift_y, Transforms::Modules::BASIC::SHIFT); },
			[]() { return std::string("平移完成"); } };
		ops[MSG_MENU_SCALE] = {
			MSG_MENU_SCALE, _T("放缩操作"), false, false,
			[](const cv::Mat& in) { return Transforms::Transform(in, g_params.scale_x, g_params.scale_y, Transforms::Modules::BASIC::SCALE); },
			[]() { return std::string("放缩完成"); } };
		ops[MSG_MENU_ROTATE] = {
			MSG_MENU_ROTATE, _T("旋转操作"), false, false,
			[](const cv::Mat& in) { return Transforms::Transform(in, g_params.rotate_theta, Transforms::Modules::BASIC::ROTATE); },
			[]() { return std::string("旋转完成"); } };

		// ---- 灰度变换 ----
		ops[MSG_MENU_NEGATIVE] = {
			MSG_MENU_NEGATIVE, _T("灰度反转操作"), false, true,
			[](const cv::Mat& in) { return Transforms::Transform(in, Transforms::Modules::LINEAR::NEGATIVE); },
			[]() { return std::string("灰度反转完成"); } };
		ops[MSG_MENU_NEGATIVE_COLORFUL] = {
			MSG_MENU_NEGATIVE_COLORFUL, _T("色彩反转操作"), false, false,
			[](const cv::Mat& in) { return Transforms::Transform(in, Transforms::Modules::LINEAR::NEGATIVE_COLORFUL); },
			[]() { return std::string("灰度反转完成"); } };
		ops[MSG_MENU_LINEAR_GRAYSCALE] = {
			MSG_MENU_LINEAR_GRAYSCALE, _T("线性灰度变换"), false, true,
			[](const cv::Mat& in) { return Transforms::Transform(in, g_params.output_grayscale, Transforms::Modules::LINEAR::SCALE_LINEAR); },
			[]() { return std::string("灰度线性变换完成"); } };
		ops[MSG_MENU_LINEAR_GRAYSCALE_MULTIRANGES] = {
			MSG_MENU_LINEAR_GRAYSCALE_MULTIRANGES, _T("分段线性灰度变换"), false, true,
			[](const cv::Mat& in) { return Transforms::Transform(in, g_params.oper_ranges, g_params.gray_scales, Transforms::Modules::LINEAR::SCALE_LINEAR_MULTIRANGES); },
			[]() { return std::string("灰度线性变换完成"); } };
		ops[MSG_MENU_NONLINEAR_LOG] = {
			MSG_MENU_NONLINEAR_LOG, _T("对数灰度变换"), false, true,
			[](const cv::Mat& in) { return Transforms::Transform(in, g_params.logK, Transforms::Modules::NONLINEAR::LOG); },
			[]() { return std::string("灰度线性变换完成"); } };
		ops[MSG_MENU_NONLINEAR_EXP] = {
			MSG_MENU_NONLINEAR_EXP, _T("指数灰度变换"), false, true,
			[](const cv::Mat& in) { return Transforms::Transform(in, g_params.expK, Transforms::Modules::NONLINEAR::EXP); },
			[]() { return std::string("灰度线性变换完成"); } };
		ops[MSG_MENU_GAMMA] = {
			MSG_MENU_GAMMA, _T("Gamma校正"), false, true,
			[](const cv::Mat& in) { return Transforms::Transform(in, g_params.gamma, Transforms::Modules::NONLINEAR::GAMMA); },
			[]() { return std::string("Gamma校正完成"); } };

		// ---- 直方图增强 ----
		ops[MSG_MENU_DHE] = {
			MSG_MENU_DHE, _T("离散直方图均衡化"), false, true,
			[](const cv::Mat& in) { return Transforms::Enhance(in, Transforms::Modules::ENHANCE::DHE); },
			[]() { return std::string("离散直方图均衡化完成"); } };
		ops[MSG_MENU_CLAHE] = {
			MSG_MENU_CLAHE, _T("CLAHE局部直方图均衡化"), false, true,
			[](const cv::Mat& in) { return Transforms::Enhance(in, g_params.clahe_clip_limit, static_cast<int>(g_params.clahe_tile_size), Transforms::Modules::ENHANCE::CLAHE); },
			[]() { return std::string("CLAHE局部直方图均衡化完成"); } };

		// ---- 空间域滤波 ----
		ops[MSG_MENU_FILTER_MAX] = {
			MSG_MENU_FILTER_MAX, _T("邻域最大值滤波"), false, true,
			[](const cv::Mat& in) { return Filters::filterate(in, g_params.filter_kernel, Filters::Modules::SPATIAL::MAX); },
			[]() { return std::string("滤波完成"); } };
		ops[MSG_MENU_FILTER_MIN] = {
			MSG_MENU_FILTER_MIN, _T("邻域最小值滤波"), false, true,
			[](const cv::Mat& in) { return Filters::filterate(in, g_params.filter_kernel, Filters::Modules::SPATIAL::MIN); },
			[]() { return std::string("滤波完成"); } };
		ops[MSG_MENU_FILTER_MEDIAN] = {
			MSG_MENU_FILTER_MEDIAN, _T("邻域中值滤波"), false, true,
			[](const cv::Mat& in) { return Filters::filterate(in, g_params.filter_kernel, Filters::Modules::SPATIAL::MEDIAN); },
			[]() { return std::string("滤波完成"); } };
		ops[MSG_MENU_FILTER_MEAN] = {
			MSG_MENU_FILTER_MEAN, _T("邻域均值滤波"), false, true,
			[](const cv::Mat& in) { return Filters::filterate(in, g_params.filter_kernel, Filters::Modules::SPATIAL::MEAN); },
			[]() { return std::string("滤波完成"); } };

		// ---- 噪声 ----
		// 注意：三种噪声结果保持单通道（与重构前一致，不强制转回 BGR）
		ops[MSG_MENU_NOISE_AWGN] = {
			MSG_MENU_NOISE_AWGN, _T("加性高斯白噪声"), false, false,
			[](const cv::Mat& in) { return Noises::noise(in, g_params.noise_AWGN_sigma, Noises::Modules::CLASSIC::AWGN); },
			[]() { return std::string("噪声添加完成"); } };
		ops[MSG_MENU_NOISE_POISSON_GAUSSIAN] = {
			MSG_MENU_NOISE_POISSON_GAUSSIAN, _T("泊松-高斯很合噪声"), false, false,
			[](const cv::Mat& in) { return Noises::noise(in, g_params.noise_Poisson_Gaussian_gain, g_params.noise_Poisson_Gaussian_sigma, Noises::Modules::CLASSIC::POISSON_GAUSSIAN); },
			[]() { return std::string("噪声添加完成"); } };
		ops[MSG_MENU_NOISE_ELD_SFRN] = {
			MSG_MENU_NOISE_ELD_SFRN, _T("信号相关联合噪声"), false, false,
			[](const cv::Mat& in) { return Noises::noise(in, g_params.noise_ELD_SFRN_alpha, g_params.noise_ELD_SFRN_sigmaT, g_params.noise_ELD_SFRN_sigmaG, g_params.noise_ELD_SFRN_q, Noises::Modules::CLASSIC::ELD_SFRN); },
			[]() { return std::string("噪声添加完成"); } };

		// ---- 傅里叶变换 ----
		ops[MSG_MENU_DFT] = {
			MSG_MENU_DFT, _T("离散傅里叶变换"), false, false,
			[](const cv::Mat& in) { return Transforms::Fourier::FFT(in); },
			[]() { return std::string("傅里叶变换完成"); } };
		ops[MSG_MENU_IDFT] = {
			MSG_MENU_IDFT, _T("逆离散傅里叶变换"), false, false,
			[](const cv::Mat& in) { return Transforms::Fourier::IFFT(in); },
			[]() { return std::string("逆傅里叶变换完成"); } };

		// ---- 频率域滤波（要求当前图为 CV_32FC2 频域数据） ----
		ops[MSG_MENU_FILTER_BUTTERWORTH_LOW] = {
			MSG_MENU_FILTER_BUTTERWORTH_LOW, _T("巴特沃斯低通滤波"), true, false,
			[](const cv::Mat& in) { return Filters::filterate(in, g_params.butterworth_level, g_params.butterworth_D0, Filters::Modules::BUTTERWORTH::LOW); },
			[]() { return std::string("低通滤波完成"); } };
		ops[MSG_MENU_FILTER_BUTTERWORTH_HIGH] = {
			MSG_MENU_FILTER_BUTTERWORTH_HIGH, _T("巴特沃斯高通滤波"), true, false,
			[](const cv::Mat& in) { return Filters::filterate(in, g_params.butterworth_level, g_params.butterworth_D0, Filters::Modules::BUTTERWORTH::HIGH); },
			[]() { return std::string("高通滤波完成"); } };
		ops[MSG_MENU_FILTER_BILATERAL] = {
			MSG_MENU_FILTER_BILATERAL, _T("双边滤波"), false, true,
			[](const cv::Mat& in) { return Filters::filterate(in, g_params.bilateral_d, g_params.bilateral_sigma_color, g_params.bilateral_sigma_space, Filters::Modules::SPATIAL::BILATERAL); },
			[]() { return std::string("滤波完成"); } };

		// ---- 形态学 ----
		ops[MSG_MENU_MORPH_ERODE] = {
			MSG_MENU_MORPH_ERODE, _T("形态学-腐蚀"), false, true,
			[](const cv::Mat& in) { return Morphology::morphology(in, g_params.morphology_kernel, Morphology::Modules::ERODE); },
			[]() { return std::string("腐蚀完成"); } };
		ops[MSG_MENU_MORPH_DILATE] = {
			MSG_MENU_MORPH_DILATE, _T("形态学-膨胀"), false, true,
			[](const cv::Mat& in) { return Morphology::morphology(in, g_params.morphology_kernel, Morphology::Modules::DILATE); },
			[]() { return std::string("膨胀完成"); } };
		ops[MSG_MENU_MORPH_OPEN] = {
			MSG_MENU_MORPH_OPEN, _T("形态学-开运算"), false, true,
			[](const cv::Mat& in) { return Morphology::morphology(in, g_params.morphology_kernel, Morphology::Modules::OPEN); },
			[]() { return std::string("开运算完成"); } };
		ops[MSG_MENU_MORPH_CLOSE] = {
			MSG_MENU_MORPH_CLOSE, _T("形态学-闭运算"), false, true,
			[](const cv::Mat& in) { return Morphology::morphology(in, g_params.morphology_kernel, Morphology::Modules::CLOSE); },
			[]() { return std::string("闭运算完成"); } };

		// ---- 分割 ----
		ops[MSG_MENU_BINARYIZE_OTSU] = {
			MSG_MENU_BINARYIZE_OTSU, _T("转换为二值图(Otsu)"), false, true,
			[](const cv::Mat& in) { return Transforms::Transform(in, Transforms::Modules::DEVIDE::OTSU); },
			[]() { return std::string("Otsu二值化完成"); } };
		// 注意：区域增长算法当前在 Transforms::Transform 中未实现有效分支
		// （调用会落入未定义行为），本次重构保留原调用方式，仅记录。
		ops[MSG_MENU_BINARYIZE_REGIONGROW] = {
			MSG_MENU_BINARYIZE_REGIONGROW, _T("转换为二值图(区域增长)"), false, true,
			[](const cv::Mat& in) { return Transforms::Transform(in, Transforms::Modules::DEVIDE::REGIONGROW); },
			[]() { return std::string("Otsu二值化完成"); } };

		// ---- 边缘检测 ----
		ops[MSG_MENU_EDGE_ROBERT] = {
			MSG_MENU_EDGE_ROBERT, _T("边缘提取（Robert算子）"), false, true,
			[](const cv::Mat& in) {
				if (g_params.auto_estimate_threshold) g_params.edge_threshold = Transforms::Threshold_Estimate::adaptiveThresholdByEdgeDensity(in);
				return Transforms::Enhance(in, g_params.edge_threshold, Transforms::Modules::EDGE::ROBERT);
			},
			[]() { return "边缘提取完成: 阈值为" + std::to_string(g_params.edge_threshold); } };
		ops[MSG_MENU_EDGE_PREWITT] = {
			MSG_MENU_EDGE_PREWITT, _T("边缘提取（Prewitt算子）"), false, true,
			[](const cv::Mat& in) {
				if (g_params.auto_estimate_threshold) g_params.edge_threshold = Transforms::Threshold_Estimate::adaptiveThresholdByEdgeDensity(in);
				return Transforms::Enhance(in, g_params.edge_threshold, Transforms::Modules::EDGE::PREWITT);
			},
			[]() { return "边缘提取完成: 阈值为" + std::to_string(g_params.edge_threshold); } };
		ops[MSG_MENU_EDGE_SOBEL] = {
			MSG_MENU_EDGE_SOBEL, _T("边缘提取（Sobel算子）"), false, true,
			[](const cv::Mat& in) {
				if (g_params.auto_estimate_threshold) g_params.edge_threshold = Transforms::Threshold_Estimate::adaptiveThresholdByEdgeDensity(in);
				return Transforms::Enhance(in, g_params.edge_threshold, Transforms::Modules::EDGE::SOBEL);
			},
			[]() { return "边缘提取完成: 阈值为" + std::to_string(g_params.edge_threshold); } };
		ops[MSG_MENU_EDGE_FRIECHEN] = {
			MSG_MENU_EDGE_FRIECHEN, _T("边缘提取（Frie-Chen算子）"), false, true,
			[](const cv::Mat& in) {
				if (g_params.auto_estimate_threshold) g_params.edge_threshold = Transforms::Threshold_Estimate::adaptiveThresholdByEdgeDensity(in);
				return Transforms::Enhance(in, g_params.edge_threshold, Transforms::Modules::EDGE::FRIE_CHEN);
			},
			[]() { return "边缘提取完成: 阈值为" + std::to_string(g_params.edge_threshold); } };
		ops[MSG_MENU_EDGE_LAPLACIAN] = {
			MSG_MENU_EDGE_LAPLACIAN, _T("边缘提取（Laplacian算子）"), false, true,
			[](const cv::Mat& in) {
				if (g_params.auto_estimate_threshold) g_params.edge_laplacian_level1_threshold = Transforms::Threshold_Estimate::adaptiveThresholdByEdgeDensity(in);
				return Transforms::Enhance(in, g_params.edge_laplacian_level1_threshold, g_params.edge_laplacian_level2_threshold, Transforms::Modules::EDGE::LAPLACIAN);
			},
			[]() { return "边缘提取完成: 阈值为 一阶：" + std::to_string(g_params.edge_laplacian_level1_threshold) + "，二阶：" + std::to_string(g_params.edge_laplacian_level2_threshold); } };
		// 注意：自动阈值更新的是 laplacian_level1，但实际传给算法的一阶参数是 g_params.edge_threshold，
		// 与重构前行为完全一致（历史遗留，仅记录）。
		ops[MSG_MENU_EDGE_LOG] = {
			MSG_MENU_EDGE_LOG, _T("边缘提取（LOG算子）"), false, true,
			[](const cv::Mat& in) {
				if (g_params.auto_estimate_threshold) g_params.edge_laplacian_level1_threshold = Transforms::Threshold_Estimate::adaptiveThresholdByEdgeDensity(in);
				return Transforms::Enhance(in, g_params.edge_threshold, g_params.edge_laplacian_level2_threshold, Transforms::Modules::EDGE::LOG);
			},
			[]() { return "边缘提取完成: 阈值为" + std::to_string(g_params.edge_threshold); } };
		ops[MSG_MENU_EDGE_CANNY] = {
			MSG_MENU_EDGE_CANNY, _T("边缘提取（Canny算子）"), false, true,
			[](const cv::Mat& in) {
				if (g_params.auto_estimate_threshold) {
					Transforms::Threshold_Estimate::CannyThresholds th = Transforms::Threshold_Estimate::estimateCannyThresholds(in);
					g_params.edge_canny_low_threshold = th.low;
					g_params.edge_canny_high_threshold = th.high;
				}
				return Transforms::Enhance(in, g_params.edge_canny_low_threshold, g_params.edge_canny_high_threshold, Transforms::Modules::EDGE::CANNY);
			},
			[]() { return "边缘提取完成: 阈值为 左值：" + std::to_string(g_params.edge_canny_low_threshold) + "，右值：" + std::to_string(g_params.edge_canny_high_threshold); } };
		ops[MSG_MENU_EDGE_WATERSHED] = {
			MSG_MENU_EDGE_WATERSHED, _T("边缘提取（分水岭）"), false, false,
			[](const cv::Mat& in) { return Transforms::Watershed(in, 3, Transforms::Modules::EDGE::WATERSHED_CANNY - 1 + g_params.edge_watershed_mode); },
			[]() { return std::string("边缘提取完成"); } };

		// ---- 检测 ----
		ops[MSG_MENU_HOUGH] = {
			MSG_MENU_HOUGH, _T("Hough直线检测"), false, false,
			[](const cv::Mat& in) { return Transforms::Detect::houghLines(in, g_params.hough_threshold, g_params.hough_min_line_length, g_params.hough_max_line_gap); },
			[]() { return std::string("Hough直线检测完成"); } };
		ops[MSG_MENU_HARRIS] = {
			MSG_MENU_HARRIS, _T("Harris角点检测"), false, false,
			[](const cv::Mat& in) { return Transforms::Detect::harrisCorners(in, g_params.harris_threshold, g_params.harris_k); },
			[]() { return std::string("Harris角点检测完成"); } };

		return ops;
	}();

	auto it = s_ops.find(nID);
	return it == s_ops.end() ? nullptr : &it->second;
}

// ===================== 特征匹配 =====================

void CMFCPicViewerDlg::OnMenuMatchOpenSecond()
{
	if (!m_doc.hasImage())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	// 打开第二张图像：同一物体另一个角度拍摄的照片
	CFileDialog dlg(TRUE, _T("jpg"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("图片文件 (*.jpg;*.png;*.bmp;*.jpeg;*.tiff;*.tif)|*.jpg;*.png;*.bmp;*.jpeg;*.tiff;*.tif||"));
	if (dlg.DoModal() != IDOK)
		return;

	m_doc.setReferencePath(dlg.GetPathName());
	m_doc.setReference(imread(Cstring_to_cvString(m_doc.referencePath())));
	if (m_doc.reference().empty())
	{
		AfxMessageBox(_T("第二张图像加载失败！"), MB_ICONWARNING);
		return;
	}
	// 在分析区显示第二张图像，便于确认
	CVMat_to_Pic(m_doc.reference(), IDC_STATIC_PIC_ANALYZE);
	Info_append("第二张图像已载入：");
	Info_append(Cstring_to_cvString(m_doc.referencePath()));
	Info_append("图像尺寸：" + std::to_string(m_doc.reference().cols) + " x " + std::to_string(m_doc.reference().rows));
}

void CMFCPicViewerDlg::RunFeatureMatch(UINT MODULE, CString method_name)
{
	if (!m_doc.hasImage())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	if (m_doc.referencePath().IsEmpty() || m_doc.reference().empty())
	{
		AfxMessageBox(_T("请先通过“匹配 - 打开第二张图像”载入同一物体的另一张照片！"), MB_ICONWARNING);
		return;
	}
	if (m_doc.current().depth() != CV_8U)
	{
		AfxMessageBox(_T("当前图像不是 8 位图像（可能是傅里叶变换结果），无法进行特征匹配！"), MB_ICONWARNING);
		return;
	}

	FeatureMatch::MatchResult res = FeatureMatch::match(m_doc.current(), m_doc.reference(), MODULE,
		g_params.match_ratio, g_params.match_max_features, g_params.match_ransac_threshold, g_params.match_max_draw_matches);
	if (!res.success)
	{
		AfxMessageBox(_T("特征匹配失败：图像中未能提取到足够的特征点！"), MB_ICONWARNING);
		return;
	}

	// 匹配结果（两图并排 + 连线）显示在主显示区，同时存入分析结果，可用“文件 - 保存分析结果到”导出
	m_doc.setAnalyze(res.visualization);
	CVMat_to_Pic(res.visualization, IDC_STATIC_PIC);

	Info_append(Cstring_to_cvString(method_name) + "完成");
	Info_append("特征点数量：图像 1 为 " + std::to_string(res.keypoints1) + "，图像 2 为 " + std::to_string(res.keypoints2));
	Info_append("比值检验后的匹配对数：" + std::to_string(res.good_matches));
	Info_append("RANSAC 单应性内点数：" + std::to_string(res.inliers));
	if (res.scale < 1.0)
		Info_append("图像已按 " + std::to_string(res.scale) + " 等比缩小后匹配");
	Info_append("匹配结果已显示在主显示区，可用“文件 - 保存图像到 / 另存为”导出");
}

void CMFCPicViewerDlg::OnMenuMatchORB()
{
	RunFeatureMatch(FeatureMatch::Modules::ORB_MATCH, _T("ORB 特征匹配"));
}

void CMFCPicViewerDlg::OnMenuMatchSIFT()
{
	RunFeatureMatch(FeatureMatch::Modules::SIFT_MATCH, _T("SIFT 特征匹配"));
}

void CMFCPicViewerDlg::OnMenuMatchAKAZE()
{
	RunFeatureMatch(FeatureMatch::Modules::AKAZE_MATCH, _T("AKAZE 特征匹配"));
}
