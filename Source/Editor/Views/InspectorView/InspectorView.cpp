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
	InspectorView::~InspectorView()
	{
		for (auto* pDrawer : m_drawerList)
			Memory::Allocator::Delete(pDrawer);
	}

	void InspectorView::OnInvoke()
	{
		m_worldService = GetContext()->pEngine->RequestService<Engine::WorldService>();
		m_reflSys = GetContext()->pEngine->GetReflectionSystem();

		List<Reflect::Type*> drawerTypes = m_reflSys->GetTypeByBase(Reflect::TypeOf<ComponentDrawer>());
		for (auto* pType : drawerTypes)
		{
			auto* pDrawer = (ComponentDrawer*)pType->Create();
			m_drawerLookups[pDrawer->GetComponentId()] = m_drawerList.GetCount();
			m_drawerList.PushBack(pDrawer);
		}
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

			auto it = m_drawerLookups.find(typeId);
			if (it == m_drawerLookups.end())
				continue;

			ComponentDrawer* pDrawer = m_drawerList[it->second];
			pDrawer->m_engine = GetContext()->pEngine;
			pDrawer->m_component = (Engine::ComponentObject*)pStorage->FindRaw(entity);

			Reflect::Type* pType = m_reflSys->GetType(typeId);

			ImGui::PushID((int)i);

			if (ImGui::CollapsingHeader(pType->GetName().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::PushFont(nullptr, ImGui::GetStyle().FontSizeBase * 0.875f);
				ImGui::Indent();
				pDrawer->OnRender();
				ImGui::Unindent();
				ImGui::PopFont();
			}

			ImGui::PopID();
		}
	}
}