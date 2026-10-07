#pragma once

#include <Editor/Components/PropertyDrawer.h>
#include <Runtime/Math/Vec4f.h>

namespace Horizon::Editor
{
	HCLASS();
	class EDITOR_API Vec4fPropertyDrawer final : public PropertyDrawer
	{
		HORIZON_TYPE_REFLECT(Vec4fPropertyDrawer);
	public:
		Vec4fPropertyDrawer() = default;
		~Vec4fPropertyDrawer() = default;

		Reflect::TypeHandle GetTargetType() const final { return Reflect::TypeOf<Math::Vec4f>(); }
		b8 OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx) final;
	};
}