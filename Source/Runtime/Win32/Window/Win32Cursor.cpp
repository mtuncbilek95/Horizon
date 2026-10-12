#include <Runtime/PAL/Window/Cursor.h>
#include <Runtime/PAL/Window/Window.h>

#include <Windows.h>

namespace Horizon::PAL
{
	namespace
	{
		HWND ToHWND(OSHandle h) { return (HWND)(uptr(h)); }

		HCURSOR ToWin32Cursor(CursorType type)
		{
			switch (type)
			{
			case CursorType::Arrow:
				return LoadCursor(NULL, IDC_ARROW);
			case CursorType::TextInput:
				return LoadCursor(NULL, IDC_IBEAM);
			case CursorType::ResizeAll:
				return LoadCursor(NULL, IDC_SIZEALL);
			case CursorType::ResizeNS:
				return LoadCursor(NULL, IDC_SIZENS);
			case CursorType::ResizeEW:
				return LoadCursor(NULL, IDC_SIZEWE);
			case CursorType::ResizeNESW:
				return LoadCursor(NULL, IDC_SIZENESW);
			case CursorType::ResizeNWSE:
				return LoadCursor(NULL, IDC_SIZENWSE);
			case CursorType::Hand:
				return LoadCursor(NULL, IDC_HAND);
			case CursorType::Wait:
				return LoadCursor(NULL, IDC_WAIT);
			case CursorType::Progress:
				return LoadCursor(NULL, IDC_APPSTARTING);
			case CursorType::NotAllowed:
				return LoadCursor(NULL, IDC_NO);
			case CursorType::Hidden:
				return NULL;
			default:
				return LoadCursor(NULL, IDC_ARROW);
			}
		}
	}

	Cursor::~Cursor()
	{
		if (m_clipApplied)
		{
			ClipCursor(nullptr);
			m_clipApplied = false;
		}
	}

	void Cursor::SetShape(CursorType type)
	{
		if (m_shape == type)
			return;

		m_shape = type;
		::SetCursor(ToWin32Cursor(type));
	}

	void Cursor::SetClipTarget(Window* pWindow)
	{
		if (m_clipTarget == pWindow)
			return;

		m_clipTarget = pWindow;
		RefreshClip();
	}

	void Cursor::Update()
	{
		RefreshClip();
	}

	void Cursor::OnWindowFocusChanged(Window& window, b8 focused)
	{
		if (&window != m_clipTarget)
			return;

		RefreshClip();
	}

	void Cursor::OnWindowRectChanged(Window& window)
	{
		if (&window != m_clipTarget)
			return;

		RefreshClip();
	}

	b8 Cursor::OnWindowSetCursor(Window& window)
	{
		::SetCursor(ToWin32Cursor(m_shape));
		return true;
	}

	void Cursor::OnWindowDestroyed(Window& window)
	{
		if (&window != m_clipTarget)
			return;

		m_clipTarget = nullptr;
		RefreshClip();
	}

	void Cursor::RefreshClip()
	{
		b8 shouldClip = false;
		HWND hwnd = nullptr;

		if (m_clipTarget != nullptr)
		{
			hwnd = ToHWND(m_clipTarget->GetOSHandle());
			shouldClip = m_clipTarget->GetFocused() && !IsIconic(hwnd);
		}

		if (!shouldClip)
		{
			if (m_clipApplied)
			{
				ClipCursor(nullptr);
				m_clipApplied = false;
			}

			return;
		}

		RECT client = {};
		GetClientRect(hwnd, &client);

		POINT topLeft = { client.left, client.top };
		POINT bottomRight = { client.right, client.bottom };
		ClientToScreen(hwnd, &topLeft);
		ClientToScreen(hwnd, &bottomRight);

		RECT screen = { topLeft.x, topLeft.y, bottomRight.x, bottomRight.y };
		ClipCursor(&screen);
		m_clipApplied = true;
	}
}