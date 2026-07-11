#include "SceneHierarchyWidget.h"

#include <Engine/Core/Engine.h>
#include <Editor/Widget/WidgetDefinitions.h>
#include <Editor/Font/IconsFontAwesome6.h>

#include <imgui.h>

namespace Horizon
{
	void SceneHierarchyWidget::OnInvoke()
	{
		if(!m_ecs)
			m_ecs = GetEngine()->TryGetSystem<EntityComponentSystem>();

#if defined(USE_TEST_ENVIRONMENT)
		m_ecs->GetWorld().CreateEntity();
		m_ecs->GetWorld().CreateEntity();
		m_ecs->GetWorld().CreateEntity();
		m_ecs->GetWorld().CreateEntity();
		m_ecs->GetWorld().CreateEntity();
		m_ecs->GetWorld().CreateEntity();
		m_ecs->GetWorld().CreateEntity();
		m_ecs->GetWorld().CreateEntity();
#endif
	}

	void SceneHierarchyWidget::OnDraw()
	{
		if (!m_ecs)
			return;

		WorldRegistry& world = m_ecs->GetWorld();

		world.ForEachEntity([&](EntityHandle entity)
			{
				ImGui::PushID((i32)entity.id);

				b8 selected = (entity.id == m_selected.id);

				std::string label = std::string(ICON_FA_CUBE) + "  Entity " + std::to_string(entity.id);

				if (ImGui::Selectable(label.c_str(), selected))
					m_selected = entity;

				ImGui::PopID();
			});
	}
}

HORIZON_BIND_WIDGET(SceneHierarchyWidget, "Scene Hierarchy", ICON_FA_DATABASE, WidgetDock::Left, true);