#pragma once

namespace Horizon
{
	class MainScreen
	{
	public:
		void Draw(u64 sceneTextureId);

	private:
		void DrawDockSpace();
		void DrawMenuBar();
		void DrawViewport(u64 sceneTextureId);
	};
}
