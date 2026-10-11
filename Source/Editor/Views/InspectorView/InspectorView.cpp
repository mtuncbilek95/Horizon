#include "InspectorView.h"

#include <Editor/Renderer/EditorContext.h>
#include <Editor/Models/SelectionModel.h>
#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/World/ECS/Scene.h>
#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/WorldService.h>
#include <Engine/World/Components/Tag/NameComponent.h>
#include <Engine/World/Components/EditorOnlyComponent.h>

#include <misc/cpp/imgui_stdlib.h>

#include <algorithm>
#include <cctype>

namespace Horizon::Editor
{
	namespace
	{
		static constexpr std::string_view sButtonName = "Add Component";
		static constexpr std::string_view sPopupName = "UsableComponents";
		static constexpr std::string_view sRootTitle = "Component";
		static constexpr f32 sPopupWidth = 260.f;
		static constexpr f32 sPopupListHeight = 300.f;

		b8 ContainsNoCase(std::string_view text, std::string_view pattern)
		{
			auto equalsNoCase = [](c8 lhs, c8 rhs)
				{
					return std::tolower((u8)lhs) == std::tolower((u8)rhs);
				};

			return std::search(text.begin(), text.end(), pattern.begin(), pattern.end(), equalsNoCase) != text.end();
		}
	}

	void InspectorView::OnInvoke()
	{
		m_worldService = GetContext()->pEngine->RequestService<Engine::WorldService>();
		m_reflSys = GetContext()->pEngine->GetReflectionSystem();

		m_properties.Initialize(GetContext()->pEngine);
	}

	void InspectorView::OnRender(const Engine::EngineFrame& context)
	{
		Engine::EntityHandle entity;
		if (GetContext()->pSelection->Is<Engine::EntityTag>())
			entity = GetContext()->pSelection->Get<Engine::EntityTag>();
		else
			return;

		auto* pCurrScene = m_worldService->GetCurrentWorld();

		const List<Engine::ComponentStorage*>& storages = pCurrScene->GetComponents().GetStorages();

		for (usize i = 0; i < storages.GetCount(); i++)
		{
			Engine::ComponentStorage* pStorage = storages[i];

			if (!pStorage->Contains(entity))
				continue;

			Engine::ComponentTypeId typeId = pStorage->GetComponentTypeId();

			if (typeId == Reflect::TypeOf<Engine::NameComponent>())
				continue;

			Reflect::Type* pType = m_reflSys->GetType(typeId);

			if (!pType)
			{
				Terminal::Error(StringOps::GetName(this), "Component storage has an unregistered type");
				continue;
			}

			const auto* pId = pType->GetCustomAttribute<Engine::ComponentIdAttribute>();
			const std::string& header = pId ? pId->GetDisplayName() : pType->GetName();

			ImGui::PushID((i32)i);

			b8 keepComponent = true;

			ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
			b8 collapsingHead = ImGui::CollapsingHeader(header.c_str(), &keepComponent, ImGuiTreeNodeFlags_DefaultOpen);
			ImGui::PopFont();

			if (!keepComponent)
			{
				pCurrScene->RemoveComponent(entity, typeId);
				ImGui::PopID();
				break;
			}

			if (collapsingHead)
				m_properties.DrawObject(*pType, pStorage->Find(entity));

			ImGui::PopID();
		}

		auto& style = ImGui::GetStyle();
		f32 buttonWidth = ImGui::CalcTextSize(sButtonName.data()).x + style.FramePadding.x * 2.f;
		f32 buttonHeight = ImGui::GetFrameHeight();
		ImVec2 availRegion = ImGui::GetContentRegionAvail();

		f32 offset = (availRegion.x - buttonWidth) * 0.5f;
		if (offset > 0)
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

		f32 offsetY = availRegion.y - buttonHeight - style.WindowPadding.y;
		if (offsetY > 0)
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY);

		if (ImGui::Button(sButtonName.data(), ImVec2(buttonWidth, 0.f)))
		{
			m_searchBuffer.clear();
			m_activeCategory.clear();
			ImGui::OpenPopup(sPopupName.data());
		}

		DrawAddComponentPopup(pCurrScene, entity);
	}

	void InspectorView::OnLibraryRegistered(const Engine::ReflectionLibrary& library)
	{
		m_properties.OnLibraryRegistered(library);
	}

	void InspectorView::OnLibraryUnregistered(const Engine::ReflectionLibrary& library)
	{
		m_properties.OnLibraryUnregistered(library);
	}

	List<InspectorView::ComponentEntry> InspectorView::CollectComponents() const
	{
		List<ComponentEntry> entries;

		List<Reflect::Type*> compTypes = m_reflSys->GetTypeByBase(Reflect::TypeOf<Engine::ComponentObject>());
		for (Reflect::Type* pCompType : compTypes)
		{
			if (pCompType->GetIsAbstract())
				continue;

			if (pCompType->GetTypeId() == Reflect::TypeOf<Engine::NameComponent>())
				continue;

			if (pCompType->GetTypeId() == Reflect::TypeOf<Engine::EditorOnlyComponent>())
				continue;

			const auto* pId = pCompType->GetCustomAttribute<Engine::ComponentIdAttribute>();

			ComponentEntry& entry = entries.EmplaceBack();
			entry.pType = pCompType;
			entry.label = pId ? pId->GetDisplayName() : pCompType->GetName();
			entry.category = pId ? pId->GetDisplayCategory() : std::string();
		}

		entries.Sort([](const ComponentEntry& lhs, const ComponentEntry& rhs)
			{
				return lhs.label < rhs.label;
			});

		return entries;
	}

	void InspectorView::DrawAddComponentPopup(Engine::Scene* pScene, Engine::EntityHandle entity)
	{
		ImGui::SetNextWindowSize(ImVec2(sPopupWidth, 0.f));

		if (!ImGui::BeginPopup(sPopupName.data()))
			return;

		List<ComponentEntry> entries = CollectComponents();

		DrawPopupHeader();

		if (ImGui::IsWindowAppearing())
			ImGui::SetKeyboardFocusHere();

		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::InputTextWithHint("##search", "Search...", &m_searchBuffer);
		ImGui::Separator();

		ImGui::BeginChild("##componentList", ImVec2(0.f, sPopupListHeight));

		if (!m_searchBuffer.empty())
			DrawSearchResults(entries, pScene, entity);
		else if (m_activeCategory.empty())
			DrawCategoryRoot(entries, pScene, entity);
		else
			DrawCategory(entries, pScene, entity);

		ImGui::EndChild();

		ImGui::EndPopup();
	}

	void InspectorView::DrawPopupHeader()
	{
		b8 inCategory = !m_activeCategory.empty() && m_searchBuffer.empty();
		std::string_view title = inCategory ? std::string_view(m_activeCategory) : sRootTitle;

		if (inCategory)
		{
			if (ImGui::ArrowButton("##back", ImGuiDir_Left))
				m_activeCategory.clear();

			ImGui::SameLine();
		}

		f32 titleWidth = ImGui::CalcTextSize(title.data(), title.data() + title.size()).x;
		f32 titleX = (ImGui::GetWindowWidth() - titleWidth) * 0.5f;

		if (titleX > ImGui::GetCursorPosX())
			ImGui::SetCursorPosX(titleX);

		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(title.data(), title.data() + title.size());
	}

	void InspectorView::DrawCategoryRoot(const List<ComponentEntry>& entries, Engine::Scene* pScene, Engine::EntityHandle entity)
	{
		List<std::string> categories;

		for (const ComponentEntry& entry : entries)
		{
			if (entry.category.empty() || categories.Contains(entry.category))
				continue;

			categories.PushBack(entry.category);
		}

		categories.Sort([](const std::string& lhs, const std::string& rhs)
			{
				return lhs < rhs;
			});

		for (const std::string& category : categories)
		{
			if (DrawNavigationRow(category))
				m_activeCategory = category;
		}

		for (const ComponentEntry& entry : entries)
		{
			if (!entry.category.empty())
				continue;

			DrawComponentRow(entry, pScene, entity, false);
		}
	}

	void InspectorView::DrawCategory(const List<ComponentEntry>& entries, Engine::Scene* pScene, Engine::EntityHandle entity)
	{
		for (const ComponentEntry& entry : entries)
		{
			if (entry.category != m_activeCategory)
				continue;

			DrawComponentRow(entry, pScene, entity, false);
		}
	}

	void InspectorView::DrawSearchResults(const List<ComponentEntry>& entries, Engine::Scene* pScene, Engine::EntityHandle entity)
	{
		for (const ComponentEntry& entry : entries)
		{
			if (!ContainsNoCase(entry.label, m_searchBuffer))
				continue;

			DrawComponentRow(entry, pScene, entity, true);
		}
	}

	b8 InspectorView::DrawNavigationRow(const std::string& label)
	{
		ImGui::PushID(label.c_str());

		f32 rowRight = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x;
		b8 clicked = ImGui::Selectable(label.c_str(), false, ImGuiSelectableFlags_NoAutoClosePopups);

		f32 iconWidth = ImGui::CalcTextSize(ICON_FA_CHEVRON_RIGHT).x;
		ImGui::SameLine(rowRight - iconWidth);
		ImGui::TextUnformatted(ICON_FA_CHEVRON_RIGHT);

		ImGui::PopID();

		return clicked;
	}

	void InspectorView::DrawComponentRow(const ComponentEntry& entry, Engine::Scene* pScene, Engine::EntityHandle entity, b8 showCategory)
	{
		ImGui::PushID(entry.pType);

		f32 rowRight = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x;
		b8 ownedByEntt = pScene->HasComponent(entity, entry.pType->GetTypeId());
		b8 clicked = ImGui::Selectable(entry.label.c_str(), false, ownedByEntt ? ImGuiSelectableFlags_Disabled : ImGuiSelectableFlags_None);

		if (showCategory && !entry.category.empty())
		{
			f32 categoryWidth = ImGui::CalcTextSize(entry.category.c_str()).x;
			ImGui::SameLine(rowRight - categoryWidth);
			ImGui::TextDisabled("%s", entry.category.c_str());
		}

		ImGui::PopID();

		if (!clicked)
			return;

		pScene->AddComponent(entity, entry.pType->GetTypeId());

		auto* pNameComp = pScene->FindComponent<Engine::NameComponent>(entity);
		Terminal::Info(StringOps::GetName(this), "{} has been added to {}", entry.label, pNameComp->m_name);
		ImGui::CloseCurrentPopup();
	}
}