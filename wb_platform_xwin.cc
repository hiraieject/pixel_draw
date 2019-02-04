#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wb_platform.h"

#define DBGXWIN_PR(fmt, args...) \
	printf("%s  [dbgxwin.%s]:%d " fmt, (server_flg ? "server":"client"), __FUNCTION__,__LINE__, ## args)

Display *WB_platform_xwin::display = NULL;
unsigned short WB_platform_xwin::width;
unsigned short WB_platform_xwin::height;
Atom WB_platform_xwin::a1;
Atom WB_platform_xwin::a2;
Window WB_platform_xwin::window;
Pixmap WB_platform_xwin::pixmap[WB_PF_SURFACE_LAYER_MAX];
XFontSet WB_platform_xwin::fs;
GC WB_platform_xwin::pgc[WB_PF_SURFACE_LAYER_MAX];
GC WB_platform_xwin::wgc;

WB_platform_xwin::WB_platform_xwin(WB_PF_SURFACE_LAYER ll, unsigned short screen_w, unsigned short screen_h) : WB_platform()
{
	layer = ll;

	if (display == NULL) {

		width  = screen_w/2;
		height = screen_h/2;

		/* ����������� */
		if(setlocale(LC_ALL, "") == NULL){
			fprintf(stderr, "Cannot set the locale.\n");
			exit(1);
		}
		
		/* X�����Ф���³ */
		display = XOpenDisplay(NULL);
		DBGXWIN_PR("connect X-Server\n");

		/* X�����Ф���³�塢�����뤬���Ѳ�ǽ���ɤ���������å����� */
		if(XSupportsLocale() == False){
			fprintf(stderr, "X does not support the locale.\n");
			exit(1);
		}

		/* ������ɥ������� */
		window = XCreateSimpleWindow(display, DefaultRootWindow(display),
									 100, 100, width, height, 1,
									 BlackPixel(display, DefaultScreen(display)),
									 WhitePixel(display, DefaultScreen(display)));
		DBGXWIN_PR("Window created\n");
		
		// ���٥�ȥޥ���������
		XSelectInput(display, window, 0
					 | ExposureMask
					 | ButtonPressMask
					 | ButtonReleaseMask
					 | PointerMotionMask
					 | EnterWindowMask
					 | LeaveWindowMask
					 );
	
		// close �ܥ��������
		a1 = XInternAtom(display, "WM_PROTOCOLS", False);
		a2 = XInternAtom(display, "WM_DELETE_WINDOW", False);
		XSetWMProtocols(display, window, &a2, 1);

		/* GC������ */
		wgc = XCreateGC(display, (Drawable)window, 0, 0);

		for (int i=0; i<WB_PF_SURFACE_LAYER_MAX; i++) {
			// pixmap ������
			pixmap[i] = XCreatePixmap(display, window, width, height, DefaultDepth(display, 0));
			/* GC������ */
			pgc[i] = XCreateGC(display, pixmap[i], 0, 0);
			// pixmap���򥯥ꥢ
			XSetForeground(display, pgc[i], RGB(255, 255, 255));
			XFillRectangle(display, pixmap[i], pgc[i], 0, 0, width, height);
		}
		// ���̤��򥯥ꥢ
		XCopyArea(display, pixmap[0], window, wgc, 0, 0, width, height, 0, 0);

		/* �ե���ȥ��åȤ��������� */
		{
			char **miss;
			char *def;
			int n_miss;
			fs = XCreateFontSet(display,
								"-misc-fixed-*",
								&miss, &n_miss, &def);
			if (fs == NULL) {
				fprintf(stderr, "fontset create failed.\n");
				exit(1);
			}
		}
	}
	// Window��ɽ��
	XMapWindow(display, window);
}

// ============================================
unsigned long 
WB_platform_xwin::RGB(unsigned char r,
					  unsigned char g,
					  unsigned char b)
{
	Colormap cm;
	XColor xc;
	
	cm = DefaultColormap(display, DefaultScreen(display));
	
	xc.red = 257 * r;
	xc.green = 257 * g;
	xc.blue = 257 * b;
	XAllocColor(display, cm, &xc);
	
	return xc.pixel;
}
