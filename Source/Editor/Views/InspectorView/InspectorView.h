#pragma once

#include <Editor/Views/EditorViewAttribute.h>
#include <Editor/Views/ViewObject.h>
#include <Editor/Font/IconsFontAwesome6.h>
#include <Editor/ContextMenu/ContextMenuRegistry.h>
#include <Editor/Components/PropertyRenderer.h>
#include <Engine/World/ECS/Definitions.h>

#include <string>

namespace Horizon::Engine
{
	class ReflectionSystem;
	class WorldService;
	class Scene;
}

namespace Horizon::Editor
{
	HCLASS(EditorView[ICON_FA_EYE, "Inspector", DockZone::Right, EditorViewFlags::OpenOnStart]);
	class EDITOR_API InspectorView : public ViewObject
	{
		HORIZON_TYPE_REFLECT(InspectorView);

		struct ComponentEntry
		{
			Reflect::Type* pType = nullptr;
			std::string label;
			std::string category;
		};

	public:
		~InspectorView() = default;

		void OnInvoke() final;
		void OnRender(const Engine::EngineFrame& context) final;

		void OnLibraryRegistered(const Engine::ReflectionLibrary& library) final;
		void OnLibraryUnregistered(const Engine::ReflectionLibrary& library) final;

	private:
		List<ComponentEntry> CollectComponents() const;

		void DrawAddComponentPopup(Engine::Scene* pScene, Engine::EntityHandle entity);
		void DrawPopupHeader();
		void DrawCategoryRoot(const List<ComponentEntry>& entries, Engine::Scene* pScene, Engine::EntityHandle entity);
		void DrawCategory(const List<ComponentEntry>& entries, Engine::Scene* pScene, Engine::EntityHandle entity);
		void DrawSearchResults(const List<ComponentEntry>& entries, Engine::Scene* pScene, Engine::EntityHandle entity);

		b8 DrawNavigationRow(const std::string& label);
		void DrawComponentRow(const ComponentEntry& entry, Engine::Scene* pScene, Engine::EntityHandle entity, b8 showCategory);

	private:
		Engine::ReflectionSystem* m_reflSys = nullptr;
		Engine::WorldService* m_worldService = nullptr;

		PropertyRenderer m_properties;

		std::string m_searchBuffer;
		std::string m_activeCategory;
	};
}