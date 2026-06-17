#include "Field.h"

namespace Horizon
{
	Field::Field(const std::string& name, Type* pFieldType, TypeEnum mode, u32 offset) : m_name(name),
		m_fieldType(pFieldType), m_mode(mode), m_offset(offset)
	{}

}