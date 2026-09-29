#pragma once

#include <Editor/Components/PropertyDrawer.h>

namespace Horizon::Editor
{
	HCLASS();
	class H_EXPORT UnsignedPropertyDrawer final : public PropertyDrawer
	{
		HORIZON_TYPE_REFLECT(UnsignedPropertyDrawer);
	public:
		UnsignedPropertyDrawer() = default;
		~UnsignedPropertyDrawer() = default;

		Reflect::TypeHandle GetTargetType() const final { return Reflect::TypeOf<u32>(); }
		b8 OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx) final;
	};
}