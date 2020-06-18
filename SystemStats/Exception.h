#pragma once

// Project headers
#include "stdafx.h"
#include "Utils.h"

// STL headers
#include <stdexcept>
#include <sstream>


// Macro used for throwing common types of exceptions
#define ERROR_THROW_LAST() system_stats::exception::ThrowRuntimeError("code == ", GetLastError())
#define ERROR_THROW_CODE(_Code) system_stats::exception::ThrowRuntimeError("code == ", (_Code))


namespace system_stats
{
namespace exception
{
namespace
{
    inline void PutIntoStream(std::stringstream&) 
    { /* End of template recursion */ }

    template<typename First, typename... Rest>
    void PutIntoStream(std::stringstream& stream, First&& first, Rest&&... rest)
    {
        stream << first << " ";
        PutIntoStream(stream, std::forward<Rest>(rest)...);
    }
}

    // Composes string with arguments and throws an exception
    // with this string as description
    template<typename... Args>
    [[noreturn]] void ThrowRuntimeError(Args&&... args)
    {
        std::stringstream stream;
        
        stream << "An error occurred.\nMessage: ";
        PutIntoStream(stream, std::forward<Args>(args)...);
        stream << "\n";

        throw std::runtime_error(stream.str());
    }

    // Function that displays a message about error
    inline void DisplayErrorMessage(const std::runtime_error& error) noexcept
    {
        ::OutputDebugStringA(error.what());
        ::MessageBoxA(nullptr, error.what(), "Error occurred", MB_ICONERROR);
    }

} // namespace exception
} // namespace system_stats