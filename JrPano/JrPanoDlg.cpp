
// JrPanoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "JrPano.h"
#include "JrPanoDlg.h"
#include "SettingDlg.h"
#include "afxdialogex.h"
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CJrPanoDlg dialog



CJrPanoDlg::CJrPanoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CJrPanoDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CJrPanoDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDIMGLIST, listbox_);
    DDX_Control(pDX, IDTEXTOUTPUT, textbox_);
}

BEGIN_MESSAGE_MAP(CJrPanoDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDSELECT, &CJrPanoDlg::OnBnClickedSelect)
    ON_BN_CLICKED(IDUP, &CJrPanoDlg::OnBnClickedUp)
    ON_BN_CLICKED(IDDOWN, &CJrPanoDlg::OnBnClickedDown)
    ON_BN_CLICKED(IDREVERSE, &CJrPanoDlg::OnBnClickedReverse)
    ON_BN_CLICKED(IDSTART, &CJrPanoDlg::OnBnClickedStart)
    ON_BN_CLICKED(IDSETTING, &CJrPanoDlg::OnBnClickedSetting)
    ON_BN_CLICKED(IDSHOWMATCH, &CJrPanoDlg::OnBnClickedShowmatch)
    ON_BN_CLICKED(IDSAVE, &CJrPanoDlg::OnBnClickedSave)
    ON_BN_CLICKED(IDCOLORPART, &CJrPanoDlg::OnBnClickedColorpart)
    ON_BN_CLICKED(IDSHOWAGAIN, &CJrPanoDlg::OnBnClickedShowagain)
END_MESSAGE_MAP()


// CJrPanoDlg message handlers

BOOL CJrPanoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CJrPanoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CJrPanoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CJrPanoDlg::OnBnClickedSelect() {
    DWORD dwflags = OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    CFileDialog filedlg(TRUE, NULL, NULL, dwflags);  // TRUE for fileopen, FALSE for filesaveas

    CString strbuf;
    const int kmaxfile = 50;
    const int kbufsize = (kmaxfile * (MAX_PATH + 1)) + 1;
    filedlg.GetOFN().lpstrFile = strbuf.GetBuffer(kbufsize);
    // fuck param name
    filedlg.GetOFN().nMaxFile = kbufsize - 1;

    if (IDOK == filedlg.DoModal()) {
        POSITION pos = filedlg.GetStartPosition();
        listbox_.ResetContent();
        imgpaths_.clear();
        while (pos != NULL) {
            CString filename = filedlg.GetNextPathName(pos);
            CT2CA filename2(filename);
            listbox_.AddString(filename);
            imgpaths_.push_back(string(filename2));
        }
        
        if (!imgpaths_.empty()) {
            GetDlgItem(IDUP)->EnableWindow(TRUE);
            GetDlgItem(IDDOWN)->EnableWindow(TRUE);
            GetDlgItem(IDREVERSE)->EnableWindow(TRUE);
            GetDlgItem(IDSTART)->EnableWindow(TRUE);
        }
    }

    strbuf.ReleaseBuffer();
}


void CJrPanoDlg::MoveListBoxItem(int direction) {
    int indexselected = listbox_.GetCurSel();
    if (LB_ERR == indexselected || indexselected + direction < 0 || indexselected + direction > (int)imgpaths_.size() - 1)
        return;

    string path = imgpaths_[indexselected];
    imgpaths_.erase(imgpaths_.begin() + indexselected);
    imgpaths_.insert(imgpaths_.begin() + indexselected + direction, path);

    listbox_.ResetContent();
    for (size_t i = 0; i < imgpaths_.size(); i++) {
        CString path2(imgpaths_[i].c_str());
        listbox_.InsertString(i, path2);
    }
    listbox_.SetCurSel(indexselected + direction);
}


void CJrPanoDlg::OnBnClickedUp() {
    MoveListBoxItem(-1);
}


void CJrPanoDlg::OnBnClickedDown() {
    MoveListBoxItem(1);
}


void CJrPanoDlg::OnBnClickedReverse() {
    reverse(imgpaths_.begin(), imgpaths_.end());

    listbox_.ResetContent();
    for (size_t i = 0; i < imgpaths_.size(); i++) {
        CString path2(imgpaths_[i].c_str());
        listbox_.InsertString(i, path2);
    }
}


void CJrPanoDlg::OnBnClickedStart() {
    int64 ticks;
    CString msg;
    textbox_.SetWindowText(_T(""));
    // use UpdateWindow() to refresh whole editctrl instantly
    textbox_.UpdateWindow();
    try {
        vector<string> imgpaths(imgpaths_.begin(), imgpaths_.end());

        ticks = cv::getTickCount();
        pipeline_.ReadImages(imgpaths);
        msg.Format(_T("读取图片: %lf秒"), (cv::getTickCount() - ticks) / cv::getTickFrequency());
        AppendLineToTextBox(msg);

        ticks = cv::getTickCount();
        pipeline_.DetectKeypoints();
        msg.Format(_T("检测特征: %lf秒"), (cv::getTickCount() - ticks) / cv::getTickFrequency());
        AppendLineToTextBox(msg);

        ticks = cv::getTickCount();
        pipeline_.FindMatchesAndHomos();
        msg.Format(_T("匹配特征: %lf秒"), (cv::getTickCount() - ticks) / cv::getTickFrequency());
        AppendLineToTextBox(msg);

        ticks = cv::getTickCount();
        pipeline_.EstimateFocal();
        pipeline_.CalculateRotation();
        pipeline_.FindSeams();
        msg.Format(_T("估计移动: %lf秒"), (cv::getTickCount() - ticks) / cv::getTickFrequency());
        AppendLineToTextBox(msg);

        ticks = cv::getTickCount();
        pipeline_.WarpAndComposite();
        msg.Format(_T("拼接图片: %lf秒"), (cv::getTickCount() - ticks) / cv::getTickFrequency());
        AppendLineToTextBox(msg);

        pipeline_.ShowResult();

        GetDlgItem(IDSHOWAGAIN)->EnableWindow(TRUE);
        GetDlgItem(IDSHOWMATCH)->EnableWindow(TRUE);
        GetDlgItem(IDCOLORPART)->EnableWindow(TRUE);
        GetDlgItem(IDSAVE)->EnableWindow(TRUE);
    } catch (JrException ex) {
        AppendLineToTextBox(CString(ex.what()));
    }
}


void CJrPanoDlg::AppendLineToTextBox(LPCTSTR pszText) {
    CString line;
    line.Format(_T("%s\r\n"), pszText);

    int len = textbox_.GetWindowTextLength();
    textbox_.SetSel(len, len);
    textbox_.ReplaceSel(line);
}


void CJrPanoDlg::OnBnClickedSetting() {
    SettingDlg dlg;
    dlg.input_hess_ = pipeline_.hess_thresh();
    dlg.input_area_ = pipeline_.work_area();
    dlg.input_conf_ = pipeline_.match_conf();
    dlg.input_blend_ = pipeline_.blend_bands();
    dlg.warp_mode_ = pipeline_.warp_mode();
    
    if (IDOK == dlg.DoModal()) {
        pipeline_.set_hess_thresh(dlg.input_hess_);
        pipeline_.set_work_area(dlg.input_area_);
        pipeline_.set_match_conf(dlg.input_conf_);
        pipeline_.set_blend_bands(dlg.input_blend_);
        pipeline_.set_warp_mode(dlg.warp_mode_);
    }
}


void CJrPanoDlg::OnBnClickedShowmatch() {
    if (pipeline_.imgs().empty() || pipeline_.matches().empty() || pipeline_.keypoints().empty()) {
        AppendLineToTextBox(_T("还没有生成全景图呢"));
        return;
    }

    pipeline_.DrawMatches();
}


void CJrPanoDlg::OnBnClickedSave() {
    if (pipeline_.result().empty()) {
        AppendLineToTextBox(_T("还没有生成全景图呢"));
        return;
    }

    CFileDialog filedlg(FALSE);

    if (IDOK == filedlg.DoModal()) {
        CString path = filedlg.GetPathName();
        CT2CA path2(path);
        pipeline_.WriteResult(string(path2));
    }
}


void CJrPanoDlg::OnBnClickedColorpart() {
    if (pipeline_.result().empty()) {
        AppendLineToTextBox(_T("还没有生成全景图呢"));
        return;
    }

    pipeline_.ShowOriginalPart();
}


void CJrPanoDlg::OnBnClickedShowagain() {
    if (pipeline_.result().empty()) {
        AppendLineToTextBox(_T("还没有生成全景图呢"));
        return;
    }

    pipeline_.ShowResult();
}
