#include "stdafx.h"
#include "Edit.h"

#define EDITID 1000
#define ID_SCAN_TIMER 0x1234
#define SCAN_TIMER	100
#define MAX_SCANCOUNT 2

CEdit::CEdit()
{
	m_type = EDIT_INPUT;
	m_hwnd = NULL;
	m_nScanCount = 0;
	m_Cursor = ::LoadCursor(NULL, IDC_SIZEALL);
	m_hFont = (HFONT)GetStockObject(SYSTEM_FIXED_FONT);
}

HWND CEdit::Create(HINSTANCE hInst, HWND hWndParent, DWORD dwStyle, const RECT& rect, const RECT& rectMax)
{
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	m_rectMax = rectMax;
	m_hwnd = CreateWindow("edit",
				 NULL,
				 dwStyle,
				 rect.left,
				 rect.top,
				 width,
				 height,
				 hWndParent,
				 (HMENU)EDITID,
				 hInst,
				 NULL);
	SendMessage(m_hwnd, WM_SETFONT, (WPARAM)m_hFont, 0);
	if(m_hwnd)
	{
		SetProp(m_hwnd, "CEDIT", (HANDLE)this);
	}
	m_oldProc = SetWindowLong(m_hwnd, GWL_WNDPROC, (LONG)EditProc);
	return m_hwnd;
}

void CEdit::Destroy()
{
	DestroyWindow(m_hwnd);
	m_hwnd = NULL;
}

BOOL CEdit::GetText(char* strEdit, int nMaxCount)
{
	if(m_hwnd)
	{
		GetWindowText(m_hwnd, strEdit, nMaxCount);
	}

	return (m_hwnd != NULL);
}

HWND CEdit::GetHwnd()
{
	return m_hwnd;
}

void CEdit::GetFontSize(int& nFontWidth, int& nFontHeight)
{
	TEXTMETRIC txt;
	HDC hdc = GetDC(m_hwnd);
	HFONT hOldFont = (HFONT)SelectObject(hdc, GetStockObject(SYSTEM_FIXED_FONT));
	GetTextMetrics(hdc, &txt);
	nFontWidth = txt.tmMaxCharWidth;
	nFontHeight = txt.tmHeight;
	SelectObject(hdc, hOldFont);
	ReleaseDC(m_hwnd, hdc);
}

void CEdit::SetFocus()
{
	if(m_hwnd)
		::SetFocus(m_hwnd);
}

void CEdit::ChangeEditSize()
{
	int nFontWidth;
	int nFontHeight;
	RECT rectClient;
	POINT point;
	GetCaretPos(&point);
	GetClientRect(m_hwnd, &rectClient);
	GetFontSize(nFontWidth, nFontHeight);
	if(point.x == 0 )//|| point.x + nFontWidth>= m_rectMax.right
	{
		rectClient.bottom += nFontHeight;
		if(rectClient.bottom > m_rectMax.bottom)
		{
			rectClient.bottom = m_rectMax.bottom;
		}
	}
	if(point.x + nFontWidth>= m_rectMax.right)
	{
		SendMessage(m_hwnd, WM_KEYDOWN, VK_RETURN, 0);
	}
	if(point.x + 3 * nFontWidth > rectClient.right)
	{
		rectClient.right = point.x + 3 * nFontWidth;
		if(rectClient.right > m_rectMax.right)
		{
			rectClient.right = m_rectMax.right;
		}
	}
	::SetWindowPos(m_hwnd, 0, 0, 0, rectClient.right, rectClient.bottom, SWP_NOMOVE|SWP_NOZORDER);
	UpdateWindow(m_hwnd);
}

BOOL CEdit::SetCursor(WPARAM wParam, LPARAM lParam)
{
	if((HWND)wParam != m_hwnd || LOWORD(lParam) != HTCLIENT)
		return FALSE;
	if(m_type == EDIT_MOVE)
	{
		::SetCursor(m_Cursor);
		return TRUE;
	}

	return FALSE;
}

void CEdit::OnPaint(HDC hdc)
{
	RECT rect;
	GetClientRect(m_hwnd, &rect);
	DrawEdittext(hdc, &rect);
	DrawEditEdge(hdc, &rect);
}

void CEdit::DrawEdittext(HDC hdc, RECT *pRect)
{
	HFONT hOldFont = (HFONT)SelectObject(hdc, GetStockObject(SYSTEM_FIXED_FONT));
	int nOldMode = SetBkMode(hdc, TRANSPARENT);
	int nOldColor = SetTextColor(hdc, RGB(255, 0, 0));

	char str[1024];
	memset(str, 0, 1024);
	GetText(str, 1024);
	
	DrawText(hdc, str, -1, pRect, DT_LEFT|DT_TOP);
	SetTextColor(hdc, nOldColor);
	SetBkMode(hdc, nOldMode);
	SelectObject(hdc, hOldFont);
}
	
void CEdit::DrawEditEdge(HDC hdc, RECT* pRect)
{
	HPEN hPen = CreatePen(PS_DOT, 1, RGB(0, 0, 0));
	HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
	int nOldROP = SetROP2(hdc, R2_COPYPEN);

	Rectangle(hdc, pRect->left, pRect->top, pRect->right, pRect->bottom);

	SetROP2(hdc, nOldROP);
	SelectObject(hdc, hOldBrush);
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);
}

LRESULT CALLBACK CEdit::EditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CEdit* pEdit = (CEdit*)GetProp(hwnd, "CEDIT");
	if(pEdit && pEdit->GetHwnd() != NULL)
	{
		return pEdit->InEditProc(hwnd, msg, wParam, lParam);
	}
	else
	{
		return DefWindowProc(hwnd, msg, wParam, lParam) ;
	}
}

LRESULT CEdit::InEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	switch(msg)
	{
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		OnPaint(hdc);
		EndPaint(hwnd, &ps);
		return 0;
	case WM_CHAR:
		if(wParam != VK_BACK && wParam != VK_DELETE)
			ChangeEditSize();
		InvalidateRect(hwnd, NULL, TRUE);
		break;
	case WM_LBUTTONDOWN:
		SetCapture(hwnd);
		m_nDx = LOWORD(lParam);
		m_nDy = HIWORD(lParam);
		if(m_type == EDIT_INPUT)
		{
			SetTimer(hwnd, ID_SCAN_TIMER, SCAN_TIMER, NULL);
		}
		ReleaseCapture();
		break;
	case WM_LBUTTONUP:
		KillTimer(hwnd, ID_SCAN_TIMER);	
		m_type = EDIT_INPUT;
		SendMessage(hwnd, WM_SETCURSOR, (WPARAM)hwnd, HTCLIENT);
		break;
	case WM_TIMER:
		if(GetAsyncKeyState(VK_LBUTTON) & 0x8000)
		{
			if(m_nScanCount < MAX_SCANCOUNT)
			{
				m_nScanCount++;
			}
			else
			{
				m_nScanCount = 0;
				m_type = EDIT_MOVE;
				KillTimer(hwnd, ID_SCAN_TIMER);
				SendMessage(hwnd, WM_SETCURSOR, (WPARAM)hwnd, HTCLIENT);
			}
		}
		else
		{
			m_nScanCount = 0;
			m_type = EDIT_INPUT;
			KillTimer(hwnd, ID_SCAN_TIMER);
		}
		return 0;
	case WM_MOUSEMOVE:
		if(m_type == EDIT_MOVE && wParam == MK_LBUTTON)
		{
			POINT point;
			GetCursorPos(&point);

			::SetWindowPos(hwnd, 0, point.x - m_nDx, point.y - m_nDy, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
			UpdateWindow(hwnd);
		}
		return 0;
	case WM_SETCURSOR:
		if(SetCursor(wParam, lParam))
			return 0;
		break;
	}
	return CallWindowProc((WNDPROC)m_oldProc, hwnd, msg, wParam, lParam);
}