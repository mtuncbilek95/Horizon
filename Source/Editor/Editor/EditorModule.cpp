#include "EditorModule.h"

#include <Engine/Engine/Engine.h>
#include <Engine/Window/WindowModule.h>
#include <Engine/Graphics/GraphicsModule.h>

namespace Horizon
{
	void EditorModule::OnAttach(Engine& engine)
	{
		IModule::OnAttach(engine);

		auto& winModule = engine.GetModule<WindowModule>();
		auto& currWindow = winModule.GetMainWindow();

		auto& grapModule = engine.GetModule<GraphicsModule>();

		Gfx::InitGfxImGui(grapModule.GetDevice(), grapModule.GetQueue(GfxQueueType::Graphics), 
			grapModule.GetDescriptorHeap(GfxDescriptorHeapType::Resource),
			currWindow.GetAPIHandle(), GfxTextureFormat::RGBA8, MaxFramesInFlight);
	}

	void EditorModule::OnSync()
	{
		Gfx::NewGfxImGuiFrame();

		auto& graphics = m_engine->GetModule<GraphicsModule>();

		Gfx::RenderGfxImGui(graphics.GetFrameCmd());
	}

	void EditorModule::OnDetach()
	{}
}