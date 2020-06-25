#pragma once

// Project headers
#include "stdafx.h"
#include "Utils.h"

// STL headers
#include <stdexcept>
#include <sstream>
#include <type_traits>
#include <iomanip>


// Macro used for throwing common types of exceptions
#define ERROR_THROW_LAST(...) system_stats::exception::CWin32Error(GetLastError(), __FILEW__, __LINE__, __VA_ARGS__)
#define ERROR_THROW_CODE(_Code, ...) system_stats::exception::CWin32Error((_Code), __FILEW__, __LINE__, __VA_ARGS__)


namespace system_stats
{
namespace exception
{
namespace
{
    void PutIntoStream(std::wstringstream&) 
    { /* End of template recursion */ }

    template<typename First, typename... Rest> 
    void PutIntoStream(std::wstringstream& stream, First&& first, Rest&&... rest)
    {
        std::stringstream::fmtflags flags = stream.flags();

        static_assert(
            _HAS_CXX17,
            "Here is a place with C++17 feature. If you want to "
            "compile this code with older language standard you "
            "have to remove constexpr. Maybe it is necessary to "
            "suppress warning about constant condition "
        );

        if constexpr (std::is_same<First, DWORD>::value) {
            stream << std::hex << std::setw(8) << std::setfill(L'0');
        }

        stream << first << L' ';
        stream.flags(flags);

        PutIntoStream(stream, std::forward<Rest>(rest)...);
    }
}

    class CWin32Error
    {
    public:
        template<typename... Args>
        CWin32Error(DWORD dwErrorCode, LPCWSTR szFile, DWORD dwLine, Args&&... args)
            : m_wsDescription({})
            , m_dwErrorCode(dwErrorCode)
        {
            std::wstringstream stream;
            stream << L"Error code: " << m_dwErrorCode << L'\n'
                   << L"File: " << szFile << L'\n'
                   << L"Line: " << dwLine << L'\n';

            if constexpr (sizeof...(Args)) {
                stream << L"Arguments: ";
                PutIntoStream(stream, std::forward<Args>(args)...);
                stream << L'\n';
            }

            m_wsDescription = std::move(stream.str());
        }

        LPCWSTR Description() const { return m_wsDescription.c_str(); }
        DWORD Code() const noexcept { return m_dwErrorCode; }

    private:
        std::wstring m_wsDescription;
        DWORD        m_dwErrorCode;
    };

    // Function that displays a message about error
    inline void DisplayErrorMessage(const CWin32Error& error, HWND hWnd = nullptr) noexcept
    {
        ::OutputDebugString(error.Description());
        ::MessageBox(hWnd, error.Description(), szApplicationName, MB_ICONERROR);
    }

} // namespace exception
} // namespace system_stats