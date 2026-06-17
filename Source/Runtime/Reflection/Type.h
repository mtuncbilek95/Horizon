#pragma once

#include <Runtime/Reflection/Attribute.h>
#include <Runtime/Reflection/Class.h>
#include <Runtime/Reflection/EnumValue.h>
#include <Runtime/Reflection/TypeCodes.h>
#include <Runtime/Reflection/TypeModes.h>

#include <vector>
#include <string>

namespace Horizon
{
	class Field;

	class Type final
	{
		friend class TypeDispatcher;
	public:
		const std::string& GetName() const noexcept { return m_name; }
		const std::string& GetNamespaces() const noexcept { return m_namespaces; }
		const std::string& GetFullName() const noexcept { return m_fullName; }
		const std::string& GetRawName() const noexcept { return m_rawName; }
		u32 GetSize() const noexcept { return m_size; }
		TypeModes GetMode() const noexcept { return m_mode; }
		TypeCodes GetCode() const noexcept { return m_code; }
		u32 GetShareCount() const noexcept { return m_shareCount; }
		const std::vector<Field*>& GetFields() const noexcept { return m_fields; }
		const std::vector<Attribute*>& GetAttributes() const noexcept { return m_attributes; }
		const std::vector<EnumValue>& GetEnums() const noexcept { return m_enums; }

		Attribute* GetCustomAttribute(const Type* pType) const noexcept;
		std::string GetEnumName(i64 value) const noexcept;
		Field* GetField(const std::string& name) const noexcept;

		template<typename T>
		std::string GetEnumName(const T& value) const noexcept
		{
			for (const EnumValue& enumValue : m_enums)
			{
				if (enumValue.enumValue == (i64)value)
					return enumValue.enumName;
			}

			return "";
		}

		template<typename T>
		T GetEnumValue(const std::string& value) const noexcept
		{
			for (const EnumValue& enumValue : m_enums)
			{
				if (enumValue.enumName == value)
					return (T)enumValue.enumValue;
			}

			return (T)0;
		}

		template<typename T>
		T* GetCustomAttribute() const noexcept
		{
			Type* pType = typeof<T>();
			for (Attribute* pAttribute : m_attributes)
			{
				if (pType == pAttribute->GetType())
					return (T*)pAttribute;
			}

			return nullptr;
		}

		template<typename T>
		std::vector<T*> GetCustomAttributes() const noexcept
		{
			std::vector<T*> attributes;

			Type* pType = typeof<T>();
			for (Attribute* pAttribute : m_attributes)
			{
				if (pType == pAttribute->GetType())
					attributes.push_back((T*)pAttribute);
			}

			return attributes;
		}

		void* CreateDefaultHeapObject() const noexcept;
		b8 IsSubClassOf(const Type* pType) const noexcept;

	private:
		Type(const std::string& name, const std::string& namespaces, u32 size, TypeModes mode, TypeCodes code, 
			DefaultHeapObjectGenerator defaultObjectGenerator, Type** ppModuleAddress);
		~Type();

		void RegisterEnum(const std::string& name, i64 value);
		void RegisterField(Field* pField);
		void SetBaseType(Type* pType);
		void RegisterAttribute(Attribute* pAttribute);
		void IncrementShareCount();
		void DecrementShareCount();

	private:
		std::string m_name;
		std::string m_namespaces;
		std::string m_fullName;
		std::string m_rawName;
		u32 m_size;
		TypeModes m_mode;
		TypeCodes m_code;
		DefaultHeapObjectGenerator m_defaultObjectGenerator;
		std::vector<Field*> m_fields;
		std::vector<Attribute*> m_attributes;
		std::vector<EnumValue> m_enums;
		Type* m_baseType;
		Type** m_moduleAddress;
		u32 m_shareCount;
	};
}