
// MFC_PicViewerDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "MFC_PicViewer.h"
#include "MFC_PicViewerDlg.h"
#include "afxdialogex.h"

#include "MyMenu.h"
#include "MyButton.h"
#include "MyStatic.h"
#include "MyOpenCV.h"
#include "MyImageStatic.h"
#include "MyTransforms.h"

#include <opencv2/opencv.hpp>
#include <stack>
#ifdef _DEBUG
#define new DEBUG_NEW

#endif
using namespace cv;

// 程序变量定义区
CString file_path;
CString save_file_path;
Mat origin_img(10, 10, CV_8UC3, cv::Scalar(39, 34, 30));
Mat NULL_img(10, 10, CV_8UC3, cv::Scalar(39, 34, 30));
Mat current_img(10, 10, CV_8UC3, cv::Scalar(39, 34, 30));
Mat next_img(10, 10, CV_8UC3, cv::Scalar(39, 34, 30));
Mat Analyze_img(10, 10, CV_8UC3, cv::Scalar(39, 34, 30));
std::stack<Mat> img_stack;

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
	ON_COMMAND(MSG_MENU_ABOUT, &CMFCPicViewerDlg::OnMenuAbout)
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
		//绘制分析菜单栏
		GetClientRect(&rtBtnClo);
		rtBtnClo.left = rtBtnClo.left + 130;
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
		//绘制帮助菜单栏
		GetClientRect(&rtBtnClo);
		rtBtnClo.left = rtBtnClo.left + 190;
		rtBtnClo.right = rtBtnClo.left + 30;
		rtBtnClo.bottom *= 0.1;
		rtBtnClo.top = rtBtnClo.bottom - 20;
		m_rtBtnHelp = rtBtnClo;
		dc.Rectangle(rtBtnClo);
		dc.FillSolidRect(m_rtBtnHelp, RGB(45, 51, 59));
		dc.SetBkMode(TRANSPARENT);
		str1 = TEXT("帮助");
		dc.SetTextColor(RGB(185, 192, 197));
		dc.DrawText(str1, m_rtBtnHelp, DT_LEFT | DT_VCENTER);

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
		if (m_rtBtnfile.PtInRect(point) || m_rtBtnEdit.PtInRect(point) || m_rtBtnAnalyze.PtInRect(point) || m_rtBtnHelp.PtInRect(point))
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
	if (img.channels() == 1) {
		cvtColor(img, img, COLOR_GRAY2BGR);
	}
	CWnd* pWnd = GetDlgItem(nID);
	if (!pWnd) return;

	CRect rect;
	pWnd->GetClientRect(&rect);
	int ctrlW = rect.Width();
	int ctrlH = rect.Height();

	if (img.empty() || ctrlW <= 0 || ctrlH <= 0)
		return;

	// 计算缩放比例（保持宽高比）
	double scaleX = static_cast<double>(ctrlW) / img.cols;
	double scaleY = static_cast<double>(ctrlH) / img.rows;
	double scale = min(scaleX, scaleY);  // 取较小比例，防止溢出

	int dstW = cvRound(img.cols * scale);
	int dstH = cvRound(img.rows * scale);

	// 计算居中偏移
	int offsetX = (ctrlW - dstW) / 2;
	int offsetY = (ctrlH - dstH) / 2;

	// 等比例缩放图像
	cv::Mat imgResized;
	cv::resize(img, imgResized, cv::Size(dstW, dstH));

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
		m_MenuEx.SetMenuWidth(60);
		m_MenuEx.InstallHook(theApp.m_hInstance);

		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_OPEN, _T("打开"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_SAVETO, _T("保存到..."), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_SAVEASTO, _T("另存为..."), _T(""), 0);

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
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_SHIFT, _T("平移操作"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_SCALE, _T("放缩操作"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_ROTATE, _T("旋转操作"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_NEGATIVE_COLORFUL, _T("色彩反转操作"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_NEGATIVE, _T("灰度反转操作"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_LINEAR_GRAYSCALE, _T("线性灰度变换"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_LINEAR_GRAYSCALE_MULTIRANGES, _T("分段线性灰度变换"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_NONLINEAR_LOG, _T("对数灰度变换"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_NONLINEAR_EXP, _T("指数灰度变换"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_UNDO, _T("撤销"), _T(""), 0);
		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_RELOAD, _T("恢复"), _T(""), 0);

		m_MenuEx.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rectDlg.left + 70, rectDlg.top + static_cast<int>((rectDlg.bottom - rectDlg.top) * 0.104), this);
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

		m_MenuEx.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rectDlg.left + 130, rectDlg.top + static_cast<int>((rectDlg.bottom - rectDlg.top) * 0.104), this);
		m_MenuEx.DestroyMenu();

		m_MenuEx.UnInstallHook();
	}

	if (m_rtBtnHelp.PtInRect(point))
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

		m_MenuEx.AppendItem(MF_STRING, MSG_MENU_ABOUT, _T("关于 PicViewer"), _T(""), 0);
		m_MenuEx.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rectDlg.left + 190, rectDlg.top + static_cast<int>((rectDlg.bottom - rectDlg.top) * 0.104), this);
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
		ok = cv::imwrite(Cstring_to_cvString(save_file_path), current_img,
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
		ok = cv::imwrite(Cstring_to_cvString(savePath), current_img,
			std::vector<int>{cv::IMWRITE_JPEG_QUALITY, 95});
	}
	else if (ext == _T(".png"))
	{
		ok = cv::imwrite(Cstring_to_cvString(savePath), current_img,
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
	AfxMessageBox(_T("灰度化完成"));
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
	AfxMessageBox(_T("二值化完成"));
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
	AfxMessageBox(_T("撤销一步！"));
}

void CMFCPicViewerDlg::OnMenuReload()
{
	if (!current_img.empty())
		img_stack.push(current_img.clone());

	current_img = origin_img.clone(); // 必须 clone，避免共享内存
	CVMat_to_Pic(current_img, IDC_STATIC_PIC);
	AfxMessageBox(_T("重置完成"));
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
	AfxMessageBox(_T("平移完成"));
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
	AfxMessageBox(_T("放缩完成"));
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
	AfxMessageBox(_T("旋转完成"));
}
void CMFCPicViewerDlg::OnMenuAbout()
{
	AfxMessageBox(_T("关于功能待实现"));
}

void Count_Gray_MinMax(Mat grayhist, UINT histSize) {
	int gmin = 256, gmax = 1;
	for (int i = 0; i < histSize; ++i) {
		gmin = gmin < cvRound(grayhist.at<float>(i)) ? gmin : cvRound(grayhist.at<float>(i));
		gmax = gmax > cvRound(grayhist.at<float>(i)) ? gmax : cvRound(grayhist.at<float>(i));
	}
	AfxMessageBox(_T("灰度最小值为：") + CString(to_string(gmin).c_str()));
	AfxMessageBox(_T("灰度最大值为：") + CString(to_string(gmax).c_str()));
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
	CVMat_to_Pic(histImage, IDC_STATIC_PIC_ANALYZE);
	AfxMessageBox(_T("分析完成"));
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
	CVMat_to_Pic(histImage, IDC_STATIC_PIC_ANALYZE);
	Count_Gray_MinMax(gray_hist,histSize);
	AfxMessageBox(_T("分析完成"));
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
	AfxMessageBox(_T("灰度反转完成"));
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
	AfxMessageBox(_T("灰度反转完成"));
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
	AfxMessageBox(_T("灰度线性变换完成"));
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
	AfxMessageBox(_T("灰度线性变换完成"));
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
	AfxMessageBox(_T("灰度线性变换完成"));
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
	AfxMessageBox(_T("灰度线性变换完成"));
}