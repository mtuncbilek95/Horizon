#include "SceneHierarchyWidget.h"

#include <Engine/Core/Engine.h>
#include <Engine/Asset/Loaders/World/WorldAsset.h>

#include <Editor/Font/IconsFontAwesome6.h>

#include <imgui.h>

#include <cstdio>

namespace Horizon
{
	namespace
	{
		const char* IconFor(WorldAsset& world, EntityHandle entity)
		{
			// if (world.GetRegistry().HasComponent<CameraComponent>(entity)) return ICON_FA_VIDEO;
			// if (world.GetRegistry().HasComponent<LightComponent>(entity))  return ICON_FA_LIGHTBULB;
			return ICON_FA_CUBE;
		}

		const char* DisplayNameOf(WorldAsset& world, EntityHandle entity, char* buf, usize size)
		{
			if (const std::string* name = world.NameOf(entity))
				return name->c_str();

			std::snprintf(buf, size, "Entity %u", (unsigned)entity.id);
			return buf;
		}
	}

	void SceneHierarchyWidget::OnInvoke()
	{
		if (!m_ecs)
			m_ecs = GetEngine()->TryGetSystem<EntityComponentSystem>();
	}

	void SceneHierarchyWidget::OnDraw()
	{
		if (!m_ecs)
			return;

		WorldAsset* world = m_ecs->GetActiveWorld();
		if (!world)
		{
			ImGui::TextDisabled("No scene open");
			return;
		}

		if (world != m_lastWorld)
		{
			m_selected = {};
			m_lastWorld = world;
		}

		ImGui::TextUnformatted(world->GetWorldName().c_str());
		ImGui::Separator();

		world->GetRegistry().ForEachEntity([&](EntityHandle entity)
			{
				ImGui::PushID((i32)entity.id);

				b8 selected = (entity.id == m_selected.id);

				char nameBuf[64];
				char label[96];
				std::snprintf(label, sizeof(label), "%s  %s",
					IconFor(*world, entity),
					DisplayNameOf(*world, entity, nameBuf, sizeof(nameBuf)));

				if (ImGui::Selectable(label, selected))
					m_selected = entity;

				ImGui::PopID();
			});

		if (ImGui::BeginPopupContextWindow("SceneHierarchyContext"))
		{
			if (ImGui::MenuItem("Add New Entity"))
				AddNewEntity(*world);

			ImGui::EndPopup();
		}
	}

	void SceneHierarchyWidget::AddNewEntity(WorldAsset& world)
	{
		EntityHandle entity = world.GetRegistry().CreateEntity();

		char name[64];
		std::snprintf(name, sizeof(name), "Entity %u", (unsigned)entity.id);
		world.SetName(entity, name);

		m_selected = entity;
	}
}