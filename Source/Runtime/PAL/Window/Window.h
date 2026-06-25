#pragma once

#include <Runtime/PAL/Window/WindowMode.h>
#include <Runtime/PAL/Window/WindowFlags.h>
#include <Runtime/PAL/Window/InputMessage.h>

#include <string>
#include <vector>

namespace Horizon
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

		const std::vector<InputMessage>& GetMessages() const { return m_messages; }
		void SubmitMessage(const InputMessage& msg);

		WindowRect GetRect() const { return { m_desc.width, m_desc.height, m_posX, m_posY }; }
		const std::string& GetName() const { return m_desc.titleName; }
		OSHandle GetOSHandle() const { return m_handle; }
		OSInstance GetOSInstance() const { return m_instance; }

		b8 GetVisible() const { return m_visible; }
		b8 GetActive() const { return m_active; }

		void Show();
		void Hide();
		void PollEvents();

	private:
		WindowDesc m_desc;

		std::vector<InputMessage> m_messages;
		OSHandle m_handle = OSHandle{};
		OSInstance m_instance = OSInstance{};

		i32 m_posX = 100, m_posY = 100;
		b8 m_visible = false;
		b8 m_active = false;
	};
}