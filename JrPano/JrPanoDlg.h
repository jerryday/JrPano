
// JrPanoDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include <deque>
#include <string>
#include <algorithm>
#include "pipeline.h"
#include "HScrollListBox.h"


// CJrPanoDlg dialog
class CJrPanoDlg : public CDialogEx
{
// Construction
public:
	CJrPanoDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_JRPANO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedSelect();
    afx_msg void OnBnClickedUp();
    afx_msg void OnBnClickedDown();
    afx_msg void OnBnClickedReverse();
    afx_msg void OnBnClickedStart();

    std::deque<std::string> imgpaths_;
    CHScrollListBox listbox_;
    CEdit textbox_;
    Pipeline pipeline_;

    void MoveListBoxItem(int direction);
    void AppendLineToTextBox(LPCTSTR pszText);
    afx_msg void OnBnClickedSetting();
    afx_msg void OnBnClickedShowmatch();
    afx_msg void OnBnClickedSave();
    afx_msg void OnBnClickedColorpart();
    afx_msg void OnBnClickedShowagain();
};
