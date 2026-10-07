#include "TestView.h"

#include <imgui.h>

namespace Horizon::TestLibrary
{
	void TestView::OnInvoke()
	{
	}

	void TestView::OnRender(const Engine::EngineFrame& context)
	{
		ImGui::Text("Hello from TestLibrary.dll");
	}
}