#ifndef MAINWND_H
#define MAINWND_H
#include "RectTracker.h"
#include "BmpOperator.h"
#include "Jpeg.h"
#include "Edit.h"

enum TRACKER_TYPE
{
	MOVE = 0,
	LINE = 1,
	TEXT = 2,
	LINING = 3
};

class CMainWnd
{
public:
	CMainWnd();
	void Init(HINSTANCE hInst, HWND hwnd, DWORD dwMenuID, int cx, int cy);
	void Release();

	void OnDraw(HDC hdc);
	void OnLbuttonDown(WPARAM wParam, LPARAM lParam);
	void OnLbuttonUp(WPARAM wParam, LPARAM lParam);
	void OnRButtonUp(WPARAM wParam, LPARAM lParam);
	void OnMouseMove(WPARAM wParam, LPARAM lParam);
	BOOL SetCursor(WPARAM wParam, LPARAM lParam);

	TRACKER_TYPE GetTrackType();
	void SetTrackType(TRACKER_TYPE type);
	HMENU GetMenu();
	BOOL SaveJPEG();
private:
	void GetTransparentText(char* str);
	void GetEditRect(int nLeft, int nTop, RECT& editRect, RECT &editMaxRect);
	void DrawLine(HWND hwnd, POINT point);
	void InitMenu(DWORD dwID);
	void InitGDI(HWND hwnd, int cx, int cy);
private:
	HBITMAP m_hBackBmp;	//窗口背景，将屏幕的拷贝图片暗化后得到的图片
	HBITMAP m_hForgroundBmp ;//前景图片，拉框选择时框中范围内选中的图片，最后存储jpg时即存储该图片的内容
	HBITMAP m_hScreenDIB ; //对屏幕的拷贝
	BYTE*   m_dibbuffer ; //m_hScreenDIB对应的rgb值，鼠标移动时直接在其中取rgb值
	RECT    m_rectTransparent;

	TRACKER_TYPE m_trackType;	
	POINT m_firstpoint;	
	HMENU   m_hMenu;
	HWND m_hwnd;
	HINSTANCE m_hInst;
	
	CRectTracker m_rectTracker;
	CBmpOperator m_bmpOperator;
	CJpeg  m_jpegFile;
	CEdit    m_edit;
};
#endif