#pragma once

// Project headers
#include "stdafx.h"
#include "i18n.h"
#include "resource.h"

// STL headers
#include <tuple>
#include <vector>


namespace utils
{
    // Function that checks capacity of a vector and extends it if necessary
    template<typename _Ty>
    size_t CheckCapacity(_Inout_ std::vector<_Ty>& vct)
    {
        if (vct.capacity() <= vct.size()) {
            vct.reserve(vct.size() * 2);
        }

        return vct.capacity();
    }


namespace {
    template<typename... Words>
    inline CString FormatSystemTime(_In_ UINT uFmtId, _In_ Words... words)
    {
        CString sTime;
        sTime.Format(
            system_stats::i18n::LoadUIString(uFmtId), words...
        );
        return sTime;
    }
}


    // Function that formats FILETIME in YYYY-MM-DDThh:mm:ss.SSS format
    inline CString FileTimeAsDatetimeString(_In_ const FILETIME* lpFileTime)
    {
        SYSTEMTIME stTime;

        BOOL bResult = ::FileTimeToSystemTime(lpFileTime, &stTime);
        if (!bResult) {
            return {};
        }

        return FormatSystemTime(
            IDS_FMT_DATETIME, stTime.wYear, stTime.wMonth, stTime.wDay,
            stTime.wHour, stTime.wMinute, stTime.wSecond, stTime.wMilliseconds
        );
    }


    // Function that formats FILETIME in hh:mm:ss format
    inline CString FileTimeAsTimeString(const FILETIME* lpFileTime)
    {
        SYSTEMTIME stTime;

        BOOL bResult = ::FileTimeToSystemTime(lpFileTime, &stTime);
        if (!bResult) {
            return {};
        }

        return FormatSystemTime(
            IDS_FMT_TIME, stTime.wHour, stTime.wMinute, stTime.wSecond
        );
    }


    // Class for setting wait cursor in constructor
    // and restoring previous state in destructor
    class CWaitCursor
    {
    public:
        CWaitCursor()
            : m_hPrev(::GetCursor())
        {
            ::SetCursor(::LoadCursor(nullptr, IDC_WAIT));
        }

        ~CWaitCursor()
        {
            if (m_hPrev) {
                ::SetCursor(m_hPrev);
            }
        }

    private:
        HCURSOR m_hPrev;
    };

} // namespace utils