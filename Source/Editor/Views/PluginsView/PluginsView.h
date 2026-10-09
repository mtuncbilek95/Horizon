#pragma once

#include <Editor/Views/EditorViewAttribute.h>
#include <Editor/Views/ViewObject.h>
#include <Editor/Font/IconsFontAwesome6.h>

namespace Horizon::Engine
{
	class PluginSystem;
}

namespace Horizon::Editor
{
	class ProjectContext;

	HCLASS(EditorView[ICON_FA_PLUG, "Plugins", DockZone::Center, EditorViewFlags::Mutable]);
	class EDITOR_API PluginsView : public ViewObject
	{
		HORIZON_TYPE_REFLECT(PluginsView);
	public:
		PluginsView() = default;
		~PluginsView() = default;

		void OnInvoke() final;
		void OnRender(const Engine::EngineFrame& context) final;

		void OnLibraryRegistered(const Engine::ReflectionLibrary& library) final;
		void OnLibraryUnregistered(const Engine::ReflectionLibrary& library) final;

	private:
		Engine::PluginSystem* m_pluginSystem = nullptr;
		ProjectContext* m_projectContext = nullptr;
	};
}