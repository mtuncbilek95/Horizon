#include "PluginsMenuItem.h"

#include <Editor/Views/PluginsView/PluginsView.h>
#include <Editor/Views/ViewRegistry.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::Editor
{
	void PluginsMenuItem::OnExecute()
	{
		if(GetContext()->pViews->HasViewObject(Reflect::TypeOf<PluginsView>()))
		{
			auto* pPluginView = GetContext()->pViews->GetViewObject<PluginsView>();
			GetContext()->pViews->CloseView(pPluginView);
		}
		else
			GetContext()->pViews->OpenView(Reflect::TypeOf<PluginsView>());
	}
}