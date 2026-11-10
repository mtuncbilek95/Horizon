#include "InspectorView.h"

#include <Editor/Components/Helpers/SearchFilter.h>
#include <Editor/Renderer/EditorContext.h>
#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/World/WorldService.h>
#include <Engine/World/ECS/Scene.h>
#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/Components/Tag/EntityName.h>
#include <Engine/World/Components/Tag/EditorOnly.h>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace Horizon::Editor
{
	void InspectorView::OnInvoke()
	{
		m_world = GetContext()->pEngine->RequestService<Engine::WorldService>();
		m_reflection = GetContext()->pEngine->GetReflectionSystem();

		m_properties.Initialize(GetContext()->pEngine);
	}

	void InspectorView::OnRender(const Engine::EngineFrame& context)
	{
		Engine::EntityHandle handl = GetContext()->pSelection->Get<Engine::EntityTag>();

		if (!handl.IsValid())
			return;

		Engine::Scene* pScene = m_world->GetCurrentWorld();

		if (!pScene || !pScene->IsAlive(handl))
			return;

		ImGui::BeginChild("##Components", ImVec2(0.f, -ImGui::GetFrameHeightWithSpacing()));

		const List<Engine::ComponentStorage*>& storages = pScene->GetComponents().GetStorages();

		for (usize i = 0; i < storages.GetCount(); i++)
		{
			Engine::ComponentStorage* pStorage = storages[i];

			if (!pStorage->Contains(handl))
				continue;

			const Reflect::Type* pType = pStorage->GetType();

			if (pType->GetTypeId() == Reflect::TypeOf<World::Component::EntityName>())
				continue;

			if (pType->GetTypeId() == Reflect::TypeOf<World::Component::EditorOnly>())
				continue;

			const auto* pId = pType->GetCustomAttribute<Engine::ComponentIdAttribute>();
			const std::string& header = pId ? pId->GetDisplayName() : pType->GetName();

			ImGui::PushID(i32(i));

			b8 keepComponent = true;
			b8 isOpen = ImGui::CollapsingHeader(header.c_str(), &keepComponent, ImGuiTreeNodeFlags_DefaultOpen);

			if (!keepComponent)
			{
				pScene->RemoveComponent(handl, pType->GetTypeId());
				ImGui::PopID();
				break;
			}

			if (isOpen)
				m_properties.DrawObject(*pType, pStorage->Find(handl));

			ImGui::PopID();
		}

		ImGui::EndChild();

		if (ImGui::Button(ICON_FA_PLUS " Add Component", ImVec2(-FLT_MIN, 0.f)))
		{
			m_search.clear();
			ImGui::OpenPopup("AddComponentPopup");
		}

		ImGui::SetNextWindowSize(ImVec2(260.f, 0.f));

		if (!ImGui::BeginPopup("AddComponentPopup"))
			return;

		if (ImGui::IsWindowAppearing())
			ImGui::SetKeyboardFocusHere();

		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::InputTextWithHint("##Search", "Search...", &m_search);
		ImGui::Separator();

		struct ComponentEntry
		{
			const Reflect::Type* pType = nullptr;
			std::string label;
			std::string category;
		};

		List<ComponentEntry> entries;
		List<Reflect::Type*> types = m_reflection->GetTypeByBase(Reflect::TypeOf<Engine::ComponentObject>());

		for (Reflect::Type* pType : types)
		{
			if (pType->GetIsAbstract())
				continue;

			if (pType->GetTypeId() == Reflect::TypeOf<World::Component::EntityName>())
				continue;

			if (pType->GetTypeId() == Reflect::TypeOf<World::Component::EditorOnly>())
				continue;

			if (pScene->HasComponent(handl, pType->GetTypeId()))
				continue;

			const auto* pId = pType->GetCustomAttribute<Engine::ComponentIdAttribute>();
			const std::string& label = pId ? pId->GetDisplayName() : pType->GetName();

			if (!SearchFilter::Matches(label, m_search))
				continue;

			ComponentEntry& entry = entries.EmplaceBack();
			entry.pType = pType;
			entry.label = label;
			entry.category = pId && !pId->GetDisplayCategory().empty() ? pId->GetDisplayCategory() : "Other";
		}

		entries.Sort([](const ComponentEntry& lhs, const ComponentEntry& rhs)
			{
				if (lhs.category != rhs.category)
					return lhs.category < rhs.category;

				return lhs.label < rhs.label;
			});

		if (entries.IsEmpty())
			ImGui::TextDisabled("No component found");

		const b8 searching = !m_search.empty();
		const std::string* pActiveCategory = nullptr;
		b8 menuOpen = false;

		for (const ComponentEntry& entry : entries)
		{
			if (!searching && (!pActiveCategory || *pActiveCategory != entry.category))
			{
				if (menuOpen)
					ImGui::EndMenu();

				menuOpen = ImGui::BeginMenu(entry.category.c_str());
				pActiveCategory = &entry.category;
			}

			if (!searching && !menuOpen)
				continue;

			ImGui::PushID(entry.pType);

			if (ImGui::MenuItem(entry.label.c_str(), searching ? entry.category.c_str() : nullptr))
				pScene->AddComponent(handl, entry.pType->GetTypeId());

			ImGui::PopID();
		}

		if (menuOpen)
			ImGui::EndMenu();

		ImGui::EndPopup();
	}
}