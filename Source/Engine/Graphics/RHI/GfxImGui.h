#pragma once

namespace Horizon
{
	struct GfxCommandList;

	namespace GfxImGui
	{
		void Init(void* glfwWindow);
		void BeginFrame();
		void Render(GfxCommandList* cmd);
		void Shutdown();
	}
}
