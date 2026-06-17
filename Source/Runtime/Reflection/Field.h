#pragma once

#include <Runtime/Reflection/Type.h>
#include <Runtime/Reflection/TypeEnum.h>

#include <string>

namespace Horizon
{
	class Field final
	{
		friend class TypeDispatcher;

		Field(const std::string& name, Type* pFieldType, TypeEnum mode, u32 offset);
		~Field() = default;

	public:
		const std::string& GetName() const { return m_name; }
		Type* GetType() const noexcept { return m_fieldType; }
		TypeEnum GetMode() const noexcept { return m_mode; }
		u32 GetOffset() const noexcept { return m_offset; }

		template<typename T>
		void SetValue(void* pObject, const T& value) const noexcept
		{
			c8* pObjInBytes = (c8*)pObject + m_offset;
			T* pVal = (T*)pObjInBytes;
			*pVal = value;
		}

		template<typename T>
		T GetValue(const void* pObject) const noexcept
		{
			c8* pObjInBytes = (c8*)pObject + m_offset;
			return *(T*)pObjInBytes;
		}

		template<typename T>
		T* GetAddress(void* pObject) const noexcept
		{
			c8* pObjInBytes = (c8*)pObject + m_offset;
			return (T*)pObjInBytes;
		}

	private:
		std::string m_name;
		Type* m_fieldType;
		TypeEnum m_mode;
		u32 m_offset;
	};
}