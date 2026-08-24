#include "SceneHierarchyView.h"

#include <Editor/Views/InspectorView/InspectorView.h>
#include <Editor/Views/ViewRegistry.h>

#include <Engine/Core/Engine.h>
#include <Engine/World/WorldService.h>
#include <Engine/World/Components/NameComponent.h>

namespace Horizon::Editor
{
	void SceneHierarchyView::OnInvoke()
	{
		auto* pWorldService = GetEngine()->RequestService<Engine::WorldService>();
		m_activeWorld = pWorldService->GetActiveWorld();

		m_contextMenu.BootstrapContext(m_engine, "SceneHierarchyView");
	}

	void SceneHierarchyView::OnRender()
	{
		Engine::EntityStorage& storage = m_activeWorld->GetEntities();

		for (u32 i = 0; i < storage.GetHighWaterMark(); i++)
		{
			Engine::EntityHandle entity = storage.GetHandleAt(i);

			if (!entity.IsValid())
				continue;

			c8 label[MaxBufferLength];
			auto* pName = m_activeWorld->FindComponent<Engine::NameComponent>(entity);

			if (pName && !pName->m_name.empty())
			{
				auto result = std::format_to_n(label, MaxBufferLength - 1, "{}", pName->m_name);
				*result.out = '\0';
			}
			else
			{
				auto result = std::format_to_n(label, MaxBufferLength - 1, "Entity {}", entity.Index());
				*result.out = '\0';
			}

			ImGui::PushID((i32)entity.Index());

			if (ImGui::Selectable(label, m_selectedEntity == entity))
				GetRegistry()->GetViewObject<InspectorView>()->SetInspectingEntity(entity);

			ImGui::PopID();
		}

		SceneHierarchyContext context = {};

		m_contextMenu.RenderGUI("SceneHierarchyContextMenu", context);
	}
}