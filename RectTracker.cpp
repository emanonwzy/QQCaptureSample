#include "RectTracker.h"

HCURSOR		g_Cursors[10] = { 0, };
HPEN		g_handlePen = 0;
int			g_handleSize = 0;

// the struct below is used to determine the qualities of a particular handle
struct AFX_HANDLEINFO
{
	size_t nOffsetX;    // offset within RECT for X coordinate
	size_t nOffsetY;    // offset within RECT for Y coordinate
	int nCenterX;       // adjust X by Width()/2 * this number
	int nCenterY;       // adjust Y by Height()/2 * this number
	int nHandleX;       // adjust X by handle size * this number
	int nHandleY;       // adjust Y by handle size * this number
	int nInvertX;       // handle converts to this when X inverted
	int nInvertY;       // handle converts to this when Y inverted
};

#define offsetof(s,m)   (size_t)&reinterpret_cast<const volatile char&>((((s *)0)->m))

// this array describes all 8 handles (clock-wise)
const AFX_HANDLEINFO _afxHandleInfo[] =
{
	// corner handles (top-left, top-right, bottom-right, bottom-left
	{ offsetof(RECT, left), offsetof(RECT, top),        0, 0, -1, -1, 1, 3 },
	{ offsetof(RECT, right), offsetof(RECT, top),       0, 0, 0, -1, 0, 2 },
	{ offsetof(RECT, right), offsetof(RECT, bottom),    0, 0, 0, 0, 3, 1 },
	{ offsetof(RECT, left), offsetof(RECT, bottom),     0, 0, -1, 0, 2, 0 },

	// side handles (top, right, bottom, left)
	{ offsetof(RECT, left), offsetof(RECT, top),        1, 0,  0,  -1, 4, 6 },
	{ offsetof(RECT, right), offsetof(RECT, top),       0, 1,  0,  0, 7, 5 },
	{ offsetof(RECT, left), offsetof(RECT, bottom),     1, 0,  0,  0, 6, 4 },
	{ offsetof(RECT, left), offsetof(RECT, top),        0, 1,  -1,  0, 5, 7 }
};

CRectTracker::CRectTracker()
{
	Construct();
	memset(&m_rect, 0, sizeof(RECT));
}

CRectTracker::CRectTracker(const RECT& rect)
{
	Construct();
	m_rect = rect;
}

void CRectTracker::InitRect(const RECT& rect)
{
	m_rect = rect;
}

void CRectTracker::Clear()
{
	memset(&m_rect, 0, sizeof(RECT));
	m_nHandleSize = 0;
	m_bErase = FALSE;
	m_bFinalErase = FALSE;
}

void CRectTracker::Construct()
{
	static BOOL bInitialized = FALSE;
	if(!bInitialized)
	{
		if(g_handlePen == NULL)
		{
			g_handlePen = CreatePen(PS_SOLID, 0, RGB(192, 0,0));
		}
		g_Cursors[0] = ::LoadCursor(NULL, IDC_SIZENWSE);
		g_Cursors[1] = ::LoadCursor(NULL, IDC_SIZENESW);
		g_Cursors[2] = g_Cursors[0];
		g_Cursors[3] = g_Cursors[1];
		g_Cursors[4] = ::LoadCursor(NULL, IDC_SIZENS);
		g_Cursors[5] = ::LoadCursor(NULL, IDC_SIZEWE);
		g_Cursors[6] = g_Cursors[4];
		g_Cursors[7] = g_Cursors[5];
		g_Cursors[8] = ::LoadCursor(NULL, IDC_SIZEALL);
		g_Cursors[9] = g_Cursors[8];

		m_nHandleSize = 0;
		m_bErase = FALSE;
		m_bFinalErase = FALSE;
	}
}

void CRectTracker::Draw(HDC hdc) const
{
	RECT rect = m_rect;
	NormalizeRect(rect);

	HPEN hOldPen = NULL;
	HBRUSH hOldBrush = NULL;
	int nOldROP;
	hOldPen = (HPEN)SelectObject(hdc, g_handlePen);
	hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
	nOldROP = SetROP2(hdc, R2_COPYPEN);
	Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
	SetROP2(hdc, nOldROP);

	for(int i = 0; i < 8; ++i)
	{
		GetHandleRect((TrackerHit)i, rect);
		::SetBkColor(hdc, RGB(192,0,0));
		::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);
	}
	if(hOldPen != NULL)
	{
		SelectObject(hdc, hOldPen);
	}
	if(hOldBrush != NULL)
	{
		SelectObject(hdc, hOldBrush);
	}
}

void CRectTracker::SetHandleSize(int nSize)
{
	m_nHandleSize = nSize;
}

void CRectTracker::GetTrueRect(RECT& trueRect) const
{
	RECT rect = m_rect;
	NormalizeRect(rect);
	InflateRect(&rect, GetHandleSize(), GetHandleSize());
	trueRect = rect;
}

int CRectTracker::HitTest(POINT point) const
{
	TrackerHit hitResult = hitNothing;

	RECT rectTrue;
	GetTrueRect(rectTrue);
	if(PtInRect(&rectTrue, point))
	{
		hitResult = (TrackerHit)HitTestHandles(point);
	}

	return hitResult;
}

BOOL CRectTracker::SetCursor() const
{
	POINT point;
	GetCursorPos(&point);

	int nHandle = HitTestHandles(point);
	if(nHandle < 0)
		return FALSE;

	nHandle = NormalizeHit(nHandle);
	::SetCursor(g_Cursors[nHandle]);
	return TRUE;
}

BOOL CRectTracker::TrackHandle(int nHandle, HWND hwnd, POINT point)
{
	if(::GetCapture() != NULL)
		return FALSE;

	RECT clientRect;
	GetClientRect(hwnd, &clientRect);
	int nWidth = m_rect.right - m_rect.left;
	int nHeight = m_rect.bottom - m_rect.top;

	SetCapture(hwnd);
	UpdateWindow(hwnd);

	int *px, *py;
	int xDis, yDis;
	GetModifiedPointers(nHandle, &px, &py, &xDis, &yDis);
	xDis = point.x - xDis;
	yDis = point.y - yDis;

	HDC drawDC = GetDC(hwnd);
	RECT rectOld;
	POINT pointOld = point;
	BOOL bMoved = FALSE;

	for(;;)
	{
		MSG msg;
		::GetMessage(&msg, NULL, 0, 0);
		if(GetCapture() != hwnd)
			break;

		switch(msg.message)
		{
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			rectOld = m_rect;
			if(px != NULL)
				*px = LOWORD(msg.lParam) - xDis;
			if(py != NULL)
				*py = HIWORD(msg.lParam) - yDis;

			if(nHandle == hitMiddle)
			{
				m_rect.right = m_rect.left + nWidth;
				m_rect.bottom = m_rect.top + nHeight;
			}
			m_bFinalErase = (msg.message == WM_LBUTTONUP);
			if(m_bFinalErase)
				goto ExitLoop;
			if(min(m_rect.left, m_rect.right) < 0 || max(m_rect.left, m_rect.right) > clientRect.right ||
				min(m_rect.top, m_rect.bottom) < 0 || max(m_rect.top, m_rect.bottom) > clientRect.bottom)
			{
				m_rect = rectOld;
				if(px != NULL)
					xDis = LOWORD(msg.lParam) - *px;
				if(py != NULL)
					yDis = HIWORD(msg.lParam) - *py;
			}
			else
			{
				pointOld.x = LOWORD(msg.lParam);
				pointOld.y = HIWORD(msg.lParam);
			}

			if(!EqualRect(&rectOld, &m_rect))
			{
				InvalidateRect(hwnd, NULL, FALSE);//&rect
			}
			break;
		case WM_KEYDOWN:
			if (msg.wParam != VK_ESCAPE)
				break;
		default:
			DispatchMessage(&msg);
			break;
		}
	}

ExitLoop:
	ReleaseCapture();
	ReleaseDC(hwnd, drawDC);
	NormalizeRect(m_rect);
	
	m_bFinalErase = FALSE;
	return !EqualRect(&rectOld, &m_rect);
}

/************************¹¤¾ßº¯Êý************************/
void CRectTracker::GetModifiedPointers(int nHandle, int** ppx, int** ppy, int* px, int* py)
{
	if(nHandle == hitMiddle)
		nHandle = hitTopLeft;
	int nWidth = m_rect.right - m_rect.left;
	int nHeight = m_rect.bottom - m_rect.top;
	*ppx = NULL;
	*ppy = NULL;
	nHandle = NormalizeHit(nHandle);
	const AFX_HANDLEINFO *pHandleInfo = &(_afxHandleInfo[nHandle]);
	if(pHandleInfo->nInvertX != nHandle)
	{
		*ppx = (int*)(((BYTE*)&m_rect) + pHandleInfo->nOffsetX);
		if(px != NULL)
			*px = **ppx;
	}
	else
	{
		if(px != NULL)
			*px = m_rect.left + (nWidth) / 2;
	}
	if(pHandleInfo->nInvertY != nHandle)
	{
		*ppy= (int*)(((BYTE*)&m_rect) + pHandleInfo->nOffsetY);
		if(py != NULL)
			*py = **ppy;
	}
	else
	{
		if(py != NULL)
			*py = m_rect.top + (nHeight) / 2;
	}
}

void CRectTracker::NormalizeRect(RECT& rect) const
{
	int nTemp;
	if(rect.left > rect.right)
	{
		nTemp = rect.left;
		rect.left = rect.right;
		rect.right = nTemp;
	}
	if(rect.top > rect.bottom)
	{
		nTemp = rect.top;
		rect.top = rect.bottom;
		rect.bottom = nTemp;
	}
}

void CRectTracker::GetHandleRect(int nHandle, RECT& handleRect) const
{
	RECT rectT = m_rect;
	NormalizeRect(rectT);
	
	nHandle = NormalizeHit(nHandle);
	int nSize = GetHandleSize();

	int nWidth = rectT.right - rectT.left;
	int nHeight = rectT.bottom - rectT.top;
	
	RECT rect;
	const AFX_HANDLEINFO* pHandleInfo = &_afxHandleInfo[nHandle];
	rect.left = *(int*)((BYTE*)&rectT + pHandleInfo->nOffsetX);
	rect.top = *(int*)((BYTE*)&rectT + pHandleInfo->nOffsetY);
	rect.left += nSize * pHandleInfo->nHandleX;
	rect.top += nSize * pHandleInfo->nHandleY;
	rect.left += pHandleInfo->nCenterX * (nWidth - nSize) / 2;
	rect.top += pHandleInfo->nCenterY * (nHeight - nSize) / 2;
	rect.right = rect.left + nSize;
	rect.bottom = rect.top + nSize;

	handleRect = rect;
}

int CRectTracker::GetHandleSize() const
{
	return m_nHandleSize;
}

int CRectTracker::NormalizeHit(int nHandle) const
{
	if(nHandle == hitMiddle || nHandle == hitNothing)
		return nHandle;

	const AFX_HANDLEINFO* pHandleInfo = &_afxHandleInfo[nHandle];
	int width = m_rect.right - m_rect.left;
	int height = m_rect.bottom - m_rect.top;

	if(width < 0)
	{
		nHandle = (TrackerHit)pHandleInfo->nInvertX;
		pHandleInfo = &_afxHandleInfo[nHandle];
	}
	if(height < 0)
		nHandle = (TrackerHit)pHandleInfo->nInvertY;
	return nHandle;
}

int CRectTracker::HitTestHandles(POINT point) const
{
	RECT rect;
	GetTrueRect(rect);
	if(!PtInRect(&rect, point))
		return hitNothing;

	for(int i =0; i < 8; i++)
	{
		GetHandleRect((TrackerHit)i, rect);
		if(PtInRect(&rect, point))
			return (TrackerHit)i;
	}

	return hitMiddle;
}