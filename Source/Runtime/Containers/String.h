#pragma once

#include <string>
#include <format>

namespace Horizon
{
	class String;

	template<>
	class TypeAccessor<String>
	{
		friend class TypeDispatcher;
	public:
		static Type* GetType() { return sType; }

	private:
		static void SetType(Type* pType) { sType = pType; }
		static Type** GetTypeAddress() { return &sType; }

	private:
		static inline Type* sType = nullptr;
	};

	template<typename T>
	class List;

	class StringView;

	class H_EXPORT String final
	{
	public:
		static i32 ToInt32(const String& text);
		static u32 ToUInt32(const String& text);
		static i64 ToInt64(const String& text);
		static u64 ToUInt64(const String& text);
		static f32 ToFloat32(const String& text);
		static f64 ToFloat64(const String& text);
		static String FromInt32(const i32 value);
		static String FromUInt32(const u32 value);
		static String FromInt64(const i64 value);
		static String FromUInt64(const u64 value);
		static String FromFloat32(const f32 value);
		static String FromFloat64(const f64 value);

	public:
		String();
		String(const c8* pData);
		String(const c8* pData, u64 sizeInBytes);
		String(const u8* pData, u64 sizeInBytes);
		String(u64 sizeInBytes);
		String(const String& other);
		String(String&& other) noexcept;
		~String();

		FORCEINLINE Type* GetType() const noexcept { return typeof(String); }

		u64 GetSize() const noexcept { return m_data.size(); }
		b8 IsEmpty() const noexcept { return m_data.empty(); }
		const c8* GetSource() const noexcept { return m_data.c_str(); }
		c8* GetSource() noexcept { return m_data.data(); }

		b8 Contains(const String& str) const;
		b8 Contains(const String& str, b8 caseSensitive) const;
		b8 Has(const c8 c) const;

		b8 StartsWith(const String& str) const;
		b8 StartsWith(c8 c) const;
		b8 EndsWith(const String& str) const;
		b8 EndsWith(c8 c) const;

		i64 FindIndex(const String& target, u64 startIndex, u64 count = 0) const;
		i64 FindIndex(const c8 target) const;
		i64 FindIndex(const String& target) const;
		i64 FindLastIndex(const String& target) const;
		i64 FindLastIndex(const c8 target) const;

		u64 GetCount(const c8 target) const;

		String GetSubset(u64 startIndex, u64 length) const;
		String GetSubset(u64 startIndex) const;

		String GetFiltered(const String& filter) const;
		String GetFiltered(c8 filter) const;
		String GetReplaced(const String& oldString, const String& newString) const;

		String RemoveSegments(const String& segment) const;
		List<String> GetSplit(const String& filter) const;

		String ToUpper() const;
		String ToLower() const;

		const c8* begin() const noexcept { return GetSource(); }
		const c8* end() const noexcept { return GetSource() + GetSize(); }
		c8* begin() noexcept { return GetSource(); }
		c8* end() noexcept { return GetSource() + GetSize(); }

		friend String operator+(const String& a, const String& b);
		friend String operator+(const String& a, const c8* b);
		friend String operator+(const c8* a, const String& b);
		friend String operator+(const String& a, c8 b);
		friend String operator+(c8 a, const String& b);

		friend b8 operator==(const String& a, const String& b);
		friend b8 operator==(const String& a, const c8* b);
		friend b8 operator!=(const String& a, const c8* b);
		friend b8 operator==(const c8* a, const String& b);
		friend b8 operator!=(const c8* a, const String& b);
		friend b8 operator!=(const String& a, const String& b);
		friend b8 operator<(const String& a, const String& b);
		friend b8 operator>(const String& a, const String& b);
		friend b8 operator<=(const String& a, const String& b);
		friend b8 operator>=(const String& a, const String& b);

		String& operator=(const String& other);
		String& operator=(String&& other) noexcept;
		String& operator=(const c8* other);

		String& operator+=(const String& other);
		String& operator+=(const c8* other);
		String& operator+=(c8 other);

		c8 operator[](u32 index) const { return GetSource()[index]; }
		c8& operator[](u32 index) { return GetSource()[index]; }

		const c8* operator*() const { return m_data.c_str(); }

		void Clear();

	private:
		// Till we optimize with intrinsics + all SIMD shit, use this son of a bitch.
		std::string m_data;
	};
}

template<>
struct std::hash<Horizon::String>
{
	std::size_t operator()(Horizon::String const& s) const noexcept
	{
		return std::hash<std::string_view>{}(std::string_view(s.GetSource(), s.GetSize()));
	}
};

template <>
struct std::formatter<Horizon::String> : std::formatter<std::string_view>
{
	auto format(const Horizon::String& str, std::format_context& ctx) const
	{
		return std::formatter<std::string_view>::format(
			std::string_view(str.GetSource(), str.GetSize()), ctx);
	}
};