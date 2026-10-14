#include "ProjectSettingsView.h"

#include <Editor/Views/ProjectSettingsView/ProjectSection.h>
#include <Editor/Views/ProjectSettingsView/ProjectSectionAttribute.h>
#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionLibrary.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>

#include <imgui.h>

namespace Horizon::Editor
{
	namespace
	{
		constexpr f32 kSectionListWidth = 220.0f;
	}

	ProjectSettingsView::~ProjectSettingsView()
	{
		ClearSections();
	}

	void ProjectSettingsView::OnInvoke()
	{
		auto* pEngine = GetContext()->pEngine;

		List<Reflect::Type*> types = pEngine->GetReflectionSystem()->GetTypeByBase(Reflect::TypeOf<ProjectSection>());
		for (auto* pType : types)
			AddSectionType(pType);
	}

	void ProjectSettingsView::OnRender(const Engine::EngineFrame& context)
	{
		if (m_sections.IsEmpty())
		{
			ImGui::TextDisabled("No project sections are registered");
			return;
		}

		RenderSectionList();
		ImGui::SameLine();
		RenderSelectedSection(context);
	}

	void ProjectSettingsView::OnLibraryRegistered(const Engine::ReflectionLibrary& library)
	{
		for (const Reflect::Type& type : library.types)
			AddSectionType(&type);
	}

	void ProjectSettingsView::OnLibraryUnregistered(const Engine::ReflectionLibrary& library)
	{
		for (const Reflect::Type& type : library.types)
			RemoveSectionType(&type);
	}

	b8 ProjectSettingsView::AddSectionType(const Reflect::Type* pNewSection)
	{
		if (pNewSection->GetBaseId() != Reflect::TypeOf<ProjectSection>())
			return false;

		if (pNewSection->GetIsAbstract())
			return false;

		auto* pAttr = pNewSection->GetCustomAttribute<ProjectSectionAttribute>();
		if (!pAttr)
		{
			Terminal::Error(StringOps::GetName(this), "{} needs ProjectSectionAttribute to be added. Skipping it..", pNewSection->GetName());
			return false;
		}

		auto* pSection = static_cast<ProjectSection*>(pNewSection->Create());
		if (!pSection)
		{
			Terminal::Error(StringOps::GetName(this), "{} could not be created. Skipping it..", pNewSection->GetName());
			return false;
		}

		pSection->Initialize(GetContext());

		SectionEntry entry = {};
		entry.pSection = pSection;
		entry.pType = pNewSection;
		entry.displayName = pAttr->GetDisplayName();

		usize insertIndex = m_sections.GetCount();
		for (usize i = 0; i < m_sections.GetCount(); i++)
		{
			if (entry.order < m_sections[i].order)
			{
				insertIndex = i;
				break;
			}
		}

		m_sections.PushAt(insertIndex, entry);

		if (insertIndex <= m_selectedIndex && m_sections.GetCount() > 1)
			m_selectedIndex++;

		Terminal::Info(StringOps::GetName(this), "{} has been registered", pNewSection->GetName());
		return true;
	}

	b8 ProjectSettingsView::RemoveSectionType(const Reflect::Type* pType)
	{
		for (usize i = m_sections.GetCount(); i > 0; i--)
		{
			SectionEntry& entry = m_sections[i - 1];

			if (entry.pType != pType)
				continue;

			entry.pSection->OnFinalize();
			Memory::Allocator::Delete(entry.pSection);
			m_sections.RemoveAt(i - 1);

			if (m_selectedIndex >= m_sections.GetCount() && m_selectedIndex > 0)
				m_selectedIndex = m_sections.GetCount() - 1;

			return true;
		}

		return false;
	}

	void ProjectSettingsView::ClearSections()
	{
		for (SectionEntry& entry : m_sections)
		{
			entry.pSection->OnFinalize();
			Memory::Allocator::Delete(entry.pSection);
		}

		m_sections.Clear();
		m_selectedIndex = 0;
	}

	void ProjectSettingsView::RenderSectionList()
	{
		if (ImGui::BeginChild("##ProjectSections", ImVec2(kSectionListWidth, 0.0f), ImGuiChildFlags_Borders))
		{
			for (usize i = 0; i < m_sections.GetCount(); i++)
			{
				const b8 selected = i == m_selectedIndex;

				if (ImGui::Selectable(m_sections[i].displayName.c_str(), selected))
					m_selectedIndex = i;
			}
		}

		ImGui::EndChild();
	}

	void ProjectSettingsView::RenderSelectedSection(const Engine::EngineFrame& context)
	{
		if (ImGui::BeginChild("##ProjectSectionContent", ImVec2(0.0f, 0.0f), ImGuiChildFlags_None))
		{
			if (m_selectedIndex < m_sections.GetCount())
			{
				SectionEntry& entry = m_sections[m_selectedIndex];

				ImGui::SeparatorText(entry.displayName.c_str());
				entry.pSection->OnRender(context);
			}
			else
				ImGui::TextDisabled("Select a section");
		}

		ImGui::EndChild();
	}
}