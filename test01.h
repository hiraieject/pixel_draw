#ifndef __TEST_H__
#define __TEST_H__

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xlocale.h>

#include <libxml/tree.h>
#include <libxml/parser.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "connection.h"

#define DBGXWIN_PR(fmt, args...) \
	printf("%s  [dbgxwin.%s]:%d " fmt, (this->server_flg ? "server":"client"), __FUNCTION__,__LINE__, ## args)

extern bool server_flg;

#define GETIMG_MAX_PIXEL 200
typedef struct PACKET_IMG_LINE_t {
	char com[10];
	unsigned short packet_no;
	unsigned short imgdt_size;
	unsigned long imgdt[GETIMG_MAX_PIXEL];
} PACKET_IMG_LINE;

typedef struct MENU_BUTTON_t {
	bool visible;
	unsigned int attribute;
	int x,y,w,h;
	char *mes;
	int id;
} MENU_BUTTON;
#define BTNW 80
#define BTNH 40
#define MENU_AREA_H (40+30)

#define BTN_ATTR_SELECT	0x0001
#define BTN_ATTR_GRAY	0x0002

enum {
	BTN_ID_QUIT,
	BTN_ID_LINE,
	BTN_ID_BOX,
	BTN_ID_MARKER,
	BTN_ID_ERASER,
	BTN_ID_CLEAR,

	BTN_ID_DISCONNECT01 = BTN_ID_CLEAR+1,
	BTN_ID_DISCONNECT02,
	BTN_ID_DISCONNECT03,
	BTN_ID_DISCONNECT04,
	BTN_ID_DISCONNECT05,

	BTN_ID_CONNECT = BTN_ID_CLEAR+1,
	BTN_ID_DISCONNECT,
};
enum {
	DRAW_MODE_LINE,
	DRAW_MODE_BOX,
	DRAW_MODE_MARKER,
	DRAW_MODE_ERASER,
	DRAW_MODE_GETIMAGE,
};
enum {
	ACTION_NONE,
	ACTION_LINE_DRAWING,
	ACTION_BOX_DRAWING,
	ACTION_MARKER_DRAWING,
	ACTION_ERASER_DRAWING,
	ACTION_ALLCLEAR,
	ACTION_DISCONNECT,
	ACTION_GET_IMAGE,
	ACTION_RPLY_IMAGE,
};
#define COMSTR_LINE_DRAWING			"LINE"
#define COMSTR_BOX_DRAWING			"BOX"
#define COMSTR_MARKER_DRAWING		"MARKER"
#define COMSTR_ERASER_DRAWING		"ERASER"
#define COMSTR_ALLCLEAR				"ALLCLEAR"
#define COMSTR_CONNECT				"CONNECT"
#define COMSTR_CONNECT_R			"CONNECT_R"
#define COMSTR_DISCONNECT			"DISCONNECT"
#define COMSTR_DISCONNECT_R			"DISCONNECT_R"
#define COMSTR_GET_IMAGE			"GETIMAGE"
#define COMSTR_GET_IMAGE_R			"GETIMAGE_R"

#define COMMONFLG_NEED_MENU_REDRAW  0x0001

#define SEND_TO_ALL        0xffff
#define SEND_TO_CLIENT01   0x0001
#define SEND_TO_CLIENT02   0x0002
#define SEND_TO_CLIENT03   0x0004
#define SEND_TO_CLIENT04   0x0008
#define SEND_TO_CLIENT05   0x0010
#define SEND_TO_ALL_CLIENT 0x001F
#define SEND_TO_SERVER     0x0100

typedef struct POINTDATA_t {
	int x, y;
} POINTDATA;


class dbgxwin {
private:
	Atom a1, a2;
    Pixmap pixmap;	
	XFontSet fs;
	GC pgc,wgc;
	bool server_flg;

	int draw_mode;
	int action_mode;

	int get_image_current;

#define POINT_MAX 20
	POINTDATA point[POINT_MAX];
	int point_used;

public:
	static Display *display;
	static unsigned int common_flags;

	int width, height;
	Window window;


	// ============================================
	dbgxwin(int width, int height, bool server_flg) {

		this->server_flg  = server_flg;
		this->width       = width;
		this->height      = height;
		//DBGXWIN_PR("server_flg = %s\n", server_flg ? "true" : "false");

		draw_mode   = DRAW_MODE_MARKER;
		action_mode = ACTION_NONE;
		point_used  = 0;
		common_flags = 0;
		get_image_current = 0;

		/* ロケールの設定 */
		if(setlocale(LC_ALL, "") == NULL){
			fprintf(stderr, "Cannot set the locale.\n");
			exit(1);
		}
		
		/* Xサーバに接続 */
		display = XOpenDisplay(NULL);
		DBGXWIN_PR("connect X-Server\n");

		/* Xサーバに接続後、ロケールが使用可能かどうかをチェックする */
		if(XSupportsLocale() == False){
			fprintf(stderr, "X does not support the locale.\n");
			exit(1);
		}

		/* ウィンドウの生成 */
		window = XCreateSimpleWindow(display, DefaultRootWindow(display),
									 100, 100, width, height, 1,
									 BlackPixel(display, DefaultScreen(display)),
									 WhitePixel(display, DefaultScreen(display)));
		DBGXWIN_PR("Window created\n");
		
		// イベントマスクの設定
		XSelectInput(display, window, 0
					 | ExposureMask
					 | ButtonPressMask
					 | ButtonReleaseMask
					 | PointerMotionMask
					 | EnterWindowMask
					 | LeaveWindowMask
					 );

		// close ボタンの設定
		a1 = XInternAtom(display, "WM_PROTOCOLS", False);
		a2 = XInternAtom(display, "WM_DELETE_WINDOW", False);
		XSetWMProtocols(display, window, &a2, 1);

		// pixmap の生成
		pixmap = XCreatePixmap(display, window, width, height, DefaultDepth(display, 0));

		/* GCの生成 */
		wgc = XCreateGC(display, window, 0, 0);
		pgc = XCreateGC(display, pixmap, 0, 0);

		// 画面を白クリア
		XSetForeground(display, pgc, RGB(255, 255, 255));
		XFillRectangle(display, pixmap, pgc, 0, 0, width, height);
		XCopyArea(display, pixmap, window, wgc, 0, 0, width, height, 0, 0);

		/* フォントセットを生成する */
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
#if 0
			{
				int i;
				
				/* 足りない文字集合を表示する */
				printf("missing charset:\n");
				for(i=0; i<n_miss; i++)
					printf("missing charset:\t%s\n", miss[i]);
				
				/* デフォルト文字を表示する */
				if(def != NULL)
					printf("default character: %s\n", def);
			}
#endif
		}

		// メニューを表示
		this->menu_event(NULL,NULL);

		// Windowを表示
		XMapWindow(display, window);

		/* リクエスト送信 */
		XFlush(display);


	}

	// ============================================
	~dbgxwin(void) {
		/* Xサーバとの接続を解除 */
		XCloseDisplay(display);
	}

	// ============================================
	unsigned long RGB(unsigned char r,
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

	// ============================================
	bool recv_ev(XEvent *event) {

		bool ret = true;
		bool ev_used = true;

		if (event == NULL) {
			// イベント無し
			if ((common_flags&COMMONFLG_NEED_MENU_REDRAW) != 0) {
				// 強制REDRAW検出
				ret = this->menu_event(event,&ev_used);
				goto SKIP_EV_PROCESS;
			}
		}

		if(event->xclient.window == window){

			ret = this->menu_event(event,&ev_used);
			if (ev_used) {
				goto SKIP_EV_PROCESS;
			}

			// 自分宛てのイベント
			switch(event->type){
			case Expose:
				//DBGXWIN_PR("recv Expose\n");
				XCopyArea(display, pixmap, window, pgc, 0, 0, width, height, 0, 0);
				break;
			case ButtonPress:
				//DBGXWIN_PR("recv ButtonPress%d(%d,%d)\n",event->xbutton.button,event->xbutton.x,event->xbutton.y);
				if (event->xbutton.y < MENU_AREA_H) break; // menu表示領域は無効
				switch (action_mode) {
				case ACTION_NONE:
					if(event->xbutton.button == 1){ /* left click */
						// 始点を保存
						switch (draw_mode) {
						case DRAW_MODE_LINE:
							action_mode = ACTION_LINE_DRAWING;
							goto PRESS_COMMON1;
						case DRAW_MODE_BOX:
							action_mode = ACTION_BOX_DRAWING;
							goto PRESS_COMMON1;
						case DRAW_MODE_MARKER:
							action_mode = ACTION_MARKER_DRAWING;
							goto PRESS_COMMON1;
						case DRAW_MODE_ERASER:
							action_mode = ACTION_ERASER_DRAWING;
							goto PRESS_COMMON1;
						PRESS_COMMON1:
							point_used = 0;
							point[point_used].x = event->xbutton.x;
							point[point_used].y = event->xbutton.y;
							point_used++;
							break;
						}
					}
					break;
				}
				break;
			case ButtonRelease:
				//DBGXWIN_PR("recv ButtonRelease%d(%d,%d)\n",event->xbutton.button,event->xbutton.x,event->xbutton.y);
				switch (action_mode) {
				case ACTION_LINE_DRAWING:
				case ACTION_BOX_DRAWING:
				case ACTION_MARKER_DRAWING:
				case ACTION_ERASER_DRAWING:
					if(event->xbutton.button == 1){ /* left click */
						int x,y,w,h; // area情報
						
						if (point[point_used].y < MENU_AREA_H) {
							goto DRAW_COMMON_SKIP_DRAW;
						} // menu表示領域は無効、最終座標は入れずに描画確定

						// TODO：Window外でReleaseしたときも上記同様にすべき

						// 終点を確定
						point[point_used].x = event->xbutton.x;
						point[point_used].y = event->xbutton.y;
						point_used++;

					DRAW_COMMON:

#define EXTEND_xywh(lw) \
 { \
	 x = (x-lw/2) < 0 ? 0 : (x-lw/2); \
	 y = (y-lw/2) < 0 ? 0 : (y-lw/2); \
	 w += lw; \
	 h += lw; \
 }
						// 描画エリアの算出
						x = (point[point_used-2].x < point[point_used-1].x) ? point[point_used-2].x : point[point_used-1].x;
						y = (point[point_used-2].y < point[point_used-1].y) ? point[point_used-2].y : point[point_used-1].y;
						w = abs(point[point_used-2].x - point[point_used-1].x) +1;
						h = abs(point[point_used-2].y - point[point_used-1].y) +1;
						
						switch (action_mode) {
						case ACTION_LINE_DRAWING:
							XSetForeground(display, pgc, RGB(0, 0, 0));
							XSetLineAttributes(display, pgc, 4/*line_w*/, LineSolid, CapButt, JoinMiter);
							XDrawLine(display, pixmap, pgc, point[point_used-2].x, point[point_used-2].y, point[point_used-1].x, point[point_used-1].y);
							EXTEND_xywh(4); // 画面転送エリアを線の太さ分広げる
							break;
						case ACTION_BOX_DRAWING:
							XSetForeground(display, pgc, RGB(0, 0, 0));
							XSetLineAttributes(display, pgc, 4/*line_w*/, LineSolid, CapButt, JoinMiter);
							XDrawRectangle(display, pixmap, pgc, x,y,w-1,h-1); // -1は暫定
							EXTEND_xywh(4); // 画面転送エリアを線の太さ分広げる
							break;
						case ACTION_MARKER_DRAWING:
							XSetForeground(display, pgc, RGB(0, 0, 0));
							XSetLineAttributes(display, pgc, 4/*line_w*/, LineSolid, CapButt, JoinMiter);
							XDrawLine(display, pixmap, pgc, point[point_used-2].x, point[point_used-2].y, point[point_used-1].x, point[point_used-1].y);
							EXTEND_xywh(4); // 画面転送エリアを線の太さ分広げる
							break;
						case ACTION_ERASER_DRAWING:
							XSetForeground(display, pgc, RGB(255, 255, 255)); // 白
							XSetLineAttributes(display, pgc, 40/*line_w*/, LineSolid, CapRound, JoinMiter);
							XDrawLine(display, pixmap, pgc, point[point_used-2].x, point[point_used-2].y, point[point_used-1].x, point[point_used-1].y);
							EXTEND_xywh(40); // 画面転送エリアを線の太さ分広げる
							break;
						}
						// 画面に表示
						XCopyArea(display, pixmap, window, pgc, x,y,w,h,x,y);

					DRAW_COMMON_SKIP_DRAW:
						if (event->type == ButtonRelease) {
							// 描画コマンドを送る
							send_xml_command(action_mode,SEND_TO_ALL);

							// 描画状態をNONEにする
							action_mode = ACTION_NONE;
							point_used = 0;
						} else if (point_used == POINT_MAX) {
							// 描画コマンドを送る
							send_xml_command(action_mode,SEND_TO_ALL);

							// 最後の座標を先頭にコピーして、描画継続
							point[0].x = point[point_used-1].x;
							point[0].y = point[point_used-1].y;
							point_used = 1;
						}

					}
					break;
				}
				break;
			case MotionNotify:
				//DBGXWIN_PR("recv MotionNoify(%d,%d)\n",event->xmotion.x,event->xmotion.y);
				switch (action_mode) {
				case ACTION_MARKER_DRAWING:
				case ACTION_ERASER_DRAWING:
					// 終点を確定
					point[point_used].x = event->xmotion.x;
					point[point_used].y = event->xmotion.y;
					if (point[point_used].y < MENU_AREA_H) {
						break;
					} // menu表示領域は無効
					point_used++;
					goto DRAW_COMMON;
				}
				break;
			case EnterNotify:
				break;
			case LeaveNotify:
				break;
			case ClientMessage:
				DBGXWIN_PR("recv ClientMessage\n");
				if(event->xclient.message_type == a1 &&
				   event->xclient.data.l[0] == a2){
					ret = false;
				}
			}
		SKIP_EV_PROCESS:
			;
		}
		return ret;
	}


	// ============================================
	void set_title(char *title) {
		XTextProperty ct;
		XmbTextListToTextProperty(display, &title, 1, XCompoundTextStyle, &ct);
		XSetWMName(display, window, &ct);
	}	

	// ============================================
	bool menu_event(XEvent *event, bool *ev_used) {
		bool ret = true;
		MENU_BUTTON draw_btns[] = {
			{ 0,0, BTNW*0,0,BTNW,BTNH, (char*)"quit",	BTN_ID_QUIT },
			{ 0,0, BTNW*1,0,BTNW,BTNH, (char*)"line",	BTN_ID_LINE },
			{ 0,0, BTNW*2,0,BTNW,BTNH, (char*)"box",	BTN_ID_BOX },
			{ 0,0, BTNW*3,0,BTNW,BTNH, (char*)"marker",	BTN_ID_MARKER },
			{ 0,0, BTNW*4,0,BTNW,BTNH, (char*)"eraser",	BTN_ID_ERASER },
			{ 0,0, BTNW*5,0,BTNW,BTNH, (char*)"clear",	BTN_ID_CLEAR },
		};
		MENU_BUTTON server_btns[] = {
			{ 0,0, BTNW*6,0,BTNW,BTNH, (char*)"disconnect01",  BTN_ID_DISCONNECT01 },
			{ 0,0, BTNW*7,0,BTNW,BTNH, (char*)"disconnect02",  BTN_ID_DISCONNECT02 },
			{ 0,0, BTNW*8,0,BTNW,BTNH, (char*)"disconnect03",  BTN_ID_DISCONNECT03 },
			{ 0,0, BTNW*9,0,BTNW,BTNH, (char*)"disconnect04",  BTN_ID_DISCONNECT04 },
			{ 0,0, BTNW*10,0,BTNW,BTNH, (char*)"disconnect05", BTN_ID_DISCONNECT05 },
		};
		MENU_BUTTON client_btns[] = {
			{ 0,0, BTNW*6,0,BTNW,BTNH, (char*)"connect",    BTN_ID_CONNECT },
			{ 0,0, BTNW*7,0,BTNW,BTNH, (char*)"disconnect",	BTN_ID_DISCONNECT },
		};
		MENU_BUTTON *pbtn;

		int i;

		unsigned int btn_flg = 0,redraw_flg = 0,mask;
#define REDRAW_FLG_DRAW_BTN   0x003f
#define REDRAW_FLG_SERVER_BTN 0x07c0
#define REDRAW_FLG_CLIENT_BTN 0x00c0

		if (server_flg) {
			btn_flg = REDRAW_FLG_DRAW_BTN | REDRAW_FLG_SERVER_BTN;
		} else {
			btn_flg = REDRAW_FLG_DRAW_BTN | REDRAW_FLG_CLIENT_BTN;
		}

		// REDRAW検出
		if ((common_flags&COMMONFLG_NEED_MENU_REDRAW) != 0) {
			redraw_flg = btn_flg;
			common_flags &= (~COMMONFLG_NEED_MENU_REDRAW); // flag clear
		}
			
		if (event) {
 			//DBGXWIN_PR("event\n");
			*ev_used = false;

		} else {
			redraw_flg = btn_flg;
		}

		// 描画前処理
		for (mask=1,i=0; i<16; mask<<=1,i++) {
 			//DBGXWIN_PR("%x %x\n", i, mask);
			if (i < (sizeof(draw_btns)/sizeof(MENU_BUTTON))) {
				pbtn = &(draw_btns[i]);
			} else {
				if (server_flg) {
					pbtn = &(server_btns[i-(sizeof(draw_btns)/sizeof(MENU_BUTTON))]);
				} else {
					pbtn = &(client_btns[i-(sizeof(draw_btns)/sizeof(MENU_BUTTON))]);
				}
			}

			// X-Window イベント処理
			if (event) {
				if ((btn_flg&mask) != 0) {
					switch(event->type){
					case ButtonPress:
						if (pbtn->x <= event->xbutton.x && event->xbutton.x < (pbtn->x+pbtn->w)
							&& pbtn->y <= event->xbutton.y && event->xbutton.y < (pbtn->y+pbtn->h)) {
							*ev_used = true;
							if(event->xbutton.button == 1){
								/* left click */
								//DBGXWIN_PR("btn pressed %s\n", pbtn->mes);
								//{
								//	char buf[50];
								//	sprintf (buf, "btn pressed %s", pbtn->mes);
								//	if (server_flg) {
								//		server_sendstr_broadcast(buf);
								//	} else {
								//		client_sendstr(buf);
								//	}
								//}
								switch(i) {
								case BTN_ID_QUIT:
									ret = false;
									break;
								case BTN_ID_LINE:
									redraw_flg |= 1<<(draw_mode+BTN_ID_LINE);
									draw_mode = DRAW_MODE_LINE;
									redraw_flg |= 1<<i;
									break;
								case BTN_ID_BOX:
									redraw_flg |= 1<<(draw_mode+BTN_ID_LINE);
									draw_mode = DRAW_MODE_BOX;
									redraw_flg |= 1<<i;
									break;
								case BTN_ID_MARKER:
									redraw_flg |= 1<<(draw_mode+BTN_ID_LINE);
									draw_mode = DRAW_MODE_MARKER;
									redraw_flg |= 1<<i;
									break;
								case BTN_ID_ERASER:
									redraw_flg |= 1<<(draw_mode+BTN_ID_LINE);
									draw_mode = DRAW_MODE_ERASER;
									redraw_flg |= 1<<i;
									break;
								case BTN_ID_CLEAR:
									XSetForeground(display, pgc, RGB(255, 255, 255));
									XFillRectangle(display, pixmap, pgc, 0, 0, width, height);
									XCopyArea(display, pixmap, window, wgc, 0, 0, width, height, 0, 0);
									redraw_flg = btn_flg; // 全ボタン再描画
									// 描画コマンドを送る
									send_xml_command(ACTION_ALLCLEAR,SEND_TO_ALL);
									// MENU 再描画必要フラグをたてる
									common_flags |= COMMONFLG_NEED_MENU_REDRAW;
									break;
								default:
									if (server_flg) {
										switch(i) {
										case BTN_ID_DISCONNECT01:
										case BTN_ID_DISCONNECT02:
										case BTN_ID_DISCONNECT03:
										case BTN_ID_DISCONNECT04:
										case BTN_ID_DISCONNECT05:
											server_connection_close(i-BTN_ID_DISCONNECT01);
											// MENU 再描画必要フラグをたてる
											dbgxwin::common_flags |= COMMONFLG_NEED_MENU_REDRAW;
											break;
										}
									} else {
										switch(i) {
										case BTN_ID_CONNECT:
											client_connection_open();
											// MENU 再描画必要フラグをたてる
											dbgxwin::common_flags |= COMMONFLG_NEED_MENU_REDRAW;
#if 0
											// 初期イメージ転送開始
											draw_mode   = DRAW_MODE_GETIMAGE;
											get_image_current = 0;
											send_xml_command(ACTION_GET_IMAGE,SEND_TO_SERVER);
#endif
											break;
										case BTN_ID_DISCONNECT:
											client_connection_close();
											// MENU 再描画必要フラグをたてる
											dbgxwin::common_flags |= COMMONFLG_NEED_MENU_REDRAW;
											break;
										}
									}
								}
							}
						}
						break;
					case ButtonRelease:
						break;
					case EnterNotify:
						break;
					case LeaveNotify:
						break;
					}
				}
			}
		}

		// 描画処理
		for (mask=1,i=0; i<16; mask<<=1,i++) {
 			//DBGXWIN_PR("%x %x\n", i, mask);
			if (i < (sizeof(draw_btns)/sizeof(MENU_BUTTON))) {
				pbtn = &(draw_btns[i]);
			} else {
				if (server_flg) {
					pbtn = &(server_btns[i-(sizeof(draw_btns)/sizeof(MENU_BUTTON))]);
				} else {
					pbtn = &(client_btns[i-(sizeof(draw_btns)/sizeof(MENU_BUTTON))]);
				}
			}
			if ((btn_flg&mask) != 0 && (redraw_flg&mask) != 0) {
				// select 状態のチェック
				int chk_mode;
				switch(i) {
				case BTN_ID_LINE:
					chk_mode = DRAW_MODE_LINE;
					goto CHK;
				case BTN_ID_BOX:
					chk_mode = DRAW_MODE_BOX;
					goto CHK;
				case BTN_ID_MARKER:
					chk_mode = DRAW_MODE_MARKER;
					goto CHK;
				case BTN_ID_ERASER:
					chk_mode = DRAW_MODE_ERASER;
					goto CHK;
				CHK:
					if (draw_mode == chk_mode) {
						pbtn->attribute |= BTN_ATTR_SELECT;
					} else {
						pbtn->attribute &= (~BTN_ATTR_SELECT);
					}
					break;
				}
				if (server_flg) {
					switch(i) {
					case BTN_ID_DISCONNECT01:
					case BTN_ID_DISCONNECT02:
					case BTN_ID_DISCONNECT03:
					case BTN_ID_DISCONNECT04:
					case BTN_ID_DISCONNECT05:
						if (server_chk_connection(i-BTN_ID_DISCONNECT01) != NULL) {
							pbtn->attribute &= (~BTN_ATTR_GRAY);
						} else {
							pbtn->attribute |= BTN_ATTR_GRAY;
						}
						break;
					}
				} else {
					switch(i) {
					case BTN_ID_CONNECT:
						if (client_chk_connection() == NULL) {
							pbtn->attribute &= (~BTN_ATTR_GRAY);
						} else {
							pbtn->attribute |= BTN_ATTR_GRAY;
						}
						break;
					case BTN_ID_DISCONNECT:
						if (client_chk_connection() != NULL) {
							pbtn->attribute &= (~BTN_ATTR_GRAY);
						} else {
							pbtn->attribute |= BTN_ATTR_GRAY;
						}
						break;
					}
				}
				// ボタン描画処理
				{
					// 背景塗り潰し
					if ((pbtn->attribute&BTN_ATTR_GRAY) != 0) {
						XSetForeground(display, pgc, RGB(200, 200, 200));
					} else if ((pbtn->attribute&BTN_ATTR_SELECT) != 0) {
						XSetForeground(display, pgc, RGB(255, 255, 70));
					} else {
						XSetForeground(display, pgc, RGB(255, 255, 255));
					}
					XFillRectangle(display, pixmap, pgc, pbtn->x, pbtn->y, pbtn->w, pbtn->h);
					// 枠
					XSetForeground(display, pgc, RGB(0, 0, 0));
					XSetLineAttributes(display, pgc, 1/*line_w*/, LineSolid, CapButt, JoinMiter);
					XDrawRectangle(display, pixmap, pgc, pbtn->x, pbtn->y, pbtn->w, pbtn->h);
					// 文字列
					if ((pbtn->attribute&BTN_ATTR_GRAY) != 0) {
						XSetForeground(display, pgc, RGB(100, 100, 100));
					} else {
						XSetForeground(display, pgc, RGB(0, 0, 0));
					}
					XmbDrawString(display, pixmap, fs, pgc, pbtn->x+5, pbtn->y+30, pbtn->mes, strlen(pbtn->mes));
					// Windowに転送
					XCopyArea(display, pixmap, window, pgc, pbtn->x, pbtn->y, pbtn->w+1, pbtn->h+1, pbtn->x, pbtn->y);
				}
			}
		}
		return ret;
	}


	bool recv_xml_command(unsigned char *xmlstr, int xmlstr_size, int recv_from) {
		int i;

		xmlDocPtr doc;
		xmlNodePtr cur;

		int xml_command_action;
		int xml_command_pcount;
		POINTDATA xml_command[POINT_MAX];

		//DBGXWIN_PR("%s\n",xmlstr);

		doc = xmlReadMemory((const char*)xmlstr, xmlstr_size, (const char*)"noname.xml", NULL, 0);
		if (doc == NULL) {
			fprintf(stderr, "Failed to parse document\n");
			return true;		// とりあえず終了させない
		}
		
		cur = xmlDocGetRootElement(doc);
		if (cur == NULL) {
			fprintf(stderr,"empty document\n");
			xmlFreeDoc(doc);
			return true;		// とりあえず終了させない
		}
		
		if (xmlStrcmp(cur->name, (const xmlChar *)"command_packet") == 0) {
			// command packet
			cur = cur->xmlChildrenNode;
			while (cur != NULL) {
				if ((!xmlStrcmp(cur->name, (const xmlChar *)"command")) && cur->children){
					DBGXWIN_PR("RECV XMLCOM %s -> %s\n", cur->name, cur->children->content);
					if (strcmp((const char*)cur->children->content,COMSTR_LINE_DRAWING) == 0)			xml_command_action = ACTION_LINE_DRAWING;
					else if (strcmp((const char*)cur->children->content,COMSTR_BOX_DRAWING) == 0)		xml_command_action = ACTION_BOX_DRAWING;
					else if (strcmp((const char*)cur->children->content,COMSTR_MARKER_DRAWING) == 0)	xml_command_action = ACTION_MARKER_DRAWING;
					else if (strcmp((const char*)cur->children->content,COMSTR_ERASER_DRAWING) == 0)	xml_command_action = ACTION_ERASER_DRAWING;
					else if (strcmp((const char*)cur->children->content,COMSTR_ALLCLEAR) == 0)			xml_command_action = ACTION_ALLCLEAR;
					else if (strcmp((const char*)cur->children->content,COMSTR_DISCONNECT) == 0)		xml_command_action = ACTION_DISCONNECT;
					else if (strcmp((const char*)cur->children->content,COMSTR_GET_IMAGE) == 0)			xml_command_action = ACTION_GET_IMAGE;
					else xml_command_action = ACTION_NONE;
				} else

				if ((!xmlStrcmp(cur->name, (const xmlChar *)"pcount")) && cur->children){
					//DBGXWIN_PR("RECV XMLCOM %s -> %s\n", cur->name, cur->children->content);
					xml_command_pcount = atoi((const char*)cur->children->content);
					if (xml_command_pcount > POINT_MAX) xml_command_pcount = POINT_MAX; // limitter
				} else

				if ((!xmlStrcmp(cur->name, (const xmlChar *)"imgno")) && cur->children){
					//DBGXWIN_PR("RECV XMLCOM %s -> %s\n", cur->name, cur->children->content);
					get_image_current = atoi((const char*)cur->children->content);
				} else

				if ((!xmlStrcmp(cur->name, (const xmlChar *)"point")) && cur->children){
					char *dp,*p0,*p;
					//DBGXWIN_PR("RECV XMLCOM %s -> %s\n", cur->name, cur->children->content);
					p0 = dp = strdup((const char*)cur->children->content);
					i = 0;
					for (i=0; i<xml_command_pcount; i++) {
						if ((p=strtok(p0,",")) == NULL) { // error 座標がたりない
							xml_command_action = ACTION_NONE;
							DBGXWIN_PR("invalid point data\n");
							break;
						}
						point[i].x = atoi(p);
						p0 = NULL;
						if ((p=strtok(p0,",")) == NULL) { // error 座標がたりない
							xml_command_action = ACTION_NONE;
							DBGXWIN_PR("invalid point data\n");
							break;
						}
						point[i].y = atoi(p);
					}
					free(dp);
				}
				cur = cur->next;
			}
			//DBGXWIN_PR("xml_command_action = %d\n",xml_command_action);
			switch (xml_command_action) {
			case ACTION_ALLCLEAR:
				DBGXWIN_PR("RECV ALLCLEAR\n");
				XSetForeground(display, pgc, RGB(255, 255, 255));
				XFillRectangle(display, pixmap, pgc, 0, 0, width, height);
				XCopyArea(display, pixmap, window, wgc, 0, 0, width, height, 0, 0);
				// MENU 再描画必要フラグをたてる
				dbgxwin::common_flags |= COMMONFLG_NEED_MENU_REDRAW;
				break;
			case ACTION_LINE_DRAWING:
			case ACTION_BOX_DRAWING:
			case ACTION_MARKER_DRAWING:
				XSetForeground(display, pgc, RGB(0, 0, 0));
				XSetLineAttributes(display, pgc, 4/*line_w*/, LineSolid, CapButt, JoinMiter);
				goto DRAW_COMMON;
			case ACTION_ERASER_DRAWING:
				XSetForeground(display, pgc, RGB(255, 255, 255)); // 白
				XSetLineAttributes(display, pgc, 40/*line_w*/, LineSolid, CapRound, JoinMiter);
				goto DRAW_COMMON;
				
			DRAW_COMMON:
				{
					int x0,y0,x1,y1;
					int x,y,w,h;

					x0 = point[0].x;
					y0 = point[0].y;
					x1 = point[0].x;
					y1 = point[0].y;

					for (i=1; i<xml_command_pcount; i++) {
						// 描画エリアの算出
						if (x0 > point[i].x) x0 = point[i].x;
						if (y0 > point[i].y) y0 = point[i].y;
						if (x1 < point[i].x) x1 = point[i].x;
						if (y1 < point[i].y) y1 = point[i].y;
						switch (xml_command_action) {
						case ACTION_LINE_DRAWING:
							XDrawLine(display, pixmap, pgc, point[i-1].x, point[i-1].y, point[i].x, point[i].y);
							break;
						case ACTION_BOX_DRAWING:
							XDrawRectangle(display, pixmap, pgc, point[i-1].x,point[i-1].y,
										   abs(point[i-1].x-point[i].x)-1,
										   abs(point[i-1].y-point[i].y)-1
										   ); // -1は暫定
							break;
						case ACTION_MARKER_DRAWING:
							XDrawLine(display, pixmap, pgc, point[i-1].x, point[i-1].y, point[i].x, point[i].y);
							break;
						case ACTION_ERASER_DRAWING:
							XDrawLine(display, pixmap, pgc, point[i-1].x, point[i-1].y, point[i].x, point[i].y);
							break;
						}
					}
					// 描画エリアの算出
					x = x0;
					y = y0;
					w = x1 - x0;
					h = y1 - y0;
					EXTEND_xywh(40); // 画面転送エリアを線の太さ分広げる
					// 画面に表示
					XCopyArea(display, pixmap, window, pgc, x,y,w,h,x,y);
				}
				break;
				
			case ACTION_DISCONNECT:
				if (!server_flg) {
					client_connection_close();
					// MENU 再描画必要フラグをたてる
					dbgxwin::common_flags |= COMMONFLG_NEED_MENU_REDRAW;
				}
				break;
			case ACTION_GET_IMAGE:
				DBGXWIN_PR("RECV GET_IMAGE line=%d\n",get_image_current);
				{
					int i,x,x0,y,y0;
					PACKET_IMG_LINE *dp = (PACKET_IMG_LINE *)malloc(sizeof(PACKET_IMG_LINE));
					XImage *image;
					strcpy(dp->com, COMSTR_GET_IMAGE_R);
					dp->imgdt_size = sizeof(unsigned long) * GETIMG_MAX_PIXEL;
					dp->packet_no  = get_image_current;

					DBGXWIN_PR("RECV XML [GETIMAGE] imgdt_size=%d, line=%d\n",dp->imgdt_size,dp->packet_no);

					x0 = x = (get_image_current*GETIMG_MAX_PIXEL) % width;
					y0 = y = (get_image_current*GETIMG_MAX_PIXEL) / width + MENU_AREA_H;
					image = XGetImage(display, pixmap, 0, y, width, 2, 0xffffffff, ZPixmap);
					for (i=0; i<GETIMG_MAX_PIXEL; i++ ){
						dp->imgdt[i] = XGetPixel(image, x, y-y0);
						if (++x > width) {
							x = 0;
							y++;
							if (y >= height) break;
						}
					}
					DBGXWIN_PR("SEND [RPLYIMAGE] size=%d imgdt_size=%d, line=%d\n",sizeof(PACKET_IMG_LINE),dp->imgdt_size,dp->packet_no);
					server_sendbin((unsigned char*)dp, sizeof(PACKET_IMG_LINE), recv_from);
				}
				break;
			}

		} else {
		}

		xmlFreeDoc(doc);
		xmlCleanupParser();

		// 受信した文字列を他のクライアントに転送する
		if (server_flg) {
			int i;
			switch (xml_command_action) {
			case ACTION_ALLCLEAR:
			case ACTION_LINE_DRAWING:
			case ACTION_BOX_DRAWING:
			case ACTION_MARKER_DRAWING:
			case ACTION_ERASER_DRAWING:
				for (i=0; i<MAX_CLIENT; i++) {
					if (i != recv_from) {
						server_sendstr((char*)xmlstr, i);
					}
				}
				break;
			}
		}
		
		return true;
	}

	bool recv_bin_packet(unsigned char *bindata, int binsize, int recv_from) {

		if (strncmp((const char*)bindata,(const char*)COMSTR_GET_IMAGE_R,sizeof(COMSTR_GET_IMAGE_R)) == 0) {
			int i,x,x0,y,y0;
			PACKET_IMG_LINE *dp = (PACKET_IMG_LINE *)bindata;
			XImage *image;
			DBGXWIN_PR("RECV BIN [RPLYIMAGE] binsize=%d imgdt_size=%d line=%d\n",binsize,dp->imgdt_size,dp->packet_no);
			x0 = x = (dp->packet_no*GETIMG_MAX_PIXEL) % width;
			y0 = y = (dp->packet_no*GETIMG_MAX_PIXEL) / width + MENU_AREA_H;
			image = XGetImage(display, pixmap, 0, y0, width, 2, 0xffffffff, ZPixmap);
			DBGXWIN_PR("(%d,%d)\n",x,y);
			for (i=0; i<GETIMG_MAX_PIXEL; i++ ){
				//XPutPixel(image, x, y, dp->imgdt[i]);
				if (++x > width) {
					x = 0;
					y++;
					if (y >= height) break;
				}
			}
			//XPutImage(display, pixmap, pgc, image, 0,0, 0,y0, width,2);
			if (y >= height) {
				draw_mode = DRAW_MODE_MARKER;
			} else {
				get_image_current++;
				DBGXWIN_PR("get_image_current++ %d\n", get_image_current);
				send_xml_command(ACTION_GET_IMAGE,SEND_TO_SERVER);
			}
		}
		return true;
	}


	// サブ関数
	xmlNodePtr xml_create_node(unsigned char *node_name, unsigned char *text)
	{
		xmlNodePtr new_node = NULL;
		xmlNodePtr new_text = NULL;
		new_node = xmlNewNode(NULL, node_name);
		if (text != NULL) {
			new_text = xmlNewText(text);
			xmlAddChild(new_node, new_text);
		}
		return new_node;
	}
	void send_xml_command(int com, unsigned int send_to) {

		int i,len;
		char str[400],*p;
		
		xmlDocPtr doc;
		xmlNodePtr root_node;
		xmlNodePtr child_node;
		xmlChar *buf; // XML文字列		

		doc = xmlNewDoc((const xmlChar*)"1.0"); // ドキュメント生成
		root_node = xml_create_node((unsigned char*)"command_packet", (unsigned char*)NULL); // ルートノード作成
		xmlNewProp(root_node, (const xmlChar*)"Version", (const xmlChar*)"0.0.1"); // 属性追加
		xmlDocSetRootElement(doc, root_node); // ルート要素設定
		
		switch (com) {
		case ACTION_LINE_DRAWING:
			child_node = xml_create_node((unsigned char*)"command", (unsigned char*)COMSTR_LINE_DRAWING);
			goto POINTS_COMMON;
		case ACTION_BOX_DRAWING:
			child_node = xml_create_node((unsigned char*)"command", (unsigned char*)COMSTR_BOX_DRAWING);
			goto POINTS_COMMON;
		case ACTION_MARKER_DRAWING:
			child_node = xml_create_node((unsigned char*)"command", (unsigned char*)COMSTR_MARKER_DRAWING);
			goto POINTS_COMMON;
		case ACTION_ERASER_DRAWING:
			child_node = xml_create_node((unsigned char*)"command", (unsigned char*)COMSTR_ERASER_DRAWING);
			goto POINTS_COMMON;
		case ACTION_ALLCLEAR:
			child_node = xml_create_node((unsigned char*)"command", (unsigned char*)COMSTR_ALLCLEAR);
			xmlAddChild(root_node, child_node);
			child_node = xml_create_node((unsigned char*)"pcount", (unsigned char*)"0");
			xmlAddChild(root_node, child_node);
			break;
		POINTS_COMMON:
			xmlAddChild(root_node, child_node);

			sprintf (str,"%d",point_used);
			child_node = xml_create_node((unsigned char*)"pcount", (unsigned char*)str);
			xmlAddChild(root_node, child_node);

			for (p=str,i=0; i<point_used; i++) {
				len = sprintf (p,"%d,%d,",point[i].x,point[i].y);
				p += len;
			}
			len = sprintf (p,"%d,%d",-1,-1);
			child_node = xml_create_node((unsigned char*)"point", (unsigned char*)str);
			xmlAddChild(root_node, child_node);
			break;

		case ACTION_DISCONNECT:
			child_node = xml_create_node((unsigned char*)"command", (unsigned char*)COMSTR_DISCONNECT);
			xmlAddChild(root_node, child_node);
			break;
		case ACTION_GET_IMAGE:
			child_node = xml_create_node((unsigned char*)"command", (unsigned char*)COMSTR_GET_IMAGE);
			xmlAddChild(root_node, child_node);
			sprintf (str,"%d",get_image_current);
			DBGXWIN_PR("send get_image_current %d\n", get_image_current);
			child_node = xml_create_node((unsigned char*)"imgno", (unsigned char*)str);
			xmlAddChild(root_node, child_node);
			break;
		}

		{
			int size; // XML文字列のサイズ
			xmlDocDumpMemoryEnc(doc, &buf, &size, "UTF-8"); // UTF-8でXML文字列を取得
			if (!server_flg) {
				//DBGXWIN_PR("%s\n",buf);
				if ((send_to&SEND_TO_SERVER) != 0) {
					// サーバーに送る
					client_sendstr((char*)buf);
				}
			} else {
				int i, mask;
				//DBGXWIN_PR("%s\n",buf);
				for (mask=1,i=0; i<MAX_CLIENT; mask<<=1,i++) {
					if ((send_to&mask) != 0) {
						server_sendstr((char*)buf, i);
					}
				}
			}
		}

		xmlFree(buf);		
		xmlFreeDoc(doc);
	}

};

#endif // __TEST_H__
