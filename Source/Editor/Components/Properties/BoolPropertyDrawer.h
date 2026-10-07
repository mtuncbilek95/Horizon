#pragma once

#include <Editor/Components/PropertyDrawer.h>

namespace Horizon::Editor
{
	HCLASS();
	class EDITOR_API BoolPropertyDrawer final : public PropertyDrawer
	{
		HORIZON_TYPE_REFLECT(BoolPropertyDrawer);
	public:
		BoolPropertyDrawer() = default;
		~BoolPropertyDrawer() = default;

		Reflect::TypeHandle GetTargetType() const final { return Reflect::TypeOf<b8>(); }
		b8 OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx) final;
	};
}