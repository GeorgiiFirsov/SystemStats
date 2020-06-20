// Project files
#include "stdafx.h"
#include "DlgSave.h"
#include "Exception.h"
#include "afxdialogex.h"

// STL headers
#include <string>
#include <regex>


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

    m_hEditFile.SetWindowText(L"SystemStats.txt");
    m_hEditRecords.SetWindowText(L"1-100");

    m_hRdAll.SetCheck(BST_CHECKED);

    return TRUE;
}

void CDlgSave::DDV_GoodRange(CDataExchange* pDX)
{
    namespace exc = system_stats::exception;

    if (!pDX->m_bSaveAndValidate) {
        return;
    }

    try
    {		
        WCHAR buffer[MAX_PATH + 1] = { 0 };
        int iRead = m_hEditRecords.GetLine(0, buffer, _countof(buffer) - 1);

        //
        // We need to check input string. First chech for
        // emptiness, two consequential commas and leading comma
        // (assume trailing comma to be OK).
        // 

        CString sText(buffer);
        sText.Trim();

        if (sText.IsEmpty() || sText[0] == L',' || sText.Find(L",,") != -1) {
            exc::ThrowRuntimeError("Invalid range");
        }

        //
        // String passed simple checks, now we need to tokenize
        // it (token is a sequence between two commas) and ensure
        // that all tokens represent a number or a range.
        // 
        // TODO: check range validity
        // 

        std::wregex SubrangeRegex(LR"(^([1-9]\d*|[1-9]\d*-[1-9]\d*)$)");

        int iTokenBegin = 0;
        for (CString sToken = ""; !sToken.IsEmpty() || iTokenBegin != -1; sToken = sText.Tokenize(L",", iTokenBegin))
        {
            if (iTokenBegin == 0) {
                continue;
            }

            sToken.Replace(L" ", L"");

            if (sText.IsEmpty() || !regex_match(std::wstring(sToken), SubrangeRegex)) {
                exc::ThrowRuntimeError("Invalid range");
            }
        }
    }
    catch(const std::runtime_error& error)
    {
        exc::DisplayErrorMessage(error, m_hWnd);
        AfxThrowUserException();
    }
}

void CDlgSave::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);

    DDV_GoodRange(pDX);

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