#pragma once

// Project headers
#include "stdafx.h"

// STL headers
#include <vector>
#include <string>

namespace system_stats
{
    
    // Class-wrapper over List View that extends a bit
    // functionality of CListCtrl class from MFC
    class CProcessesView : public CListCtrl
    {
    public:

        _On_failure_(return == FALSE) 
        BOOL DeleteAllItems();

        void InsertColumns(_In_ const std::vector<LPCWSTR>& columns);

        _On_failure_(return == -1)
        int InsertItem(_In_ const PROCESSENTRY32& Info);

        _On_failure_(return == ERROR_SUCCESS)
        DWORD GetNthProcess(_In_ size_t nIndex, _Out_ PROCESSENTRY32& Entry);

    protected:
        DECLARE_MESSAGE_MAP()

        afx_msg void OnLButtonDblClk(_In_ UINT nFlags, _In_ CPoint point);

    private:
        std::vector<PROCESSENTRY32> m_Items;
    };

} // namespace system_stats
