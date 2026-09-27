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
		
		auto* pSceneAsset = (Engine::SceneAsset*)pAssetService->FindAsset(pUsedAsset->GetID());

		pSceneAsset->LoadAsync();
		
		ViewObject* pViewer = pContext->pViews->GetViewObject(Reflect::TypeOf<SceneHierarchyView>());
		pViewer->SetConnectedFile(pUsedAsset); // To be able to save

		pWorldService->SetCurrentWorld(pSceneAsset->GetScene());
	}
}