#pragma once

// Project files
#include "stdafx.h"
#include "resource.h"

// STL headers
#include <set>
#include <string>


// Class-wrapper over dialog shown when user
// clicks on "Dump" button. It asks user about
// file name and records count he or she wants
// to save.
class CDlgSave : public CDialog
{
    DECLARE_DYNAMIC(CDlgSave)

public:
    CDlgSave(CWnd* pParent = nullptr)
        : CDialog(IDD_SAVEDLG, pParent)
    { }

    virtual ~CDlgSave() = default;

    //
    // Magic for MFC
    // 

    enum { IDD = IDD_SAVEDLG };

    virtual BOOL OnInitDialog() override;

    std::set<size_t> GetSavingRange() { return m_Indexes; }
    std::wstring GetFileName() { return m_wsFileName; }

protected:
    virtual void DoDataExchange(CDataExchange* pDX) override;

    void DDV_GoodRange(CDataExchange* pDX);

    //
    // Message handlers
    // 

    DECLARE_MESSAGE_MAP()

    afx_msg void OnRdChoosePressed();
    afx_msg void OnRdAllPressed();
    afx_msg void OnOK() override;

protected:
    // Edit control with file name
    CEdit   m_hEditFile;

    // Edit control with specified records to save
    CEdit   m_hEditRecords;

    // Radio button corresponds to saving of all records
    CButton m_hRdAll;

    // Radio button that enables edit control with 
    // request to specify which records to save
    CButton m_hRdChoose;

    // Storage for zero-based indexes, set by user. 
    // Empty if all items need to be saved
    std::set<size_t> m_Indexes;

    // File name that is used to store saved data
    std::wstring     m_wsFileName;
};