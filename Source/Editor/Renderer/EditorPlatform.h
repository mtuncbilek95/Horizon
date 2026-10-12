#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/PAL/Window/WindowFlags.h>
#include <Runtime/PAL/Window/KeyCode.h>

struct ImGuiViewport;
struct ImVec2;

namespace Horizon::PAL
{
	class Window;
	class Cursor;
}

namespace Horizon::Engine
{
	class WindowService;
}

namespace Horizon::Editor
{
	class EDITOR_API EditorPlatform final
	{
	public:
		EditorPlatform(Engine::WindowService* pWindowService);
		~EditorPlatform();

		EditorPlatform(const EditorPlatform&) = delete;
		EditorPlatform(EditorPlatform&&) = delete;

		EditorPlatform& operator=(const EditorPlatform&) = delete;
		EditorPlatform& operator=(EditorPlatform&&) = delete;

		void NewFrame();

	private:
		void ProcessMessages(PAL::Window* pWindow);
		void AddKeyEvent(PAL::KeyCode key, b8 down);

		void UpdateMonitors();
		void UpdateDisplaySize();
		void UpdateFocus();
		void UpdateMousePosition();
		void UpdateMouseCursor();

		void BeginCapture(PAL::Window* pWindow, i32 button);
		void EndCapture(i32 button);
		void ResetCapture();

		static EditorPlatform* GetBackend();
		static PAL::WindowFlags ToWindowFlags(i32 viewportFlags);

		static void OnCreateWindow(ImGuiViewport* pViewport);
		static void OnDestroyWindow(ImGuiViewport* pViewport);
		static void OnShowWindow(ImGuiViewport* pViewport);
		static void OnUpdateWindow(ImGuiViewport* pViewport);
		static void OnSetWindowPos(ImGuiViewport* pViewport, ImVec2 pos);
		static ImVec2 OnGetWindowPos(ImGuiViewport* pViewport);
		static void OnSetWindowSize(ImGuiViewport* pViewport, ImVec2 size);
		static ImVec2 OnGetWindowSize(ImGuiViewport* pViewport);
		static void OnSetWindowFocus(ImGuiViewport* pViewport);
		static bool OnGetWindowFocus(ImGuiViewport* pViewport);
		static bool OnGetWindowMinimized(ImGuiViewport* pViewport);
		static void OnSetWindowTitle(ImGuiViewport* pViewport, const char* pTitle);
		static void OnSetWindowAlpha(ImGuiViewport* pViewport, float alpha);

	private:
		Engine::WindowService* m_windowService = nullptr;
		PAL::Cursor* m_cursor = nullptr;

		PAL::Window* m_captureWindow = nullptr;
		u32 m_heldButtons = 0;
		b8 m_appFocused = false;
	};
}