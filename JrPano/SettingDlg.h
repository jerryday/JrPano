#pragma once
#include "afxwin.h"


// SettingDlg dialog

class SettingDlg : public CDialogEx
{
	DECLARE_DYNAMIC(SettingDlg)

public:
	SettingDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~SettingDlg();

// Dialog Data
	enum { IDD = IDD_SETTING_DIALOG };

protected:
    virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    double input_hess_;
    float input_conf_;
    double input_area_;
    int input_blend_;
    int warp_mode_;
};
