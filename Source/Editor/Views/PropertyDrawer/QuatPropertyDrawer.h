#pragma once

#include <Editor/Views/PropertyDrawer/PropertyDrawer.h>
#include <Runtime/Math/Quat.h>

namespace Horizon::Editor
{
	HCLASS();
	class H_EXPORT QuatPropertyDrawer : public PropertyDrawer
	{
		HORIZON_TYPE_REFLECT(QuatPropertyDrawer);
	public:
		QuatPropertyDrawer() = default;
		~QuatPropertyDrawer() = default;

		Reflect::TypeHandle GetTargetType() const final { return Reflect::TypeOf<Math::Quat>(); }
		b8 OnDraw(const PropertyContext& context) final;
	};
}