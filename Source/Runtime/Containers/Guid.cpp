#include "Guid.h"

#include <Runtime/Log/Terminal.h>

#include <charconv>
#include <cstdio>
#include <cstring>

#if defined(HORIZON_WINDOWS)
#include <objbase.h>
#endif

namespace Horizon
{
	Guid Guid::Generate()
	{
#if defined(HORIZON_WINDOWS)
		GUID native = {};
		const HRESULT hr = CoCreateGuid(&native);

		if (FAILED(hr))
		{
			Terminal::Error("Guid", "CoCreateGuid failed with 0x{:08x}", u32(hr));
			return Guid();
		}

		Guid guid;
		guid.m_a = native.Data1;
		guid.m_b = native.Data2;
		guid.m_c = native.Data3;
		std::memcpy(guid.m_d, native.Data4, sizeof(guid.m_d));

		return guid;
#else
		Terminal::Fatal("Guid", "Guid::Generate has no implementation on this platform");
		return Guid();
#endif
	}

	Guid::Guid() : m_a(0), m_b(0), m_c(0), m_d{}
	{
	}

	Guid::Guid(u32 a, u16 b, u16 c, const u8(&d)[8]) : m_a(a), m_b(b), m_c(c), m_d{}
	{
		std::memcpy(m_d, d, sizeof(m_d));
	}

	Guid::Guid(const std::string& str) : m_a(0), m_b(0), m_c(0), m_d{}
	{
		if (str.size() != 36 || str[8] != '-' || str[13] != '-' || str[18] != '-' || str[23] != '-')
		{
			Terminal::Warn("Guid", "Malformed guid string '{}'", str);
			return;
		}

		auto parseHex = [&](usize offset, usize length, auto& out) -> b8
			{
				const c8* begin = str.data() + offset;
				const c8* end = begin + length;

				const auto result = std::from_chars(begin, end, out, 16);
				return result.ec == std::errc{} && result.ptr == end;
			};

		u32 a = 0;
		u16 b = 0;
		u16 c = 0;
		u8 d[8] = {};

		b8 ok = parseHex(0, 8, a) && parseHex(9, 4, b) && parseHex(14, 4, c);

		if (ok)
		{
			usize cursor = 19;

			for (usize i = 0; i < 8 && ok; ++i)
			{
				ok = parseHex(cursor, 2, d[i]);
				cursor += 2;

				if (i == 1)
					cursor++;
			}
		}

		if (!ok)
		{
			Terminal::Warn("Guid", "Malformed guid string '{}'", str);
			return;
		}

		m_a = a;
		m_b = b;
		m_c = c;
		std::memcpy(m_d, d, sizeof(m_d));
	}

	b8 Guid::IsValid() const
	{
		if (m_a != 0 || m_b != 0 || m_c != 0)
			return true;

		for (usize i = 0; i < sizeof(m_d); ++i)
		{
			if (m_d[i] != 0)
				return true;
		}

		return false;
	}

	std::string Guid::ToString() const
	{
		c8 buffer[40];

		std::snprintf(buffer, sizeof(buffer), "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			m_a, m_b, m_c,
			m_d[0], m_d[1], m_d[2], m_d[3], m_d[4], m_d[5], m_d[6], m_d[7]);

		return buffer;
	}

	b8 Guid::operator==(const Guid& other) const
	{
		return m_a == other.m_a && m_b == other.m_b && m_c == other.m_c
			&& std::memcmp(m_d, other.m_d, sizeof(m_d)) == 0;
	}

	b8 Guid::operator!=(const Guid& other) const
	{
		return !(*this == other);
	}
}