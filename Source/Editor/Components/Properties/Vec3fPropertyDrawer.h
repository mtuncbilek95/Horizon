#pragma once

#include <Editor/Components/PropertyDrawer.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::Editor
{
	HCLASS();
	class EDITOR_API Vec3fPropertyDrawer final : public PropertyDrawer
	{
		HORIZON_TYPE_REFLECT(Vec3fPropertyDrawer);
	public:
		Vec3fPropertyDrawer() = default;
		~Vec3fPropertyDrawer() = default;

		Reflect::TypeHandle GetTargetType() const final { return Reflect::TypeOf<Math::Vec3f>(); }
		b8 OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx) final;
	};
}