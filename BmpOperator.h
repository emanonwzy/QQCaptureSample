#ifndef CMBPOPERATOR_H
#define CMBPOPERATOR_H

class CBmpOperator
{
public:
	HBITMAP GetScreenBmp();
	void CopyBmp(HBITMAP hDestBmp, HBITMAP hSrcBmp,const RECT& destRect, const RECT& srcRect);
	HBITMAP GetDarkenBmp(HBITMAP himage);
	HBITMAP GetDIBColorsFromHbitmap(BYTE*& buffer, HBITMAP himage, int bitmap_count, const RECT& rect); 
	void DrawTransparent(HDC hdc, const char* val, const RECT& rect, int nAlpha);
	void DrawTexttoBmp(HBITMAP himage, const char* str, RECT* pRect);
private:
	HBITMAP CreateDIB(void *&pBuffer, int nBitsperPiexl, int width, int height);
};

#endif