#include "stdafx.h"
#include "BmpOperator.h"

HBITMAP CBmpOperator::GetScreenBmp()
{
	int cx = GetSystemMetrics(SM_CXSCREEN);
	int cy = GetSystemMetrics(SM_CYSCREEN);

	HDC hScreenDC = GetDC(NULL);
	HDC hMemDC = CreateCompatibleDC(hScreenDC);
	HBITMAP hScreenBmp = CreateCompatibleBitmap(hScreenDC, cx, cy);
	HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemDC, hScreenBmp);
	
	BitBlt(hMemDC, 0, 0, cx, cy, hScreenDC, 0, 0, SRCCOPY);

	SelectObject(hMemDC, hOldBmp);
	DeleteDC(hMemDC);
	ReleaseDC(NULL, hScreenDC);

	return hScreenBmp;
}

void CBmpOperator::DrawTransparent(HDC hdc, const char* val, const RECT& rect, int nAlpha)
{
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	BYTE* image_buffer = NULL;
	HBITMAP hDib = CreateDIB((void*&)image_buffer, GetDeviceCaps(hdc, BITSPIXEL), width, height);
	if(image_buffer)
	{
		HDC hDstdc = CreateCompatibleDC(hdc);

		HBITMAP hDstOldBmp = (HBITMAP)SelectObject(hDstdc, hDib);
		BitBlt(hDstdc, 0, 0, width, height, hdc, rect.left, rect.top, SRCCOPY);

		for(int i=0; i<height; i++)
		{
			int start = i * width * 4;
			for(int j=0, k=0; j<width; j++)
			{
				 image_buffer[start+k] = (image_buffer[start+k] * (256 - nAlpha) + 10 * nAlpha)>>8;
				 image_buffer[start+k+1] = (image_buffer[start+k+1] * (256 - nAlpha) + 10 * nAlpha)>>8;
				 image_buffer[start+k+2] = (image_buffer[start+k+2] * (256 - nAlpha) + 10 * nAlpha)>>8;
				 k+=4;
			}
		}

		RECT rectDest;
		HFONT hOldFont = (HFONT)SelectObject(hDstdc, GetStockObject(SYSTEM_FIXED_FONT));
		int nOldMode = SetBkMode(hDstdc, TRANSPARENT);
		SetTextColor(hDstdc, RGB(200, 200, 200));

		rectDest.left = 0;
		rectDest.top = 0;
		rectDest.right = width;
		rectDest.bottom = height;
		DrawText(hDstdc, val, -1, &rectDest, DT_TOP|DT_LEFT);

		BitBlt(hdc, rect.left, rect.top, width, height, hDstdc, 0, 0, SRCCOPY);
		 
		DeleteDC(hDstdc);
		DeleteObject(hDib);
	}
	
}

HBITMAP CBmpOperator::GetDarkenBmp(HBITMAP himage)
{
	HBITMAP hDarkenBmp = NULL;
	BYTE* image_buffer = NULL;
	HBITMAP hDib = NULL;
	RECT rect;

	BITMAP bmp;
	GetObject(himage, sizeof(BITMAP), &bmp);
	int width = bmp.bmWidth;
	int height = bmp.bmHeight;
	rect.left = 0;
	rect.right = width;
	rect.top = 0;
	rect.bottom = height;

	hDib = GetDIBColorsFromHbitmap(image_buffer, himage, 24, rect);
	if(image_buffer)
	{
		for(int i=0; i<height; i++)
		{
			int start = i* (((width * 3 + 3) >> 2) << 2);
			for(int j=0; j<width; j++)
			{
				image_buffer[start+j*3] = (image_buffer[start+j*3]+40)/2;
				image_buffer[start+j*3+1] = (image_buffer[start+j*3+1]+40)/2;
				image_buffer[start+j*3+2] = (image_buffer[start+j*3+2]+40)/2;
			}
		}
		HDC hdc = GetDC(NULL);
		hDarkenBmp = CreateCompatibleBitmap(hdc, width, height);
		ReleaseDC(NULL, hdc);

		CopyBmp(hDarkenBmp, hDib, rect, rect);
		DeleteObject(hDib);	
	}

	return hDarkenBmp;
}

HBITMAP CBmpOperator::GetDIBColorsFromHbitmap(BYTE*& buffer, HBITMAP himage, int bitmap_count, const RECT& rect)
{
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	HBITMAP hDib = CreateDIB((void*&)buffer, bitmap_count, width, height);
	if(buffer)
	{
		RECT rectDib;
		rectDib.left = 0;
		rectDib.top = 0;
		rectDib.right = width;
		rectDib.bottom = height;
		CopyBmp(hDib, himage, rectDib, rect);
	}

	return hDib;
}

void CBmpOperator::CopyBmp(HBITMAP hDestBmp, HBITMAP hSrcBmp,const RECT& destRect, const RECT& srcRect)
{
	int nWidth = destRect.right - destRect.left;
	int nHeight = destRect.bottom - destRect.top;

	HDC hdc = GetDC(NULL);
	HDC hSrcdc = CreateCompatibleDC(hdc);
	HDC hDestdc = CreateCompatibleDC(hdc);
	
	HBITMAP hSrcOldBmp = (HBITMAP)SelectObject(hSrcdc, hSrcBmp);
	HBITMAP hDestOldBmp = (HBITMAP)SelectObject(hDestdc, hDestBmp);
	
	BitBlt(hDestdc, destRect.left, destRect.top, nWidth, nHeight, hSrcdc, srcRect.left, srcRect.top, SRCCOPY);

	DeleteDC(hSrcdc);
	DeleteDC(hDestdc);
	ReleaseDC(NULL, hdc);
}

HBITMAP CBmpOperator::CreateDIB(void *&pBuffer, int nBitsperPiexl, int width, int height)
{
	BITMAPINFOHEADER bmp_info;
	memset(&bmp_info, 0, sizeof(BITMAPINFOHEADER));
	bmp_info.biSize = sizeof(BITMAPINFOHEADER);
	bmp_info.biWidth = width;
	bmp_info.biHeight = height;
	bmp_info.biPlanes = 1;
	bmp_info.biCompression = BI_RGB;
	bmp_info.biBitCount = nBitsperPiexl;

	HBITMAP hDib = CreateDIBSection(NULL, (BITMAPINFO*)&bmp_info,
									DIB_RGB_COLORS, (void**)&pBuffer, NULL, 0);
	return hDib;
}

void CBmpOperator::DrawTexttoBmp(HBITMAP himage, const char* str, RECT* pRect)
{
	HDC hdc = GetDC(NULL);
	HDC hMemdc = CreateCompatibleDC(hdc);

	HBITMAP hOldBmp = (HBITMAP)SelectObject(hMemdc, himage);
	HFONT hOldFont = (HFONT)SelectObject(hMemdc, GetStockObject(SYSTEM_FIXED_FONT));
	int nOldMode = SetBkMode(hMemdc, TRANSPARENT);
	SetTextColor(hMemdc, RGB(255, 0, 0));
	
	DrawText(hMemdc, str, -1, pRect, DT_LEFT|DT_TOP);
	
	DeleteDC(hMemdc);
	ReleaseDC(NULL, hdc);
}