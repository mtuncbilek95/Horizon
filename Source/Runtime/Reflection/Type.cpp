#include "Type.h"

#include <Runtime/Reflection/Field.h>

namespace Horizon
{
	Type::Type(const String& name, const String& namespaces, const u32 size, const TypeModes mode,
		const TypeCodes code, DefaultHeapObjectGenerator defaultObjectGenerator, Type** ppModuleAddress) :
		m_name(name), m_namespaces(namespaces), m_fullName(namespaces.IsEmpty() ? name : namespaces + "::" + name),
		m_rawName(name), m_size(size), m_mode(mode), m_code(code), m_defaultObjectGenerator(defaultObjectGenerator),
		m_baseType(nullptr), m_moduleAddress(ppModuleAddress), m_shareCount(1)
	{
	}

	Type::~Type()
	{
	}

	Attribute* Type::GetCustomAttribute(const Type* pType) const noexcept
	{
		for (Attribute* pAttribute : m_attributes)
			if (pAttribute->GetType() == pType)
				return pAttribute;

		return nullptr;
	}

	String Type::GetEnumName(const i64 value) const noexcept
	{
		for (const EnumValue& enumValue : m_enums)
			if (enumValue.Value == value)
				return enumValue.Name;

		return "";
	}

	Field* Type::GetField(const String& name) const noexcept
	{
		for (Field* pField : m_fields)
			if (pField->GetName() == name)
				return pField;

		return nullptr;
	}

	void* Type::CreateDefaultHeapObject() const noexcept
	{
		if (m_defaultObjectGenerator == nullptr)
			return nullptr;

		return m_defaultObjectGenerator();
	}

	b8 Type::IsSubClassOf(const Type* pType) const noexcept
	{
		const Type* pCurrent = this;
		while (pCurrent != nullptr)
		{
			if (pCurrent == pType)
				return true;

			pCurrent = pCurrent->m_baseType;
		}

		return false;
	}

	void Type::RegisterEnum(const String& name, const i64 value)
	{
		EnumValue enumValue;
		enumValue.Name = name;
		enumValue.Value = value;
		m_enums.PushBack(enumValue);
	}

	void Type::RegisterField(Field* pField)
	{
		m_fields.PushBack(pField);
	}

	void Type::SetBaseType(Type* pType)
	{
		m_baseType = pType;
	}

	void Type::RegisterAttribute(Attribute* pAttribute)
	{
		m_attributes.PushBack(pAttribute);
	}

	void Type::IncrementShareCount()
	{
		m_shareCount++;
	}

	void Type::DecrementShareCount()
	{
		m_shareCount--;
	}
}