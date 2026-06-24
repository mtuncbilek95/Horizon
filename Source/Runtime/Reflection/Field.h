#pragma once

#include <Runtime/Reflection/Type.h>
#include <Runtime/Reflection/FieldMode.h>

namespace Horizon
{
	class H_EXPORT Field final
	{
		friend class TypeDispatcher;
		friend struct Allocator;
	public:
		FORCEINLINE String GetName() const noexcept { return m_name; }
		FORCEINLINE const Type* GetType() const noexcept { return m_fieldType; }
		FORCEINLINE FieldMode GetMode() const noexcept { return m_mode; }
		FORCEINLINE u32 GetOffset() const noexcept { return m_offset; }

		template<typename T>
		void SetValue(void* pObject, const T& value) const noexcept
		{
			u8* pBytes = (u8*)pObject + m_offset;
			*(T*)pBytes = value;
		}

		template<typename T>
		T GetValue(const void* pObject) const noexcept
		{
			u8* pBytes = (u8*)pObject + m_offset;
			return *(T*)pBytes;
		}

		template<typename T>
		T* GetAddress(void* pObject) const noexcept
		{
			u8* pBytes = (u8*)pObject + m_offset;
			return (T*)pBytes;
		}

	private:
		Field(const String& name, const Type* pFieldType, const FieldMode mode, const u32 offset);
		~Field();

	private:
		const String m_name;
		const Type* m_fieldType;
		const FieldMode m_mode;
		const u32 m_offset;
	};
}