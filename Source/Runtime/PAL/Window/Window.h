#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/PAL/Window/WindowMode.h>
#include <Runtime/PAL/Window/WindowFlags.h>
#include <Runtime/PAL/Window/WindowChrome.h>
#include <Runtime/PAL/Window/InputMessage.h>
#include <Runtime/PAL/Window/CursorType.h>

#include <Runtime/Containers/List.h>
#include <Runtime/Containers/ReadOnlyList.h>

#include <string>
#include <span>

namespace Horizon::PAL
{
	enum class OSHandle : uptr {};
	enum class OSInstance : uptr {};

	struct WindowRect
	{
		u32 width, height;
		i32 posX, posY;
	};

	struct WindowDesc final
	{
		std::string titleName = "Horizon";
		u32 width = 1920, height = 1080;
		WindowMode mode = WindowMode::Borderless;
		WindowFlags flags = WindowFlags::None;
	};

	class H_EXPORT Window final
	{
	public:
		Window(const WindowDesc& desc);
		~Window();

		Window(const Window&) = delete;
		Window(Window&&) = delete;

		Window& operator=(const Window&) = delete;
		Window& operator=(Window&&) = delete;

		ReadOnlyList<const InputMessage> GetMessages() const { return m_messages; }
		void SubmitMessage(const InputMessage& msg);

		WindowRect GetRect() const { return { m_desc.width, m_desc.height, m_posX, m_posY }; }
		const std::string& GetName() const { return m_desc.titleName; }
		OSHandle GetOSHandle() const { return m_handle; }
		OSInstance GetOSInstance() const { return m_instance; }

		b8 GetVisible() const { return m_visible; }
		b8 GetActive() const { return m_active; }
		b8 GetMaximized() const { return m_maximized; }
		b8 GetCustomFrame() const { return m_customFrame; }
		CursorType GetCursorShape() const { return m_cursorType; }

		const WindowChrome& GetChrome() const { return m_chrome; }
		void SetChrome(const WindowChrome& chrome) { m_chrome = chrome; }

		ChromeButton GetPressedChromeButton() const { return m_pressedChromeButton; }
		void SetPressedChromeButton(ChromeButton button) { m_pressedChromeButton = button; }

		void SetMouseCapture(b8 enabled);
		void SetCursorClip(b8 enabled);
		void SetCursorShape(CursorType type);

		void OnCaptureLost() { m_mouseCaptured = false; }
		void OnSizeState(b8 maximized) { m_maximized = maximized; }

		void Show();
		void Hide();
		void PollEvents();

	private:
		WindowDesc m_desc;
		WindowChrome m_chrome;

		List<InputMessage> m_messages;
		OSHandle m_handle = OSHandle{};
		OSInstance m_instance = OSInstance{};

		i32 m_posX = 100, m_posY = 100;
		
		b8 m_visible = false;
		b8 m_active = false;

		b8 m_maximized = false;
		b8 m_customFrame = false;

		b8 m_mouseCaptured = false;
		b8 m_cursorClipped = false;

		CursorType m_cursorType = CursorType::Arrow;
		ChromeButton m_pressedChromeButton = ChromeButton::None;
	};
}