#ifndef CRECTTRACKER_H
#define CRECTTRACKER_H
#include "stdafx.h"

class CRectTracker
{
public:
	CRectTracker();
	CRectTracker(const RECT& rect);
	enum TrackerHit
	{
		hitNothing = -1,
		hitTopLeft = 0, hitTopRight = 1, hitBottomRight = 2, hitBottomLeft = 3,
		hitTop = 4, hitRight = 5, hitBottom = 6, hitLeft = 7, hitMiddle = 8
	};

	RECT			m_rect;
	int				m_nHandleSize;
	

	void Draw(HDC hdc) const;
	int  GetHandleSize() const;
	void SetHandleSize(int nSize);
	void GetTrueRect(RECT& trueRect) const;
	BOOL SetCursor() const;
	int  HitTest(POINT point) const;
	BOOL TrackHandle(int nHandle, HWND hwnd, POINT point);
	void InitRect(const RECT& rect);
	void Clear();
protected:
	RECT m_rectLast;
	SIZE m_sizeLast;
	BOOL m_bErase;
	BOOL m_bFinalErase;
	void	GetModifiedPointers(int nHandle, int** ppx, int** ppy, int* px, int* py);
	int		HitTestHandles(POINT point) const;
	void	GetHandleRect(int nHandle, RECT& handleRect) const;
	
	int		NormalizeHit(int nHandle) const;
	void	Construct();
private:
	void NormalizeRect(RECT& rect) const;
};

#endif