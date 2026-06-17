#pragma once

#include <Runtime/Reflection/Type.h>
#include <Runtime/Reflection/FieldMode.h>

#include <string>

namespace Horizon
{
	class Field final
	{
		friend class TypeDispatcher;
	public:
		const std::string& GetName() const noexcept { return m_name; }
		Type* GetType() const noexcept { return m_fieldType; }
		FieldMode GetMode() const noexcept { return m_mode; }
		u32 GetOffset() const noexcept { return m_offset; }

		template<typename T>
		void SetValue(void* pObj, const T& value) const noexcept
		{
			c8* pObjInByte = (c8*)pObj + m_offset;
			T* pVal = (T*)pObjInByte;
			*pVal = value;
		}

		template<typename T>
		T GetValue(const void* pObj) const noexcept
		{
			c8* pObjInByte = (c8*)pObj + m_offset;
			return *(T*)pObjInByte;
		}

		template<typename T>
		T* GetAddress(void* pObj) const noexcept
		{
			c8* pObjInByte = (c8*)pObj + m_offset;
			return (T*)pObjInByte;
		}

	private:
		Field(const std::string& name, Type* pFieldType, FieldMode mode, u32 offset);
		~Field() = default;

	private:
		std::string m_name;
		Type* m_fieldType;
		FieldMode m_mode;
		u32 m_offset;
	};
}