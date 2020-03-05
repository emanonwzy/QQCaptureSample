# QQCaptureSample
A small win32 application that implement QQ capture screen function

1、设计思路
	1、程序启动时创建一个与屏幕等大的窗口并将其设置为topmost
	2、拷屏，将窗口背景拷贝下来，创建如下四个位图：
		HBITMAP m_hBackBmp;	//将m_hScreenBmp暗化后得到的图片
		HBITMAP m_hForgroundBmp ; //前景图片，拉框选择时框中范围内选中的图片，
									最后存储jpg时即存储该图片的内容
		HBITMAP m_hScreenDIB ; //对屏幕的拷贝
		BYTE*   m_dibbuffer ; //m_hScreenDIB对应的rgb值，鼠标移动时直接在其中取rgb值

	3、程序刷新过程
	   --创建与窗口等大的兼容位图hBmp
	   --拷贝背景图片m_hBackBmp到hBmp上
	   --如果当前为移动状态则将相应位置上的m_hScreenDIB拷贝到m_hForgroundBmp上
	   --拷贝m_hForgroundBmp到hBmp上
	   --hBmp选入内存dc
	   --画选择框
	   --如果选择框大小大于0，画半透明的信息框
	4、edit鼠标处理
	   --窗口默认为INPUT状态，按下次数为0
	   --WM_LBUTTONDOWN 按下鼠标开始计时器
	   --WM_TIMER       查看鼠标状态，
						为按下，按下次数加1
						   次数达到上限，设置窗口状态为MOVE，销毁定时器
						为弹起，销毁定时器
	   --WM_MOUSEMOVE   如果鼠标按下并且当前状态为MOVE，则随鼠标移动窗口
2、存在的问题
   一、对edit的绘图机制还不是很清楚
      --现在是截获WM_CHAR消息强制调用InvalidateRect刷新窗口，
	    否则自定义的OnPaint相应不起作用
	  --拖动时效果比较差，因为绘制是直接在hdc上的
	  --未能实现自动换行
   
3、主要类
   CMainWnd   //主窗口相关操作
    +Init(HINSTANCE hInst, HWND hwnd, DWORD dwMenuID, int cx, int cy); 
	+void OnDraw(HDC hdc);
	+void OnLbuttonDown(WPARAM wParam, LPARAM lParam);
	+void OnLbuttonUp(WPARAM wParam, LPARAM lParam);
	+void OnRButtonUp(WPARAM wParam, LPARAM lParam);
	+void OnMouseMove(WPARAM wParam, LPARAM lParam);
	+BOOL SetCursor(WPARAM wParam, LPARAM lParam);
	+BOOL SaveJPEG();
	
	-HBITMAP m_hBackBmp;	//窗口背景，将屏幕的拷贝图片暗化后得到的图片
	-HBITMAP m_hForgroundBmp ;//前景图片，拉框选择时框中范围内选中的图片，
	                            最后存储jpg时即存储该图片的内容
	-HBITMAP m_hScreenDIB ; //对屏幕的拷贝
	-BYTE*   m_dibbuffer ; //m_hScreenDIB对应的rgb值，鼠标移动时直接在其中取rgb值
    -CRectTracker m_rectTracker; //橡皮筋
	-CBmpOperator m_bmpOperator; //常用对HBITMAP的操作，如获取屏幕拷贝，拷贝HBITMAP
	-CJpeg  m_jpegFile; //存储jpeg
	-CEdit    m_edit; //编辑框
   CBmpOperator  //HBITMAP相关操作
    +void CopyBmp(HBITMAP hDestBmp, HBITMAP hSrcBmp,const RECT& destRect, const RECT& srcRect);
	+HBITMAP GetDarkenBmp(HBITMAP himage);
	+HBITMAP GetDIBColorsFromHbitmap(BYTE*& buffer, HBITMAP himage, int bitmap_count, const RECT& rect); 
	+void DrawTransparent(HDC hdc, const char* val, const RECT& rect, int nAlpha);
	+void DrawTexttoBmp(HBITMAP himage, const char* str, RECT* pRect);
	
   CRectTracker //橡皮筋类，移植自MFC
    +void Draw(HDC hdc) const;
	+void GetTrueRect(RECT& trueRect) const; //获取选择框大小
	+BOOL SetCursor() const;  //设置图标
	+int  HitTest(POINT point) const;
	+BOOL TrackHandle(int nHandle, HWND hwnd, POINT point); //进入移动状态后的消息处理
	+void Clear();
  
   CEdit //编辑框类
    +HWND Create(HINSTANCE hInst, HWND hWndParent, DWORD dwStyle, const RECT& rect, const RECT& rectMax);
	-void OnPaint(HDC hdc);
	-BOOL SetCursor(WPARAM wParam, LPARAM lParam); 
   CJpeg //使用libjpeg进行jpg文件存储
        +write_JPEG_file(const char* filename, unsigned char* image_buffer,
					 int image_width, int image_height, int quality);
