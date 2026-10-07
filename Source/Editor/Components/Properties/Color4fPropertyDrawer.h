#pragma once

#include <Editor/Components/PropertyDrawer.h>
#include <Runtime/Math/Color4f.h>

namespace Horizon::Editor
{
	HCLASS();
	class EDITOR_API Color4fPropertyDrawer final : public PropertyDrawer
	{
		HORIZON_TYPE_REFLECT(Color4fPropertyDrawer);
	public:
		Color4fPropertyDrawer() = default;
		~Color4fPropertyDrawer() = default;

		Reflect::TypeHandle GetTargetType() const final { return Reflect::TypeOf<Math::Color4f>(); }
		b8 OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx) final;
	};
}