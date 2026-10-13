#include "ScriptComponentDrawer.h"

#include <Editor/Components/PropertyRenderer.h>

#include <Engine/Core/Engine.h>
#include <Engine/Game/GameScript.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/World/WorldService.h>
#include <Engine/World/SystemGroup.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <algorithm>
#include <cctype>

namespace Horizon::Editor
{
	b8 ScriptComponentDrawer::OnDraw(const Reflect::Field& field, void* pValue, const PropertyContext& ctx)
	{
		auto* pComp = static_cast<Engine::ScriptComponent*>(pValue);
		auto* pWorld = ctx.pEngine->RequestService<Engine::WorldService>();
		const b8 isEditing = HasFlag(pWorld->GetRunningSystems(), Engine::SystemGroup::Editor);

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(6.0f, 2.0f));

		if (!ImGui::BeginTable("properties", 2, ImGuiTableFlags_SizingFixedFit))
		{
			ImGui::PopStyleVar();
			return false;
		}

		ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

		const b8 changed = DrawPicker(*pComp, ctx, isEditing);

		if (pComp->m_script)
			DrawScriptFields(*pComp, ctx);

		ImGui::EndTable();
		ImGui::PopStyleVar();

		return changed;
	}

	b8 ScriptComponentDrawer::DrawPicker(Engine::ScriptComponent& comp, const PropertyContext& ctx, b8 isEditing)
	{
		const Reflect::Type* pCurrent = comp.m_script ? ctx.pReflection->GetType(comp.m_script->GetTypeId()) : nullptr;
		const std::string preview = pCurrent ? pCurrent->GetName() : "None";

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted("Script");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(-FLT_MIN);

		ImGui::BeginDisabled(!isEditing);

		if (!ImGui::BeginCombo("##script", preview.c_str(), ImGuiComboFlags_HeightLarge))
		{
			ImGui::EndDisabled();
			return false;
		}

		if (ImGui::IsWindowAppearing())
		{
			m_searchFilter.clear();
			ImGui::SetKeyboardFocusHere();
		}

		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::InputTextWithHint("##filter", "Search", &m_searchFilter);

		List<Reflect::Type*> scripts = ctx.pReflection->GetTypeByBase(Reflect::TypeOf<Engine::GameScript>());
		scripts.Sort([](const Reflect::Type* pLeft, const Reflect::Type* pRight)
			{
				return pLeft->GetName() < pRight->GetName();
			});

		b8 changed = false;

		if (ImGui::Selectable("None", pCurrent == nullptr) && pCurrent != nullptr)
		{
			Memory::Allocator::Delete(comp.m_script);
			comp.m_script = nullptr;
			changed = true;
		}

		for (const Reflect::Type* pType : scripts)
		{
			if (!MatchesFilter(pType->GetName(), m_searchFilter))
				continue;

			const b8 isSelected = pType == pCurrent;

			if (!ImGui::Selectable(pType->GetName().c_str(), isSelected) || isSelected)
				continue;

			if (!pType->CanConstruct())
			{
				Terminal::Error(StringOps::GetName(this), "{} is not default constructible", pType->GetName());
				continue;
			}

			Memory::Allocator::Delete(comp.m_script);
			comp.m_script = static_cast<Engine::GameScript*>(pType->Create());
			changed = true;
		}

		ImGui::EndCombo();
		ImGui::EndDisabled();

		return changed;
	}

	void ScriptComponentDrawer::DrawScriptFields(Engine::ScriptComponent& comp, const PropertyContext& ctx)
	{
		const Reflect::Type* pType = ctx.pReflection->GetType(comp.m_script->GetTypeId());

		if (pType == nullptr)
			return;

		ctx.pRenderer->DrawFields(*pType, comp.m_script);
	}

	b8 ScriptComponentDrawer::MatchesFilter(std::string_view name, std::string_view filter)
	{
		if (filter.empty())
			return true;

		auto it = std::search(name.begin(), name.end(), filter.begin(), filter.end(), [](c8 left, c8 right)
			{
				return std::tolower(static_cast<u8>(left)) == std::tolower(static_cast<u8>(right));
			});

		return it != name.end();
	}
}