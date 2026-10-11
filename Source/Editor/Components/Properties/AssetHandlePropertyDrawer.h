#pragma once

#include <Editor/Components/PropertyDrawer.h>
#include <Editor/Components/Helpers/AssetPickerWidget.h>
#include <Runtime/Containers/Guid.h>

namespace Horizon::Editor
{
	HCLASS();
	class EDITOR_API AssetHandlePropertyDrawer final : public PropertyDrawer
	{
		HORIZON_TYPE_REFLECT(AssetHandlePropertyDrawer);
	public:
		AssetHandlePropertyDrawer() = default;
		~AssetHandlePropertyDrawer() = default;

		Reflect::TypeHandle GetTargetType() const final { return Reflect::TypeOf<Guid>(); }
		b8 OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx) final;

	private:
		AssetPickerWidget m_picker;
	};
}