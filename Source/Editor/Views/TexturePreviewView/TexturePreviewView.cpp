#include "TexturePreviewView.h"

#include <Editor/Domain/DomainFile.h>
#include <Editor/Renderer/EditorContext.h>
#include <Editor/Renderer/Utils/ImGuiUtils.h>
#include <Editor/Models/SelectionModel.h>

#include <Engine/Asset/AssetService.h>
#include <Engine/Asset/Texture/Texture.h>
#include <Engine/Core/Engine.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Presentation/PresentationService.h>

#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/Math/Mat4f.h>
#include <Runtime/Math/Quat.h>
#include <Runtime/Math/Scalar.h>
#include <Runtime/Math/Vec2u.h>
#include <Runtime/Math/Vec3f.h>
#include <Runtime/RHI/Device/Device.h>
#include <Runtime/RHI/Texture/Texture.h>
#include <Runtime/RHI/Texture/TextureView.h>

#include <imgui.h>
#include <ImGuizmo.h>

#include <algorithm>

namespace Horizon::Editor
{
	TexturePreviewView::~TexturePreviewView()
	{
		if (m_device)
			m_device->UnregisterImGuiTexture(m_textureId);
	}

	void TexturePreviewView::OnInvoke()
	{
		m_assetService = GetContext()->pEngine->RequestService<Engine::AssetService>();
		if (!m_assetService)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "AssetService is missing, texture will not be displayed");
			return;
		}

		auto* pGraphics = GetContext()->pEngine->RequestContext<Engine::GraphicsContext>();
		if (!pGraphics)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "GraphicsContext is missing, texture will not be displayed");
			return;
		}

		m_device = pGraphics->GetDevice();
	}

	void TexturePreviewView::OnRender(const Engine::EngineFrame& context)
	{
		if (!m_assetService || !m_device)
			return;

		DomainFile* pFile = GetConnectedFile();
		if (!pFile)
			return;

		auto* pAsset = m_assetService->FindAsset<Asset::Texture>(pFile->GetID());
		if (!pAsset)
		{
			ImGui::TextUnformatted("Texture asset is not registered");
			return;
		}

		const Engine::AssetResidency residency = pAsset->GetResidency();
		if (residency == Engine::AssetResidency::Unloaded)
			pAsset->LoadAsync();

		if (residency == Engine::AssetResidency::Failed)
		{
			ImGui::TextUnformatted("Texture could not be loaded");
			return;
		}

		const ImVec2 available = ImGui::GetContentRegionAvail();
		if (available.x < 1.0f || available.y < 1.0f)
			return;

		Asset::TextureUsePermit permit = pAsset->BeginUse();
		if (!permit.IsValid())
		{
			ImGui::TextUnformatted("Loading...");
			return;
		}

		if (permit.pView != m_boundView)
		{
			m_device->UnregisterImGuiTexture(m_textureId);
			m_textureId = m_device->RegisterImGuiTexture(permit.pView, nullptr);
			m_boundView = permit.pView;
		}

		if (m_textureId != 0)
		{
			const f32 textureWidth = static_cast<f32>(permit.pView->GetTexture()->GetWidth());
			const f32 textureHeight = static_cast<f32>(permit.pView->GetTexture()->GetHeight());
			const f32 scale = std::min(available.x / textureWidth, available.y / textureHeight);

			ImGui::Image((ImTextureID)m_textureId, ImVec2(textureWidth * scale, textureHeight * scale));
		}

		pAsset->EndUse(permit);
	}
}