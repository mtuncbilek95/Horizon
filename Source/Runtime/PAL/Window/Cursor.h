#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/PAL/Window/CursorType.h>
#include <Runtime/PAL/Window/WindowListener.h>

namespace Horizon::PAL
{
	class RUNTIME_API Cursor final : public WindowListener
	{
	public:
		Cursor() = default;
		~Cursor();

		Cursor(const Cursor&) = delete;
		Cursor(Cursor&&) = delete;

		Cursor& operator=(const Cursor&) = delete;
		Cursor& operator=(Cursor&&) = delete;

		CursorType GetShape() const { return m_shape; }
		void SetShape(CursorType type);

		Window* GetClipTarget() const { return m_clipTarget; }
		void SetClipTarget(Window* pWindow);

		void Update();

		void OnWindowFocusChanged(Window& window, b8 focused) final;
		void OnWindowRectChanged(Window& window) final;
		b8 OnWindowSetCursor(Window& window) final;
		void OnWindowDestroyed(Window& window) final;

	private:
		void RefreshClip();

	private:
		CursorType m_shape = CursorType::Arrow;
		Window* m_clipTarget = nullptr;
		b8 m_clipApplied = false;
	};
}