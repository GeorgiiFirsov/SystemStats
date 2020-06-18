// Project files
#include "stdafx.h"
#include "DlgSave.h"
#include "afxdialogex.h"


IMPLEMENT_DYNAMIC(CDlgSave, CDialog)


/* Message handlers registration */

BEGIN_MESSAGE_MAP(CDlgSave, CDialog)
    ON_BN_CLICKED(IDC_SAVEDLG_RDSPCF, OnRdChoosePressed)
    ON_BN_CLICKED(IDC_SAVEDLG_RDALL, OnRdAllPressed)
END_MESSAGE_MAP()


BOOL CDlgSave::OnInitDialog()
{
    CDialog::OnInitDialog();

    //
    // Set initial state of elements
    // 

    // TODO: put strings to resources

    SetWindowText(L"Save to file");

    m_hEditFile.SetWindowText(L"SystemStats.txt");
    m_hEditRecords.SetWindowText(L"1-100");

    m_hRdAll.SetCheck(BST_CHECKED);

    return TRUE;
}

void CDlgSave::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SAVEDLG_FILENAME, m_hEditFile);
    DDX_Control(pDX, IDC_SAVEDLG_RDALL, m_hRdAll);
    DDX_Control(pDX, IDC_SAVEDLG_RDSPCF, m_hRdChoose);
    DDX_Control(pDX, IDC_SAVEDLG_RECORDS, m_hEditRecords);
}


/* CDlgSave message handlers */

afx_msg void CDlgSave::OnRdChoosePressed()
{
    if (m_hRdChoose.GetCheck() == BST_CHECKED) {
        m_hEditRecords.EnableWindow(TRUE);
    }
}

afx_msg void CDlgSave::OnRdAllPressed()
{
    if (m_hRdAll.GetCheck() == BST_CHECKED) {
        m_hEditRecords.EnableWindow(FALSE);
    }
}
