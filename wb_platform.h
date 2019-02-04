
#ifndef __WB_PLATFORM_H__
#define __WB_PLATFORM_H__

#ifdef USE_XWIN
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xlocale.h>
#endif // USE_XWIN

typedef enum {
	// ≤º¡ÿ
	WB_PF_SURFACE_LAYER_CONF,
	WB_PF_SURFACE_LAYER_TEMP,
	WB_PF_SURFACE_LAYER_MENU,
	// æÂ¡ÿ
	WB_PF_SURFACE_LAYER_MAX,
} WB_PF_SURFACE_LAYER;

class WB_platform {
public:
	WB_platform(void) {
	};
};

#ifdef USE_DFB
class WB_platform_dfb : public WB_platform {
public:
	WB_platform_dfb(void);
};
#endif //USE_DFB

#ifdef USE_XWIN
class WB_platform_xwin : public WB_platform {
private:
	unsigned long RGB(unsigned char r,unsigned char g,unsigned char b);
public:
	static Display *display;
	static unsigned short width;
	static unsigned short height;
	static Atom a1, a2;
	static Window window;
    static Pixmap pixmap[WB_PF_SURFACE_LAYER_MAX];
	static XFontSet fs;
	static GC pgc[WB_PF_SURFACE_LAYER_MAX],wgc;

	WB_PF_SURFACE_LAYER layer;
	
	WB_platform_xwin(WB_PF_SURFACE_LAYER ll, unsigned short screen_w, unsigned short screen_h);

	void allclean(void);
	void line(void);
	void lines(void);
	void box(void);
};

extern bool server_flg;

#endif // USE_XWIN


#endif // __WB_PLATFORM_H__
