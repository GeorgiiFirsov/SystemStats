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

    _On_failure_(return == FALSE) 
    BOOL CProcessesView::DeleteAllItems()
    {
        BOOL bResult = CListCtrl::DeleteAllItems();
        if (bResult) {
            m_cItems = 0;
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
    int CProcessesView::InsertItem(_In_ const CProcessInfo& Info)
    {
        int iResult = CListCtrl::InsertItem((int)m_cItems, Info.m_wsExecutable.c_str());

        if (iResult != -1)
        {
            // PID
            CListCtrl::SetItemText((int)m_cItems, 1, std::to_wstring(Info.m_dwPID).c_str());

            // PPID
            CListCtrl::SetItemText((int)m_cItems, 2, std::to_wstring(Info.m_dwPPID).c_str());

            // Thread count
            CListCtrl::SetItemText((int)m_cItems, 3, std::to_wstring(Info.m_cThreads).c_str());

            m_cItems++;
        }

        return iResult;
    }

} // namespace system_stats