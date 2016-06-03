#pragma once

#include <Windows.h>
#include "Dispatcher.h"
#include <unordered_map>

enum class Key
{
	LEFT_ARROW,
	UP_ARROW,
	RIGHT_ARROW,
	DOWN_ARROW,
	A,
	D,
	E,
	F,
	Q,
	R,
	S,
	T,
	W,
	Y
};

class Window
{
public:
	Window(LONG width, LONG height);
	HWND getHandle();
	POINT getMousePosition();

public:
	Dispatcher<void(Key)> onKeyDown;
	Dispatcher<void(Key)> onKeyUp;
	Dispatcher<void(int, int)> onLeftMouseDown;
	Dispatcher<void(int, int)> onLeftMouseUp;
	Dispatcher<void(int, int)> onRightMouseDown;
	Dispatcher<void(int, int)> onRightMouseUp;
	Dispatcher<void(int, int)> onMouseMove;

private:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	HWND hWnd;
	static std::unordered_map<WPARAM, Key> keyByWParam;
};