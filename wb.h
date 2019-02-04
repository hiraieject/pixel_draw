
#ifndef __WB_H__
#define __WB_H__

#include <stdio.h>
#include <stdbool.h>

#include <vector>

#include "wb_platform.h"

// 相互参照のための proto type 宣言 
// ===============================
class WB_event;
class WB_basic;
class WB_object;
class WB_layer;
class WB_rootscreen;

// define/variable 宣言
// ===============================
typedef unsigned short RGBA32;

#define RGBA32_WHITE ((RGBA32)0xFFFFFFFF)
#define RGBA32_BLACK ((RGBA32)0xFFFFFFFF)
#define RGBA32_RED   ((RGBA32)0xFF0000FF)
#define RGBA32_GREEN ((RGBA32)0x00FF00FF)
#define RGBA32_BLUE  ((RGBA32)0x0000FFFF)
#define RGBA32_TP    ((RGBA32)0x00000000)

#define SCREEN_WIDTH  (1920/2)
#define SCREEN_HEIGHT (1080/2)
//#define SCREEN_WIDTH  (1920)
//#define SCREEN_HEIGHT (1080)

typedef enum {
	WB_EVENT_GES_DOWN,
	WB_EVENT_GES_MOVE,
	WB_EVENT_GES_UP,
	WB_EVENT_KEY_DOWN,
	WB_EVENT_KEY_UP,
} WB_EVENT_NO;

typedef enum {
	WB_PRESS_TP_TOUCH,
	WB_PRESS_MS_BTN1,
	WB_PRESS_MS_BTN2,
	WB_PRESS_MS_BTN3,
} WB_PRESS_BIT;

typedef struct {
	unsigned short x,y;
} WB_point;
typedef struct {
	unsigned short x,y;
	unsigned short w,h;
} WB_area;

// ===============================
// class 宣言
// ===============================

/*
  <data-tree>

  root_screen
	+-menu_layer
		+-menu_objects_list
	+-temporary_layer
		+-current_drawing_object
		+-current_selecting_object
	+-confirmed_layer
		+-confirmed_objects_list
*/		

// ----------------------------------------------------
// ################# WBアプリ内イベント
class WB_event {
public:
#define EV_RESERVE_MAX 40
	// ## variable
	static bool boot_initialed;
	static WB_basic *reservelist[EV_RESERVE_MAX];
	static unsigned short pressbitlist[EV_RESERVE_MAX];
	WB_basic *reserver;
	bool used;
	WB_EVENT_NO event_no;
	unsigned short tracking_id;
	unsigned short x,y,w,h;

	// ## constructor/destructor
	WB_event(WB_EVENT_NO evno, unsigned short trkid);
	~WB_event(void);

	// ## methods
	void set_reserve(bool rsv, WB_basic *evr);
};



// ----------------------------------------------------
// ################# WBアプリ 基本クラス
class WB_basic {
public:
	// ## variable
	static bool wb_enable;
	unsigned short now_tracking_id;
	WB_rootscreen *rootscreen;

	// ## constructor/destructor
	WB_basic (WB_basic *rt);

	// ## methods
	virtual void recv_ev (WB_event &ev);
	virtual void set_enable (bool enb);
	virtual void judge_inarea (WB_area &area, WB_point &p);
};



// ----------------------------------------------------
// ################# 描画オブジェクトクラス
class WB_object : public WB_basic {
public:
	// ## variable
	static unsigned short serial_count;
	unsigned short serial_no;
	// ## constructor/destructor
	WB_object(WB_basic *rt);
};

// - - - - - - - - - - - - - - - - - - - 
// ### 連続ライン描画クラス(派生クラス)
class WB_object_lines : public WB_object {
public:
	// ## variable
	std::vector<WB_point> points;

	// ## constructor/destructor
	WB_object_lines(WB_basic *rt);

	void recv_ev (WB_event &ev);
};
// - - - - - - - - - - - - - - - - - - - 
// ### ボックス描画クラス(派生クラス)
class WB_object_box : public WB_object {
public:
	// ## variable
	WB_point area;

	// ## constructor/destructor
	WB_object_box(WB_basic *rt);

	void recv_ev (WB_event &ev);
};

// - - - - - - - - - - - - - - - - - - - 
// ### ボックス描画クラス(派生クラス)
class WB_object_menu : public WB_object {
public:
	// ## variable
	WB_area area;

	// ## constructor/destructor
	WB_object_menu(WB_basic *rt);

	void recv_ev (WB_event &ev);
};


// ----------------------------------------------------
// ################# レイヤークラス
class WB_layer : public WB_basic {
protected:
	// ## variable
	unsigned char transparency;
	bool visible;
public:
	// ## constructor/destructor
	WB_layer(WB_basic *rt);

	// ## methods
	//void draw_lines (unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, RGBA32 col);
	//void draw_box (unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, RGBA32 col);
	//void draw_boxfill (unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1, RGBA32 col);
};

// - - - - - - - - - - - - - - - - - - - 
// ### メニューレイヤークラス(派生クラス)
class WB_layer_menu : public WB_layer {
public:
	// ## constructor/destructor
	WB_layer_menu(WB_basic *rt);

	// ## methods
	//void recv_ev (WB_event &ev);
};
// - - - - - - - - - - - - - - - - - - - 
// ### テンンポラリレイヤークラス(派生クラス)
class WB_layer_temporary : public WB_layer {
public:
	// ## constructor/destructor
	WB_layer_temporary(WB_basic *rt);

	// ## methods
	//void recv_ev (WB_event &ev);
};
// - - - - - - - - - - - - - - - - - - - 
// ### コンフォームドレイヤークラス(派生クラス)
class WB_layer_confirmed : public WB_layer {
public:
	// ## constructor/destructor
	WB_layer_confirmed(WB_basic *rt);

	// ## methods
	//void recv_ev (WB_event &ev);
};

// ----------------------------------------------------
// ################# rootスクリーンクラス
class WB_rootscreen : public WB_basic {
private:
	// ## variable
	WB_layer *menu_layer;
	WB_layer *temporary_layer;
	WB_layer *confirmed_layer;
public:
	// ## constructor/destructor
	WB_rootscreen (void);

	// ## methods
	void set_enable (bool enb);
	void recv_ev (WB_event &ev);
};


#endif // __WB_H__
