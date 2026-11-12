#include "MeshPreviewView.h"

#include <Editor/Domain/DomainFile.h>
#include <Editor/Domain/DomainService.h>
#include <Editor/ImportPipeline/Mesh/MeshAssetImportSettings.h>
#include <Editor/Renderer/EditorContext.h>

#include <Engine/Asset/AssetService.h>
#include <Engine/Asset/Material/Material.h>
#include <Engine/Asset/Mesh/Mesh.h>
#include <Engine/Core/Engine.h>
#include <Engine/Reflection/ReflectionSystem.h>

#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>

#include <imgui.h>

namespace Horizon::Editor
{
	void MeshPreviewView::OnInvoke()
	{
		m_assetService = GetContext()->pEngine->RequestService<Engine::AssetService>();
		if (!m_assetService)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "AssetService is missing, mesh slots will not be displayed");
			return;
		}

		m_domain = GetContext()->pEngine->RequestService<DomainService>();
		if (!m_domain)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "DomainService is missing, mesh slots will not be displayed");
			return;
		}

		m_materialType = GetContext()->pEngine->GetReflectionSystem()->GetType(Reflect::TypeOf<Asset::Material>());
		if (!m_materialType)
			Terminal::Error(StringHelper::GetTypeName(this), "Asset::Material is not a reflected type, mesh slots will not be displayed");
	}

	void MeshPreviewView::OnRender(const Engine::EngineFrame& context)
	{
		if (!m_assetService || !m_domain || !m_materialType)
			return;

		DomainFile* pFile = GetConnectedFile();
		if (!pFile)
			return;

		auto* pAsset = m_assetService->FindAsset<Asset::Mesh>(pFile->GetID());
		if (!pAsset)
		{
			ImGui::TextUnformatted("Mesh asset is not registered");
			return;
		}

		const Engine::AssetResidency residency = pAsset->GetResidency();
		if (residency == Engine::AssetResidency::Unloaded)
			pAsset->LoadAsync();

		if (residency == Engine::AssetResidency::Failed)
		{
			ImGui::TextUnformatted("Mesh could not be loaded");
			return;
		}

		if (residency != Engine::AssetResidency::Resident)
		{
			ImGui::TextUnformatted("Loading...");
			return;
		}

		List<Engine::AssetHandle<Asset::Material>>& slots = pAsset->GetMaterialSlots();
		if (slots.GetCount() == 0)
		{
			ImGui::TextUnformatted("Mesh has no material slots, reimport it to generate them");
			return;
		}

		if (ImGui::Button("Save"))
			SaveSlots();

		ImGui::SeparatorText("Material Slots");

		if (!ImGui::BeginTable("slots", 2, ImGuiTableFlags_SizingFixedFit))
			return;

		ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

		for (usize i = 0; i < slots.GetCount(); ++i)
		{
			ImGui::PushID(static_cast<i32>(i));

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Slot %u", static_cast<u32>(i));

			ImGui::TableNextColumn();

			Guid id = slots[i].GetId();
			if (m_picker.Draw(&id, m_materialType, m_domain))
				slots[i].SetId(id);

			ImGui::PopID();
		}

		ImGui::EndTable();
	}

	b8 MeshPreviewView::OnCommand(ViewCommand command)
	{
		if (command != ViewCommand::Save)
			return false;

		return SaveSlots();
	}

	b8 MeshPreviewView::SaveSlots()
	{
		if (!m_assetService || !m_domain || !m_connectedFile)
			return false;

		auto* pAsset = m_assetService->FindAsset<Asset::Mesh>(m_connectedFile->GetID());
		if (!pAsset || pAsset->GetResidency() != Engine::AssetResidency::Resident)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} is not loaded, slots cannot be saved", m_connectedFile->GetName());
			return false;
		}

		auto* pSettings = static_cast<MeshAssetImportSettings*>(m_connectedFile->GetSettings());
		if (!pSettings)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} has no import settings", m_connectedFile->GetName());
			return false;
		}

		const List<Engine::AssetHandle<Asset::Material>>& slots = pAsset->GetMaterialSlots();

		pSettings->m_materialSlots.Clear();

		for (usize i = 0; i < slots.GetCount(); ++i)
			pSettings->m_materialSlots.PushBack(Engine::AssetHandle<Asset::Material>(slots[i].GetId()));

		return m_domain->SaveSettings(m_connectedFile);
	}
}
