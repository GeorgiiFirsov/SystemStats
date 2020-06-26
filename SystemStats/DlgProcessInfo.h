#pragma once

// Project files
#include "stdafx.h"
#include "resource.h"


// Class-wrapper over dialog with process information
class CDlgProcessInfo : public CDialog
{
    DECLARE_DYNAMIC(CDlgProcessInfo)

public:
    CDlgProcessInfo(PROCESSENTRY32 Entry, CWnd* pParent = nullptr)
        : CDialog(IDD_DLGINFO, pParent)
        , m_ProcessInfo(Entry)
    { }

    virtual ~CDlgProcessInfo() = default;

    //
    // Magic for MFC
    // 
    
    enum { IDD = IDD_DLGINFO };

    virtual BOOL OnInitDialog() override;

protected:
    virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

protected:
    // Described process entry
    PROCESSENTRY32 m_ProcessInfo;

    // Name of executable
    CStatic m_Executable;

    // PID text
    CStatic m_Pid;

    // Priority text
    CStatic m_Priority;

    // PPID text
    CStatic m_Ppid;

    // "Access denied" caption in extended information block
    CStatic m_AccessDeniedText;

    // Process creation time title and text
    CStatic m_CreationTitle;
    CStatic m_CreationText;

    // Kernel time title and text
    CStatic m_KernelTitle;
    CStatic m_KernelText;

    // User time title and text
    CStatic m_UserTitle;
    CStatic m_UserText;

    // Protection info title and text
    CStatic m_ProtectionTitle;
    CStatic m_ProtectionText;
};
