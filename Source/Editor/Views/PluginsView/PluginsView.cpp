#include "PluginsView.h"

#include <Editor/Project/ProjectContext.h>
#include <Editor/Renderer/EditorContext.h>
#include <Engine/Core/Engine.h>
#include <Engine/Plugin/PluginSystem.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

#include <imgui.h>

namespace Horizon::Editor
{
	void PluginsView::OnInvoke()
	{
		auto* pEngine = GetContext()->pEngine;

		if (!m_pluginSystem)
			m_pluginSystem = pEngine->GetPluginSystem();

		if (!m_projectContext)
			m_projectContext = pEngine->RequestContext<ProjectContext>();

		if (!m_pluginSystem || !m_projectContext)
			Terminal::Error(StringOps::GetName(this), "Plugin service or project context is unavailable");
	}

	void PluginsView::OnRender(const Engine::EngineFrame& context)
	{
		if (!m_pluginSystem || !m_projectContext)
		{
			ImGui::TextDisabled("Plugin service or project context is unavailable");
			return;
		}

		if (ImGui::Button(ICON_FA_ROTATE " Refresh"))
			m_pluginSystem->DiscoverPlugins();

		ImGui::SameLine();
		ImGui::BeginDisabled();
		ImGui::Button(ICON_FA_FILE_IMPORT " Import...");
		ImGui::EndDisabled();
		ImGui::SetItemTooltip("Needs a file dialog, coming later");

		ImGui::Separator();

		const ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY;

		if (!ImGui::BeginTable("##Plugins", 6, flags))
			return;

		ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 24.f);
		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableSetupColumn("Origin", ImGuiTableColumnFlags_WidthFixed, 70.f);
		ImGui::TableSetupColumn("State", ImGuiTableColumnFlags_WidthFixed, 80.f);
		ImGui::TableSetupColumn("Types", ImGuiTableColumnFlags_WidthFixed, 50.f);
		ImGui::TableSetupColumn("Details", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableHeadersRow();

		auto* pReflect = GetContext()->pEngine->GetReflectionSystem();

		for (const Engine::PluginEntry& entry : m_pluginSystem->GetPlugins())
		{
			ImGui::PushID(entry.name.c_str());
			ImGui::TableNextRow();

			ImGui::TableNextColumn();
			b8 enabled = m_projectContext->IsPluginEnabled(entry.name);

			if (ImGui::Checkbox("##Enabled", &enabled))
			{
				m_projectContext->SetPluginEnabled(entry.name, enabled);

				if (enabled)
					m_pluginSystem->RequestLoad(entry.name);
				else
					m_pluginSystem->RequestUnload(entry.name);
			}

			ImGui::TableNextColumn();
			ImGui::TextUnformatted(entry.name.c_str());

			ImGui::TableNextColumn();
			ImGui::TextUnformatted(entry.origin == Engine::PluginOrigin::Engine ? "Engine" : "Project");

			ImGui::TableNextColumn();

			switch (entry.state)
			{
			case Engine::PluginState::Loaded:
				ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.f), "Loaded");
				break;
			case Engine::PluginState::Failed:
				ImGui::TextColored(ImVec4(0.9f, 0.35f, 0.35f, 1.f), "Failed");
				break;
			default:
				ImGui::TextDisabled("Available");
				break;
			}

			ImGui::TableNextColumn();

			const Engine::ReflectionLibrary* pLibrary = entry.pLibrary ? pReflect->FindLibrary(entry.pLibrary) : nullptr;

			if (pLibrary)
				ImGui::Text("%llu", static_cast<u64>(pLibrary->types.GetCount()));
			else
				ImGui::TextDisabled("-");

			ImGui::TableNextColumn();

			if (entry.state == Engine::PluginState::Failed)
				ImGui::TextColored(ImVec4(0.9f, 0.35f, 0.35f, 1.f), "%s", entry.failReason.c_str());
			else
				ImGui::TextDisabled("%s", entry.directory.c_str());

			ImGui::PopID();
		}

		ImGui::EndTable();
	}

	void PluginsView::OnLibraryRegistered(const Engine::ReflectionLibrary& library)
	{
	}

	void PluginsView::OnLibraryUnregistered(const Engine::ReflectionLibrary& library)
	{
	}
}