#include "InputActionMapAssetAction.h"

#include <Editor/Views/InputActionMapView/InputActionMapView.h>
#include <Editor/Views/ViewRegistry.h>

namespace Horizon::Editor
{
	void InputActionMapAssetAction::OnTrigger(EditorContext* pContext, DomainFile* pUsedAsset)
	{
		ViewObject* pViewer = pContext->pViews->OpenView(Reflect::TypeOf<InputActionMapView>());
		pViewer->SetConnectedFile(pUsedAsset);
	}
}
