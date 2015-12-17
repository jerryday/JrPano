// SettingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "JrPano.h"
#include "SettingDlg.h"
#include "afxdialogex.h"


// SettingDlg dialog

IMPLEMENT_DYNAMIC(SettingDlg, CDialogEx)

SettingDlg::SettingDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(SettingDlg::IDD, pParent)
    , input_hess_(0), input_conf_(0), input_area_(0), input_blend_(0), warp_mode_(0) {

}

SettingDlg::~SettingDlg()
{
}


BOOL SettingDlg::OnInitDialog() {
    CDialogEx::OnInitDialog();
    if (warp_mode_ >= 3 || warp_mode_ < 0)
        warp_mode_ = 0;
    ((CComboBox *) GetDlgItem(IDWARP))->SetCurSel(warp_mode_);
    return TRUE;
}


void SettingDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDHESS, input_hess_);
    DDX_Text(pDX, IDCONF, input_conf_);
    DDV_MinMaxFloat(pDX, input_conf_, 0, 1);
    DDX_Text(pDX, IDAREA, input_area_);
    DDX_Text(pDX, IDBLEND, input_blend_);
    DDV_MinMaxInt(pDX, input_blend_, 0, 10);
    DDX_CBIndex(pDX, IDWARP, warp_mode_);
}


BEGIN_MESSAGE_MAP(SettingDlg, CDialogEx)
END_MESSAGE_MAP()


// SettingDlg message handlers
