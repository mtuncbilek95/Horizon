#pragma once

#include <Editor/Components/PropertyDrawer.h>

namespace Horizon::Editor
{
	HCLASS();
	class EDITOR_API SignedPropertyDrawer final : public PropertyDrawer
	{
		HORIZON_TYPE_REFLECT(SignedPropertyDrawer);
	public:
		SignedPropertyDrawer() = default;
		~SignedPropertyDrawer() = default;

		Reflect::TypeHandle GetTargetType() const final { return Reflect::TypeOf<i32>(); }
		b8 OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx) final;
	};
}