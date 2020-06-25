#pragma once

// Project headers
#include "stdafx.h"

// STL headers
#include <tuple>
#include <vector>


namespace utils
{
    // Function that checks capacity of a vector and extends it if necessary
    template<typename _Ty>
    size_t CheckCapacity(std::vector<_Ty>& vct)
    {
        if (vct.capacity() <= vct.size()) {
            vct.reserve(vct.size() * 2);
        }

        return vct.capacity();
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