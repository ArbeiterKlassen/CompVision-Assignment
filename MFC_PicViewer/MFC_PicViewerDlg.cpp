
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
#include "MyEditControl.h"
//STL头
#include <opencv2/opencv.hpp>
#include <stack>
#include <cmath>
#include <algorithm>
#include <complex>
#include <vector>

//窗口头
#include "MFC_PicViewer_OptionDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW

#endif
using namespace cv;

// 程序变量定义区
CString file_path;
CString save_file_path;
CString ref_file_path;
Mat origin_img(10, 10, CV_8UC3, cv::Scalar(39, 34, 30));
Mat NULL_img(10, 10, CV_8UC3, cv::Scalar(39, 34, 30));
Mat current_img(10, 10, CV_8UC3, cv::Scalar(39, 34, 30));
Mat next_img(10, 10, CV_8UC3, cv::Scalar(39, 34, 30));
Mat Analyze_img(10, 10, CV_8UC3, cv::Scalar(39, 34, 30));
Mat referrence_img(10, 10, CV_8UC3, cv::Scalar(39, 34, 30));
std::stack<Mat> img_stack;
UINT Infobar_line = 0;

//自定义参数区
float scale_x = 1.1;
float scale_y = 1.3;
float rotate_theta = 90;
UINT shift_x = 50;
UINT shift_y = 50;
float logK = 2;
float expK = 0.5;
std::pair<int, int> output_grayscale = make_pair(100, 155);
std::vector<std::pair<int, int>> oper_ranges{ make_pair(0,49),make_pair(50,199),make_pair(200,255)};
std::vector<std::pair<int, int>> gray_scales{ make_pair(0,24),make_pair(50,224),make_pair(227,255) };

UINT filter_kernel = 7;
float noise_AWGN_sigma = 25.0;
double noise_Poisson_Gaussian_gain = 4.0;
double noise_Poisson_Gaussian_sigma = 5.0;
double noise_ELD_SFRN_alpha = 4.0;
double noise_ELD_SFRN_sigmaT = 3.0;
double noise_ELD_SFRN_sigmaG = 2.0;
double noise_ELD_SFRN_q = 1.0;
double butterworth_D0 = 500.0;
UINT butterworth_level = 1;
double edge_threshold = 100;
double edge_laplacian_level1_threshold = 100;
double edge_laplacian_level2_threshold = 10;
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
	ON_COMMAND(MSG_MENU_GRAYIZE, &CMFCPicViewerDlg::OnMenuGrayize)
	ON_COMMAND(MSG_MENU_BINARYIZE, &CMFCPicViewerDlg::OnMenuBinaryize)
	ON_COMMAND(MSG_MENU_UNDO, &CMFCPicViewerDlg::OnMenuUndo)
	ON_COMMAND(MSG_MENU_RELOAD, &CMFCPicViewerDlg::OnMenuReload)
	ON_COMMAND(MSG_MENU_OPTION, &CMFCPicViewerDlg::OnMenuOption)
	ON_COMMAND(MSG_MENU_SHIFT, &CMFCPicViewerDlg::OnMenuShift)
	ON_COMMAND(MSG_MENU_SCALE, &CMFCPicViewerDlg::OnMenuScale)
	ON_COMMAND(MSG_MENU_ROTATE, &CMFCPicViewerDlg::OnMenuRotate)
	ON_COMMAND(MSG_MENU_HISTOGRAM, &CMFCPicViewerDlg::OnMenuHistogram)
	ON_COMMAND(MSG_MENU_HISTOGRAM_GRAY, &CMFCPicViewerDlg::OnMenuHistogramGray)
	//ON_COMMAND(MSG_MENU_TRANSFORM, &CMFCPicViewerDlg::OnMenuTransform)
	ON_COMMAND(MSG_MENU_NEGATIVE, &CMFCPicViewerDlg::OnMenuNegative)
	ON_COMMAND(MSG_MENU_NEGATIVE_COLORFUL, &CMFCPicViewerDlg::OnMenuNegativeColorful)
	ON_COMMAND(MSG_MENU_LINEAR_GRAYSCALE, &CMFCPicViewerDlg::OnMenuLinearGrayScale)
	ON_COMMAND(MSG_MENU_LINEAR_GRAYSCALE_MULTIRANGES, &CMFCPicViewerDlg::OnMenuLinearGrayScaleMultiranges)
	ON_COMMAND(MSG_MENU_NONLINEAR_LOG, &CMFCPicViewerDlg::OnMenuNonlinearLog)
	ON_COMMAND(MSG_MENU_NONLINEAR_EXP, &CMFCPicViewerDlg::OnMenuNonlinearExp)
	ON_COMMAND(MSG_MENU_DHE, &CMFCPicViewerDlg::OnMenuDHE)
	ON_COMMAND(MSG_MENU_DHS, &CMFCPicViewerDlg::OnMenuDHS)
	ON_COMMAND(MSG_MENU_FILTER_MAX, &CMFCPicViewerDlg::OnMenuSpatialFilterMax)
	ON_COMMAND(MSG_MENU_FILTER_MIN, &CMFCPicViewerDlg::OnMenuSpatialFilterMin)
	ON_COMMAND(MSG_MENU_FILTER_MEDIAN, &CMFCPicViewerDlg::OnMenuSpatialFilterMedian)
	ON_COMMAND(MSG_MENU_FILTER_MEAN, &CMFCPicViewerDlg::OnMenuSpatialFilterMean)
	ON_COMMAND(MSG_MENU_NOISE_AWGN, &CMFCPicViewerDlg::OnMenuNoiseAWGN)
	ON_COMMAND(MSG_MENU_NOISE_POISSON_GAUSSIAN, &CMFCPicViewerDlg::OnMenuNoisePoissonGaussian)
	ON_COMMAND(MSG_MENU_NOISE_ELD_SFRN, &CMFCPicViewerDlg::OnMenuNoiseELDSFRN)
	ON_COMMAND(MSG_MENU_SAVE_ANALYZE_TO, &CMFCPicViewerDlg::OnMenuSaveAnalyzeTo)
	ON_COMMAND(MSG_MENU_DFT, &CMFCPicViewerDlg::OnMenuDFT)
	ON_COMMAND(MSG_MENU_IDFT, &CMFCPicViewerDlg::OnMenuIDFT)
	ON_COMMAND(MSG_MENU_FILTER_BUTTERWORTH_LOW, &CMFCPicViewerDlg::OnMenuButterworthLowFilter)
	ON_COMMAND(MSG_MENU_FILTER_BUTTERWORTH_HIGH, &CMFCPicViewerDlg::OnMenuButterworthHighFilter)
	ON_COMMAND(MSG_MENU_BINARYIZE_OTSU, &CMFCPicViewerDlg::OnMenuBinaryize_Otsu)
	ON_COMMAND(MSG_MENU_BINARYIZE_REGIONGROW, &CMFCPicViewerDlg::OnMenuBinaryize_RegionGrow)
	ON_COMMAND(MSG_MENU_EDGE_ROBERT, &CMFCPicViewerDlg::OnMenuEdgeRobert)
	ON_COMMAND(MSG_MENU_EDGE_PREWITT, &CMFCPicViewerDlg::OnMenuEdgePrewitt)
	ON_COMMAND(MSG_MENU_EDGE_SOBEL, &CMFCPicViewerDlg::OnMenuEdgeSobel)
	ON_COMMAND(MSG_MENU_EDGE_FRIECHEN, &CMFCPicViewerDlg::OnMenuEdgeFrieChen)
	ON_COMMAND(MSG_MENU_EDGE_LAPLACIAN, &CMFCPicViewerDlg::OnMenuEdgeLaplacian)
	ON_COMMAND(MSG_MENU_EDGE_LOG, &CMFCPicViewerDlg::OnMenuEdgeLOG)
	ON_COMMAND(MSG_MENU_EDGE_CANNY, &CMFCPicViewerDlg::OnMenuEdgeCanny)
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
		rect.bottom *= 0.1;
		/*int nCount = 165 - 115 + 186 - 158 + 190 - 115;
		int nIncrecs = (rect.right - rect.left) / nCount;*/
		dc.FillSolidRect(rect, RGB(45, 51, 60));
		dc.SetBkMode(TRANSPARENT);
		CRect rtBtnClo;
		//绘制文件菜单栏
		GetClientRect(&rtBtnClo);
		rtBtnClo.left = rtBtnClo.left + 10;
		rtBtnClo.right = rtBtnClo.left + 30;
		rtBtnClo.bottom *= 0.1;
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
		rtBtnClo.bottom *= 0.1;
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
		rtBtnClo.bottom *= 0.1;
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
		rtBtnClo.bottom *= 0.1;
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
		rtBtnClo.bottom *= 0.1;
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
		rtBtnClo.bottom *= 0.1;
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
		rtBtnClo.bottom *= 0.1;
		rtBtnClo.top = rtBtnClo.bottom - 20;
		m_rBtnOption = rtBtnClo;
		dc.Rectangle(rtBtnClo);
		dc.FillSolidRect(m_rBtnOption, RGB(45, 51, 59));
		dc.SetBkMode(TRANSPARENT);
		str1 = TEXT("配置");
		dc.SetTextColor(RGB(185, 192, 197));
		dc.DrawText(str1, m_rBtnOption, DT_LEFT | DT_VCENTER);

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

	UINT nHitTest = CDialogEx::OnNcHitTest(point);
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
	Invalidate();
	// TODO: 在此处添加消息处理程序代码
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

void CMFCPicViewerDlg::CVMat_to_Pic(cv::Mat& img, UINT nID)
{
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
		// 注意：如果stride > ctrlW*3，则每行后面会有多余的空间，但我们已经初始化为0，不过这里我们只拷贝实际数据，多余部分保留为0（黑色）
		// 但我们的canvas已经初始化了背景色，所以多余部分应该是背景色？不对，我们只拷贝了ctrlW*3，所以多余部分实际上是未初始化的。
		// 所以我们需要确保buffer的初始值是我们想要的背景色？但这里我们直接逐行拷贝，所以每行只拷贝了ctrlW*3，后面的部分我们并没有设置。
		// 因此，我们需要在创建buffer时初始化为背景色，或者单独设置背景色。
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
	// TODO: Add your message handler code here and/or call default
	if (m_rtBtnfile.PtInRect(point))
	{
		CMenuEx m_MenuEx;
		m_MenuEx.CreatePopupMenu();

		m_MenuEx.SetItemHeight(30);
		m_MenuEx.SetFontInfo(14);
		m_MenuEx.SetBorderColor(RGB(45, 51, 60));
		m_MenuEx.SetMenuWidth(130);
		m_MenuEx.InstallHook(theApp.m_hInstance);

		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_OPEN, _T("打开"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_SAVETO, _T("保存图像到..."), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_SAVE_ANALYZE_TO, _T("保存分析结果到..."), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_SAVEASTO, _T("另存为..."), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_UNDO, _T("撤销"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_RELOAD, _T("恢复"), _T(""), 0);

		CRect rectDlg;
		GetWindowRect(rectDlg);//获得窗体的大小
		m_MenuEx.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rectDlg.left + 10, rectDlg.top + static_cast<int>((rectDlg.bottom - rectDlg.top) * 0.104), this);
		m_MenuEx.DestroyMenu();

		m_MenuEx.UnInstallHook();

	}

	if (m_rtBtnEdit.PtInRect(point))
	{
		CRect rectDlg;
		GetWindowRect(rectDlg);//获得窗体的大小
		CMenuEx m_MenuEx;
		m_MenuEx.CreatePopupMenu();

		m_MenuEx.SetItemHeight(30);
		m_MenuEx.SetFontInfo(14);
		m_MenuEx.SetBorderColor(RGB(45, 51, 60));
		m_MenuEx.SetMenuWidth(130);
		m_MenuEx.InstallHook(theApp.m_hInstance);

		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_GRAYIZE, _T("转换为灰度值"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_BINARYIZE, _T("转换为二值图"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_BINARYIZE_OTSU, _T("转换为二值图(Otsu)"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_BINARYIZE_REGIONGROW, _T("转换为二值图(区域增长)"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_SHIFT, _T("平移操作"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_SCALE, _T("放缩操作"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_ROTATE, _T("旋转操作"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_NEGATIVE_COLORFUL, _T("色彩反转操作"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_NEGATIVE, _T("灰度反转操作"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_LINEAR_GRAYSCALE, _T("线性灰度变换"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_LINEAR_GRAYSCALE_MULTIRANGES, _T("分段线性灰度变换"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_NONLINEAR_LOG, _T("对数灰度变换"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_NONLINEAR_EXP, _T("指数灰度变换"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_DFT, _T("离散傅里叶变换"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_IDFT, _T("逆离散傅里叶变换"), _T(""), 0);

		m_MenuEx.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rectDlg.left + 70, rectDlg.top + static_cast<int>((rectDlg.bottom - rectDlg.top) * 0.104), this);
		m_MenuEx.DestroyMenu();

		m_MenuEx.UnInstallHook();
	}

	if (m_rtBtnEnhance.PtInRect(point)) {
		CRect rectDlg;
		GetWindowRect(rectDlg);//获得窗体的大小
		CMenuEx m_MenuEx;
		m_MenuEx.CreatePopupMenu();

		m_MenuEx.SetItemHeight(30);
		m_MenuEx.SetFontInfo(14);
		m_MenuEx.SetBorderColor(RGB(45, 51, 60));
		m_MenuEx.SetMenuWidth(130);
		m_MenuEx.InstallHook(theApp.m_hInstance);

		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_DHE, _T("离散直方图均衡化"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_DHS, _T("离散直方图规定化"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_EDGE_ROBERT, _T("边缘提取（Robert算子）"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_EDGE_PREWITT, _T("边缘提取（Prewitt算子）"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_EDGE_SOBEL, _T("边缘提取（Sobel算子）"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_EDGE_FRIECHEN, _T("边缘提取（Frie-Chen算子）"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_EDGE_LAPLACIAN, _T("边缘提取（Laplacian算子）"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_EDGE_LOG, _T("边缘提取（LOG算子）"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_EDGE_CANNY, _T("边缘提取（Canny算子）"), _T(""), 0);

		m_MenuEx.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rectDlg.left + 130, rectDlg.top + static_cast<int>((rectDlg.bottom - rectDlg.top) * 0.104), this);
		m_MenuEx.DestroyMenu();

		m_MenuEx.UnInstallHook();
	}

	if (m_rtBtnFilterate.PtInRect(point)) {
		CRect rectDlg;
		GetWindowRect(rectDlg);//获得窗体的大小
		CMenuEx m_MenuEx;
		m_MenuEx.CreatePopupMenu();

		m_MenuEx.SetItemHeight(30);
		m_MenuEx.SetFontInfo(14);
		m_MenuEx.SetBorderColor(RGB(45, 51, 60));
		m_MenuEx.SetMenuWidth(130);
		m_MenuEx.InstallHook(theApp.m_hInstance);

		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_FILTER_MAX, _T("邻域最大值滤波"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_FILTER_MIN, _T("邻域最小值滤波"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_FILTER_MEDIAN, _T("邻域中值滤波"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_FILTER_MEAN, _T("邻域均值滤波"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_FILTER_BUTTERWORTH_LOW, _T("巴特沃斯低通滤波"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_FILTER_BUTTERWORTH_HIGH, _T("巴特沃斯高通滤波"), _T(""), 0);

		m_MenuEx.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rectDlg.left + 190, rectDlg.top + static_cast<int>((rectDlg.bottom - rectDlg.top) * 0.104), this);
		m_MenuEx.DestroyMenu();

		m_MenuEx.UnInstallHook();
	}

	if (m_rtBtnNoise.PtInRect(point)) {
		CRect rectDlg;
		GetWindowRect(rectDlg);//获得窗体的大小
		CMenuEx m_MenuEx;
		m_MenuEx.CreatePopupMenu();

		m_MenuEx.SetItemHeight(30);
		m_MenuEx.SetFontInfo(14);
		m_MenuEx.SetBorderColor(RGB(45, 51, 60));
		m_MenuEx.SetMenuWidth(130);
		m_MenuEx.InstallHook(theApp.m_hInstance);

		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_NOISE_AWGN, _T("加性高斯白噪声"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_NOISE_POISSON_GAUSSIAN, _T("泊松-高斯很合噪声"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_NOISE_ELD_SFRN, _T("信号相关联合噪声"), _T(""), 0);

		m_MenuEx.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rectDlg.left + 250, rectDlg.top + static_cast<int>((rectDlg.bottom - rectDlg.top) * 0.104), this);
		m_MenuEx.DestroyMenu();

		m_MenuEx.UnInstallHook();
	}
	if (m_rtBtnAnalyze.PtInRect(point))
	{
		CRect rectDlg;
		GetWindowRect(rectDlg);//获得窗体的大小
		CMenuEx m_MenuEx;
		m_MenuEx.CreatePopupMenu();

		m_MenuEx.SetItemHeight(30);
		m_MenuEx.SetFontInfo(14);
		m_MenuEx.SetBorderColor(RGB(45, 51, 60));
		m_MenuEx.SetMenuWidth(130);
		m_MenuEx.InstallHook(theApp.m_hInstance);

		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_HISTOGRAM, _T("色彩直方图分析"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_HISTOGRAM_GRAY, _T("灰度直方图分析"), _T(""), 0);

		m_MenuEx.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rectDlg.left + 310, rectDlg.top + static_cast<int>((rectDlg.bottom - rectDlg.top) * 0.104), this);
		m_MenuEx.DestroyMenu();

		m_MenuEx.UnInstallHook();
	}

	if (m_rBtnOption.PtInRect(point))
	{
		CRect rectDlg;
		GetWindowRect(rectDlg);//获得窗体的大小
		CMenuEx m_MenuEx;
		m_MenuEx.CreatePopupMenu();

		m_MenuEx.SetItemHeight(30);
		m_MenuEx.SetFontInfo(14);
		m_MenuEx.SetBorderColor(RGB(45, 51, 60));
		m_MenuEx.SetMenuWidth(130);
		m_MenuEx.InstallHook(theApp.m_hInstance);

		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_OPTION, _T("参数配置"), _T(""), 0);
		m_MenuEx.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rectDlg.left + 370, rectDlg.top + static_cast<int>((rectDlg.bottom - rectDlg.top) * 0.104), this);
		m_MenuEx.DestroyMenu();

		m_MenuEx.UnInstallHook();
	}

	//CDialogEx::OnLButtonDown(nFlags, point);

}
cv::String CMFCPicViewerDlg::Cstring_to_cvString(CString cstr) {
	return CT2A(cstr.GetString());
}
void CMFCPicViewerDlg::OnMenuOpen()
{
	// 打开文件
	CFileDialog dlg(TRUE, _T("jpg"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("图片文件 (*.jpg;*.png;*.bmp;*.jpeg;*.tiff;*.tif)|*.jpg;*.png;*.bmp;*.jpeg;*.tiff;*.tif||"));
	if (dlg.DoModal() == IDOK)
	{
		while (!img_stack.empty())img_stack.pop();
		file_path = dlg.GetPathName();
		// TODO: 加载并显示图片
		origin_img = imread(Cstring_to_cvString(file_path));
		img_stack.push(origin_img);
		CVMat_to_Pic(origin_img, IDC_STATIC_PIC);
		current_img = origin_img.clone();
	}
}

void CMFCPicViewerDlg::OnMenuSaveTo()
{
	if (img_stack.empty())
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
	CString strExt = PathFindExtension(file_path); // .jpg 等
	if (strExt.IsEmpty())
		strExt = _T(".png");

	save_file_path = strFolder + _T("result") + strExt;

	// 3. OpenCV 保存
	bool ok = false;
	if (strExt.CompareNoCase(_T(".jpg")) == 0 || strExt.CompareNoCase(_T(".jpeg")) == 0)
		if (current_img.type() == CV_32FC2) {
			ok = cv::imwrite(Cstring_to_cvString(save_file_path), Transforms::Fourier::spectrumView(current_img),
				std::vector<int>{cv::IMWRITE_JPEG_QUALITY, 95});
		}
		else ok = cv::imwrite(Cstring_to_cvString(save_file_path), current_img,
			std::vector<int>{cv::IMWRITE_JPEG_QUALITY, 95});
	else
		ok = cv::imwrite(Cstring_to_cvString(save_file_path), current_img);

	// 4. 提示
	if (ok)
		AfxMessageBox(_T("已保存到：\n") + save_file_path, MB_ICONINFORMATION);
	else
		AfxMessageBox(_T("保存失败！"), MB_ICONERROR);
}

void CMFCPicViewerDlg::OnMenuSaveAsTo()
{
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}

	// 获取原扩展名（含点号）
	CString strExt = PathFindExtension(file_path);
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
	bool ok = false;
	if (ext == _T(".jpg") || ext == _T(".jpeg"))
	{
		if (current_img.type() == CV_32FC2) {
			ok = cv::imwrite(Cstring_to_cvString(save_file_path), Transforms::Fourier::spectrumView(current_img),
				std::vector<int>{cv::IMWRITE_JPEG_QUALITY, 95});
		}
		else ok = cv::imwrite(Cstring_to_cvString(save_file_path), current_img,
			std::vector<int>{cv::IMWRITE_JPEG_QUALITY, 95});
	}
	else if (ext == _T(".png"))
	{
		if (current_img.type() == CV_32FC2) {
			ok = cv::imwrite(Cstring_to_cvString(savePath), Transforms::Fourier::spectrumView(current_img),
				std::vector<int>{cv::IMWRITE_PNG_COMPRESSION, 9});
		}
		else ok = cv::imwrite(Cstring_to_cvString(savePath), current_img,
			std::vector<int>{cv::IMWRITE_PNG_COMPRESSION, 9});
	}
	else
	{
		ok = cv::imwrite(Cstring_to_cvString(savePath), current_img);
	}

	if (ok)
		AfxMessageBox(_T("图像已保存至：\n") + savePath, MB_ICONINFORMATION);
	else
		AfxMessageBox(_T("保存失败！"), MB_ICONERROR);
}

void CMFCPicViewerDlg::OnMenuGrayize()
{
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	img_stack.push(current_img.clone());
	Mat gray = Transforms::Transform(current_img,Transforms::Modules::BASIC::GRAYIZE);
	cvtColor(gray,current_img,COLOR_GRAY2BGR);
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("灰度化完成"));
}


void CMFCPicViewerDlg::OnMenuBinaryize()
{
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	img_stack.push(current_img.clone());
	Mat binary = Transforms::Transform(current_img,Transforms::Modules::BASIC::BINARYIZE);
	cvtColor(binary, current_img, COLOR_GRAY2BGR);
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("二值化完成"));
}

void CMFCPicViewerDlg::OnMenuUndo()
{
	if (img_stack.size() == 1) // ← 至少要留一张原图
	{
		AfxMessageBox(_T("已经是最初状态，无法继续撤销！"), MB_ICONINFORMATION);
		return;
	}
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	current_img = img_stack.top();
	img_stack.pop();
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("撤销一步！"));
}

void CMFCPicViewerDlg::OnMenuReload()
{
	if (!current_img.empty())
		img_stack.push(current_img.clone());

	current_img = origin_img.clone(); // 必须 clone，避免共享内存
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("重置完成"));
}

void CMFCPicViewerDlg::OnMenuShift() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	img_stack.push(current_img.clone());
	current_img = Transforms::Transform(current_img,shift_x,shift_y, Transforms::Modules::BASIC::SHIFT);
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("平移完成"));
}

void CMFCPicViewerDlg::OnMenuScale() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	img_stack.push(current_img.clone());                             // 结果图像
	current_img = Transforms::Transform(current_img,scale_x,scale_y,Transforms::Modules::BASIC::SCALE);
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("放缩完成"));
}
void CMFCPicViewerDlg::OnMenuRotate() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	img_stack.push(current_img.clone());
	current_img = Transforms::Transform(current_img,rotate_theta,Transforms::Modules::BASIC::ROTATE);
	// 执行旋转                           // 返回旋转结果
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("旋转完成"));
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
	for (int i = 0; i < histSize; ++i) {
		gmin = gmin < cvRound(grayhist.at<float>(i)) ? gmin : cvRound(grayhist.at<float>(i));
		gmax = gmax > cvRound(grayhist.at<float>(i)) ? gmax : cvRound(grayhist.at<float>(i));
	}
	return make_pair(gmin, gmax);
}
void CMFCPicViewerDlg::OnMenuHistogram()
{
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	std::vector<cv::Mat> bgr;
	cv::split(current_img, bgr);          // bgr[0]=B, bgr[1]=G, bgr[2]=R

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
	Analyze_img = histImage;
	CVMat_to_Pic(histImage, IDC_STATIC_PIC_ANALYZE);
	Info_append(("分析完成"));
}
void CMFCPicViewerDlg::OnMenuHistogramGray() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	Mat gray(current_img.rows, current_img.cols, CV_8UC1);
	current_img.copyTo(gray);
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
	Analyze_img = histImage;
	CVMat_to_Pic(histImage, IDC_STATIC_PIC_ANALYZE);
	pair<int,int> key = Count_Gray_MinMax(gray_hist,histSize);
	Info_append("灰度最小值为：");
	Info_append(to_string(key.first).c_str());
	Info_append("灰度最大值为：");
	Info_append(to_string(key.second).c_str());
	/* ---------- 1. 归一化为概率 ---------- */
	double total = gray.total();          // 总像素数
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
	Info_append(("分析完成"));
}

void CMFCPicViewerDlg::OnMenuTransform(Mat(*func)(Mat), Mat input) {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	Mat temp = (*func)(input);
	CVMat_to_Pic(temp, IDC_STATIC_PIC);
	return;
};

void CMFCPicViewerDlg::OnMenuNegative() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	img_stack.push(current_img.clone());
	Mat gray = Transforms::Transform(current_img,Transforms::Modules::LINEAR::NEGATIVE);
	cvtColor(gray, current_img, COLOR_GRAY2BGR);
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("灰度反转完成"));
}
void CMFCPicViewerDlg::OnMenuNegativeColorful() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	img_stack.push(current_img.clone());
	current_img = Transforms::Transform(current_img,Transforms::Modules::LINEAR::NEGATIVE_COLORFUL);
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("灰度反转完成"));
}
void CMFCPicViewerDlg::OnMenuLinearGrayScale() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	img_stack.push(current_img.clone());
	current_img = Transforms::Transform(current_img,output_grayscale,Transforms::Modules::LINEAR::SCALE_LINEAR);
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("灰度线性变换完成"));
}
void CMFCPicViewerDlg::OnMenuLinearGrayScaleMultiranges() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	img_stack.push(current_img.clone());
	current_img = Transforms::Transform(current_img,oper_ranges,gray_scales,Transforms::Modules::LINEAR::SCALE_LINEAR_MULTIRANGES);
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("灰度线性变换完成"));
}
void CMFCPicViewerDlg::OnMenuNonlinearLog() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	img_stack.push(current_img.clone());
	current_img = Transforms::Transform(current_img, logK,Transforms::Modules::NONLINEAR::LOG);
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("灰度线性变换完成"));
}
void CMFCPicViewerDlg::OnMenuNonlinearExp() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	img_stack.push(current_img.clone());
	current_img = Transforms::Transform(current_img, expK, Transforms::Modules::NONLINEAR::EXP);
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("灰度线性变换完成"));
}
void CMFCPicViewerDlg::OnMenuDHE() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	img_stack.push(current_img.clone());
	current_img = Transforms::Enhance(current_img, Transforms::Modules::ENHANCE::DHE);
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("离散直方图均衡化完成"));
}
void CMFCPicViewerDlg::OnMenuDHS() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	// 打开文件
	CFileDialog dlg(TRUE, _T("jpg"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("图片文件 (*.jpg;*.png;*.bmp;*.jpeg;*.tiff;*.tif)|*.jpg;*.png;*.bmp;*.jpeg;*.tiff;*.tif||"));
	if (dlg.DoModal() == IDOK)
	{
		while (!img_stack.empty())img_stack.pop();
		ref_file_path = dlg.GetPathName();
		// TODO: 加载并显示图片
		referrence_img = imread(Cstring_to_cvString(ref_file_path));
	}
	img_stack.push(current_img.clone());
	current_img = Transforms::Enhance(current_img, referrence_img, Transforms::Modules::ENHANCE::DHS);
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("离散直方图规定化完成"));
}
void CMFCPicViewerDlg::OnMenuSpatialFilterMax() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	img_stack.push(current_img.clone());
	current_img = Filters::filterate(current_img, filter_kernel ,Filters::Modules::SPATIAL::MAX);
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("滤波完成"));
}
void CMFCPicViewerDlg::OnMenuSpatialFilterMin() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	img_stack.push(current_img.clone());
	current_img = Filters::filterate(current_img, filter_kernel, Filters::Modules::SPATIAL::MIN);
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("滤波完成"));
}
void CMFCPicViewerDlg::OnMenuSpatialFilterMedian() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	img_stack.push(current_img.clone());
	current_img = Filters::filterate(current_img, filter_kernel, Filters::Modules::SPATIAL::MEDIAN);
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("滤波完成"));
}
void CMFCPicViewerDlg::OnMenuSpatialFilterMean() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	img_stack.push(current_img.clone());
	current_img = Filters::filterate(current_img, filter_kernel, Filters::Modules::SPATIAL::MEAN);
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("滤波完成"));
}
void CMFCPicViewerDlg::OnMenuNoiseAWGN() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	img_stack.push(current_img.clone());
	current_img = Noises::noise(current_img,noise_AWGN_sigma,Noises::Modules::CLASSIC::AWGN);
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("噪声添加完成"));
}
void CMFCPicViewerDlg::OnMenuNoisePoissonGaussian() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	img_stack.push(current_img.clone());
	current_img = Noises::noise(current_img, noise_Poisson_Gaussian_gain, noise_Poisson_Gaussian_sigma, Noises::Modules::CLASSIC::POISSON_GAUSSIAN);
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("噪声添加完成"));
}
void CMFCPicViewerDlg::OnMenuNoiseELDSFRN() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	img_stack.push(current_img.clone());
	current_img = Noises::noise(current_img, noise_ELD_SFRN_alpha, noise_ELD_SFRN_sigmaT, noise_ELD_SFRN_sigmaG, noise_ELD_SFRN_q, Noises::Modules::CLASSIC::ELD_SFRN);
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("噪声添加完成"));
}
void CMFCPicViewerDlg::OnMenuSaveAnalyzeTo() {
	if (img_stack.empty() || current_img.empty())
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
	CString strExt = PathFindExtension(file_path); // .jpg 等
	if (strExt.IsEmpty())
		strExt = _T(".png");

	save_file_path = strFolder + _T("result") + strExt;

	// 3. OpenCV 保存
	bool ok = false;
	if (strExt.CompareNoCase(_T(".jpg")) == 0 || strExt.CompareNoCase(_T(".jpeg")) == 0)
		ok = cv::imwrite(Cstring_to_cvString(save_file_path), Analyze_img,
			std::vector<int>{cv::IMWRITE_JPEG_QUALITY, 95});
	else
		ok = cv::imwrite(Cstring_to_cvString(save_file_path), Analyze_img);

	// 4. 提示
	if (ok)
		AfxMessageBox(_T("已保存到：\n") + save_file_path, MB_ICONINFORMATION);
	else
		AfxMessageBox(_T("保存失败！"), MB_ICONERROR);
}
void CMFCPicViewerDlg::OnMenuDFT() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	img_stack.push(current_img.clone());
	current_img = Transforms::Fourier::FFT(current_img);
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("傅里叶变换完成"));
}
void CMFCPicViewerDlg::OnMenuIDFT() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	img_stack.push(current_img.clone());
	//current_img = Transforms::Transform(current_img, Transforms::Modules::FOURIER::IDFT);
	current_img = Transforms::Fourier::IFFT(current_img);
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("逆傅里叶变换完成"));
}
void CMFCPicViewerDlg::OnMenuButterworthLowFilter() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	if (current_img.type() != CV_32FC2) {
		AfxMessageBox(_T("还没有转换傅里叶滤波图像！"), MB_ICONWARNING);
		return;
	}
	img_stack.push(current_img.clone());
	current_img = Filters::filterate(current_img, butterworth_level, butterworth_D0, Filters::Modules::BUTTERWORTH::LOW);
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("低通滤波完成"));
};
void CMFCPicViewerDlg::OnMenuButterworthHighFilter() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	if (current_img.type() != CV_32FC2) {
		AfxMessageBox(_T("还没有转换傅里叶滤波图像！"), MB_ICONWARNING);
		return;
	}
	img_stack.push(current_img.clone());
	current_img = Filters::filterate(current_img, butterworth_level, butterworth_D0, Filters::Modules::BUTTERWORTH::HIGH);
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("高通滤波完成"));
};
void CMFCPicViewerDlg::Info_append(std::string str) {
	CString now;
	CString cstr = CA2T(str.c_str());
	Info_Bar_Cedit.GetWindowTextW(now);
	now = now + _T("\r\n") + CA2T(to_string(++Infobar_line).c_str()) + _T(" ") + cstr;
	Info_Bar_Cedit.SetWindowTextW(now);
	Info_Bar_Cedit.RedrawWindow();
	Info_Bar_Cedit.SetSel(1, 1, FALSE);
	Info_Bar_Cedit.SetFocus();
}
void CMFCPicViewerDlg::Info_clear() {
	Info_Bar_Cedit.SetWindowTextW(_T(""));
	return;
}
void CMFCPicViewerDlg::OnMenuBinaryize_Otsu() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	img_stack.push(current_img.clone());
	current_img = Transforms::Transform(current_img, Transforms::Modules::DEVIDE::OTSU);
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("Otsu二值化完成"));
}
void CMFCPicViewerDlg::OnMenuBinaryize_RegionGrow() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	img_stack.push(current_img.clone());
	current_img = Transforms::Transform(current_img, Transforms::Modules::DEVIDE::REGIONGROW);
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("Otsu二值化完成"));
}
void CMFCPicViewerDlg::OnMenuEdgeRobert() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	img_stack.push(current_img.clone());
	current_img = Transforms::Enhance(current_img, edge_threshold, Transforms::Modules::EDGE::ROBERT);
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("边缘提取完成"));
};
void CMFCPicViewerDlg::OnMenuEdgePrewitt() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	img_stack.push(current_img.clone());
	current_img = Transforms::Enhance(current_img, edge_threshold, Transforms::Modules::EDGE::PREWITT);
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("边缘提取完成"));
};
void CMFCPicViewerDlg::OnMenuEdgeSobel() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	img_stack.push(current_img.clone());
	current_img = Transforms::Enhance(current_img, edge_threshold, Transforms::Modules::EDGE::SOBEL);
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("边缘提取完成"));
};
void CMFCPicViewerDlg::OnMenuEdgeFrieChen() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	img_stack.push(current_img.clone());
	current_img = Transforms::Enhance(current_img, edge_threshold, Transforms::Modules::EDGE::FRIE_CHEN);
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("边缘提取完成"));
};
void CMFCPicViewerDlg::OnMenuEdgeLaplacian() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
	img_stack.push(current_img.clone());
	current_img = Transforms::Enhance(current_img, edge_laplacian_level1_threshold,edge_laplacian_level2_threshold, Transforms::Modules::EDGE::LAPLACIAN);
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	Info_append(("边缘提取完成"));
};
void CMFCPicViewerDlg::OnMenuEdgeLOG() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
}
void CMFCPicViewerDlg::OnMenuEdgeCanny() {
	if (img_stack.empty() || current_img.empty())
	{
		AfxMessageBox(_T("还没有加载图像！"), MB_ICONWARNING);
		return;
	}
};
