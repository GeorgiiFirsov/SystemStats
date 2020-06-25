// Project headers
#include "stdafx.h"
#include "i18n.h"

extern HINSTANCE g_hInstance;

namespace system_stats {
namespace i18n {

    LPCWSTR CResourceCache::Get(_In_ UINT uId)
    {
        auto ResIterator = m_Storage.find(uId);
        if (ResIterator != m_Storage.end()) {
            return ResIterator->second.c_str();
        }

        DWORD dwResult = Load(uId, ResIterator);

        if (dwResult != ERROR_SUCCESS) {
            ERROR_THROW_CODE(dwResult);
        }
        
        return ResIterator->second.c_str();
    }

    _Check_return_
    DWORD CResourceCache::Load(_In_ UINT uId, _Inout_ mapping_t::iterator& Position)
    {
        std::vector<WCHAR> buffer(MAX_PATH + 1, 0); // Ensure last symbol to be zero

        int cchBuffer = ::LoadString(
            g_hInstance,
            uId,
            buffer.data(),
            MAX_PATH
        );

        if (cchBuffer == 0) {
            return GetLastError();
        }

        Position = m_Storage.emplace_hint(Position, uId, std::wstring(buffer.data(), cchBuffer));

        return ERROR_SUCCESS;
    }


    /* Functions */

    LPCWSTR LoadUIString(_In_ UINT uId)
    {
        namespace exc = system_stats::exception;

        static CResourceCache cache;
        try
        {
            return cache.Get(uId);
        }
        catch (const exc::CWin32Error& error)
        {
            exc::DisplayErrorMessage(error);
            return nullptr;
        }
    }

} // namespace i18n
} // namespace system_stats