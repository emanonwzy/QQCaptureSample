#include "stdafx.h"
#include "MainWnd.h"

#define TRANSPARENTRECT_WIDTH  280
#define TRANSPARENTRECT_HEIGHT 50

CMainWnd::CMainWnd()
{
	m_hBackBmp = NULL;	
	m_hScreenDIB = NULL;
	m_hForgroundBmp = NULL;
	m_dibbuffer = NULL;
	memset(&m_rectTransparent, 0, sizeof(RECT));
	m_trackType = MOVE;	

	m_firstpoint.x = 0;	
	m_firstpoint.y = 0;
	m_hMenu = NULL;
	m_hwnd = NULL;
	m_hInst = NULL;
}

void CMainWnd::Init(HINSTANCE hInst, HWND hwnd, DWORD dwMenuID, int cx, int cy)
{
	m_hInst = hInst;
	m_hwnd = hwnd;
	InitGDI(hwnd, cx, cy);
	InitMenu(dwMenuID);
}

void CMainWnd::InitGDI(HWND hwnd, int cx, int cy)
{
	HBITMAP hScreenBmp = m_bmpOperator.GetScreenBmp();
	m_hBackBmp = m_bmpOperator.GetDarkenBmp(hScreenBmp);

	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = cx;
	rect.bottom = cy;
	m_hScreenDIB = m_bmpOperator.GetDIBColorsFromHbitmap(m_dibbuffer, hScreenBmp, 32, rect);

	HDC hdc = GetDC(hwnd);
	m_hForgroundBmp = CreateCompatibleBitmap(hdc, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	ReleaseDC(hwnd, hdc);	

	DeleteObject(hScreenBmp);
}

void CMainWnd::InitMenu(DWORD dwID)
{
	m_hMenu = LoadMenu(NULL, MAKEINTRESOURCE(dwID));
	m_hMenu = GetSubMenu (m_hMenu, 0);
}

void CMainWnd::Release()
{
	if(m_hBackBmp)
	{
		DeleteObject(m_hBackBmp);
		m_hBackBmp = NULL;
	}
	if(m_hForgroundBmp)
	{
		DeleteObject(m_hForgroundBmp);
		m_hForgroundBmp = NULL;
	}
	if(m_hScreenDIB)
	{
		DeleteObject(m_hScreenDIB);
		m_hScreenDIB = NULL;
	}
}

void CMainWnd::OnDraw(HDC hdc)
{
	RECT rect;
	RECT clientRect;
	GetClientRect(m_hwnd, &clientRect);
	m_rectTracker.GetTrueRect(rect);
	int nWidth = rect.right - rect.left;
	int nHeight = rect.bottom - rect.top;

	HBITMAP hBmp = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);

	m_bmpOperator.CopyBmp(hBmp, m_hBackBmp, clientRect, clientRect);
	//拷贝窗口图片到前景
	RECT rectTemp = rect;
	int nHandleSize = m_rectTracker.GetHandleSize();
	InflateRect(&rectTemp, -1*nHandleSize, -1*nHandleSize);
	if(m_trackType == MOVE)
	{	
		m_bmpOperator.CopyBmp(m_hForgroundBmp, m_hScreenDIB, rectTemp, rectTemp);//m_hScreenBmp
	}
	m_bmpOperator.CopyBmp(hBmp, m_hForgroundBmp, rectTemp, rectTemp);

	HDC hMemDC = CreateCompatibleDC(hdc);
	HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC, hBmp);	

	m_rectTracker.Draw(hMemDC);
	if(nWidth > 0 && nHeight > 0)
	{		
		m_rectTransparent.left = rect.left + m_rectTracker.GetHandleSize();
		m_rectTransparent.bottom = rect.top + m_rectTracker.GetHandleSize() - 1;
		m_rectTransparent.right = m_rectTransparent.left + TRANSPARENTRECT_WIDTH;
		m_rectTransparent.top = m_rectTransparent.bottom - TRANSPARENTRECT_HEIGHT;
		if(m_rectTransparent.top < 0)
		{
			m_rectTransparent.top = rect.top + m_rectTracker.GetHandleSize();
			m_rectTransparent.bottom = m_rectTransparent.top + TRANSPARENTRECT_HEIGHT;
		}
		char val[128];
		GetTransparentText(val);
		m_bmpOperator.DrawTransparent(hMemDC, val, m_rectTransparent, 128);//m_dibbuffer
	}

	BitBlt(hdc, clientRect.left, clientRect.top, clientRect.right, clientRect.bottom, hMemDC, clientRect.left, clientRect.top, SRCCOPY);
	SelectObject(hMemDC, hOldBmp);
	DeleteObject(hBmp);
	DeleteDC(hMemDC);
}

void CMainWnd::DrawLine(HWND hwnd, POINT point)
{
	if(PtInRect(&m_rectTracker.m_rect, point))
	{
		HDC hdc = GetDC(m_hwnd);
		HPEN hpen = CreatePen(PS_SOLID, 5, RGB(255, 0, 0));
		HDC hMemdc = CreateCompatibleDC(hdc);

		HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemdc, m_hForgroundBmp);
		HPEN hOldPen = (HPEN)SelectObject(hMemdc, hpen);

		MoveToEx(hMemdc, m_firstpoint.x, m_firstpoint.y, NULL);
		LineTo(hMemdc, point.x, point.y);
		
		DeleteDC(hMemdc);
		DeleteObject(hpen);
		ReleaseDC(hwnd, hdc);
		InvalidateRect(hwnd, &m_rectTracker.m_rect, FALSE);
	}
}

void CMainWnd::OnLbuttonDown(WPARAM wParam, LPARAM lParam)
{	
	POINT point;
	point.x = LOWORD(lParam);
	point.y = HIWORD(lParam);
	if(m_trackType == MOVE)
	{
		int nWidth = m_rectTracker.m_rect.right - m_rectTracker.m_rect.left;
		int nHeight = m_rectTracker.m_rect.bottom - m_rectTracker.m_rect.top;
		if(nWidth == 0 || nHeight == 0)
		{
			RECT rect;
			rect.right = point.x;
			rect.bottom = point.y;
			rect.left = rect.right - 1;
			rect.top = rect.bottom - 1;
						
			m_rectTracker.SetHandleSize(4);
			m_rectTracker.InitRect(rect);
			m_rectTracker.GetTrueRect(rect);
						
			InvalidateRect(m_hwnd, &rect, TRUE);
		}
		int nHandle = m_rectTracker.HitTest(point);
		if(nHandle >= 0)
		{
			m_rectTracker.TrackHandle(nHandle, m_hwnd, point);
		}
	}
	else if(m_trackType == LINE)
	{
		if(PtInRect(&m_rectTracker.m_rect, point))
		{
			m_trackType = LINING;
			m_firstpoint = point;
		}
	}
	else if(m_trackType == TEXT)
	{
		if(m_edit.GetHwnd() != NULL)
		{
			char str[100];
			RECT rect;
			memset(str, 0, 100);
			
			m_edit.GetText(str, 100);
			GetWindowRect(m_edit.GetHwnd(), &rect);
			m_edit.Destroy();

			m_bmpOperator.DrawTexttoBmp(m_hForgroundBmp, str, &rect);
			InvalidateRect(m_hwnd, &rect, FALSE);
		}
		if(PtInRect(&m_rectTracker.m_rect, point))
		{
			RECT rect;
			RECT rectMax;
			GetEditRect(point.x, point.y, rect, rectMax);
			DWORD dwStyle = WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL;//| ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN
			m_edit.Create(m_hInst, m_hwnd, dwStyle, rect, rectMax);
			m_edit.SetFocus();
		}
	}
}

void CMainWnd::GetTransparentText(char* str)
{
	RECT rectClient;
	POINT point;
	GetClientRect(m_hwnd, &rectClient);
	GetCursorPos(&point);
	int yStart = (rectClient.bottom - point.y - 1) * (rectClient.right * 4);
	int xStart = point.x * 4;
	sprintf(str, "矩形框大小:[%dX%d]\n  像素值:[R:%d, G:%d, B:%d]", 
				abs(m_rectTracker.m_rect.right - m_rectTracker.m_rect.left),
				abs(m_rectTracker.m_rect.bottom - m_rectTracker.m_rect.top), 
				m_dibbuffer[yStart+xStart+2],
				m_dibbuffer[yStart+xStart+1],
				m_dibbuffer[yStart+xStart]);
}

void CMainWnd::GetEditRect(int nLeft, int nTop, RECT& editRect, RECT &editMaxRect)
{
	int nFontWidth;
	int nFontHeight;
	m_edit.GetFontSize(nFontWidth, nFontHeight);
	editRect.left = nLeft;
	editRect.top = nTop;
	editRect.right = nLeft + 6 * nFontWidth;
	editRect.bottom = nTop + nFontHeight;

	if(editRect.right > m_rectTracker.m_rect.right)
	{
		editRect.right = m_rectTracker.m_rect.right;
		editRect.left = (editRect.right - 6 * nFontWidth) ;
		editRect.left = (editRect.left < 0) ? 0 : editRect.left;
	}
	if(editRect.bottom > m_rectTracker.m_rect.bottom)
	{
		editRect.bottom = m_rectTracker.m_rect.bottom;
		editRect.top = (editRect.bottom - nFontHeight);
		editRect.top = (editRect.top < 0) ? 0 : editRect.top;
	}

	editMaxRect.left = 0;
	editMaxRect.top = 0;
	editMaxRect.right = m_rectTracker.m_rect.right - editRect.left;
	editMaxRect.bottom = m_rectTracker.m_rect.bottom - editRect.top;
}

void CMainWnd::OnLbuttonUp(WPARAM wParam, LPARAM lParam)
{
	if(m_trackType == LINING)
	{
		SetTrackType(LINE);
		SendMessage(m_hwnd, WM_SETCURSOR, (WPARAM)m_hwnd, 1);
	}
}

void CMainWnd::OnRButtonUp(WPARAM wParam, LPARAM lParam)
{
	POINT point;
	point.x = LOWORD(lParam);
	point.y = HIWORD(lParam);
	int nHandle = m_rectTracker.HitTest(point);
	if(nHandle < 0)
	{
		RECT rect;
		m_rectTracker.GetTrueRect(rect);
		rect.top -= TRANSPARENTRECT_HEIGHT;
		rect.right = max(rect.right, rect.left+TRANSPARENTRECT_WIDTH+m_rectTracker.GetHandleSize());

		m_edit.Destroy();
		m_rectTracker.Clear();
		memset(&m_rectTransparent, 0, sizeof(RECT));

		InvalidateRect(m_hwnd, &rect, TRUE);
		m_trackType = MOVE;
	}
	else
	{
		TrackPopupMenu(m_hMenu, TPM_RIGHTBUTTON, point.x, point.y, 0, m_hwnd, NULL);
	}
}

void CMainWnd::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	TRACKER_TYPE type = GetTrackType();
	if(wParam == MK_LBUTTON)
	{
		if((type == LINE || type == LINING))
		{
			POINT point;
			point.x = LOWORD(lParam);
			point.y = HIWORD(lParam);
			DrawLine(m_hwnd, point);
			m_firstpoint = point;
		}
	}
	else
	{
		InvalidateRect(m_hwnd, &m_rectTransparent, FALSE);
	}
}

BOOL CMainWnd::SetCursor(WPARAM wParam, LPARAM lParam)
{
	if((HWND)wParam != m_hwnd || LOWORD(lParam) != HTCLIENT)
		return FALSE;
	TRACKER_TYPE type = GetTrackType();
	if(type == MOVE)
	{
		if(m_rectTracker.SetCursor())
			return TRUE;
	}
	else 
	{
		POINT point;
		GetCursorPos(&point);
		if(PtInRect(&(m_rectTracker.m_rect), point))
		{
			::SetCursor(LoadCursor(NULL, IDC_CROSS));
			return TRUE;
		}
		else
		{
			if(type == LINING)
			{
				::SetCursor(LoadCursor(NULL, IDC_CROSS));
				return TRUE;
			}
		}
	}
	
	return FALSE;
}

TRACKER_TYPE CMainWnd::GetTrackType()
{
	return m_trackType;
}

void CMainWnd::SetTrackType(TRACKER_TYPE type)
{
	if(m_edit.GetHwnd() != NULL)
	{
		m_edit.Destroy();
	}
	m_trackType = type;
}

HMENU CMainWnd::GetMenu()
{
	return m_hMenu;
}

BOOL CMainWnd::SaveJPEG()
{
	OPENFILENAME  ofn;
	char filename[MAX_PATH];
	char titlename[MAX_PATH];
				
	memset(filename, 0, MAX_PATH);
	memset(&ofn, 0, sizeof(OPENFILENAME));
	static TCHAR szFilter[] = TEXT ("Image Files (*.jpg)\0*.jpg\0")  \
							 TEXT ("All Files (*.*)\0*.*\0\0") ;  
	ofn.lStructSize       = sizeof (OPENFILENAME) ;
	ofn.hwndOwner         = m_hwnd ;
	ofn.lpstrFilter       = szFilter ;
	ofn.nMaxFile          = MAX_PATH ;
	ofn.nMaxFileTitle     = MAX_PATH ;
	ofn.lpstrDefExt       = TEXT ("jpg") ;
	ofn.lpstrFile = filename;
	ofn.lpstrFileTitle = titlename;
	ofn.Flags = OFN_OVERWRITEPROMPT;
				
	if(GetSaveFileName(&ofn))
	{
		int width = m_rectTracker.m_rect.right - m_rectTracker.m_rect.left;
		int height = m_rectTracker.m_rect.bottom - m_rectTracker.m_rect.top;
		BYTE* image_buffer = NULL;
		HBITMAP hDib = NULL;
		hDib = m_bmpOperator.GetDIBColorsFromHbitmap(image_buffer, m_hForgroundBmp, 24, m_rectTracker.m_rect);
		if(image_buffer)
		{
			for(int i=0; i<height; i++)
			{
				int start = i* (((width * 3 + 3) >> 2) << 2);
				for(int j=0; j<width; j++)
				{
					BYTE temp = image_buffer[start+j*3];
					image_buffer[start+j*3] = image_buffer[start+j*3+2];
					image_buffer[start+j*3+2] = temp;
				}
			}
			m_jpegFile.write_JPEG_file(filename, image_buffer, width, height, 80);
			DeleteObject(hDib);
			return TRUE;
		}
	}		
	return FALSE;
}