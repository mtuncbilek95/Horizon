#include "AssetContentView.h"

#include <Editor/AssetAction/ActionTypeAttribute.h>
#include <Editor/Renderer/EditorContext.h>
#include <Editor/Domain/DomainService.h>
#include <Editor/Font/IconsMaterialDesignIcons.h>
#include <Engine/Asset/AssetService.h>
#include <Engine/Asset/Texture/Texture.h>
#include <Engine/Core/Engine.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/File/Directory.h>
#include <Runtime/PAL/File/File.h>
#include <Runtime/RHI/Device/Device.h>
#include <Runtime/RHI/Texture/Texture.h>
#include <Runtime/RHI/Texture/TextureView.h>

#include <imgui_internal.h>
#include <map>

namespace Horizon::Editor
{
	namespace
	{
		ImGuiID HashPath(const std::string& path)
		{
			return static_cast<ImGuiID>(std::hash<std::string>{}(path));
		}

		std::string TruncateToWidth(const std::string& text, f32 maxWidth)
		{
			if (ImGui::CalcTextSize(text.c_str()).x <= maxWidth)
				return text;

			const f32 ellipsisWidth = ImGui::CalcTextSize("...").x;

			std::string result;
			f32 width = 0.f;

			for (const c8& character : text)
			{
				const f32 charWidth = ImGui::CalcTextSize(&character, &character + 1).x;

				if (width + charWidth + ellipsisWidth > maxWidth)
					break;

				result.push_back(character);
				width += charWidth;
			}

			return result + "...";
		}

		std::string DomainToAssetIcon(Reflect::Type* pAssetType)
		{
			static std::map<std::string, std::string> nameMap =
			{
				{ "Asset::Texture",			ICON_MDI_CHECKERBOARD },
				{ "Asset::Mesh",			ICON_FA_CUBES },
				{ "Asset::Scene",			ICON_FA_EARTH_AMERICAS },
				{ "Asset::Audio",			ICON_MDI_WAVEFORM },
				{ "Asset::Material",		ICON_MDI_SPHERE },
				{ "Asset::Shader",			ICON_MDI_LAYERS_TRIPLE_OUTLINE },
				{ "Asset::Prefab",			ICON_MDI_CUBE_OUTLINE },
				{ "Asset::Animation",		ICON_MDI_RUN },
				{ "Asset::Font",			ICON_MDI_FORMAT_FONT },
				{ "Asset::InputActionMap",  ICON_FA_GAMEPAD }
			};

			if (!pAssetType)
				return ICON_FA_FILE;

			auto it = nameMap.find(pAssetType->GetFullName());
			if (it == nameMap.end())
				return ICON_FA_FILE;

			return it->second;
		}
	}

	const std::string& AssetContentView::BrowserEntry::GetPath() const
	{
		return pFolder ? pFolder->GetAbsolutePath() : pFile->GetSourcePath();
	}

	const std::string& AssetContentView::BrowserEntry::GetName() const
	{
		if (pFolder)
			return pFolder->GetName();

		return pFile->GetName();
	}

	AssetContentView::~AssetContentView()
	{
		if (m_assetService && m_device)
			ReleaseThumbnails();
	}

	void AssetContentView::OnInvoke()
	{
		auto* pDomain = GetContext()->pEngine->RequestService<DomainService>();
		m_currentFolder = pDomain->GetRoot();

		auto* pReflect = GetContext()->pEngine->GetReflectionSystem();

		for (const Reflect::Type* pType : pReflect->GetTypeByBase(Reflect::TypeOf<AssetAction>()))
			AddAction(pType);

		m_contextMenu.BootstrapContext(GetContext()->pEngine, "ContentBrowserView");

		m_assetService = GetContext()->pEngine->RequestService<Engine::AssetService>();
		if (!m_assetService)
			Terminal::Error(StringHelper::GetTypeName(this), "AssetService is missing, texture thumbnails will not be displayed");

		auto* pGraphics = GetContext()->pEngine->RequestContext<Engine::GraphicsContext>();
		if (!pGraphics)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "GraphicsContext is missing, texture thumbnails will not be displayed");
			return;
		}

		m_device = pGraphics->GetDevice();
	}

	b8 AssetContentView::AddAction(const Reflect::Type* pType)
	{
		auto* pAttr = pType->GetCustomAttribute<ActionTypeAttribute>();

		if (!pAttr)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Can't use AssetActions without ActionTypeAttribute");
			return false;
		}

		RefPtr<AssetAction> pAction = MakeRef<AssetAction>(pType);

		if (!pAction)
		{
			Terminal::Warn(StringHelper::GetTypeName(this), "Forgetting to implement a virtual function can cause the previous error!");
			return false;
		}

		auto* pReflect = GetContext()->pEngine->GetReflectionSystem();
		Reflect::Type* pAssetType = pReflect->GetType(pAttr->GetAssetType());

		if (!pAssetType)
			return false;

		m_actionNameLookup[pAssetType->GetName()] = pAction;
		m_openActions.EmplaceBack(pType, pAction);

		Terminal::Info(StringHelper::GetTypeName(this), "{} has been registered!", pType->GetName());
		return true;
	}

	void AssetContentView::RemoveAction(const Reflect::Type* pType)
	{
		for (usize i = m_openActions.GetCount(); i > 0; --i)
		{
			ActionEntry& entry = m_openActions[i - 1];

			if (entry.pType != pType)
				continue;

			std::erase_if(m_actionNameLookup, [&](const auto& pair) { return pair.second == entry.pAction; });
			m_openActions.RemoveAt(i - 1);
		}
	}

	void AssetContentView::OnRender(const Engine::EngineFrame& context)
	{
		if (m_releaseThumbnails)
		{
			ReleaseThumbnails();
			m_releaseThumbnails = false;
		}

		if (m_currentFolder == nullptr)
		{
			ImGui::TextDisabled("Root folder is unavailable");
			return;
		}

		if (!GetContext()->upcomingFiles.IsEmpty())
			GetContext()->upcomingFiles.Clear();

		DomainFolder* pFolder = m_currentFolder;

		RenderToolbar(pFolder);
		ImGui::Separator();

		RebuildEntries(pFolder);

		const f32 cell = ImGui::GetFontSize() * 6.f;
		const f32 step = cell + ImGui::GetStyle().ItemSpacing.x;
		const usize columnCount = static_cast<usize>(ImMax(1, static_cast<i32>(ImGui::GetContentRegionAvail().x / step)));
		const ImVec2 startPos = ImGui::GetCursorScreenPos();

		ImGuiMultiSelectFlags msFlags = ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_SelectOnClickRelease;

		if (!ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
			msFlags |= ImGuiMultiSelectFlags_BoxSelect2d | ImGuiMultiSelectFlags_ClearOnClickVoid;

		ImGuiMultiSelectIO* pMultiIO = ImGui::BeginMultiSelect(msFlags, m_selection.Size, static_cast<i32>(m_entries.GetCount()));

		m_selection.UserData = this;
		m_selection.AdapterIndexToStorageId = [](ImGuiSelectionBasicStorage* pSelf, int index)
			{
				AssetContentView* pView = static_cast<AssetContentView*>(pSelf->UserData);
				return pView->m_entries[static_cast<usize>(index)].id;
			};

		m_selection.ApplyRequests(pMultiIO);

		DomainFolder* pEnterFolder = nullptr;
		DomainFile* pOpenFile = nullptr;
		ImDrawList* pDrawList = ImGui::GetWindowDrawList();

		for (usize i = 0; i < m_entries.GetCount(); i++)
		{
			const BrowserEntry& entry = m_entries[i];
			const ImVec2 cellMin = ImVec2(startPos.x + (i % columnCount) * step, startPos.y + (i / columnCount) * step);

			ImGui::SetCursorScreenPos(cellMin);
			ImGui::PushID(static_cast<i32>(entry.id));

			ImGui::SetNextItemSelectionUserData(static_cast<ImGuiSelectionUserData>(i));
			ImGui::SetNextItemAllowOverlap();
			ImGui::Selectable("", m_selection.Contains(entry.id), ImGuiSelectableFlags_None, ImVec2(cell, cell));

			if (entry.IsFolder() && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				pEnterFolder = entry.pFolder;

			if (!entry.IsFolder() && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				pOpenFile = entry.pFile;

			if (!entry.IsFolder() && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover))
			{
				ImGui::SetDragDropPayload("HZ_ASSET_FILE", &entry.pFile, sizeof(DomainFile*));

				ImGui::TextUnformatted(ICON_FA_FILE);
				ImGui::SameLine();
				ImGui::TextUnformatted(entry.GetName().c_str());

				ImGui::EndDragDropSource();
			}

			const ImU32 textColor = ImGui::GetColorU32(ImGuiCol_Text);
			const ThumbnailEntry* pThumbnail = nullptr;

			if (!entry.IsFolder() && ImGui::IsRectVisible(cellMin, ImVec2(cellMin.x + cell, cellMin.y + cell)))
				pThumbnail = RequestThumbnail(entry.pFile);

			if (pThumbnail)
			{
				const f32 box = cell * 0.62f;
				const f32 scale = ImMin(box / pThumbnail->width, box / pThumbnail->height);
				const ImVec2 imageSize = ImVec2(pThumbnail->width * scale, pThumbnail->height * scale);
				const ImVec2 imageMin = ImVec2(cellMin.x + (cell - imageSize.x) * 0.5f, cellMin.y + cell * 0.08f + (box - imageSize.y) * 0.5f);

				pDrawList->AddImage((ImTextureID)pThumbnail->textureId, imageMin, ImVec2(imageMin.x + imageSize.x, imageMin.y + imageSize.y));
			}
			else
			{
				std::string fileIcon = ICON_FA_FOLDER;

				if (!entry.IsFolder())
					fileIcon = DomainToAssetIcon(entry.pFile->GetAssetType());

				ImGui::PushFont(ImGui::GetFont(), cell * 0.5f);

				const ImVec2 iconSize = ImGui::CalcTextSize(fileIcon.c_str());
				const ImVec2 iconPos = ImVec2(cellMin.x + (cell - iconSize.x) * 0.5f, cellMin.y + cell * 0.14f);

				pDrawList->AddText(iconPos, textColor, fileIcon.c_str());
				ImGui::PopFont();
			}

			const std::string label = TruncateToWidth(entry.GetName(), cell - 6.f);
			const ImVec2 nameSize = ImGui::CalcTextSize(label.c_str());
			const ImVec2 namePos = ImVec2(cellMin.x + (cell - nameSize.x) * 0.5f, cellMin.y + cell - nameSize.y - 4.f);

			pDrawList->AddText(namePos, textColor, label.c_str());

			if (label != entry.GetName() && ImGui::IsItemHovered())
				ImGui::SetTooltip("%s", entry.GetName().c_str());

			ImGui::PopID();
		}

		pMultiIO = ImGui::EndMultiSelect();
		m_selection.ApplyRequests(pMultiIO);

		const usize lineCount = (m_entries.GetCount() + columnCount - 1) / columnCount;
		ImGui::SetCursorScreenPos(startPos);
		ImGui::Dummy(ImVec2(columnCount * step, lineCount * step));

		if (pEnterFolder != nullptr)
			Navigate(pEnterFolder);

		if (pOpenFile != nullptr)
		{
			auto it = m_actionNameLookup.find(pOpenFile->GetAssetType()->GetName());
			if (it == m_actionNameLookup.end())
			{
				Terminal::Warn(StringHelper::GetTypeName(this), "{} is {} and there is no action for this asset.", pOpenFile->GetName(), pOpenFile->GetAssetType()->GetName());
				return;
			}

			it->second->OnTrigger(GetContext(), pOpenFile);
		}

		AssetBrowserContext browserContext = {};
		browserContext.pEngine = GetContext()->pEngine;
		browserContext.pCurrentFolder = pFolder;

		CollectSelected(browserContext);

		m_contextMenu.RenderGUI("AssetBrowserContextMenu", browserContext);

		if (!browserContext.renamePath.empty())
			BeginRename(browserContext.renamePath);

		RenderRenameModal();
		RenderDialog();
	}

	void AssetContentView::RenderToolbar(DomainFolder* pFolder)
	{
		ImGui::BeginDisabled(pFolder->IsRoot());

		if (ImGui::Button(ICON_FA_ARROW_LEFT) && !pFolder->IsRoot())
			Navigate(pFolder->GetParent());

		ImGui::EndDisabled();

		ImGui::SameLine();
		ImGui::AlignTextToFramePadding();

		List<const std::string*> chain;
		DomainFolder* pWalk = pFolder;

		while (pWalk != nullptr)
		{
			chain.PushBack(&pWalk->GetName());
			pWalk = pWalk->GetParent();
		}

		chain.ReverseOrder();

		std::string display;

		for (const std::string* pName : chain)
			display += *pName + "/";

		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
		ImGui::TextUnformatted(display.c_str());
		ImGui::PopStyleColor();
	}

	void AssetContentView::RebuildEntries(DomainFolder* pFolder)
	{
		m_entries.Clear();

		for (DomainFolder* pSubFolder : pFolder->GetFolders())
		{
			BrowserEntry& entry = m_entries.EmplaceBack(pSubFolder, nullptr, 0);
			entry.id = HashPath(entry.GetPath());
		}

		for (DomainFile* pFile : pFolder->GetFiles())
		{
			BrowserEntry& entry = m_entries.EmplaceBack(nullptr, pFile, 0);
			entry.id = HashPath(entry.GetPath());
		}
	}

	void AssetContentView::CollectSelected(AssetBrowserContext& context)
	{
		for (const BrowserEntry& entry : m_entries)
		{
			if (!m_selection.Contains(entry.id))
				continue;

			if (entry.IsFolder())
				context.selectedFolders.PushBack(entry.pFolder);
			else
				context.selectedFiles.PushBack(entry.pFile);
		}
	}

	void AssetContentView::Navigate(DomainFolder* pTarget)
	{
		if (pTarget == nullptr)
			return;

		m_currentFolder = pTarget;
		m_selection.Clear();
		m_releaseThumbnails = true;
	}

	void AssetContentView::BeginRename(const std::string& sourcePath)
	{
		if (!m_renamePath.empty())
			return;

		const BrowserEntry* pEntry = FindEntry(sourcePath);

		if (pEntry == nullptr)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} is not a tracked browser entry", sourcePath);
			return;
		}

		m_renamePath = sourcePath;
		m_renameIsFolder = pEntry->IsFolder();

		std::snprintf(m_renameBuffer, sizeof(m_renameBuffer), "%s", pEntry->GetName().c_str());
		ImGui::OpenPopup("Rename - Asset Browser");
	}

	void AssetContentView::RenderRenameModal()
	{
		if (!ImGui::BeginPopupModal("Rename - Asset Browser", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			return;

		b8 accepted = ImGui::InputText("##name", m_renameBuffer, sizeof(m_renameBuffer), ImGuiInputTextFlags_EnterReturnsTrue);

		if (ImGui::Button("OK") || accepted)
		{
			auto* pEntry = FindEntry(m_renamePath);
			pEntry->IsFolder() ? pEntry->pFolder->Rename(m_renameBuffer) : pEntry->pFile->Rename(m_renameBuffer);
			m_renamePath.clear();
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape))
		{
			m_renamePath.clear();
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	AssetContentView::BrowserEntry* AssetContentView::FindEntry(const std::string& path)
	{
		for (auto& entry : m_entries)
		{
			if (entry.GetPath() == path)
				return &entry;
		}

		return nullptr;
	}

	void AssetContentView::ImportFiles(const List<std::string>& files)
	{
	}

	void AssetContentView::RenderDialog()
	{
	}

	const AssetContentView::ThumbnailEntry* AssetContentView::RequestThumbnail(DomainFile* pFile)
	{
		if (!m_assetService || !m_device)
			return nullptr;

		Reflect::Type* pAssetType = pFile->GetAssetType();
		if (!pAssetType || pAssetType->GetFullName() != "Asset::Texture")
			return nullptr;

		auto* pAsset = m_assetService->FindAsset<Asset::Texture>(pFile->GetID());
		if (!pAsset)
			return nullptr;

		ThumbnailEntry& thumbnail = m_thumbnails[pFile];
		thumbnail.assetId = pFile->GetID();

		if (pAsset->GetResidency() == Engine::AssetResidency::Unloaded)
		{
			pAsset->LoadAsync();
			thumbnail.ownsLoad = true;
		}

		Asset::TextureUsePermit permit = pAsset->BeginUse();
		if (!permit.IsValid())
			return nullptr;

		if (permit.pView != thumbnail.pView)
		{
			m_device->UnregisterImGuiTexture(thumbnail.textureId);
			thumbnail.textureId = m_device->RegisterImGuiTexture(permit.pView, nullptr);
			thumbnail.pView = permit.pView;
			thumbnail.width = static_cast<f32>(permit.pView->GetTexture()->GetWidth());
			thumbnail.height = static_cast<f32>(permit.pView->GetTexture()->GetHeight());
		}

		pAsset->EndUse(permit);

		if (thumbnail.textureId == 0)
			return nullptr;

		return &thumbnail;
	}

	void AssetContentView::ReleaseThumbnails()
	{
		for (auto& [pFile, thumbnail] : m_thumbnails)
		{
			m_device->UnregisterImGuiTexture(thumbnail.textureId);

			if (!thumbnail.ownsLoad)
				continue;

			auto* pAsset = m_assetService->FindAsset<Asset::Texture>(thumbnail.assetId);
			if (pAsset)
				pAsset->UnloadAsync();
		}

		m_thumbnails.clear();
	}
}