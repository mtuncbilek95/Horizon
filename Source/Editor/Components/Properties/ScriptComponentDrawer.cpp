#include "ScriptComponentDrawer.h"

#include <Editor/Components/PropertyRenderer.h>
#include <Editor/Components/Helpers/FloatWidget.h>
#include <Editor/Components/Helpers/IntegerWidget.h>
#include <Editor/Domain/DomainService.h>

#include <Engine/Asset/AssetService.h>
#include <Engine/Asset/Script/ScriptAsset.h>
#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/World/WorldService.h>
#include <Engine/World/SystemGroup.h>

#include <Runtime/Containers/StringOps.h>
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
		const Reflect::Type* pType = ctx.pReflection->GetType(Reflect::TypeOf<Engine::ScriptComponent>());

		if (pType == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "ScriptComponent is not registered in the reflection system");
			return false;
		}

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(6.0f, 2.0f));

		if (!ImGui::BeginTable("properties", 2, ImGuiTableFlags_SizingFixedFit))
		{
			ImGui::PopStyleVar();
			return false;
		}

		ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

		ctx.pRenderer->DrawFields(*pType, pValue);

		const b8 changed = DrawScript(*pComp, ctx);

		ImGui::EndTable();
		ImGui::PopStyleVar();

		return changed;
	}

	b8 ScriptComponentDrawer::DrawScript(Engine::ScriptComponent& comp, const PropertyContext& ctx)
	{
		const Guid& id = comp.m_script.GetId();

		if (!id.IsValid())
			return false;

		auto* pAssets = ctx.pEngine->RequestService<Engine::AssetService>();
		auto* pWorld = ctx.pEngine->RequestService<Engine::WorldService>();

		const b8 isEditing = HasFlag(pWorld->GetRunningSystems(), Engine::SystemGroup::Editor);

		if (!pAssets->HasAsset(id))
		{
			DrawStatusRow("Fields", "Script is not registered yet");
			return false;
		}

		auto* pAsset = pAssets->FindAsset<Engine::ScriptAsset>(id);

		if (isEditing && pAsset->IsStale())
			pAsset->ReloadAsync();
		else
			pAsset->LoadAsync();

		const Engine::AssetResidency residency = pAsset->GetResidencyState();

		if (residency == Engine::AssetResidency::Failed)
		{
			DrawStatusRow("Fields", "Script failed to load, see console");
			return false;
		}

		if (residency != Engine::AssetResidency::Resident)
		{
			DrawStatusRow("Fields", "Loading...");
			return false;
		}

		auto* pDomain = ctx.pEngine->RequestService<DomainService>();
		Engine::Scene* pScene = pWorld->GetCurrentWorld();
		const List<Engine::ScriptField>& fields = pAsset->GetFields();

		if (isEditing)
			PruneStale(comp, fields);

		b8 changed = false;

		for (const Engine::ScriptField& scriptField : fields)
		{
			if (scriptField.kind == Engine::ScriptFieldKind::Quaternion)
				continue;

			ImGui::PushID(scriptField.name.c_str());

			if (isEditing)
				changed |= DrawEditField(comp, scriptField, pDomain, pScene);
			else
				changed |= DrawLiveField(comp, scriptField, pDomain, pScene);

			ImGui::PopID();
		}

		return changed;
	}

	b8 ScriptComponentDrawer::DrawEditField(Engine::ScriptComponent& comp, const Engine::ScriptField& field, DomainService* pDomain, Engine::Scene* pScene)
	{
		const Engine::ScriptFieldOverride* pOverride = FindOverride(comp, field.name);
		const b8 hasOverride = pOverride != nullptr && pOverride->m_kind == field.kind;

		Engine::ScriptFieldValue value = hasOverride ? pOverride->m_value : field.defaultValue;
		Engine::EntityHandle entity = FindEntityRef(comp, field.name);

		const b8 isEntity = field.kind == Engine::ScriptFieldKind::Entity;
		const b8 isOverridden = isEntity ? entity.IsValid() : hasOverride;

		DrawLabel(field, isOverridden);

		b8 changed = false;

		if (isOverridden && !field.readOnly && ImGui::BeginPopupContextItem("##fieldContext"))
		{
			if (ImGui::MenuItem("Reset to Default"))
			{
				if (isEntity)
					ResetEntityRef(comp, field.name);
				else
					ResetOverride(comp, field.name);

				changed = true;
			}

			ImGui::EndPopup();
		}

		ImGui::TableNextColumn();

		if (field.readOnly)
			ImGui::BeginDisabled();

		const b8 edited = DrawValue(field, value, entity, pDomain, pScene);

		if (field.readOnly)
			ImGui::EndDisabled();

		if (!edited || changed)
			return changed;

		if (!isEntity)
		{
			SetOverride(comp, field).m_value = value;
			return true;
		}

		if (entity.IsValid())
			SetEntityRef(comp, field.name, entity);
		else
			ResetEntityRef(comp, field.name);

		return true;
	}

	b8 ScriptComponentDrawer::DrawLiveField(Engine::ScriptComponent& comp, const Engine::ScriptField& field, DomainService* pDomain, Engine::Scene* pScene)
	{
		Engine::ScriptLiveValue* pLive = FindLiveValue(comp, field.name);

		DrawLabel(field, false);
		ImGui::TableNextColumn();

		if (pLive == nullptr || pLive->kind != field.kind)
		{
			ImGui::TextDisabled("Waiting for script");
			return false;
		}

		if (field.readOnly)
			ImGui::BeginDisabled();

		const b8 edited = DrawValue(field, pLive->value, pLive->entity, pDomain, pScene);

		if (field.readOnly)
			ImGui::EndDisabled();

		if (edited)
			pLive->dirty = true;

		return edited;
	}

	b8 ScriptComponentDrawer::DrawValue(const Engine::ScriptField& field, Engine::ScriptFieldValue& value, Engine::EntityHandle& entity, DomainService* pDomain, Engine::Scene* pScene)
	{
		switch (field.kind)
		{
		case Engine::ScriptFieldKind::Bool:
		{
			bool checked = value.boolValue != 0;

			if (!ImGui::Checkbox("##value", &checked))
				return false;

			value.boolValue = checked;
			return true;
		}
		case Engine::ScriptFieldKind::Integer:
			return IntegerWidget<i64, ImGuiDataType_S64>::Draw(Reflect::Field(), &value.integerValue);
		case Engine::ScriptFieldKind::Number:
		{
			ImGui::SetNextItemWidth(-FLT_MIN);

			if (field.hasRange)
				return ImGui::SliderScalar("##value", ImGuiDataType_Double, &value.numberValue, &field.min, &field.max, "%.3f", ImGuiSliderFlags_AlwaysClamp);

			return ImGui::DragScalar("##value", ImGuiDataType_Double, &value.numberValue, 0.01f, nullptr, nullptr, "%.3f");
		}
		case Engine::ScriptFieldKind::String:
		{
			ImGui::SetNextItemWidth(-FLT_MIN);
			return ImGui::InputText("##value", &value.stringValue);
		}
		case Engine::ScriptFieldKind::Vector2:
		{
			f32 arr[2];
			value.vec2Value.Store(arr);

			if (!FloatWidget::Draw(Reflect::Field(), arr, 2))
				return false;

			value.vec2Value.Set(arr[0], arr[1]);
			return true;
		}
		case Engine::ScriptFieldKind::Vector3:
		{
			f32 arr[3];
			value.vec3Value.Store(arr);

			if (!FloatWidget::Draw(Reflect::Field(), arr, 3))
				return false;

			value.vec3Value.Set(arr[0], arr[1], arr[2]);
			return true;
		}
		case Engine::ScriptFieldKind::Asset:
			return m_assetPicker.Draw(&value.assetValue, field.assetTypeName, pDomain);
		case Engine::ScriptFieldKind::Entity:
			return m_entityPicker.Draw(&entity, pScene);
		default:
			return false;
		}
	}

	void ScriptComponentDrawer::DrawLabel(const Engine::ScriptField& field, b8 isOverridden)
	{
		std::string label = PropertyRenderer::ToDisplayLabel(field.name);

		if (!label.empty())
			label[0] = static_cast<c8>(std::toupper(static_cast<u8>(label[0])));

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::AlignTextToFramePadding();

		if (isOverridden)
			ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

		ImGui::TextUnformatted(label.c_str());

		if (isOverridden)
			ImGui::PopFont();
	}

	void ScriptComponentDrawer::DrawStatusRow(const c8* pLabel, const c8* pText)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(pLabel);
		ImGui::TableNextColumn();
		ImGui::TextDisabled("%s", pText);
	}

	const Engine::ScriptField* ScriptComponentDrawer::FindField(const List<Engine::ScriptField>& fields, std::string_view name)
	{
		for (const Engine::ScriptField& field : fields)
		{
			if (field.name == name)
				return &field;
		}

		return nullptr;
	}

	Engine::ScriptFieldOverride* ScriptComponentDrawer::FindOverride(Engine::ScriptComponent& comp, std::string_view name)
	{
		for (Engine::ScriptFieldOverride& entry : comp.m_overrides)
		{
			if (entry.m_name == name)
				return &entry;
		}

		return nullptr;
	}

	Engine::ScriptFieldOverride& ScriptComponentDrawer::SetOverride(Engine::ScriptComponent& comp, const Engine::ScriptField& field)
	{
		if (Engine::ScriptFieldOverride* pEntry = FindOverride(comp, field.name))
		{
			if (pEntry->m_kind != field.kind)
			{
				pEntry->m_kind = field.kind;
				pEntry->m_value = field.defaultValue;
			}

			return *pEntry;
		}

		Engine::ScriptFieldOverride& entry = comp.m_overrides.EmplaceBack();
		entry.m_name = field.name;
		entry.m_kind = field.kind;
		entry.m_value = field.defaultValue;
		return entry;
	}

	void ScriptComponentDrawer::ResetOverride(Engine::ScriptComponent& comp, std::string_view name)
	{
		for (usize i = 0; i < comp.m_overrides.GetCount(); ++i)
		{
			if (comp.m_overrides[i].m_name != name)
				continue;

			comp.m_overrides.RemoveAt(i);
			return;
		}
	}

	Engine::EntityHandle ScriptComponentDrawer::FindEntityRef(const Engine::ScriptComponent& comp, std::string_view name)
	{
		const usize count = std::min(comp.m_entityNames.GetCount(), comp.m_entityRefs.GetCount());

		for (usize i = 0; i < count; ++i)
		{
			if (comp.m_entityNames[i] == name)
				return comp.m_entityRefs[i];
		}

		return Engine::EntityHandle();
	}

	void ScriptComponentDrawer::SetEntityRef(Engine::ScriptComponent& comp, std::string_view name, Engine::EntityHandle entity)
	{
		const usize count = std::min(comp.m_entityNames.GetCount(), comp.m_entityRefs.GetCount());

		for (usize i = 0; i < count; ++i)
		{
			if (comp.m_entityNames[i] != name)
				continue;

			comp.m_entityRefs[i] = entity;
			return;
		}

		comp.m_entityNames.EmplaceBack(name);
		comp.m_entityRefs.PushBack(entity);
	}

	void ScriptComponentDrawer::ResetEntityRef(Engine::ScriptComponent& comp, std::string_view name)
	{
		const usize count = std::min(comp.m_entityNames.GetCount(), comp.m_entityRefs.GetCount());

		for (usize i = 0; i < count; ++i)
		{
			if (comp.m_entityNames[i] != name)
				continue;

			comp.m_entityNames.RemoveAt(i);
			comp.m_entityRefs.RemoveAt(i);
			return;
		}
	}

	Engine::ScriptLiveValue* ScriptComponentDrawer::FindLiveValue(Engine::ScriptComponent& comp, std::string_view name)
	{
		for (Engine::ScriptLiveValue& entry : comp.m_liveValues)
		{
			if (entry.name == name)
				return &entry;
		}

		return nullptr;
	}

	void ScriptComponentDrawer::PruneStale(Engine::ScriptComponent& comp, const List<Engine::ScriptField>& fields)
	{
		for (usize i = comp.m_overrides.GetCount(); i > 0; --i)
		{
			const Engine::ScriptFieldOverride& entry = comp.m_overrides[i - 1];
			const Engine::ScriptField* pField = FindField(fields, entry.m_name);

			if (pField == nullptr || pField->kind != entry.m_kind)
				comp.m_overrides.RemoveAt(i - 1);
		}

		for (usize i = comp.m_entityNames.GetCount(); i > 0; --i)
		{
			const Engine::ScriptField* pField = FindField(fields, comp.m_entityNames[i - 1]);

			if (pField != nullptr && pField->kind == Engine::ScriptFieldKind::Entity)
				continue;

			comp.m_entityNames.RemoveAt(i - 1);

			if (i - 1 < comp.m_entityRefs.GetCount())
				comp.m_entityRefs.RemoveAt(i - 1);
		}
	}
}