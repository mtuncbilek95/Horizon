#include "Guid.h"

#include <Runtime/Log/Terminal.h>

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
		if (str.size() != 36 || str[8] != '-' || str[13] != '-' || str[18] != '-' || str[23] != '-')
		{
			Terminal::Warn("Guid", "Malformed guid string '{}'", str);
			return;
		}

		auto parseHex = [&](usize offset, usize len, auto& out) -> b8
			{
				const c8* begin = str.data() + offset;
				const c8* end = begin + len;

				auto res = std::from_chars(begin, end, out, 16);
				return res.ec == std::errc{} && res.ptr == end;
			};

		u16 low16 = 0;
		u64 high48 = 0;

		b8 ok = parseHex(0, 8, m_a)
			&& parseHex(9, 4, m_b)
			&& parseHex(14, 4, m_c)
			&& parseHex(19, 4, low16)
			&& parseHex(24, 12, high48);

		if (!ok)
		{
			Terminal::Warn("Guid", "Malformed guid string '{}'", str);
			m_a = 0; m_b = 0; m_c = 0; m_d = 0;
			return;
		}

		m_d = (high48 << 16) | u64(low16);
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