#include "EditorModule.h"

#include <Runtime/Graphics/RHI/Device/GfxDevice.h>

#include <Engine/Engine/Engine.h>
#include <Engine/Window/WindowModule.h>
#include <Engine/Graphics/GraphicsModule.h>

#include <Editor/Editor/MainScreen.h>

#include <imgui.h>

namespace Horizon
{
	EditorModule::EditorModule() = default;
	EditorModule::~EditorModule() = default;

	void EditorModule::OnAttach(Engine& engine)
	{
		IModule::OnAttach(engine);

		auto& windowModule = engine.GetModule<WindowModule>();
		auto& graphicsModule = engine.GetModule<GraphicsModule>();

		m_device = graphicsModule.GetDevice();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ImGui::StyleColorsDark();

		m_device->InitializeImGui(windowModule.GetMainWindow().GetAPIHandle(), graphicsModule.GetGraphicsQueue());

		m_mainScreen = std::make_unique<MainScreen>();
	}

	void EditorModule::OnSync()
	{
		m_device->NewFrameImGui();
		ImGui::NewFrame();

		const u64 sceneTextureId = m_engine->GetModule<GraphicsModule>().GetSceneTextureId();
		m_mainScreen->Draw(sceneTextureId);

		ImGui::Render();
	}

	void EditorModule::OnDetach()
	{
		m_device->ShutdownImGui();
		ImGui::DestroyContext();

		m_mainScreen.reset();
	}
}
