
#include <stdio.h>
#include <string.h>

#include "wb.h"

// =====================================================
// ===================================================== class WB_event
// =====================================================

bool WB_event::boot_initialed = false;
WB_basic *WB_event::reservelist[EV_RESERVE_MAX];

// reserve 属性の設定／解除
void WB_event::set_reserve(bool rsv, WB_basic *evr) {
	int i;

	if (rsv) {
		// reserve
		if (evr == NULL) {
			printf ("[%s:%d] ERROR: param error trkid=%04X\n", __FUNCTION__,__LINE__,tracking_id);
			return;	// param error
		}
		if (reserver != NULL) {
			return;		// RESERVE: this tracking_id is already reserved
		} else {
			for (i=0; i<EV_RESERVE_MAX; i++) {
				if (reservelist[i] == NULL) {
					reservelist[i] = evr;
					reserver = evr;
					evr->now_tracking_id = tracking_id;
					printf ("[%s:%d] reserve trkid=%04X\n", __FUNCTION__,__LINE__,tracking_id);
					return; // RESERVE:success
				}
			}
			printf ("[%s:%d] ERROR: fail reserve, no resource trkid=%04X\n", __FUNCTION__,__LINE__,tracking_id);
		}
	} else {
		// release
		if (reserver == NULL) {
			return;		// RESERVE:not reserved
		} else {
			for (i=0; i<EV_RESERVE_MAX; i++) {
				if (reservelist[i] == reserver) {
					reserver->now_tracking_id = 0xffff;
					reservelist[i] = NULL;
					reserver = NULL;
					printf ("[%s:%d] reserve trkid=%04X\n", __FUNCTION__,__LINE__,tracking_id);
					return; // RELEASE:success
				}
			}
			reserver = NULL;	// fail safe でクリア
			printf ("[%s:%d] ERROR: fail release trkid=%04X\n", __FUNCTION__,__LINE__,tracking_id);
		}
	}
}
// コンストラクタ
WB_event::WB_event(WB_EVENT_NO evno, unsigned short trkid)
{
	int i;
	if (boot_initialed == false) {
		// 起動時初期化
		memset (reservelist,0,sizeof(reservelist));
		boot_initialed = true;
	}
	tracking_id = trkid;
	event_no    = evno;
	used        = false;
	reserver    = NULL;
	for (i=0; i<EV_RESERVE_MAX; i++) {
		if (reservelist[i] != NULL
			&& reservelist[i]->now_tracking_id == tracking_id) {
			reserver = reservelist[i];	// リザーブしているオブジェクトのポインタを参照用にコピー
			break;
		}
	}
}
// デストラクタ
WB_event::~WB_event(void) {
	switch (event_no) {
	case WB_EVENT_GES_UP:
		set_reserve(false, (WB_basic *)NULL); // de reserve
		break;
	}
}
// =====================================================
// ===================================================== class WB_basic
// =====================================================
bool WB_basic::wb_enable;

WB_basic::WB_basic (WB_basic *rt) {
	rootscreen = (WB_rootscreen*)rt;
	now_tracking_id = 0xffff;
}
void WB_basic::recv_ev (WB_event &ev)
{
}
void WB_basic::set_enable (bool enb) {
	wb_enable = enb;
}
void WB_basic::judge_inarea (WB_area &area, WB_point &p) {

}


// =====================================================
// ===================================================== class WB_object
// =====================================================
unsigned short WB_object::serial_count = 0;

WB_object::WB_object(WB_basic *rt) : WB_basic(rt) {
	serial_no = serial_count++;
}
// - - - - - - - - - - - - - - - - - - - 
WB_object_lines::WB_object_lines(WB_basic *rt) : WB_object(rt) {
}
void WB_object_lines::recv_ev (WB_event &ev) {
}

// - - - - - - - - - - - - - - - - - - - 
WB_object_box::WB_object_box(WB_basic *rt) : WB_object(rt) {
}
void WB_object_box::recv_ev (WB_event &ev) {
}

// - - - - - - - - - - - - - - - - - - - 
WB_object_menu::WB_object_menu(WB_basic *rt) : WB_object(rt) {
}
void WB_object_menu::recv_ev (WB_event &ev) {
}

// =====================================================
// ===================================================== class WB_layer
// =====================================================
WB_layer::WB_layer(WB_basic *rt) : WB_basic(rt) {
	transparency = 0xff;
	visible = false;
}

// - - - - - - - - - - - - - - - - - - - 
WB_layer_menu::WB_layer_menu (WB_basic *rt) : WB_layer(rt) {
}
// - - - - - - - - - - - - - - - - - - - 
WB_layer_temporary::WB_layer_temporary (WB_basic *rt) : WB_layer(rt) {
}
// - - - - - - - - - - - - - - - - - - - 
WB_layer_confirmed::WB_layer_confirmed (WB_basic *rt) : WB_layer(rt) {
}


// =====================================================
// ===================================================== class WB_rootscreen
// =====================================================
WB_rootscreen::WB_rootscreen (void) : WB_basic (this)
{
	// disable 状態でレイヤーオブジェクトを生成する
	menu_layer      = new WB_layer_menu(this);
	temporary_layer = new WB_layer_temporary(this);
	confirmed_layer = new WB_layer_confirmed(this);
};
void WB_rootscreen::set_enable (bool enb)
{
	if (wb_enable != enb) {
		if (enb == true) {
			// 開始処理
		} else {
			// 終了処理
		}
		menu_layer->set_enable(enb);
		temporary_layer->set_enable(enb);
		confirmed_layer->set_enable(enb);
		WB_basic::set_enable(enb);		// 親クラスのメソッド呼出
	}
	
}

void WB_rootscreen::recv_ev (WB_event &ev) 
{
	if (wb_enable) {
		if (ev.reserver != NULL) {
			// リザーブされている
			//  -> リザーブしているOBJECTのみに配信する
			(ev.reserver)->recv_ev(ev);
		} else {
			// リザーブされていない
			//  -> 上のレイヤーから順番に配信する
			menu_layer->recv_ev(ev);
			temporary_layer->recv_ev(ev);
			confirmed_layer->recv_ev(ev);
		}
	}
};
