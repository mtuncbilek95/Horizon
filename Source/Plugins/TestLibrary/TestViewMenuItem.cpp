#include "TestViewMenuItem.h"

#include <TestView.h>
#include <Editor/Views/ViewRegistry.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::TestLibrary
{
	void TestViewMenuItem::OnExecute()
	{
		if (GetContext()->pViews->HasViewObject(Reflect::TypeOf<TestView>()))
		{
			auto* pPluginView = GetContext()->pViews->GetViewObject<TestView>();
			GetContext()->pViews->CloseView(pPluginView);
		}
		else
			GetContext()->pViews->OpenView(Reflect::TypeOf<TestView>());
	}
}