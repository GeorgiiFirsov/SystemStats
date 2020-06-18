#pragma once

// Project headers
#include "stdafx.h"

// STL headers
#include <tuple>
#include <vector>


namespace utils
{

    // Class-wrapper over CallOnExit result, that represents a guard
    // for any opertion that MUST be executed before exit out of a scope
    // It is an analogue to __finally block, but it seems to be more
    // flexible - it supports any callable.
    template<typename _Fn, typename... _Args>
    class _CallOnExit_Wrapper
    {
    private:
        using args_tuple_t = std::tuple<_Args&...>;

    public:
        _CallOnExit_Wrapper(_In_ _Fn func, _In_opt_ _Args&&... args)
            : m_func(func)
            , m_args(std::forward_as_tuple(args...))
        { }
        
        ~_CallOnExit_Wrapper()
        {
            // Destructor must be exception-safe
            try {
                static_assert(
                    _HAS_CXX17, 
                    "Here is a place with C++17 feature. If you want to "
                    "compile this code with older language standard you "
                    "have to write own std::apply. It is not quite difficult :) "
                );

                std::apply(m_func, m_args);
            }
            catch(...) {
                // Ignore for now
            }
        }

        // This class must be non-copyable and non-movable
        _CallOnExit_Wrapper() = delete;
        _CallOnExit_Wrapper(const _CallOnExit_Wrapper&) = delete;
        _CallOnExit_Wrapper& operator=(const _CallOnExit_Wrapper&) = delete;
        _CallOnExit_Wrapper(_CallOnExit_Wrapper&&) = delete;
        _CallOnExit_Wrapper& operator=(_CallOnExit_Wrapper&&) = delete;

    private:
        _Fn          m_func;
        args_tuple_t m_args;
    };


    // Function template that creates a wrapper over an operation, that
    // must be executed right before exitin out of a scope
    template<typename _Fn, typename... _Args>
    _CallOnExit_Wrapper<_Fn, _Args...> CallOnExit(_In_ _Fn func, _In_opt_ _Args&&... args)
    {
        return _CallOnExit_Wrapper<_Fn, _Args...>(
            func, std::forward<_Args>(args)...
        );
    }


    // Function that checks capacity of a vector and extends it if necessary
    template<typename _Ty>
    size_t CheckCapacity(std::vector<_Ty>& vct)
    {
        if (vct.capacity() <= vct.size()) {
            vct.reserve(vct.size() * 2);
        }

        return vct.capacity();
    }

} // namespace utils