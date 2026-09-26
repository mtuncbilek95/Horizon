#include <Runtime/PAL/Window/Window.h>

#include <Runtime/Log/Terminal.h>
#include <Runtime/Win32/Helpers/Win32WindowHelpers.h>

#include <Windows.h>
#include <windowsx.h>
#include <dwmapi.h>

namespace Horizon::PAL
{
	namespace
	{
		HWND ToHWND(OSHandle h) { return (HWND)(uptr(h)); }
		HINSTANCE ToHINSTANCE(OSInstance i) { return (HINSTANCE)(uptr(i)); }
		OSHandle ToOSHandle(HWND h) { return (OSHandle)(uptr(h)); }
		OSInstance ToOSInstance(HMODULE m) { return (OSInstance)(uptr(m)); }
		Window* GetWindowFromHandle(HWND hwnd) { return (Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA); }

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

		ChromeButton ToChromeButton(WPARAM hitCode)
		{
			switch (hitCode)
			{
			case HTMINBUTTON:
				return ChromeButton::Minimize;
			case HTMAXBUTTON:
				return ChromeButton::Maximize;
			case HTCLOSE:
				return ChromeButton::Close;
			default:
				return ChromeButton::None;
			}
		}

		LRESULT HitTestCustomFrame(HWND hwnd, Window* pWindow, i32 screenX, i32 screenY)
		{
			POINT pt = { screenX, screenY };
			ScreenToClient(hwnd, &pt);

			RECT client = {};
			GetClientRect(hwnd, &client);

			const WindowChrome& chrome = pWindow->GetChrome();
			const i32 border = i32(chrome.resizeBorder);

			if (!pWindow->GetMaximized())
			{
				const b8 top = pt.y < border;
				const b8 bottom = pt.y >= client.bottom - border;
				const b8 left = pt.x < border;
				const b8 right = pt.x >= client.right - border;

				if (top && left)
					return HTTOPLEFT;
				if (top && right)
					return HTTOPRIGHT;
				if (bottom && left)
					return HTBOTTOMLEFT;
				if (bottom && right)
					return HTBOTTOMRIGHT;
				if (top)
					return HTTOP;
				if (bottom)
					return HTBOTTOM;
				if (left)
					return HTLEFT;
				if (right)
					return HTRIGHT;
			}

			if (pt.y >= i32(chrome.captionHeight))
				return HTCLIENT;

			if (chrome.closeButton.Contains(pt.x, pt.y))
				return HTCLOSE;
			if (chrome.maximizeButton.Contains(pt.x, pt.y))
				return HTMAXBUTTON;
			if (chrome.minimizeButton.Contains(pt.x, pt.y))
				return HTMINBUTTON;

			for (const auto& area : chrome.clientAreas)
			{
				if (area.Contains(pt.x, pt.y))
					return HTCLIENT;
			}

			return HTCAPTION;
		}

		void ExecuteChromeButton(HWND hwnd, ChromeButton button)
		{
			switch (button)
			{
			case ChromeButton::Minimize:
				ShowWindow(hwnd, SW_MINIMIZE);
				break;
			case ChromeButton::Maximize:
				ShowWindow(hwnd, IsZoomed(hwnd) ? SW_RESTORE : SW_MAXIMIZE);
				break;
			case ChromeButton::Close:
				PostMessage(hwnd, WM_CLOSE, 0, 0);
				break;
			default:
				break;
			}
		}

		void SubmitScreenMouseMove(HWND hwnd, Window* pWindow, i32 screenX, i32 screenY)
		{
			POINT pt = { screenX, screenY };
			ScreenToClient(hwnd, &pt);

			InputMessage message = {};
			message.type = InputMessageType::MouseMove;
			message.mouseX = pt.x;
			message.mouseY = pt.y;

			pWindow->SubmitMessage(message);
		}

		LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			switch (msg)
			{
			case WM_NCCREATE:
			{
				Window* pWindow = (Window*)((LPCREATESTRUCT)lParam)->lpCreateParams;
				SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pWindow);
				return DefWindowProc(hwnd, msg, wParam, lParam);
			}
			case WM_NCCALCSIZE:
			{
				Window* pWindow = GetWindowFromHandle(hwnd);

				if (pWindow == nullptr || !pWindow->GetCustomFrame() || wParam == FALSE)
					return DefWindowProc(hwnd, msg, wParam, lParam);

				if (IsZoomed(hwnd))
				{
					NCCALCSIZE_PARAMS* pParams = (NCCALCSIZE_PARAMS*)lParam;

					const UINT dpi = GetDpiForWindow(hwnd);
					const i32 padded = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);
					const i32 frameX = GetSystemMetricsForDpi(SM_CXSIZEFRAME, dpi) + padded;
					const i32 frameY = GetSystemMetricsForDpi(SM_CYSIZEFRAME, dpi) + padded;

					pParams->rgrc[0].left += frameX;
					pParams->rgrc[0].top += frameY;
					pParams->rgrc[0].right -= frameX;
					pParams->rgrc[0].bottom -= frameY;
				}

				return 0;
			}
			case WM_NCHITTEST:
			{
				Window* pWindow = GetWindowFromHandle(hwnd);

				if (pWindow == nullptr || !pWindow->GetCustomFrame())
					return DefWindowProc(hwnd, msg, wParam, lParam);

				return HitTestCustomFrame(hwnd, pWindow, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			}
			case WM_NCMOUSEMOVE:
			{
				Window* pWindow = GetWindowFromHandle(hwnd);

				if (pWindow == nullptr || !pWindow->GetCustomFrame())
					return DefWindowProc(hwnd, msg, wParam, lParam);

				SubmitScreenMouseMove(hwnd, pWindow, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
				return DefWindowProc(hwnd, msg, wParam, lParam);
			}
			case WM_NCLBUTTONDOWN:
			{
				Window* pWindow = GetWindowFromHandle(hwnd);
				const ChromeButton button = ToChromeButton(wParam);

				if (pWindow == nullptr || !pWindow->GetCustomFrame() || button == ChromeButton::None)
					return DefWindowProc(hwnd, msg, wParam, lParam);

				pWindow->SetPressedChromeButton(button);
				return 0;
			}
			case WM_NCLBUTTONUP:
			{
				Window* pWindow = GetWindowFromHandle(hwnd);
				const ChromeButton button = ToChromeButton(wParam);

				if (pWindow == nullptr || !pWindow->GetCustomFrame() || button == ChromeButton::None)
					return DefWindowProc(hwnd, msg, wParam, lParam);

				const ChromeButton pressed = pWindow->GetPressedChromeButton();
				pWindow->SetPressedChromeButton(ChromeButton::None);

				if (pressed == button)
					ExecuteChromeButton(hwnd, button);

				return 0;
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
				pWindow->OnSizeState(wParam == SIZE_MAXIMIZED);

				InputMessage message = {};
				message.type = InputMessageType::Resize;
				message.width = LOWORD(lParam);
				message.height = HIWORD(lParam);

				pWindow->SubmitMessage(message);
				break;
			}
			case WM_SETFOCUS:
			{
				Window* pWindow = GetWindowFromHandle(hwnd);

				InputMessage message = {};
				message.type = InputMessageType::Focus;

				pWindow->SubmitMessage(message);
				break;
			}
			case WM_KILLFOCUS:
			{
				Window* pWindow = GetWindowFromHandle(hwnd);

				InputMessage message = {};
				message.type = InputMessageType::LostFocus;

				pWindow->SubmitMessage(message);
				break;
			}
			case WM_CAPTURECHANGED:
			{
				Window* pWindow = GetWindowFromHandle(hwnd);

				if ((HWND)lParam != hwnd)
					pWindow->OnCaptureLost();

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

					message.filePaths.PushBack(std::string(buffer));
				}
				DragFinish(hDrop);

				pWindow->SubmitMessage(message);
				break;
			}
			case WM_SETCURSOR:
			{
				if (LOWORD(lParam) == HTCLIENT)
				{
					Window* pWindow = GetWindowFromHandle(hwnd);
					::SetCursor(ToWin32Cursor(pWindow->GetCursorShape()));
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

			if (HasFlag(flags, WindowFlags::CustomTitleBar))
			{
				SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

				MARGINS margins = { 0, 0, 1, 0 };
				DwmExtendFrameIntoClientArea(hwnd, &margins);

				COLORREF borderColor = RGB(0x38, 0x38, 0x38);
				DwmSetWindowAttribute(hwnd, DWMWA_BORDER_COLOR, &borderColor, sizeof(borderColor));
			}
		}
	}

	Window::Window(const WindowDesc& desc) : m_desc(desc), m_handle(OSHandle{}),
		m_instance(OSInstance{}), m_visible(false), m_active(false),
		m_customFrame(HasFlag(desc.flags, WindowFlags::CustomTitleBar))
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

		HWND hwnd = CreateWindowEx(WS_EX_ACCEPTFILES, WindowClassName, desc.titleName.c_str(), WS_OVERLAPPEDWINDOW,
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

	void Window::SetMouseCapture(b8 enabled)
	{
		if (m_mouseCaptured == enabled)
			return;

		HWND hwnd = ToHWND(m_handle);

		if (enabled)
		{
			SetCapture(hwnd);
			m_mouseCaptured = true;
			return;
		}

		if (GetCapture() == hwnd)
			ReleaseCapture();

		m_mouseCaptured = false;
	}

	void Window::SetCursorClip(b8 enabled)
	{
		if (m_cursorClipped == enabled)
			return;

		m_cursorClipped = enabled;

		if (!enabled)
		{
			ClipCursor(nullptr);
			return;
		}

		HWND hwnd = ToHWND(m_handle);

		RECT client = {};
		GetClientRect(hwnd, &client);

		POINT topLeft = { client.left, client.top };
		POINT bottomRight = { client.right, client.bottom };
		ClientToScreen(hwnd, &topLeft);
		ClientToScreen(hwnd, &bottomRight);

		RECT screen = { topLeft.x, topLeft.y, bottomRight.x, bottomRight.y };
		ClipCursor(&screen);
	}

	void Window::SetCursorShape(CursorType type)
	{
		if (m_cursorType == type)
			return;

		m_cursorType = type;
		::SetCursor(ToWin32Cursor(type));
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
		m_messages.Clear();

		MSG msg = {};
		while (PeekMessage(&msg, ToHWND(m_handle), 0, 0, PM_REMOVE) != 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}