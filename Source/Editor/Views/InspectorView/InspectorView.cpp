#include "InspectorView.h"

#include <Editor/Renderer/EditorContext.h>
#include <Editor/Models/SelectionModel.h>
#include <Editor/Attributes/HideInInspectorAttribute.h>
#include <Editor/Attributes/RangeAttribute.h>
#include <Editor/Attributes/TooltipAttribute.h>

#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/World/ECS/Scene.h>
#include <Engine/World/WorldService.h>
#include <Engine/World/Components/TransformComponent.h>

namespace Horizon::Editor
{
	void InspectorView::OnInvoke()
	{
		m_worldService = GetContext()->pEngine->RequestService<Engine::WorldService>();
		m_reflSys = GetContext()->pEngine->GetReflectionSystem();
	}

	void InspectorView::OnRender()
	{
		auto* pSelectionModel = GetContext()->pSelection;

		Engine::EntityHandle entity;
		if (pSelectionModel->Is<Engine::EntityTag>())
			entity = pSelectionModel->Get<Engine::EntityTag>();
		else
			return;

		auto* currScene = m_worldService->GetCurrentWorld();

		const List<Engine::IComponentStorage*>& storages = currScene->GetComponents().GetStorages();

		for (usize i = 0; i < storages.GetCount(); i++)
		{
			Engine::IComponentStorage* pStorage = storages[i];

			if (!pStorage->Contains(entity))
				continue;

			Engine::ComponentTypeId typeId = pStorage->GetComponentTypeId();

			auto* pType = m_reflSys->GetType(typeId);

			if (typeId == Reflect::TypeOf<Engine::TransformComponent>())
			{
				auto* tComp = (Engine::TransformComponent*)pStorage->FindRaw(entity);

				f32 pos[3] = { tComp->m_position.X(), tComp->m_position.Y(), tComp->m_position.Z() };
				ImGui::DragFloat3("Position", pos, 0.1f);
				tComp->m_position = { pos[0], pos[1], pos[2] };
			}
		}
	}
}