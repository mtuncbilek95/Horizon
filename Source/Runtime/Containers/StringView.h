#pragma once

#include <Runtime/Containers/String.h>

#include <format>

namespace Horizon
{
	class H_EXPORT StringView
	{
	public:
		static b8 IsSameMemory(const StringView& a, const StringView& b);

	public:
		StringView();
		StringView(const c8* str);
		StringView(const c8* str, u64 length);
		StringView(const StringView& other);
		StringView(const String& str);
		~StringView();

		StringView& operator=(const c8* str);
		StringView& operator=(const String& str);
		StringView& operator=(const StringView& other);

		b8 operator==(const String& other) const;
		b8 operator!=(const String& other) const;
		b8 operator==(const StringView& other) const;
		b8 operator!=(const StringView& other) const;

		const c8* GetSource() const;
		u64 GetSize() const;

		const c8* begin() const noexcept { return m_source; }
		const c8* cbegin() const noexcept { return m_source; }
		const c8* end() const noexcept { return m_source + m_size; }
		const c8* cend() const noexcept { return m_source + m_size; }

	private:
		const c8* m_source;
		u64 m_size;
	};
}

template <>
struct std::formatter<Horizon::StringView> : std::formatter<std::string_view>
{
	auto format(const Horizon::StringView& str, std::format_context& ctx) const
	{
		return std::formatter<std::string_view>::format(
			std::string_view(str.GetSource(), str.GetSize()), ctx);
	}
};