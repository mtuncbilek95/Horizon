#pragma once

#include <Editor/Components/PropertyDrawer.h>

namespace Horizon::Editor
{
	HCLASS();
	class H_EXPORT FloatPropertyDrawer final : public PropertyDrawer
	{
		HORIZON_TYPE_REFLECT(FloatPropertyDrawer);
	public:
		FloatPropertyDrawer() = default;
		~FloatPropertyDrawer() = default;

		Reflect::TypeHandle GetTargetType() const final { return Reflect::TypeOf<f32>(); }
		b8 OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx) final;
	};
}