#pragma once

#include <Editor/Components/PropertyDrawer.h>

#include <string>

namespace Horizon::Editor
{
	HCLASS();
	class EDITOR_API StringPropertyDrawer final : public PropertyDrawer
	{
		HORIZON_TYPE_REFLECT(StringPropertyDrawer);
	public:
		StringPropertyDrawer() = default;
		~StringPropertyDrawer() = default;

		Reflect::TypeHandle GetTargetType() const final { return Reflect::TypeOf<std::string>(); }
		b8 OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx) final;
	};
}