#include "Type.h"

#include <Runtime/Reflection/TypeDispatcher.h>
#include <Runtime/Reflection/Field.h>

namespace Horizon
{
	Attribute* Type::GetCustomAttribute(const Type* pType) const noexcept
	{
		for (Attribute* pAttribute : m_attributes)
		{
			if (pType == pAttribute->GetType())
				return pAttribute;
		}

		return nullptr;
	}

	std::string Type::GetEnumName(i64 value) const noexcept
	{
		for (const EnumValue& enumValue : m_enums)
		{
			if (enumValue.enumValue == value)
				return enumValue.enumName;
		}

		return "";
	}

	Field* Type::GetField(const std::string& name) const noexcept
	{
		for (Field* pField : m_fields)
		{
			if (pField->GetName() == name)
				return pField;
		}

		return nullptr;
	}

	void* Type::CreateDefaultHeapObject() const noexcept
	{
		return m_defaultObjectGenerator();
	}

	b8 Type::IsSubClassOf(const Type* pType) const noexcept
	{
		if (pType == m_baseType)
			return true;

		if (m_baseType == nullptr)
			return false;

		return m_baseType->IsSubClassOf(pType);
	}

	Type::Type(const std::string& name, const std::string& namespaces, u32 size, TypeModes mode, TypeCodes code,
		DefaultHeapObjectGenerator defaultObjectGenerator, Type** ppModuleAddress) : m_name(name), m_namespaces(namespaces),
		m_fullName(namespaces + "::" + name), 
		m_rawName([&] { std::string s = namespaces; s.erase(std::remove(s.begin(), s.end(), ':'), s.end()); return s + name; }()),
		m_size(size), m_mode(mode), m_code(code), m_defaultObjectGenerator(defaultObjectGenerator), m_baseType(nullptr), 
		m_moduleAddress(ppModuleAddress), m_shareCount(0)
	{}

	Type::~Type()
	{
		m_enums.clear();

		for (Field* pField : m_fields)
			TypeDispatcher::DeleteField(pField);

		m_fields.clear();
		m_attributes.clear();
	}

	void Type::RegisterEnum(const std::string& name, i64 value) { m_enums.push_back({ name, value }); }
	void Type::RegisterField(Field* pField) { m_fields.push_back(pField); }
	void Type::SetBaseType(Type* pType) { m_baseType = pType; }
	void Type::RegisterAttribute(Attribute* pAttribute) { m_attributes.push_back(pAttribute); }
	void Type::IncrementShareCount() { m_shareCount++; }

	void Type::DecrementShareCount()
	{
		if (m_shareCount == 0)
			return;

		m_shareCount--;
	}
}