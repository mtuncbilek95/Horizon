#pragma once

#include <Editor/Components/PropertyDrawer.h>
#include <Editor/Components/Helpers/EntityPickerWidget.h>
#include <Engine/World/ECS/Definitions.h>

namespace Horizon::Editor
{
	HCLASS();
	class EDITOR_API EntityHandlePropertyDrawer final : public PropertyDrawer
	{
		HORIZON_TYPE_REFLECT(EntityHandlePropertyDrawer);
	public:
		EntityHandlePropertyDrawer() = default;
		~EntityHandlePropertyDrawer() = default;

		Reflect::TypeHandle GetTargetType() const final { return Reflect::TypeOf<Engine::EntityHandle>(); }
		b8 OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx) final;

	private:
		EntityPickerWidget m_picker;
	};
}