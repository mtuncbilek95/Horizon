#include "MaterialAssetAction.h"

#include <Editor/Views/MaterialGraphView/MaterialGraphView.h>
#include <Editor/Views/ViewRegistry.h>
#include <Engine/Core/Engine.h>
#include <Engine/Asset/AssetService.h>

namespace Horizon::Editor
{
	void MaterialAssetAction::OnTrigger(EditorContext* pContext, DomainFile* pUsedAsset)
	{
		auto* pAssetService = pContext->pEngine->RequestService<Engine::AssetService>();

		auto* pMaterialAsset = pAssetService->FindAsset<Asset::Material>(pUsedAsset->GetID());
		if (!pMaterialAsset)
			return;

		pMaterialAsset->LoadAsync();

		ViewObject* pViewer = pContext->pViews->OpenView(Reflect::TypeOf<MaterialGraphView>());
		pViewer->SetConnectedFile(pUsedAsset);
	}
}