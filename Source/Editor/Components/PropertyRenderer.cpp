#include "PropertyRenderer.h"

#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionSystem.h>

#include <Runtime/Log/Terminal.h>
#include <Runtime/RTTR/Attributes/HeaderAttribute.h>
#include <Runtime/RTTR/Attributes/HideInInspectorAttribute.h>
#include <Runtime/RTTR/Attributes/TooltipAttribute.h>
#include <Runtime/RTTR/Attributes/ReadOnlyAttribute.h>

#include <imgui.h>

namespace Horizon::Editor
{
	PropertyRenderer::~PropertyRenderer()
	{
		for (PropertyDrawer* pDrawer : m_drawers)
			Memory::Allocator::Delete(pDrawer);
	}

	void PropertyRenderer::Initialize(Engine::Engine* pEngine)
	{
		m_context.pEngine = pEngine;
		m_context.pReflection = pEngine->GetReflectionSystem();

		List<Reflect::Type*> drawerTypes = m_context.pReflection->GetTypeByBase(Reflect::TypeOf<PropertyDrawer>());

		for (Reflect::Type* pType : drawerTypes)
		{
			if (pType->GetIsAbstract())
				continue;

			auto* pDrawer = static_cast<PropertyDrawer*>(pType->Create());
			Reflect::TypeHandle target = pDrawer->GetTargetType();

			if (m_drawerLookups.contains(target))
			{
				Terminal::Warn("PropertyRenderer", "{} targets a type that already has a drawer, skipped", pType->GetName());
				Memory::Allocator::Delete(pDrawer);
				continue;
			}

			m_drawerLookups[target] = m_drawers.GetCount();
			m_drawers.PushBack(pDrawer);
		}
	}

	void PropertyRenderer::DrawObject(const Reflect::Type& type, void* pInstance)
	{
		if (PropertyDrawer* pWhole = FindDrawer(type.GetTypeId()))
		{
			pWhole->OnDraw(Reflect::Field(), pInstance, m_context);
			return;
		}

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(6.0f, 2.0f));

		if (!ImGui::BeginTable("properties", 2, ImGuiTableFlags_SizingFixedFit))
		{
			ImGui::PopStyleVar();
			return;
		}

		ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

		DrawFields(type, pInstance);

		ImGui::EndTable();
		ImGui::PopStyleVar();
	}

	void PropertyRenderer::DrawFields(const Reflect::Type& type, void* pInstance)
	{
		for (const Reflect::Field& field : type.GetFields())
		{
			if (field.GetCustomAttribute<Reflect::HideInInspectorAttribute>())
				continue;

			if (const auto* pHeader = field.GetCustomAttribute<Reflect::HeaderAttribute>())
				DrawHeaderRow(pHeader->GetHeaderName());

			DrawField(field, field.GetValue(pInstance));
		}
	}

	void PropertyRenderer::DrawField(const Reflect::Field& field, void* pValue)
	{
		PropertyDrawer* pDrawer = nullptr;

		if (field.GetMode() == Reflect::TypeMode::Compose)
		{
			if (field.GetKind() == Reflect::TypeKind::Enum)
				pDrawer = FindDrawer(Reflect::TypeOf<Reflect::EnumValue>());
			else
				pDrawer = FindDrawer(field.GetTypeId());

			if (!pDrawer && field.GetKind() == Reflect::TypeKind::Object)
			{
				DrawNested(field, pValue);
				return;
			}
		}

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(ToDisplayLabel(field.GetName()).c_str());

		if (const auto* pTooltip = field.GetCustomAttribute<Reflect::TooltipAttribute>())
		{
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s", pTooltip->GetTooltip().c_str());
		}

		ImGui::TableNextColumn();
		ImGui::PushID(field.GetName().c_str());

		const b8 readOnly = field.GetCustomAttribute<Reflect::ReadOnlyAttribute>() != nullptr;

		if (readOnly)
			ImGui::BeginDisabled();

		if (pDrawer)
			pDrawer->OnDraw(field, pValue, m_context);
		else if (field.GetMode() == Reflect::TypeMode::Array)
			ImGui::TextDisabled("array");
		else if (field.GetMode() == Reflect::TypeMode::Pointer)
			ImGui::TextDisabled("pointer");
		else
			ImGui::TextDisabled("no drawer");

		if (readOnly)
			ImGui::EndDisabled();

		ImGui::PopID();
	}

	void PropertyRenderer::DrawNested(const Reflect::Field& field, void* pValue)
	{
		Reflect::Type* pNested = m_context.pReflection->GetType(field.GetTypeId());

		if (!pNested)
		{
			Terminal::Error("PropertyRenderer", "{} nested type is not registered", field.GetName());
			return;
		}

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::PushID(field.GetName().c_str());

		const b8 open = ImGui::TreeNodeEx(ToDisplayLabel(field.GetName()).c_str(), ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_DefaultOpen);

		if (const auto* pTooltip = field.GetCustomAttribute<Reflect::TooltipAttribute>())
		{
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s", pTooltip->GetTooltip().c_str());
		}

		ImGui::TableNextColumn();

		if (open)
		{
			DrawFields(*pNested, pValue);
			ImGui::TreePop();
		}

		ImGui::PopID();
	}

	void PropertyRenderer::DrawHeaderRow(const std::string& header)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::TextDisabled("%s", header.c_str());
		ImGui::TableNextColumn();
	}

	PropertyDrawer* PropertyRenderer::FindDrawer(Reflect::TypeHandle handle) const
	{
		auto it = m_drawerLookups.find(handle);

		if (it == m_drawerLookups.end())
			return nullptr;

		return m_drawers[it->second];
	}

	std::string PropertyRenderer::ToDisplayLabel(const std::string& fieldName)
	{
		std::string label;
		label.reserve(fieldName.size() + 4);

		for (usize i = 0; i < fieldName.size(); i++)
		{
			const c8 ch = fieldName[i];

			if (i > 0 && ch >= 'A' && ch <= 'Z')
			{
				const c8 prev = fieldName[i - 1];

				if ((prev >= 'a' && prev <= 'z') || (prev >= '0' && prev <= '9'))
					label.push_back(' ');
			}

			label.push_back(ch);
		}

		return label;
	}
}