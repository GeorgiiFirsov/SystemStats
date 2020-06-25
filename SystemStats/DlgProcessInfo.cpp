// Project files
#include "stdafx.h"
#include "DlgProcessInfo.h"
#include "resource.h"
#include "i18n.h"
#include "Utils.h"

// STL headers
#include <string>


IMPLEMENT_DYNAMIC(CDlgProcessInfo, CDialog)


/* Message handlers registration */

BEGIN_MESSAGE_MAP(CDlgProcessInfo, CDialog)
END_MESSAGE_MAP()


BOOL CDlgProcessInfo::OnInitDialog()
{
	utils::CWaitCursor wc;

    CDialog::OnInitDialog();

    m_Executable.SetWindowText(m_ProcessInfo.szExeFile);
    m_Pid.SetWindowText(std::to_wstring(m_ProcessInfo.th32ProcessID).c_str());
    m_Priority.SetWindowText(std::to_wstring(m_ProcessInfo.pcPriClassBase).c_str());
    m_Ppid.SetWindowText(std::to_wstring(m_ProcessInfo.th32ParentProcessID).c_str());

    //
    // Ok, main info extracted. Now we need to query some more
    // information about process.
    // 

    DWORD dwProcessID = m_ProcessInfo.th32ProcessID;

    ATL::CHandle hProcess;
    hProcess.m_h = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwProcessID);

    if (!hProcess) 
    {
        //
        // Well... Something bad happened. Ususally it is access denial
        // 

        CString sMessage;
        sMessage.Format(
            system_stats::i18n::LoadUIString(IDS_ERROR_PROCESSACCESS),
            GetLastError()
        );

        ::MessageBox(m_hWnd, sMessage, szApplicationName, MB_ICONWARNING);

        return TRUE;
    }

    //
    // Information can be collected, so I hide "Access denied" text
    // and start quering information.
    // 

    m_AccessDeniedText.ShowWindow(SW_HIDE);

    // TODO: get process information

    return TRUE;
}

void CDlgProcessInfo::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_DLGINFO_EXETEXT, m_Executable);
    DDX_Control(pDX, IDC_DLGINFO_PIDTEXT, m_Pid);
    DDX_Control(pDX, IDC_DLGINFO_PRTEXT, m_Priority);
    DDX_Control(pDX, IDC_DLGINFO_PPIDTEXT, m_Ppid);
    DDX_Control(pDX, IDC_DLGINFO_ACCESSTEXT, m_AccessDeniedText);
}


/* CDlgProcessInfo message handlers */
