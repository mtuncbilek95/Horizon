#include <Runtime/PAL/Window/Window.h>
#include <Runtime/Win32/Helpers/Win32WindowHelpers.h>

#include <Windows.h>
#include <windowsx.h>

namespace Horizon
{
	namespace
	{
		HWND ToHWND(OSHandle h) { return (HWND)(uptr(h)); }
		HINSTANCE ToHINSTANCE(OSInstance i) { return (HINSTANCE)(uptr(i)); }
		OSHandle ToOSHandle(HWND h) { return (OSHandle)(uptr(h)); }
		OSInstance ToOSInstance(HMODULE m) { return (OSInstance)(uptr(m)); }
		Window* GetWindowFromHandle(HWND hwnd) { return (Window*)GetWindowLongPtr(hwnd, -21); }

		LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			switch (msg)
			{
			case WM_CREATE:
			{
				Window* pWindow = (Window*)((LPCREATESTRUCT)lParam)->lpCreateParams;
				SetWindowLongPtr(hwnd, -21, (LONG_PTR)pWindow);
				break;
			}
			case WM_CLOSE:
			{
				Window* pWindow = GetWindowFromHandle(hwnd);

				InputMessage message = {};
				message.type = InputMessageType::Close;

				pWindow->SubmitMessage(message);
				break;
			}
			case WM_MOVE:
			{
				Window* pWindow = GetWindowFromHandle(hwnd);

				InputMessage message = {};
				message.type = InputMessageType::Move;
				message.posX = GET_X_LPARAM(lParam);
				message.posY = GET_Y_LPARAM(lParam);

				pWindow->SubmitMessage(message);
				break;
			}
			case WM_SIZE:
			{
				Window* pWindow = GetWindowFromHandle(hwnd);

				InputMessage message = {};
				message.type = InputMessageType::Resize;
				message.width = LOWORD(lParam);
				message.height = HIWORD(lParam);

				pWindow->SubmitMessage(message);
				break;
			}
			case WM_MOUSEMOVE:
			{
				Window* pWindow = GetWindowFromHandle(hwnd);

				InputMessage message = {};
				message.type = InputMessageType::MouseMove;
				message.mouseX = GET_X_LPARAM(lParam);
				message.mouseY = GET_Y_LPARAM(lParam);

				pWindow->SubmitMessage(message);
				break;
			}
			case WM_MOUSEWHEEL:
			{
				Window* pWindow = GetWindowFromHandle(hwnd);

				InputMessage message = {};
				message.type = InputMessageType::MouseScroll;
				message.scrollY = f32(GET_WHEEL_DELTA_WPARAM(wParam)) / f32(WHEEL_DELTA);

				pWindow->SubmitMessage(message);
				break;
			}
			case WM_LBUTTONDOWN:
			{
				Window* pWindow = GetWindowFromHandle(hwnd);

				InputMessage message = {};
				message.type = InputMessageType::MouseDown;
				message.button = MouseButton::Left;

				pWindow->SubmitMessage(message);
				break;
			}
			case WM_LBUTTONUP:
			{
				Window* pWindow = GetWindowFromHandle(hwnd);

				InputMessage message = {};
				message.type = InputMessageType::MouseUp;
				message.button = MouseButton::Left;

				pWindow->SubmitMessage(message);
				break;
			}
			case WM_RBUTTONDOWN:
			{
				Window* pWindow = GetWindowFromHandle(hwnd);

				InputMessage message = {};
				message.type = InputMessageType::MouseDown;
				message.button = MouseButton::Right;

				pWindow->SubmitMessage(message);
				break;
			}
			case WM_RBUTTONUP:
			{
				Window* pWindow = GetWindowFromHandle(hwnd);

				InputMessage message = {};
				message.type = InputMessageType::MouseUp;
				message.button = MouseButton::Right;

				pWindow->SubmitMessage(message);
				break;
			}
			case WM_MBUTTONDOWN:
			{
				Window* pWindow = GetWindowFromHandle(hwnd);

				InputMessage message = {};
				message.type = InputMessageType::MouseDown;
				message.button = MouseButton::Middle;

				pWindow->SubmitMessage(message);
				break;
			}
			case WM_MBUTTONUP:
			{
				Window* pWindow = GetWindowFromHandle(hwnd);

				InputMessage message = {};
				message.type = InputMessageType::MouseUp;
				message.button = MouseButton::Middle;

				pWindow->SubmitMessage(message);
				break;
			}
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			{
				Window* pWindow = GetWindowFromHandle(hwnd);

				InputMessage message = {};
				message.type = InputMessageType::KeyDown;
				message.key = WindowHelpers::ToWinKey(wParam);

				pWindow->SubmitMessage(message);
				break;
			}
			case WM_KEYUP:
			case WM_SYSKEYUP:
			{
				Window* pWindow = GetWindowFromHandle(hwnd);

				InputMessage message = {};
				message.type = InputMessageType::KeyUp;
				message.key = WindowHelpers::ToWinKey(wParam);

				pWindow->SubmitMessage(message);
				break;
			}
			case WM_CHAR:
			{
				Window* pWindow = GetWindowFromHandle(hwnd);

				InputMessage message = {};
				message.type = InputMessageType::Char;
				message.character = u32(wParam);

				pWindow->SubmitMessage(message);
				break;
			}
			case WM_DROPFILES:
			{
				Window* pWindow = GetWindowFromHandle(hwnd);

				InputMessage message = {};
				message.type = InputMessageType::DropFiles;

				HDROP hDrop = (HDROP)wParam;
				u32 fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);

				for (u32 it = 0; it < fileCount; it++)
				{
					c8 buffer[MAX_PATH];
					DragQueryFile(hDrop, it, buffer, MAX_PATH);

					message.filePaths.PushBack(std::filesystem::path(buffer));
				}
				DragFinish(hDrop);

				pWindow->SubmitMessage(message);
				break;
			}
			case WM_SETCURSOR:
			{
				if (LOWORD(lParam) == HTCLIENT)
				{
					SetCursor(LoadCursor(NULL, IDC_ARROW));
					return TRUE;
				}

				return DefWindowProc(hwnd, msg, wParam, lParam);
			}
			default:
				return DefWindowProc(hwnd, msg, wParam, lParam);
			}

			return 0;
		}

		void EnableFlags(HWND hwnd, WindowFlags flags)
		{
			if (HasFlag(flags, WindowFlags::EnableDragDrop))
				DragAcceptFiles(hwnd, TRUE);
		}
	}

	Window::Window(const WindowDesc& desc) : m_handle(OSHandle{}), 
		m_instance(OSInstance{}), m_visible(false), m_active(false)
	{
		constexpr char WindowClassName[] = "HorizonRuntimeWindowClassName";

		HINSTANCE instance = GetModuleHandle(nullptr);
		m_instance = ToOSInstance(instance);

		WNDCLASSEX windowClass = {};
		windowClass.cbSize = sizeof(windowClass);
		windowClass.cbClsExtra = 0;
		windowClass.cbWndExtra = sizeof(Window*);
		windowClass.hInstance = instance;
		windowClass.lpfnWndProc = WindowProc;
		windowClass.lpszClassName = WindowClassName;
		windowClass.lpszMenuName = NULL;
		windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		windowClass.hIcon = LoadIcon(NULL, IDI_WINLOGO);
		windowClass.hIconSm = NULL;
		windowClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		windowClass.style = 0;

		if (RegisterClassEx(&windowClass) == 0)
		{
			const DWORD err = GetLastError();
			if (err != ERROR_CLASS_ALREADY_EXISTS)
				Terminal::Fatal("Win32Window", "RegisterClassEx failed (err {}).", err);
		}

		HWND hwnd = CreateWindowEx(WS_EX_ACCEPTFILES, WindowClassName, desc.titleName.GetSource(), WS_OVERLAPPEDWINDOW,
			m_posX, m_posY, desc.width, desc.height, nullptr, nullptr, instance, this);

		if (hwnd == nullptr)
			Terminal::Fatal("Win32Window", "CreateWindowEx failed (err {}).", GetLastError());

		m_handle = ToOSHandle(hwnd);
		m_active = true;

		EnableFlags(hwnd, desc.flags);
	}

	Window::~Window()
	{
		DestroyWindow(ToHWND(m_handle));

		m_instance = OSInstance{};
		m_handle = OSHandle{};
	}

	void Window::SubmitMessage(const InputMessage& msg)
	{
		switch (msg.type)
		{
		case InputMessageType::Close:
			m_active = false;
			break;

		case InputMessageType::Move:
			m_posX = msg.posX; m_posY = msg.posY;
			break;

		case InputMessageType::Resize:
			m_desc.width = msg.width; m_desc.height = msg.height;
			break;
		}

		m_messages.PushBack(msg);
	}

	void Window::Show()
	{
		m_visible = ShowWindow(ToHWND(m_handle), SW_SHOW);
	}

	void Window::Hide()
	{
		m_visible = ShowWindow(ToHWND(m_handle), SW_HIDE);
	}

	void Window::PollEvents()
	{
		m_messages.Free();

		MSG msg = {};
		while (PeekMessage(&msg, ToHWND(m_handle), 0, 0, PM_REMOVE) != 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}
