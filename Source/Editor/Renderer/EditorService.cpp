#include "EditorService.h"

#include <Editor/Domain/DomainService.h>
#include <Editor/Models/SelectionModel.h>
#include <Editor/Renderer/EditorRenderer.h>
#include <Editor/Views/ViewRegistry.h>
#include <Editor/MainMenu/MenuRegistry.h>

#include <Engine/Core/Engine.h>
#include <Engine/Window/WindowService.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/World/WorldService.h>

#include <Runtime/RHI/Device/GfxDevice.h>
#include <Runtime/RHI/Swapchain/GfxSwapchain.h>
#include <Runtime/RHI/Fence/GfxFence.h>
#include <Runtime/RHI/Queue/GfxQueue.h>

namespace Horizon::Editor
{
	Engine::ModuleReport EditorService::OnInitialize()
	{
		auto* pWindowSub = GetEngine()->RequestService<Engine::WindowService>();
		if (!pWindowSub)
			return Engine::ModuleReport("Failed to get WindowService. Nothing will work...");

		m_engineWindow = pWindowSub->GetWindow();

		auto* pGraphSub = GetEngine()->RequestContext<Engine::GraphicsContext>();
		if (!pGraphSub)
			return Engine::ModuleReport("Failed to get GraphicsContext. Nothing will work...");

		m_fence = pGraphSub->GetDevice()->CreateFence();
		m_swapchain = pGraphSub->GetSwapchain();
		m_queue = pGraphSub->GetGraphicsQueue();

		EditorRendererDesc renderDesc = {};
		renderDesc.pDevice = pGraphSub->GetDevice();
		renderDesc.pQueue = pGraphSub->GetGraphicsQueue();
		renderDesc.colorFormat = m_swapchain->GetDesc().format;

		m_editorRenderer = Memory::Allocator::Create<EditorRenderer>(Memory::CurrLoc(), renderDesc);
		Terminal::Debug(StringOps::GetName(this), "EditorRenderer has been initialized!");

		m_editorContext.pEngine = GetEngine();
		m_editorContext.pSelection = Memory::Allocator::Create<SelectionModel>(Memory::CurrLoc());

		m_viewRegistry = Memory::Allocator::Create<ViewRegistry>(Memory::CurrLoc());
		m_viewRegistry->BootstrapViews(m_editorContext);

		m_menuRegistry = Memory::Allocator::Create<MenuRegistry>(Memory::CurrLoc());
		m_menuRegistry->BootstrapMenus(m_editorContext);

		return Engine::ModuleReport();
	}

	void EditorService::OnExecute(const Engine::EngineFrame& ctx)
	{
		const auto& messages = m_engineWindow->GetMessages();

		b8 onResizeDone = true;
		i32 newWindowWidth = -1, newWindowHeight = -1;
		for (const auto& message : messages)
		{
			switch (message.type)
			{
			case PAL::InputMessageType::MouseMove:
			{
				m_editorRenderer->OnMousePosition(message.mouseX, message.mouseY);
				break;
			}
			case PAL::InputMessageType::MouseDown:
			{
				m_editorRenderer->OnMouseButtonDown(message.button);
				break;
			}
			case PAL::InputMessageType::MouseUp:
			{
				m_editorRenderer->OnMouseButtonUp(message.button);
				break;
			}
			case PAL::InputMessageType::MouseScroll:
			{
				m_editorRenderer->OnMouseWheel(message.scrollY);
				break;
			}
			case PAL::InputMessageType::KeyDown:
			{
				m_editorRenderer->OnKeyboardDown(message.key);
				break;
			}
			case PAL::InputMessageType::KeyUp:
			{
				m_editorRenderer->OnKeyboardUp(message.key);
				break;
			}
			case PAL::InputMessageType::Char:
			{
				m_editorRenderer->OnKeyboardChar(message.character);
				break;
			}
			case PAL::InputMessageType::Resize:
			{
				newWindowWidth = message.width;
				newWindowHeight = message.height;
				onResizeDone = false;
				break;
			}
			case PAL::InputMessageType::Focus:
				m_editorRenderer->OnWindowFocus(true);
				break;
			case PAL::InputMessageType::LostFocus:
				m_editorRenderer->OnWindowFocus(false);
				break;
			}
		}

		if (!onResizeDone)
		{
			m_swapchain->Resize(newWindowWidth, newWindowHeight);
			onResizeDone = true;
		}

		PAL::WindowRect rect = m_engineWindow->GetRect();
		m_editorRenderer->OnResizeWindow(rect.width, rect.height);

		if (!m_swapchain->AcquireNextImage(m_fence))
			return;

		m_editorRenderer->BeginRender(ctx.DeltaTime());

		m_menuRegistry->RenderGUI();
		m_viewRegistry->RenderGUI(ctx);

		const b8 dragging = m_editorRenderer->CheckMouseDragging();
		m_engineWindow->SetMouseCapture(dragging);
		m_engineWindow->SetCursorClip(dragging);
		m_engineWindow->SetCursorShape(m_editorRenderer->GetMouseCursor());

		m_editorRenderer->EndRender(m_swapchain->GetImage(m_swapchain->GetCurrentImageIndex()));

		m_swapchain->Present(m_queue, m_fence);
	}

	void EditorService::OnFinalize()
	{
		const u64 value = m_queue->Signal(m_fence);
		m_fence->WaitCPU(value);
		
		Memory::Allocator::Delete(m_fence);

		Memory::Allocator::Delete(m_menuRegistry);
		Memory::Allocator::Delete(m_viewRegistry);
		Memory::Allocator::Delete(m_editorRenderer);

		Memory::Allocator::Delete(m_editorContext.pSelection);
	}

	void EditorService::DeclareDependencies(Engine::ModuleGraph& graph)
	{
		graph.Requires<Engine::WindowService>();
		graph.Requires<Engine::GraphicsContext>();
		graph.Requires<DomainService>();
		graph.Requires<Engine::WorldService>();
	}
}
