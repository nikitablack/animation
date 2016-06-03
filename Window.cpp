#include "Headers/Window.h"
#include <stdexcept>
#include <Windowsx.h>

using namespace std;

unordered_map<WPARAM, Key> Window::keyByWParam
{
	{ 37, Key::LEFT_ARROW },
	{ 38, Key::UP_ARROW },
	{ 39, Key::RIGHT_ARROW },
	{ 40, Key::DOWN_ARROW },
	{ 65, Key::A },
	{ 68, Key::D },
	{ 69, Key::E },
	{ 70, Key::F },
	{ 81, Key::Q },
	{ 82, Key::R },
	{ 83, Key::S },
	{ 84, Key::T },
	{ 87, Key::W },
	{ 89, Key::Y }
};


Window::Window(LONG width, LONG height)
{
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = &WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = (HMODULE)GetModuleHandle(0);
	wcex.hIcon = LoadIcon(NULL, IDI_SHIELD);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = "WindowClass";
	wcex.hIconSm = LoadIcon(NULL, IDI_WARNING);

	if (RegisterClassEx(&wcex) == 0)
	{
		throw(runtime_error{ "Error registering window." });
	}

	RECT rect{ 0, 0, width, height };
	AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, 0);

	hWnd = CreateWindowEx(0, "WindowClass", "Hello, Triangle!", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, nullptr, this);
	if (!hWnd)
	{
		throw(runtime_error{ "Error creating window." });
	}

	ShowWindow(hWnd, SW_SHOW);
}

HWND Window::getHandle()
{
	return hWnd;
}

POINT Window::getMousePosition()
{
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(hWnd, &p);
	return p;
}

LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (message)
	{
	case WM_CREATE:
	{
		LPCREATESTRUCT pCreateStruct{ reinterpret_cast<LPCREATESTRUCT>(lParam) };
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
	{
		auto it = keyByWParam.find(wParam);
		if (it != keyByWParam.end())
		{
			window->onKeyDown(it->second);
		}
	}
		break;
	case WM_KEYUP:
	{
		auto it = keyByWParam.find(wParam);
		if (it != keyByWParam.end())
		{
			window->onKeyUp(it->second);
		}
	}
		break;
	case WM_LBUTTONDOWN:
		window->onLeftMouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_LBUTTONUP:
		window->onLeftMouseUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_MOUSEMOVE:
		window->onMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}