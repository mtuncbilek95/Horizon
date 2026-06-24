#include "String.h"

#include <Runtime/Containers/List.h>

#include <format>
#include <algorithm>

namespace Horizon
{
	i32 String::ToInt32(const String& text)
	{
		return std::atoi(*text);
	}

	u32 String::ToUInt32(const String& text)
	{
		return std::strtoul(*text, nullptr, 10);
	}

	i64 String::ToInt64(const String& text)
	{
		return std::atoll(*text);
	}

	u64 String::ToUInt64(const String& text)
	{
		return std::strtoull(*text, nullptr, 10);
	}

	f32 String::ToFloat32(const String& text)
	{
		return std::strtof(*text, nullptr);
	}

	f64 String::ToFloat64(const String& text)
	{
		return std::strtod(*text, nullptr);
	}

	String String::FromInt32(const i32 value)
	{
		return std::format("{}", value).c_str();
	}

	String String::FromUInt32(const u32 value)
	{
		return std::format("{}", value).c_str();
	}

	String String::FromInt64(const i64 value)
	{
		return std::format("{}", value).c_str();
	}

	String String::FromUInt64(const u64 value)
	{
		return std::format("{}", value).c_str();
	}

	String String::FromFloat32(const f32 value)
	{
		return std::format("{}", value).c_str();
	}

	String String::FromFloat64(const f64 value)
	{
		return std::format("{}", value).c_str();
	}

	String::String() : m_data()
	{
	}

	String::String(const c8* pData) : m_data(pData)
	{
	}

	String::String(const c8* pData, u64 sizeInBytes) : m_data(pData, sizeInBytes)
	{
	}

	String::String(const u8* pData, u64 sizeInBytes) : m_data(reinterpret_cast<const c8*>(pData), sizeInBytes)
	{
	}

	String::String(u64 sizeInBytes) : m_data(sizeInBytes, '\0')
	{
	}

	String::String(const String& other) : m_data(other.m_data)
	{
	}

	String::String(String&& other) noexcept : m_data(std::move(other.m_data))
	{
	}

	String::~String()
	{
	}

	b8 String::Contains(const String& str) const
	{
		return m_data.find(str.m_data) != std::string::npos;
	}

	b8 String::Contains(const String& str, b8 caseSensitive) const
	{
		if (caseSensitive)
		{
			return Contains(str);
		}

		std::string lowerData = m_data;
		std::string lowerStr = str.m_data;

		std::transform(lowerData.begin(), lowerData.end(), lowerData.begin(), ::tolower);
		std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);

		return lowerData.find(lowerStr) != std::string::npos;
	}

	b8 String::Has(const c8 c) const
	{
		return m_data.find(c) != std::string::npos;
	}

	b8 String::StartsWith(const String& str) const
	{
		return m_data.starts_with(str.m_data);
	}

	b8 String::StartsWith(c8 c) const
	{
		return m_data.starts_with(c);
	}

	b8 String::EndsWith(const String& str) const
	{
		return m_data.ends_with(str.m_data);
	}

	b8 String::EndsWith(c8 c) const
	{
		return m_data.ends_with(c);
	}

	i64 String::FindIndex(const String& target, u64 startIndex, u64 count) const
	{
		if (startIndex >= GetSize())
			return -1;

		const u64 searchLength = count == 0 ? GetSize() - startIndex : std::min(count, GetSize() - startIndex);
		const std::string_view searchView(m_data.c_str() + startIndex, searchLength);
		const usize foundIndex = searchView.find(target.m_data);

		return foundIndex != std::string_view::npos ? static_cast<i64>(startIndex + foundIndex) : -1;
	}

	i64 String::FindIndex(const c8 target) const
	{
		const usize foundIndex = m_data.find(target);
		return foundIndex != std::string::npos ? static_cast<i64>(foundIndex) : -1;
	}

	i64 String::FindIndex(const String& target) const
	{
		const usize foundIndex = m_data.find(target.m_data);
		return foundIndex != std::string::npos ? static_cast<i64>(foundIndex) : -1;
	}

	i64 String::FindLastIndex(const String& target) const
	{
		const auto pos = m_data.rfind(target.m_data);
		return pos != std::string::npos ? i64(pos) : -1;
	}

	i64 String::FindLastIndex(const c8 target) const
	{
		const auto pos = m_data.rfind(target);
		return pos != std::string::npos ? i64(pos) : -1;
	}

	u64 String::GetCount(const c8 target) const
	{
		return std::count(m_data.begin(), m_data.end(), target);
	}

	String String::GetSubset(u64 startIndex, u64 length) const
	{
		if (startIndex >= GetSize())
			return String();

		const u64 maxLength = GetSize() - startIndex;
		const u64 actualLength = std::min(length, maxLength);

		return String(m_data.c_str() + startIndex, actualLength);
	}

	String String::GetSubset(u64 startIndex) const
	{
		if (startIndex >= GetSize())
			return String();

		return String(m_data.c_str() + startIndex, GetSize() - startIndex);
	}

	String String::GetFiltered(const String& filter) const
	{
		if (filter.IsEmpty())
			return *this;

		String result(*this);
		std::string::size_type pos = 0;
		while ((pos = result.m_data.find(filter.m_data, pos)) != std::string::npos)
		{
			result.m_data.erase(pos, filter.GetSize());
		}
		return result;
	}

	String String::GetFiltered(c8 filter) const
	{
		String result(*this);
		std::erase(result.m_data, filter);
		return result;
	}

	String String::GetReplaced(const String& oldString, const String& newString) const
	{
		if (oldString.IsEmpty())
			return *this;

		String result(*this);
		std::string::size_type pos = 0;

		while ((pos = result.m_data.find(oldString.m_data, pos)) != std::string::npos)
		{
			result.m_data.replace(pos, oldString.GetSize(), newString.m_data);
			pos += newString.GetSize();
		}

		return result;
	}

	String String::RemoveSegments(const String& segment) const
	{
		if (segment.IsEmpty())
			return *this;

		String result(*this);
		std::string::size_type pos = 0;

		while ((pos = result.m_data.find(segment.m_data, pos)) != std::string::npos)
			result.m_data.erase(pos, segment.GetSize());

		return result;
	}

	List<String> String::GetSplit(const String& filter) const
	{
		List<String> result;
		if (filter.IsEmpty())
		{
			result.PushBack(*this);
			return result;
		}
		usize start = 0;
		usize end = m_data.find(filter.m_data);

		while (end != std::string::npos)
		{
			result.PushBack(String(m_data.c_str() + start, end - start));
			start = end + filter.GetSize();
			end = m_data.find(filter.m_data, start);
		}

		result.PushBack(String(m_data.c_str() + start, GetSize() - start));
		return result;
	}

	String String::ToUpper() const
	{
		String result(*this);

		std::transform(result.m_data.begin(), result.m_data.end(), result.m_data.begin(),
			[](unsigned char c) { return std::toupper(c); });
		return result;
	}

	String String::ToLower() const
	{
		String result(*this);

		std::transform(result.m_data.begin(), result.m_data.end(), result.m_data.begin(),
			[](unsigned char c) { return std::tolower(c); });
		return result;
	}

	String operator+(const String& a, const String& b)
	{
		String result(a);
		result += b;
		return result;
	}

	String operator+(const String& a, const c8* b)
	{
		String result(a);
		result += b;
		return result;
	}

	String operator+(const c8* a, const String& b)
	{
		String result(a);
		result += b;
		return result;
	}

	String operator+(const String& a, c8 b)
	{
		String result(a);
		result += b;
		return result;
	}

	String operator+(c8 a, const String& b)
	{
		String result(a);
		result += b;
		return result;
	}

	b8 operator==(const String& a, const String& b)
	{
		return a.m_data == b.m_data;
	}

	b8 operator==(const String& a, const c8* b)
	{
		return a.m_data == b;
	}

	b8 operator!=(const String& a, const c8* b)
	{
		return a.m_data != b;
	}

	b8 operator==(const c8* a, const String& b)
	{
		return a == b.m_data;
	}

	b8 operator!=(const c8* a, const String& b)
	{
		return a != b.m_data;
	}

	b8 operator!=(const String& a, const String& b)
	{
		return a.m_data != b.m_data;
	}

	b8 operator<(const String& a, const String& b)
	{
		return a.m_data < b.m_data;
	}

	b8 operator>(const String& a, const String& b)
	{
		return a.m_data > b.m_data;
	}

	b8 operator<=(const String& a, const String& b)
	{
		return a.m_data <= b.m_data;
	}

	b8 operator>=(const String& a, const String& b)
	{
		return a.m_data >= b.m_data;
	}

	String& String::operator=(const String& other)
	{
		if (this != &other)
			m_data = other.m_data;
		return *this;
	}

	String& String::operator=(String&& other) noexcept
	{
		if (this != &other)
			m_data = std::move(other.m_data);
		return *this;
	}

	String& String::operator=(const c8* other)
	{
		m_data = other ? other : "";
		return *this;
	}

	String& String::operator+=(const String& other)
	{
		m_data += other.m_data;
		return *this;
	}

	String& String::operator+=(const c8* other)
	{
		m_data += other;
		return *this;
	}

	String& String::operator+=(c8 other)
	{
		m_data += other;
		return *this;
	}

	void String::Clear()
	{
		m_data.clear();
	}
}
