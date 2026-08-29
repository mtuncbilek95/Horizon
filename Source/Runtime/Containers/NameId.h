#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <string>
#include <string_view>
#include <xhash>

namespace Horizon
{
	class H_EXPORT NameId final
	{
	public:
		NameId() = default;
		explicit NameId(std::string_view text);

		b8 IsValid() const { return m_hash != 0; }
		u64 GetHash() const { return m_hash; }

		std::string_view ToString() const;

		b8 operator==(const NameId& other) const { return m_hash == other.m_hash; }
		b8 operator!=(const NameId& other) const { return m_hash != other.m_hash; }

	private:
		u64 m_hash = 0;
	};
}

namespace std
{
	template<>
	struct hash<Horizon::NameId>
	{
		size_t operator()(const Horizon::NameId& value) const noexcept
		{
			return static_cast<size_t>(value.GetHash());
		}
	};
}