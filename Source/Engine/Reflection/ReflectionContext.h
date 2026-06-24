#pragma once

#include <Runtime/Reflection/Type.h>

namespace Horizon
{
	class H_EXPORT ReflectionContext
	{
	public:
		void Register(Type* pType) { m_types.PushBack(pType); }
		const List<Type*>& GetTypes() const noexcept { return m_types; }

		Type* FindByFullName(const String& name) const;

	private:
		List<Type*> m_types;
	};
}