#pragma once

#include <Runtime/RTTR/Reflection.h>
#include <Runtime/Containers/List.h>
#include <string>

namespace Horizon::Editor
{
	class H_EXPORT ActionTypeAttribute : public Reflect::Attribute
	{
		HORIZON_ATTRIBUTE_REFLECT(ActionTypeAttribute);
	public:
		ActionTypeAttribute(Reflect::TypeHandle handle) : m_workingType(handle)
		{
		}
		~ActionTypeAttribute() = default;

		Reflect::TypeHandle GetAssetType() const { return m_workingType; }

	private:
		Reflect::TypeHandle m_workingType;
	};
}