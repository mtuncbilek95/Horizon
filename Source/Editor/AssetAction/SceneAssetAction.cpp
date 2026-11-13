#include "SceneAssetAction.h"

#include <Editor/Views/SceneHierarchyView/SceneHierarchyView.h>
#include <Editor/Views/ViewRegistry.h>
#include <Engine/Core/Engine.h>
#include <Engine/Asset/AssetService.h>
#include <Engine/World/WorldService.h>

namespace Horizon::Editor
{
	void SceneAssetAction::OnTrigger(EditorContext* pContext, DomainFile* pUsedAsset)
	{
		auto* pAssetService = pContext->pEngine->RequestService<Engine::AssetService>();
		auto* pWorldService = pContext->pEngine->RequestService<Engine::WorldService>();

		auto* pSceneAsset = pAssetService->FindAsset<Asset::Scene>(pUsedAsset->GetID());
		if (!pSceneAsset)
			return;

		ViewObject* pViewer = pContext->pViews->GetViewObject(Reflect::TypeOf<SceneHierarchyView>());
		DomainFile* pPrevFile = pViewer->GetConnectedFile();
		if (pPrevFile == pUsedAsset)
			return;

		pSceneAsset->LoadSync();

		if (pSceneAsset->GetResidency() != Engine::AssetResidency::Resident)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} could not be loaded, current world stays as it is", pUsedAsset->GetName());
			return;
		}

		pWorldService->SetCurrentWorld(pSceneAsset->GetScene());
		pViewer->SetConnectedFile(pUsedAsset);

		if (pPrevFile)
		{
			auto* pPrevSceneAsset = pAssetService->FindAsset<Asset::Scene>(pPrevFile->GetID());
			if (pPrevSceneAsset)
				pPrevSceneAsset->UnloadSync();
		}
	}
}