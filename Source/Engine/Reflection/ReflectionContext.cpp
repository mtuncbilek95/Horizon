#include "ReflectionContext.h"

namespace Horizon
{
	Type* ReflectionContext::FindByFullName(const String& name) const
	{
		for (Type* pType : m_types)
		{
			if (pType->GetFullName() == name)
				return pType;
		}

		return nullptr;
	}
}