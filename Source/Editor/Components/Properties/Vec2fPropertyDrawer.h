#pragma once

#include <Editor/Components/PropertyDrawer.h>
#include <Runtime/Math/Vec2f.h>

namespace Horizon::Editor
{
	HCLASS();
	class EDITOR_API Vec2fPropertyDrawer final : public PropertyDrawer
	{
		HORIZON_TYPE_REFLECT(Vec2fPropertyDrawer);
	public:
		Vec2fPropertyDrawer() = default;
		~Vec2fPropertyDrawer() = default;

		Reflect::TypeHandle GetTargetType() const final { return Reflect::TypeOf<Math::Vec2f>(); }
		b8 OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx) final;
	};
}