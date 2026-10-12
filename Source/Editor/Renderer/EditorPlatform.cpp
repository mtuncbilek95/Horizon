#include "EditorPlatform.h"

#include <Editor/Renderer/Utils/ImGuiUtils.h>

#include <Engine/Window/WindowService.h>

#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/Hardware/Monitor.h>
#include <Runtime/PAL/Window/Cursor.h>
#include <Runtime/PAL/Window/Window.h>

#include <imgui.h>
#include <imgui_internal.h>

namespace Horizon::Editor
{
	namespace
	{
		PAL::Window* ToWindow(ImGuiViewport* pViewport)
		{
			return (PAL::Window*)pViewport->PlatformHandle;
		}

		void* ToRawHandle(PAL::Window* pWindow)
		{
			return (void*)uptr(pWindow->GetOSHandle());
		}
	}

	EditorPlatform::EditorPlatform(Engine::WindowService* pWindowService) : m_windowService(pWindowService),
		m_cursor(pWindowService->GetCursor())
	{
		ImGuiIO& io = ImGui::GetIO();
		io.BackendPlatformUserData = this;
		io.BackendPlatformName = "Horizon_PAL";
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors | ImGuiBackendFlags_HasSetMousePos |
			ImGuiBackendFlags_PlatformHasViewports | ImGuiBackendFlags_HasParentViewport;

		ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
		platformIO.Platform_CreateWindow = &EditorPlatform::OnCreateWindow;
		platformIO.Platform_DestroyWindow = &EditorPlatform::OnDestroyWindow;
		platformIO.Platform_ShowWindow = &EditorPlatform::OnShowWindow;
		platformIO.Platform_UpdateWindow = &EditorPlatform::OnUpdateWindow;
		platformIO.Platform_SetWindowPos = &EditorPlatform::OnSetWindowPos;
		platformIO.Platform_GetWindowPos = &EditorPlatform::OnGetWindowPos;
		platformIO.Platform_SetWindowSize = &EditorPlatform::OnSetWindowSize;
		platformIO.Platform_GetWindowSize = &EditorPlatform::OnGetWindowSize;
		platformIO.Platform_SetWindowFocus = &EditorPlatform::OnSetWindowFocus;
		platformIO.Platform_GetWindowFocus = &EditorPlatform::OnGetWindowFocus;
		platformIO.Platform_GetWindowMinimized = &EditorPlatform::OnGetWindowMinimized;
		platformIO.Platform_SetWindowTitle = &EditorPlatform::OnSetWindowTitle;
		platformIO.Platform_SetWindowAlpha = &EditorPlatform::OnSetWindowAlpha;

		PAL::Window* pMainWindow = m_windowService->GetWindow();
		ImGuiViewport* pMainViewport = ImGui::GetMainViewport();
		pMainViewport->PlatformHandle = pMainWindow;
		pMainViewport->PlatformHandleRaw = ToRawHandle(pMainWindow);

		UpdateMonitors();
	}

	EditorPlatform::~EditorPlatform()
	{
		ImGui::DestroyPlatformWindows();
		ResetCapture();

		ImGui::GetPlatformIO().ClearPlatformHandlers();

		ImGuiIO& io = ImGui::GetIO();
		io.BackendPlatformUserData = nullptr;
		io.BackendPlatformName = nullptr;
		io.BackendFlags &= ~(ImGuiBackendFlags_HasMouseCursors | ImGuiBackendFlags_HasSetMousePos |
			ImGuiBackendFlags_PlatformHasViewports | ImGuiBackendFlags_HasParentViewport);
	}

	void EditorPlatform::NewFrame()
	{
		for (PAL::Window* pWindow : m_windowService->GetWindows())
			ProcessMessages(pWindow);

		UpdateMonitors();
		UpdateDisplaySize();
		UpdateFocus();
		UpdateMousePosition();
		UpdateMouseCursor();
	}

	void EditorPlatform::ProcessMessages(PAL::Window* pWindow)
	{
		ImGuiIO& io = ImGui::GetIO();

		const b8 viewportsEnabled = (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0;
		const b8 isMain = pWindow == m_windowService->GetWindow();
		ImGuiViewport* pViewport = ImGui::FindViewportByPlatformHandle(pWindow);
		const PAL::WindowRect rect = pWindow->GetRect();

		for (const PAL::InputMessage& message : pWindow->GetMessages())
		{
			switch (message.type)
			{
			case PAL::InputMessageType::MouseMove:
			{
				if (viewportsEnabled)
					io.AddMousePosEvent(f32(message.screenX), f32(message.screenY));
				else if (isMain)
					io.AddMousePosEvent(f32(message.mouseX), f32(message.mouseY));
				break;
			}
			case PAL::InputMessageType::MouseDown:
			{
				const i32 button = ImGuiUtils::GetMouseButton(message.button);
				BeginCapture(pWindow, button);
				io.AddMouseButtonEvent(button, true);
				break;
			}
			case PAL::InputMessageType::MouseUp:
			{
				const i32 button = ImGuiUtils::GetMouseButton(message.button);
				io.AddMouseButtonEvent(button, false);
				EndCapture(button);
				break;
			}
			case PAL::InputMessageType::MouseScroll:
			{
				io.AddMouseWheelEvent(message.scrollX, message.scrollY);
				break;
			}
			case PAL::InputMessageType::KeyDown:
			{
				AddKeyEvent(message.key, true);
				break;
			}
			case PAL::InputMessageType::KeyUp:
			{
				AddKeyEvent(message.key, false);
				break;
			}
			case PAL::InputMessageType::Char:
			{
				io.AddInputCharacter(message.character);
				break;
			}
			case PAL::InputMessageType::Close:
			{
				if (pViewport != nullptr && !isMain)
					pViewport->PlatformRequestClose = true;
				break;
			}
			case PAL::InputMessageType::Move:
			{
				if (pViewport == nullptr)
					break;

				if (i32(pViewport->Pos.x) != rect.posX || i32(pViewport->Pos.y) != rect.posY)
					pViewport->PlatformRequestMove = true;
				break;
			}
			case PAL::InputMessageType::Resize:
			{
				if (pViewport == nullptr)
					break;

				if (u32(pViewport->Size.x) != rect.width || u32(pViewport->Size.y) != rect.height)
					pViewport->PlatformRequestResize = true;
				break;
			}
			default:
				break;
			}
		}
	}

	void EditorPlatform::AddKeyEvent(PAL::KeyCode key, b8 down)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddKeyEvent(ImGuiUtils::GetKeyboardKey(key), down);

		if (key == PAL::KeyCode::LeftControl)
			io.AddKeyEvent(ImGuiKey_ReservedForModCtrl, down);
		if (key == PAL::KeyCode::LeftShift)
			io.AddKeyEvent(ImGuiKey_ReservedForModShift, down);
		if (key == PAL::KeyCode::LeftAlt)
			io.AddKeyEvent(ImGuiKey_ReservedForModAlt, down);
		if (key == PAL::KeyCode::LeftSuper)
			io.AddKeyEvent(ImGuiKey_ReservedForModSuper, down);
	}

	void EditorPlatform::UpdateMonitors()
	{
		ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
		platformIO.Monitors.resize(0);

		for (const PAL::MonitorInfo& info : PAL::Monitor::EnumerateMonitors())
		{
			ImGuiPlatformMonitor monitor = {};
			monitor.MainPos = ImVec2(f32(info.posX), f32(info.posY));
			monitor.MainSize = ImVec2(f32(info.width), f32(info.height));
			monitor.WorkPos = ImVec2(f32(info.workPosX), f32(info.workPosY));
			monitor.WorkSize = ImVec2(f32(info.workWidth), f32(info.workHeight));
			monitor.DpiScale = info.dpiScale;

			platformIO.Monitors.push_back(monitor);
		}
	}

	void EditorPlatform::UpdateDisplaySize()
	{
		const PAL::WindowRect rect = m_windowService->GetWindow()->GetRect();

		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(f32(rect.width), f32(rect.height));
	}

	void EditorPlatform::UpdateFocus()
	{
		b8 focused = false;

		for (PAL::Window* pWindow : m_windowService->GetWindows())
		{
			if (pWindow->GetFocused())
			{
				focused = true;
				break;
			}
		}

		if (focused == m_appFocused)
			return;

		m_appFocused = focused;
		ImGui::GetIO().AddFocusEvent(focused);

		if (!focused)
			ResetCapture();
	}

	void EditorPlatform::UpdateMousePosition()
	{
		if (!m_appFocused)
			return;

		ImGuiIO& io = ImGui::GetIO();

		const b8 viewportsEnabled = (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0;
		const PAL::WindowRect rect = m_windowService->GetWindow()->GetRect();
		const i32 originX = viewportsEnabled ? 0 : rect.posX;
		const i32 originY = viewportsEnabled ? 0 : rect.posY;

		if (io.WantSetMousePos)
		{
			m_cursor->SetScreenPosition(originX + i32(io.MousePos.x), originY + i32(io.MousePos.y));
			return;
		}

		const PAL::CursorPoint point = m_cursor->GetScreenPosition();
		io.AddMousePosEvent(f32(point.x - originX), f32(point.y - originY));
	}

	void EditorPlatform::UpdateMouseCursor()
	{
		const ImGuiIO& io = ImGui::GetIO();

		if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
			return;

		if (io.MouseDrawCursor)
		{
			m_cursor->SetShape(PAL::CursorType::Hidden);
			return;
		}

		m_cursor->SetShape(ImGuiUtils::ToCursorType(ImGui::GetMouseCursor()));
	}

	void EditorPlatform::BeginCapture(PAL::Window* pWindow, i32 button)
	{
		if (m_heldButtons == 0 && m_captureWindow == nullptr)
		{
			pWindow->SetMouseCapture(true);
			m_captureWindow = pWindow;
		}

		m_heldButtons |= 1u << u32(button);
	}

	void EditorPlatform::EndCapture(i32 button)
	{
		m_heldButtons &= ~(1u << u32(button));

		if (m_heldButtons == 0)
			ResetCapture();
	}

	void EditorPlatform::ResetCapture()
	{
		if (m_captureWindow != nullptr)
			m_captureWindow->SetMouseCapture(false);

		m_captureWindow = nullptr;
		m_heldButtons = 0;
	}

	EditorPlatform* EditorPlatform::GetBackend()
	{
		return (EditorPlatform*)ImGui::GetIO().BackendPlatformUserData;
	}

	b8 EditorPlatform::IsPanelViewport(ImGuiViewport* pViewport)
	{
		const ImGuiWindow* pOwner = ((ImGuiViewportP*)pViewport)->Window;

		if (pOwner == nullptr)
			return false;

		const ImGuiWindowFlags excluded = ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_Popup |
			ImGuiWindowFlags_ChildMenu | ImGuiWindowFlags_DockNodeHost;

		return (pOwner->Flags & excluded) == 0;
	}

	PAL::WindowFlags EditorPlatform::ToWindowFlags(i32 viewportFlags, b8 customFrame)
	{
		PAL::WindowFlags flags = PAL::WindowFlags::None;

		if (customFrame)
			flags |= PAL::WindowFlags::CustomTitleBar;
		else if (viewportFlags & ImGuiViewportFlags_NoDecoration)
			flags |= PAL::WindowFlags::NoDecoration;

		if (viewportFlags & ImGuiViewportFlags_NoTaskBarIcon)
			flags |= PAL::WindowFlags::NoTaskbarIcon;
		if (viewportFlags & ImGuiViewportFlags_TopMost)
			flags |= PAL::WindowFlags::TopMost;
		if (viewportFlags & (ImGuiViewportFlags_NoFocusOnAppearing | ImGuiViewportFlags_NoFocusOnClick))
			flags |= PAL::WindowFlags::NoActivate;
		if (viewportFlags & ImGuiViewportFlags_NoInputs)
			flags |= PAL::WindowFlags::PassThroughInput;

		return flags;
	}

	void EditorPlatform::OnCreateWindow(ImGuiViewport* pViewport)
	{
		EditorPlatform* pBackend = GetBackend();

		PAL::WindowDesc desc = {};
		desc.titleName = "Untitled";
		desc.posX = i32(pViewport->Pos.x);
		desc.posY = i32(pViewport->Pos.y);
		desc.width = u32(pViewport->Size.x);
		desc.height = u32(pViewport->Size.y);
		desc.mode = PAL::WindowMode::Windowed;
		desc.flags = ToWindowFlags(pViewport->Flags, IsPanelViewport(pViewport));
		desc.pOwner = pViewport->ParentViewport != nullptr ? ToWindow(pViewport->ParentViewport) : nullptr;

		PAL::Window* pWindow = pBackend->m_windowService->SpawnWindow(desc);

		if (pWindow == nullptr)
		{
			Terminal::Error("EditorPlatform", "Platform window for viewport {} could not be created", pViewport->ID);
			return;
		}

		pViewport->PlatformHandle = pWindow;
		pViewport->PlatformHandleRaw = ToRawHandle(pWindow);
	}

	void EditorPlatform::OnDestroyWindow(ImGuiViewport* pViewport)
	{
		EditorPlatform* pBackend = GetBackend();
		PAL::Window* pWindow = ToWindow(pViewport);

		if (pWindow != nullptr && pWindow != pBackend->m_windowService->GetWindow())
		{
			if (pWindow == pBackend->m_captureWindow)
				pBackend->ResetCapture();

			pBackend->m_windowService->DestroyWindow(pWindow);
		}

		pViewport->PlatformHandle = nullptr;
		pViewport->PlatformHandleRaw = nullptr;
	}

	void EditorPlatform::OnShowWindow(ImGuiViewport* pViewport)
	{
		ToWindow(pViewport)->Show();
	}

	void EditorPlatform::OnUpdateWindow(ImGuiViewport* pViewport)
	{
		EditorPlatform* pBackend = GetBackend();
		PAL::Window* pWindow = ToWindow(pViewport);

		if (pWindow == pBackend->m_windowService->GetWindow())
			return;

		const b8 customFrame = HasFlag(pWindow->GetFlags(), PAL::WindowFlags::CustomTitleBar);
		pWindow->SetFlags(ToWindowFlags(pViewport->Flags, customFrame));
	}

	void EditorPlatform::OnSetWindowPos(ImGuiViewport* pViewport, ImVec2 pos)
	{
		ToWindow(pViewport)->SetPosition(i32(pos.x), i32(pos.y));
	}

	ImVec2 EditorPlatform::OnGetWindowPos(ImGuiViewport* pViewport)
	{
		const PAL::WindowRect rect = ToWindow(pViewport)->GetRect();
		return ImVec2(f32(rect.posX), f32(rect.posY));
	}

	void EditorPlatform::OnSetWindowSize(ImGuiViewport* pViewport, ImVec2 size)
	{
		ToWindow(pViewport)->SetSize(u32(size.x), u32(size.y));
	}

	ImVec2 EditorPlatform::OnGetWindowSize(ImGuiViewport* pViewport)
	{
		const PAL::WindowRect rect = ToWindow(pViewport)->GetRect();
		return ImVec2(f32(rect.width), f32(rect.height));
	}

	void EditorPlatform::OnSetWindowFocus(ImGuiViewport* pViewport)
	{
		ToWindow(pViewport)->SetFocus();
	}

	bool EditorPlatform::OnGetWindowFocus(ImGuiViewport* pViewport)
	{
		return ToWindow(pViewport)->GetFocused();
	}

	bool EditorPlatform::OnGetWindowMinimized(ImGuiViewport* pViewport)
	{
		return ToWindow(pViewport)->GetMinimized();
	}

	void EditorPlatform::OnSetWindowTitle(ImGuiViewport* pViewport, const c8* pTitle)
	{
		ToWindow(pViewport)->SetTitle(pTitle);
	}

	void EditorPlatform::OnSetWindowAlpha(ImGuiViewport* pViewport, f32 alpha)
	{
		ToWindow(pViewport)->SetAlpha(alpha);
	}
}