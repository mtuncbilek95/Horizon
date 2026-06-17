#pragma once

#include <Runtime/Reflection/TypeEnum.h>
#include <Runtime/Reflection/TypeKind.h>
#include <Runtime/Reflection/EnumValue.h>

#include <string>
#include <vector>

namespace Horizon
{
	class Field;
	class Attribute;
	using DefaultHeapGenerator = void*;

	class Type final
	{
	public:

	private:

	private:
		std::string m_name;
		std::string m_namespaces;
		std::string m_fullName;
		std::string m_rawName;
		
		u32 m_size;
		TypeEnum m_mode;
		TypeKind m_kind;
		DefaultHeapGenerator m_generator;

		std::vector<Field*> m_fields;
		std::vector<Attribute*> m_attributes;
		std::vector<EnumValue> m_enums;

		Type* m_baseType;
		u32 m_shareCount;
	};
}