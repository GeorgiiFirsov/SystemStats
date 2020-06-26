// Project files
#include "stdafx.h"
#include "DlgProcessInfo.h"
#include "resource.h"
#include "i18n.h"
#include "Utils.h"

// STL headers
#include <string>
#include <map>


// Declaration of process protection values mapping macro
#define PROCESS_PROTECTION_MAP_BEGIN(_Name) const std::map<DWORD, std::wstring> _Name {
#define PROCESS_PROTECTION_MAP_END }
#define PROCESS_PROTECTION_ENTRY(_Value) { _Value, _T(#_Value) }


// Process protection values
PROCESS_PROTECTION_MAP_BEGIN(ProcessProtectionMap)
    PROCESS_PROTECTION_ENTRY(PROTECTION_LEVEL_WINTCB_LIGHT),
    PROCESS_PROTECTION_ENTRY(PROTECTION_LEVEL_WINDOWS),
    PROCESS_PROTECTION_ENTRY(PROTECTION_LEVEL_WINDOWS_LIGHT),
    PROCESS_PROTECTION_ENTRY(PROTECTION_LEVEL_ANTIMALWARE_LIGHT),
    PROCESS_PROTECTION_ENTRY(PROTECTION_LEVEL_LSA_LIGHT),
    PROCESS_PROTECTION_ENTRY(PROTECTION_LEVEL_WINTCB),
    PROCESS_PROTECTION_ENTRY(PROTECTION_LEVEL_CODEGEN_LIGHT),
    PROCESS_PROTECTION_ENTRY(PROTECTION_LEVEL_AUTHENTICODE),
    PROCESS_PROTECTION_ENTRY(PROTECTION_LEVEL_PPL_APP),
    PROCESS_PROTECTION_ENTRY(PROTECTION_LEVEL_NONE)
PROCESS_PROTECTION_MAP_END;


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

		m_CreationTitle.ShowWindow(SW_HIDE);
		m_CreationText.ShowWindow(SW_HIDE);
		m_KernelTitle.ShowWindow(SW_HIDE);
		m_KernelText.ShowWindow(SW_HIDE);
		m_UserTitle.ShowWindow(SW_HIDE);
		m_UserText.ShowWindow(SW_HIDE);
		m_ProtectionTitle.ShowWindow(SW_HIDE);
		m_ProtectionText.ShowWindow(SW_HIDE);

        return TRUE;
    }

    //
    // Information can be collected, so I hide "Access denied" text
    // and start quering information.
    // 

    m_AccessDeniedText.ShowWindow(SW_HIDE);

    //
    // Process times
    // 

    FILETIME ftCreationTime, ftKernelTime, ftUserTime, ftUnused;
    BOOL bResult = ::GetProcessTimes(
        hProcess, 
        &ftCreationTime, 
        &ftUnused, 
        &ftKernelTime, 
        &ftUserTime
    );

    if (bResult)
    {
        FILETIME   ftLocalTime;

        ::FileTimeToLocalFileTime(&ftCreationTime, &ftLocalTime);
        CString sTime = utils::FileTimeAsDatetimeString(&ftLocalTime);
        if (!sTime.IsEmpty()) {
            m_CreationText.SetWindowText(sTime);
        }

        ::FileTimeToLocalFileTime(&ftKernelTime, &ftLocalTime);
        sTime = utils::FileTimeAsTimeString(&ftLocalTime);
        if (!sTime.IsEmpty()) {
            m_KernelText.SetWindowText(sTime);
        }

        ::FileTimeToLocalFileTime(&ftUserTime, &ftLocalTime);
        sTime = utils::FileTimeAsTimeString(&ftLocalTime);
        if (!sTime.IsEmpty()) {
            m_UserText.SetWindowText(sTime);
        }
    }

    //
    // Process protection info
    // 

    PROCESS_PROTECTION_LEVEL_INFORMATION ProtectionInfo = { 0 };
    bResult = ::GetProcessInformation(
        hProcess, 
        ProcessProtectionLevelInfo,
        &ProtectionInfo,
        sizeof(ProtectionInfo)
    );

    if (bResult) {
        m_ProtectionText.SetWindowText(
            ProcessProtectionMap.at(ProtectionInfo.ProtectionLevel).c_str()
        );
    }


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
    DDX_Control(pDX, IDC_DLGINFO_CREATIONTITLE, m_CreationTitle);
    DDX_Control(pDX, IDC_DLGINFO_CREATIONTEXT, m_CreationText);
    DDX_Control(pDX, IDC_DLGINFO_KERNELTITLE, m_KernelTitle);
    DDX_Control(pDX, IDC_DLGINFO_KERNELTEXT, m_KernelText);
    DDX_Control(pDX, IDC_DLGINFO_USERTITLE, m_UserTitle);
    DDX_Control(pDX, IDC_DLGINFO_USERTEXT, m_UserText);
    DDX_Control(pDX, IDC_DLGINFO_PROTECTIONTITLE, m_ProtectionTitle);
    DDX_Control(pDX, IDC_DLGINFO_PROTECTIONTEXT, m_ProtectionText);
}


/* CDlgProcessInfo message handlers */
