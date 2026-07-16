#include "Guid.h"

#include <format>

#if defined(HORIZON_WINDOWS)
#include <objbase.h>
#endif

namespace Horizon
{
	Guid Guid::Generate()
	{
#if defined(HORIZON_WINDOWS)
		Guid guid = {};
		HRESULT hr = CoCreateGuid((GUID*)(&guid.m_a));
		return guid;
#else
#endif
	}

	Guid::Guid() : m_a(0), m_b(0), m_c(0), m_d(0)
	{
	}

	Guid::Guid(u32 a, u16 b, u16 c, u64 d) : m_a(a), m_b(b), m_c(c), m_d(d)
	{
	}

	Guid::Guid(const std::string& str) : m_a(0), m_b(0), m_c(0), m_d(0)
	{
		std::from_chars_result result = std::from_chars(str.data(), str.size() + str.data(), m_a, 16);
	}

	std::string Guid::ToString() const
	{
		char buffer[64];
		std::snprintf(buffer, sizeof(buffer), "%08x-%04x-%04x-%04x-%012llx", m_a, m_b, m_c, (u16)m_d, m_d >> 16);
		return buffer;
	}

	b8 Guid::operator==(const Guid& other) const
	{
		return other.m_a == m_a && other.m_b == m_b && other.m_c == m_c && other.m_d == m_d;
	}

	b8 Guid::operator!=(const Guid& other) const
	{
		return !(other == *this);
	}
}