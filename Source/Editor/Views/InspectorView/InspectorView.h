#pragma once

#include <Editor/Views/EditorViewAttribute.h>
#include <Editor/Views/ViewObject.h>
#include <Editor/Font/IconsFontAwesome6.h>
#include <Editor/ContextMenu/ContextMenuRegistry.h>
#include <Editor/Components/PropertyRenderer.h>

#include <string>

namespace Horizon::Engine
{
	class ReflectionSystem;
	class WorldService;
}

namespace Horizon::Editor
{
	HCLASS(EditorView[ICON_FA_EYE, "Inspector", DockZone::Right, EditorViewFlags::OpenOnStart]);
	class EDITOR_API InspectorView : public ViewObject
	{
		HORIZON_TYPE_REFLECT(InspectorView);
	public:
		~InspectorView() = default;

		void OnInvoke() final;
		void OnRender(const Engine::EngineFrame& context) final;

		void OnLibraryRegistered(const Engine::ReflectionLibrary& library) final { m_properties.OnLibraryRegistered(library); }
		void OnLibraryUnregistered(const Engine::ReflectionLibrary& library) final { m_properties.OnLibraryUnregistered(library); }

	private:
		Engine::ReflectionSystem* m_reflSys = nullptr;
		Engine::WorldService* m_worldService = nullptr;

		PropertyRenderer m_properties;

		std::string m_searchBuffer;
	};
}