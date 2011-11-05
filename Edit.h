#ifndef EDIT_H
#define EDIT_H

enum EDIT_TYPE
{
	EDIT_INPUT = 0,
	EDIT_MOVE = 1
};

class CEdit
{
public:
	CEdit();
	HWND Create(HINSTANCE hInst, HWND hWndParent, DWORD dwStyle, const RECT& rect, const RECT& rectMax);
	void Destroy();
	BOOL GetText(char* strEdit, int nMaxCount);
	HWND GetHwnd();
	void GetFontSize(int& nFontWidth, int& nFontHeight);
	void SetFocus();
private:
	void ChangeEditSize();
	BOOL SetCursor(WPARAM wParam, LPARAM lParam);
	void OnPaint(HDC hdc);
	void DrawEdittext(HDC hdc, RECT *pRect);
	void DrawEditEdge(HDC hdc, RECT *pRect);
	static LRESULT CALLBACK EditProc(HWND, UINT, WPARAM, LPARAM);
	LRESULT InEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
private:
	EDIT_TYPE m_type;
	int m_nScanCount;
	HWND m_hwnd;
	LONG m_oldProc;
	HCURSOR m_Cursor;
	int m_nDx;
	int m_nDy;
	HFONT m_hFont;
	RECT m_rectMax;
};

#endif