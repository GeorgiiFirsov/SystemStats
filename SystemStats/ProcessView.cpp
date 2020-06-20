// Project headers
#include "stdafx.h"
#include "Exception.h"
#include "ProcessesView.h"

// STL headers
#include <cmath>

// Defined in SystemStats.cpp
extern HINSTANCE g_hInstance;

namespace system_stats
{

    /* Message handlers registration */

    BEGIN_MESSAGE_MAP(CProcessesView, CListCtrl)
        ON_WM_LBUTTONDBLCLK()
    END_MESSAGE_MAP()


    _On_failure_(return == FALSE) 
    BOOL CProcessesView::DeleteAllItems()
    {
        BOOL bResult = CListCtrl::DeleteAllItems();
        if (bResult) {
            m_Items.clear();
        }

        return bResult;
    }
    
    void CProcessesView::InsertColumns(_In_ const std::vector<LPCWSTR>& columns)
    {
        RECT Rect;
        GetClientRect(&Rect);

        auto cColumns = static_cast<int>(columns.size());
        auto ullWidth = static_cast<size_t>(
            std::floor(Rect.right - Rect.left) / columns.size()
        ) - 4; // Magic number

        for (int index = 0; index < cColumns; index++) {
            CListCtrl::InsertColumn(index, columns[index], LVCFMT_LEFT, static_cast<int>(ullWidth));
        }
    }

    _On_failure_(return == -1)
    int CProcessesView::InsertItem(_In_ const PROCESSENTRY32& Info)
    {
        int iResult = CListCtrl::InsertItem((int)m_Items.size(), Info.szExeFile);

        if (iResult != -1)
        {
            // PID
            CListCtrl::SetItemText((int)m_Items.size(), 1, std::to_wstring(Info.th32ProcessID).c_str());

            // PPID
            CListCtrl::SetItemText((int)m_Items.size(), 2, std::to_wstring(Info.th32ParentProcessID).c_str());

            // Thread count
            CListCtrl::SetItemText((int)m_Items.size(), 3, std::to_wstring(Info.cntThreads).c_str());

            m_Items.push_back(Info);
        }

        return iResult;
    }

    _On_failure_(return == ERROR_SUCCESS)
    DWORD CProcessesView::GetNthProcess(_In_ size_t nIndex, _Out_ PROCESSENTRY32& Entry)
    {
        if (nIndex >= m_Items.size()) {
            return ERROR_NOT_FOUND;
        }

        Entry = m_Items[nIndex];

        return ERROR_SUCCESS;
    }


    /* CProcessesView message handlers */

    afx_msg void CProcessesView::OnLButtonDblClk(UINT nFlags, CPoint point)
    {
        CListCtrl::OnLButtonDblClk(nFlags, point);

        int nIndex = GetNextItem(-1, LVNI_SELECTED);
        if (nIndex != -1) {
            ::SendMessage(GetParent()->m_hWnd, WM_VIEWITEMDBLCLICK, nIndex, 0);
        }
    }

} // namespace system_stats