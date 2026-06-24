#include "Path.h"

#include <Runtime/Containers/List.h>

namespace Horizon
{
	Path::Path(const c8* pPath) : m_path(pPath)
	{
		NormalizeSeparators();
	}

	Path::Path(const Path& other) : m_path(other.m_path)
	{
		NormalizeSeparators();
	}

	Path::Path(Path&& other) noexcept : m_path(std::move(other.m_path))
	{
		NormalizeSeparators();
	}

	Path::Path(const String& path) : m_path(path)
	{
		NormalizeSeparators();
	}

	Path::Path(String&& path) noexcept : m_path(std::move(path))
	{
		NormalizeSeparators();
	}

	Path& Path::operator=(const Path& other)
	{
		m_path = other.m_path;
		NormalizeSeparators();
		return *this;
	}

	Path& Path::operator=(Path&& other) noexcept
	{
		m_path = std::move(other.m_path);
		NormalizeSeparators();
		return *this;
	}

	Path& Path::operator=(const String& path)
	{
		m_path = path;
		NormalizeSeparators();
		return *this;
	}

	Path& Path::operator=(String&& path) noexcept
	{
		m_path = std::move(path);
		NormalizeSeparators();
		return *this;
	}

	Path& Path::Append(const c8* pPath)
	{
		if (!pPath || *pPath == '\0')
			return *this;

		Path temp(pPath);
		if (temp.IsAbsolute())
		{
			m_path = pPath;
			return *this;
		}

		if (*pPath == '/')
			pPath++;

		if (*pPath == '\0')
			return *this;

		if (!m_path.IsEmpty() && m_path[m_path.GetSize() - 1] != '/')
			m_path += '/';

		m_path += pPath;
		return *this;
	}

	Path& Path::Append(const String& path)
	{
		if (path.IsEmpty())
			return *this;

		Path temp(path);
		if (temp.IsAbsolute())
		{
			m_path = path;
			return *this;
		}

		u64 startIndex = 0;
		while (startIndex < path.GetSize() && path[startIndex] == '/')
			startIndex++;

		if (startIndex >= path.GetSize())
			return *this;

		if (!m_path.IsEmpty() && m_path[m_path.GetSize() - 1] != '/')
			m_path += '/';

		m_path += path.GetSubset(startIndex);
		return *this;
	}

	Path& Path::Append(const Path& other)
	{
		return Append(other.m_path);
	}

	Path Path::operator/(const c8* pPath) const
	{
		Path result(*this);
		result.Append(pPath);
		return result;
	}

	Path Path::operator/(const String& path) const
	{
		Path result(*this);
		result.Append(path);
		return result;
	}

	Path Path::operator/(const Path& other) const
	{
		Path result(*this);
		result.Append(other);
		return result;
	}

	Path& Path::operator/=(const c8* pPath)
	{
		return Append(pPath);
	}

	Path& Path::operator/=(const String& path)
	{
		return Append(path);
	}

	Path& Path::operator/=(const Path& other)
	{
		return Append(other);
	}

	b8 Path::operator==(const Path& other) const noexcept
	{
		return m_path == other.m_path;
	}

	Path operator+(const Path& a, const Path& b)
	{
		return Path(a.GetString() + b.GetString());
	}

	Path operator+(const Path& a, const String& b)
	{
		return Path(a.GetString() + b);
	}

	Path operator+(const Path& a, const c8* b)
	{
		return Path(a.GetString() + b);
	}

	Path operator+(const String& a, const Path& b)
	{
		return Path(a + b.GetString());
	}

	Path operator+(const c8* a, const Path& b)
	{
		return Path(a + b.GetString());
	}

	Path Path::GetFiltered(const Path& target) const noexcept
	{
		return m_path.GetFiltered(target.GetString());
	}

	Path Path::GetReplaced(const Path& oldString, const Path& newString) const noexcept
	{
		return m_path.GetReplaced(oldString, newString);
	}

	b8 Path::IsParentOf(const Path& target) const noexcept
	{
		return target.GetString().Contains(m_path);
	}

	b8 Path::IsAbsolute() const noexcept
	{
		if (m_path.IsEmpty())
			return false;

#if defined(HORIZON_WINDOWS)
		if (m_path.GetSize() >= 2)
		{
			c8 firstChar = m_path[0];
			if (((firstChar >= 'A' && firstChar <= 'Z') || (firstChar >= 'a' && firstChar <= 'z')) && m_path[1] == ':')
				return true;
		}
		return false;
#else
		if (m_path[0] == '/') {
			return true;
		}
#endif
	}

	b8 Path::IsRelative() const noexcept
	{
		return !IsAbsolute();
	}

	String Path::GetFileName() const
	{
		i64 lastSlashIndex = m_path.FindLastIndex('/');
		if (lastSlashIndex == -1)
			return m_path;

		i64 lastDotIndex = m_path.FindLastIndex('.');
		if (lastDotIndex == -1 || lastDotIndex < lastSlashIndex)
			return m_path.GetSubset(lastSlashIndex + 1);

		return m_path.GetSubset(lastSlashIndex + 1);
	}

	String Path::GetExtension() const
	{
		i64 lastDotIndex = m_path.FindLastIndex('.');
		if (lastDotIndex == -1)
			return String();

		return m_path.GetSubset(lastDotIndex);
	}

	Path Path::GetStem() const
	{
		i64 lastSlashIndex = m_path.FindLastIndex('/');
		i64 lastDotIndex = m_path.FindLastIndex('.');

		i64 filenameStart = (lastSlashIndex == -1) ? 0 : lastSlashIndex + 1;

		if (lastDotIndex == -1 || lastDotIndex < filenameStart)
		{
			if (lastSlashIndex == -1)
				return Path(m_path);
			else
				return Path(m_path.GetSubset(lastSlashIndex + 1));
		}

		if (lastDotIndex == filenameStart)
		{
			if (lastSlashIndex == -1)
				return Path(m_path);
			else
				return Path(m_path.GetSubset(lastSlashIndex + 1));
		}

		String stem;
		if (lastSlashIndex == -1)
			stem = m_path.GetSubset(0, lastDotIndex);
		else
			stem = m_path.GetSubset(lastSlashIndex + 1, lastDotIndex - lastSlashIndex - 1);

		return Path(stem);
	}

	Path Path::GetRootPath() const
	{
		if (!IsAbsolute())
			return Path();

		return Path(m_path.GetSubset(0, 3)); // e.g., "C:/"
	}

	Path Path::GetParentPath() const
	{
		i64 lastSlashIndex = m_path.FindLastIndex('/');
		if (lastSlashIndex == -1)
			return Path();
		return Path(m_path.GetSubset(0, lastSlashIndex));
	}

	Path Path::GetRelativeTo(const String& filter)
	{
		i64 index = m_path.FindIndex(filter);
		if (index == -1)
			return Path();

		return Path(m_path.GetSubset(index + filter.GetSize()));
	}

	u32 Path::GetDepth() const noexcept
	{
		return m_path.GetCount('/');
	}

	List<String> Path::GetPathNodes() const
	{
		return m_path.GetSplit("/");
	}

	void Path::NormalizeSeparators()
	{
		m_path = m_path.GetReplaced("\\", "/");
	}

}