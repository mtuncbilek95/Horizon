#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <memory>
#include <string>

namespace Horizon
{
	enum class WindowMode : u8
	{
		Windowed,
		Fullscreen,
		Borderless
	};

	struct WindowDesc
	{
		std::string name = "Horizon - Sponza";
		glm::uvec2 size = { 1920, 1080 };
		WindowMode mode = WindowMode::Windowed;
	};

	class Window
	{
	public:
		Window(const WindowDesc& desc = WindowDesc());
		~Window();

		void Show();
		void Hide();
		void PollEvents();

		void SetSize(const glm::uvec2& size) { m_desc.size = size; }

		const std::string& GetTitle() const { return m_desc.name; }
		const glm::uvec2& GetSize() const { return m_desc.size; }
		WindowMode GetMode() const { return m_desc.mode; }

		b8 IsHidden() const { return m_hidden; }
		b8 IsActive() const;

		void* GetAPIHandle() const;
		void* GetInstance() const { return m_instance; }
		void* GetOSHandle() const { return m_osHandle; }

	private:
		WindowDesc m_desc;

		void* m_instance = nullptr;
		void* m_osHandle = nullptr;

		b8 m_hidden = true;
	};

	inline Window& MainWindow()
	{
		static Window instance;
		return instance;
	}
}
