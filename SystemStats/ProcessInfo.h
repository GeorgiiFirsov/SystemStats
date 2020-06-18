#pragma once

// Project headers
#include "stdafx.h"
#include "Utils.h"

// STL headers
#include <string>
#include <vector>

namespace system_stats
{

    class CProcessInfo
    {
    public:
        CProcessInfo(_In_ LPCWSTR pszExecutable, _In_ DWORD dwPID, _In_ DWORD dwPPID, _In_ DWORD cThreads)
            : m_wsExecutable(pszExecutable)
            , m_dwPID(dwPID)
            , m_dwPPID(dwPPID)
            , m_cThreads(cThreads)
        { }

    public:
        std::wstring m_wsExecutable;
        DWORD        m_dwPID;
        DWORD        m_dwPPID;
        DWORD        m_cThreads;
    };

} // namespace system_stats
