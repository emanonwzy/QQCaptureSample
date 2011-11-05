#include "stdafx.h"
#include "resource.h"
#include "MainWnd.h"

HINSTANCE g_hInst;
CMainWnd g_mainWnd;
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   PSTR szCmdLine, int iCmdShow)
{
	static TCHAR	szAppName[] = TEXT("CaptureWin");
	HWND			hwnd;
	MSG				msg;
	WNDCLASS		wndclass;

	g_hInst = hInstance;

	wndclass.style		 = CS_HREDRAW|CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra	 = 0;
	wndclass.cbWndExtra  = 0;
	wndclass.hInstance   = hInstance;
	wndclass.hIcon		 = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor	 = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName =  szAppName;

	if(!RegisterClass(&wndclass))
	{
		return 0;
	}

	hwnd = CreateWindow(szAppName,
						TEXT("CaptureWindow"),
						WS_VISIBLE|WS_POPUP,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						NULL,
						NULL,
						hInstance,
						NULL);

	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);

	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC			hdc;
	PAINTSTRUCT ps;	
	int			cx;
	int			cy;
	HMENU		hMenu;
	DWORD		dwRet;
	switch(message)
	{
	case WM_CREATE:
		SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		SetForegroundWindow(hwnd);
		return 0;
	case WM_SIZE:
		cx = GetSystemMetrics(SM_CXSCREEN);
		cy = GetSystemMetrics(SM_CYSCREEN);
		MoveWindow(hwnd, 0, 0, cx, cy, TRUE);
		g_mainWnd.Init(g_hInst, hwnd, IDR_RMENU, cx, cy);
		return 0;
	case WM_KEYDOWN:
		switch(wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;
		}
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);		
		g_mainWnd.OnDraw(hdc);
		EndPaint(hwnd, &ps);
		return 0;
	case WM_INITMENUPOPUP:
		hMenu = g_mainWnd.GetMenu();
		dwRet = CheckMenuItem(hMenu, IDC_MENU_MOVE, (g_mainWnd.GetTrackType() == MOVE) ? MF_CHECKED : MF_UNCHECKED );
		CheckMenuItem(hMenu, IDC_MENU_LINE, (g_mainWnd.GetTrackType() == LINE) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hMenu, IDC_MENU_TEXT, (g_mainWnd.GetTrackType() == TEXT) ? MF_CHECKED : MF_UNCHECKED );
		return 0;
	case WM_COMMAND:
		hMenu = g_mainWnd.GetMenu();
		switch(LOWORD(wParam))
		{	
		case IDC_MENU_SAVE:
			g_mainWnd.SaveJPEG();
			break;
		case IDC_MENU_MOVE:
			g_mainWnd.SetTrackType(MOVE);
			break;
		case IDC_MENU_LINE:
			g_mainWnd.SetTrackType(LINE);
			break;
		case IDC_MENU_TEXT:
			g_mainWnd.SetTrackType(TEXT);
			break;
		}
		InvalidateRect(hwnd, NULL, FALSE);
		return 0;
	case WM_SETCURSOR:
		if(g_mainWnd.SetCursor(wParam, lParam))
			return 0;
		break;
	case WM_LBUTTONDOWN:
		g_mainWnd.OnLbuttonDown(wParam, lParam);
		return 0;
	case WM_LBUTTONUP:
		g_mainWnd.OnLbuttonUp(wParam, lParam);
		return 0;
	case WM_RBUTTONUP:
		g_mainWnd.OnRButtonUp(wParam, lParam);
		return 0;
	case WM_MOUSEMOVE:
		g_mainWnd.OnMouseMove(wParam, lParam);
		return 0;
	case WM_DESTROY:
		g_mainWnd.Release();
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}