#include "TextureAssetAction.h"

#include <Editor/Views/TexturePreviewView/TexturePreviewView.h>
#include <Editor/Views/ViewRegistry.h>
#include <Engine/Core/Engine.h>
#include <Engine/Asset/AssetService.h>

namespace Horizon::Editor
{
	void TextureAssetAction::OnTrigger(EditorContext* pContext, DomainFile* pUsedAsset)
	{
		auto* pAssetService = pContext->pEngine->RequestService<Engine::AssetService>();

		auto* pTextureAsset = pAssetService->FindAsset<Asset::Texture>(pUsedAsset->GetID());
		if (!pTextureAsset)
			return;

		pTextureAsset->LoadAsync();

		ViewObject* pViewer = pContext->pViews->OpenView(Reflect::TypeOf<TexturePreviewView>());
		pViewer->SetConnectedFile(pUsedAsset);
	}
}