#include "ProjectSettingsMenuItem.h"

#include <Editor/Views/ProjectSettingsView/ProjectSettingsView.h>
#include <Editor/Views/ViewRegistry.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::Editor
{
	void ProjectSettingsMenuItem::OnExecute()
	{
		if (GetContext()->pViews->HasViewObject(Reflect::TypeOf<ProjectSettingsView>()))
		{
			auto* pPluginView = GetContext()->pViews->GetViewObject<ProjectSettingsView>();
			GetContext()->pViews->CloseView(pPluginView);
		}
		else
			GetContext()->pViews->OpenView(Reflect::TypeOf<ProjectSettingsView>());
	}
}