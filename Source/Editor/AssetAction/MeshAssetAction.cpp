#include "MeshAssetAction.h"

#include <Editor/Views/MeshPreviewView/MeshPreviewView.h>
#include <Editor/Views/ViewRegistry.h>
#include <Engine/Core/Engine.h>
#include <Engine/Asset/AssetService.h>

namespace Horizon::Editor
{
	void MeshAssetAction::OnTrigger(EditorContext* pContext, DomainFile* pUsedAsset)
	{
		auto* pAssetService = pContext->pEngine->RequestService<Engine::AssetService>();

		auto* pMeshAsset = pAssetService->FindAsset<Asset::Mesh>(pUsedAsset->GetID());
		if (!pMeshAsset)
			return;

		pMeshAsset->LoadAsync();

		ViewObject* pViewer = pContext->pViews->OpenView(Reflect::TypeOf<MeshPreviewView>());
		pViewer->SetConnectedFile(pUsedAsset);
	}
}
