#pragma once

// Project headers
#include "stdafx.h"
#include "Exception.h"

// STL headers
#include <map>
#include <string>


namespace system_stats {
namespace i18n {

	// Cache for resource strings to load
	// each one only once
	class CResourceCache
	{
		using mapping_t = std::map<UINT, std::wstring>;

	public:
		LPCWSTR Get(_In_ UINT uId);

	private:
		_Check_return_
		DWORD Load(_In_ UINT uId, _Inout_ mapping_t::iterator& Position);

	private:
		mapping_t m_Storage;
	};


	// Function to retrieve a resource string
	// from current module
	LPCWSTR LoadUIString(_In_ UINT uId);

} // namespace i18n
} // namespace system_stats
