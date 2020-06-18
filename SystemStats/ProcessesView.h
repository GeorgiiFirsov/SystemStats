#pragma once

// Project headers
#include "stdafx.h"
#include "ProcessInfo.h"

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
        int InsertItem(_In_ const CProcessInfo& Info);

    private:
        size_t m_cItems = 0;
    };

} // namespace system_stats
