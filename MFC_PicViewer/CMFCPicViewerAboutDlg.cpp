// CMFCPicViewerAboutDlg.cpp: 实现文件
//

#include "pch.h"
#include "MFC_PicViewer.h"
#include "afxdialogex.h"
#include "CMFCPicViewerAboutDlg.h"
#include "MyButton.h"
#include "MyStatic.h"
#include "resource.h"
// CMFCPicViewerAboutDlg 对话框

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



IMPLEMENT_DYNAMIC(CMFCPicViewerAboutDlg, CDialogEx)

CMFCPicViewerAboutDlg::CMFCPicViewerAboutDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ABOUTDIALOG, pParent)
{

}

CMFCPicViewerAboutDlg::~CMFCPicViewerAboutDlg()
{
}

void CMFCPicViewerAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON11, m_btnMax);
	DDX_Control(pDX, IDC_BUTTON21, m_btnMin);
	DDX_Control(pDX, IDC_BUTTON31, m_btnExit);
	DDX_Control(pDX, IDC_STATIC, m_myText);
}


BEGIN_MESSAGE_MAP(CMFCPicViewerAboutDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_NCHITTEST()
	ON_WM_QUERYDRAGICON()
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON11, &CMFCPicViewerAboutDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON21, &CMFCPicViewerAboutDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON31, &CMFCPicViewerAboutDlg::OnBnClickedButton3)
END_MESSAGE_MAP()


// CMFCPicViewerAboutDlg 消息处理程序
BOOL CMFCPicViewerAboutDlg::OnInitDialog()
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

void CMFCPicViewerAboutDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMFCPicViewerAboutDlg::OnPaint()
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
		SelectObject(dc, loldfont);
		DeleteObject(hNewFont);
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMFCPicViewerAboutDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CMFCPicViewerAboutDlg::OnNcHitTest(CPoint point)
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
	}
	return nHitTest;
}

void CMFCPicViewerAboutDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	//重新获得窗口尺寸
	GetClientRect(&m_wndRect);

	//退出按钮重新绘制
	CWnd* pWnd = GetDlgItem(IDC_BUTTON31);
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


	pWnd = GetDlgItem(IDC_BUTTON11);
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

	pWnd = GetDlgItem(IDC_BUTTON21);
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





void CMFCPicViewerAboutDlg::OnBnClickedButton1()
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


void CMFCPicViewerAboutDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	ShowWindow(SW_SHOWMINIMIZED);
}


void CMFCPicViewerAboutDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialog::OnCancel();
}

HBRUSH CMFCPicViewerAboutDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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