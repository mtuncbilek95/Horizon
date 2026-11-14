#pragma once

#include <Editor/Views/EditorViewAttribute.h>
#include <Editor/Views/ViewObject.h>
#include <Editor/Font/IconsFontAwesome6.h>
#include <Engine/Asset/Input/InputAction.h>
#include <Runtime/Containers/Vector/List.h>

namespace Horizon::Engine
{
	class ReflectionSystem;
}

namespace Horizon::Editor
{
	HCLASS(EditorView[ICON_FA_GAMEPAD, "Input Action Map", DockZone::Center, EditorViewFlags::Mutable | EditorViewFlags::MultiInstance]);
	class InputActionMapView : public ViewObject
	{
		HORIZON_TYPE_REFLECT(InputActionMapView);
	public:
		InputActionMapView() = default;
		~InputActionMapView();

		void OnInvoke() final;
		void OnRender(const Engine::EngineFrame& context) final;
		b8 OnCommand(ViewCommand command) final;

	private:
		b8 LoadSource(DomainFile* pFile);
		b8 SaveSource();

		void DrawAction(Asset::InputAction& action);
		void DrawBinding(Asset::InputBinding& binding);
		b8 DrawCode(Asset::InputBinding& binding);
		b8 DrawEnumCombo(const c8* pLabel, Reflect::TypeHandle enumType, u64& value);

	private:
		Engine::ReflectionSystem* m_reflection = nullptr;

		List<Asset::InputAction> m_actions;

		DomainFile* m_loadedFile = nullptr;
		b8 m_isLoaded = false;
	};
}
