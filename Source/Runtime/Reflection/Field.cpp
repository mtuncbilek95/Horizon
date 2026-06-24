#include "Field.h"

namespace Horizon
{
	Field::Field(const String& name, const Type* pFieldType, const FieldMode mode, const u32 offset)
		: m_name(name), m_fieldType(pFieldType), m_mode(mode), m_offset(offset)
	{
	}

	Field::~Field()
	{
	}
}