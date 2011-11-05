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
	HBITMAP m_hBackBmp;	//���ڱ���������Ļ�Ŀ���ͼƬ������õ���ͼƬ
	HBITMAP m_hForgroundBmp ;//ǰ��ͼƬ������ѡ��ʱ���з�Χ��ѡ�е�ͼƬ�����洢jpgʱ���洢��ͼƬ������
	HBITMAP m_hScreenDIB ; //����Ļ�Ŀ���
	BYTE*   m_dibbuffer ; //m_hScreenDIB��Ӧ��rgbֵ������ƶ�ʱֱ��������ȡrgbֵ
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