#include "StringView.h"

namespace Horizon
{
	b8 StringView::IsSameMemory(const StringView& a, const StringView& b)
	{
		return a.GetSource() == b.GetSource() && a.GetSize() == b.GetSize();
	}

	StringView::StringView() : m_source(nullptr), m_size(0)
	{
	}

	StringView::StringView(const c8* str) : m_source(str), m_size(strlen(str))
	{
	}

	StringView::StringView(const c8* str, u64 length) : m_source(str), m_size(length)
	{
	}

	StringView::StringView(const StringView& other) : m_source(other.m_source), m_size(other.m_size)
	{
	}

	StringView::StringView(const String& str) : m_source(*str), m_size(str.GetSize())
	{
	}

	StringView::~StringView()
	{
	}

	StringView& StringView::operator=(const c8* str)
	{
		m_source = str;
		m_size = strlen(str);
		return *this;
	}

	StringView& StringView::operator=(const String& str)
	{
		m_source = *str;
		m_size = str.GetSize();
		return *this;
	}

	StringView& StringView::operator=(const StringView& other)
	{
		m_source = other.m_source;
		m_size = other.m_size;
		return *this;
	}

	b8 StringView::operator==(const StringView& other) const
	{
		u64 target0Length = GetSize();
		u64 target1Length = other.GetSize();
		const c8* target0Source = GetSource();
		const c8* target1Source = other.GetSource();

		if (target0Length != target1Length)
			return false;

		return std::memcmp(target0Source, target1Source, target0Length) == 0;
	}

	b8 StringView::operator!=(const StringView& other) const
	{
		return !(*this == other);
	}

	b8 StringView::operator==(const String& other) const
	{
		u64 target0Length = GetSize();
		u64 target1Length = other.GetSize();
		const c8* target0Source = GetSource();
		const c8* target1Source = *other;

		if (target0Length != target1Length)
			return false;

		return std::memcmp(target0Source, target1Source, target0Length) == 0;
	}

	b8 StringView::operator!=(const String& other) const
	{
		return !(*this == other);
	}

	const c8* StringView::GetSource() const
	{
		return m_source;
	}

	u64 StringView::GetSize() const
	{
		return m_size;
	}
}