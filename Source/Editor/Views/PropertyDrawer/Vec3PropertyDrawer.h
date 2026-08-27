#pragma once

#include <Editor/Views/PropertyDrawer/PropertyDrawer.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::Editor
{
	HCLASS();
	class H_EXPORT Vec3PropertyDrawer : public PropertyDrawer
	{
		HORIZON_TYPE_REFLECT(Vec3PropertyDrawer);
	public:
		Vec3PropertyDrawer() = default;
		~Vec3PropertyDrawer() = default;

		Reflect::TypeHandle GetTargetType() const final { return Reflect::TypeOf<Math::Vec3f>(); }
		b8 OnDraw(const PropertyContext& context) final;
	};
}