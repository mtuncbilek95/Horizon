#pragma once

namespace Horizon
{
	class GraphicsModule;

	class GfxDevice;
	class GfxQueue;
	
	class EditorRenderer
	{
	public:
		EditorRenderer(GraphicsModule* pModule = nullptr);
		~EditorRenderer();

		void BeginFrame();
		void EndFrame();

	private:
		GfxDevice* m_device;
		GfxQueue* m_graphicsQueue;

	};
}