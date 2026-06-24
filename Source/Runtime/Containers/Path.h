#pragma once

#include <Runtime/Containers/String.h>

namespace Horizon
{
	class H_EXPORT Path final
	{
	public:
		Path() = default;
		Path(const c8* pPath);
		Path(const Path& other);
		Path(Path&& other) noexcept;
		Path(const String& path);
		Path(String&& path) noexcept;
		~Path() = default;

		FORCEINLINE const String& GetString() const noexcept { return m_path; }
		FORCEINLINE u64 GetLength() const noexcept { return m_path.GetSize(); }
		FORCEINLINE b8 IsEmpty() const noexcept { return m_path.IsEmpty(); }

		Path GetFiltered(const Path& target) const noexcept;
		Path GetReplaced(const Path& oldString, const Path& newString) const noexcept;
		b8 IsParentOf(const Path& target) const noexcept;
		b8 IsAbsolute() const noexcept;
		b8 IsRelative() const noexcept;
		String GetFileName() const;
		String GetExtension() const;
		Path GetStem() const;
		Path GetRootPath() const;
		Path GetParentPath() const;
		Path GetRelativeTo(const String& filter);
		u32 GetDepth() const noexcept;

		List<String> GetPathNodes() const;

		Path& operator=(const c8* pPath);
		Path& operator=(const Path& other);
		Path& operator=(Path&& other) noexcept;
		Path& operator=(const String& path);
		Path& operator=(String&& path) noexcept;

		Path& Append(const c8* pPath);
		Path& Append(const String& path);
		Path& Append(const Path& other);
		Path operator/(const c8* pPath) const;
		Path operator/(const String& path) const;
		Path operator/(const Path& other) const;

		Path& operator/=(const c8* pPath);
		Path& operator/=(const String& path);
		Path& operator/=(const Path& other);
		c8 operator[](u32 index) const { return m_path[index]; }
		c8& operator[](u32 index) { return m_path[index]; }
		const c8* operator*() const { return *m_path; }
		b8 operator==(const Path& other) const noexcept;
		b8 operator!=(const Path& other) const noexcept { return !(*this == other); }
		operator const String& () const { return m_path; }

		friend Path operator+(const Path& a, const Path& b);
		friend Path operator+(const Path& a, const String& b);
		friend Path operator+(const Path& a, const c8* b);
		friend Path operator+(const String& a, const Path& b);
		friend Path operator+(const c8* a, const Path& b);

	private:
		void NormalizeSeparators();

	private:
		String m_path;
	};
}
