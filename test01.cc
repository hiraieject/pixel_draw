#include <unistd.h>
#include <string.h>

#include "wb.h"
#include "test01.h"

#define DBG_PR(fmt, args...) \
	printf("%s  [%s]:%d " fmt, (server_flg ? "server":"client"), __FUNCTION__,__LINE__, ## args)

// static member variable
Display *dbgxwin::display = NULL;
unsigned int dbgxwin::common_flags = 0;

bool server_flg = false;
static unsigned short port;
static char *ipstr = NULL;
static dbgxwin *dw = NULL;

bool
bin_packet_callback(unsigned char *bindata, int binsize, int recv_from)
{
	if (dw != NULL) {
		if (bindata != NULL && binsize != 0) {
			if (dw->recv_bin_packet(bindata,binsize,recv_from) == false) {
				DBG_PR("detect exit\n");
				delete dw;
				dw = NULL;
				return false;
			}
		}
	}
	return true;
}
bool
xml_event_callback(unsigned char *xmlstr, int xmlstr_size, int recv_from)
{
	if (dw != NULL) {
		if (xmlstr != NULL && xmlstr_size != 0) {
			if (dw->recv_xml_command(xmlstr,xmlstr_size,recv_from) == false) {
				DBG_PR("detect exit\n");
				delete dw;
				dw = NULL;
				return false;
			}
		}
	}
	return true;
}
bool
draw_event_callback(void *rcvdt)
{
	XEvent event;

	if (dw == NULL) {
		char buf[60];
		dw = new dbgxwin(1920/2/*w*/, 1200/2/*h*/, server_flg);

		if (server_flg) {
			sprintf (buf, "server window port=%d", port);
		} else {
			sprintf (buf, "client window server_ip=%s server_port=%d", ipstr, port);
		}
		dw->set_title(buf);
	}
	
	if (rcvdt != NULL) {

	} else {
		/* X-Window イベントチェック */
		while (XPending(dw->display) > 0) {
			XNextEvent(dw->display, &event);
			
			if (dw->recv_ev(&event) == false) {
				DBG_PR("detect exit\n");
				delete dw;
				dw = NULL;
				return false;
			}
		}
		if ((dbgxwin::common_flags&COMMONFLG_NEED_MENU_REDRAW) != 0) {
			// Xのイベント無いけど、modifyがたってるので、コールする
			if (dw->recv_ev(NULL) == false) {
				DBG_PR("detect exit\n");
				delete dw;
				dw = NULL;
				return false;
			}
		}
	}
	return true;
}

int main(int argc, char *argv[])
{
	puts("");

	// 引数の解析
	{
		int flags, opt;
		int nsecs, tfnd;
		
		nsecs = 0;
		tfnd = 0;
		flags = 0;
		while ((opt = getopt(argc, argv, "sci:p:")) != -1) {
			switch (opt) {
			case 's':			// server mode(default)
				server_flg = true;
				break;
			case 'c':			// client mode
				server_flg = false;
				break;
			case 'i':			// ipv4 address (server only)
				ipstr = strdup(optarg);
				break;
			case 'p':			// port number
				port = atoi(optarg);
				break;
			default: /* '?' */
				fprintf(stderr, "Usage: %s [-m <:port>] [-s <ipaddress:port>]\n",
						argv[0]);
				exit(EXIT_FAILURE);
			}
		}
	}

	if (server_flg) {
		DBG_PR("call server_main\n");
		server_main(port,draw_event_callback, xml_event_callback, bin_packet_callback);
	} else {
		DBG_PR("call client_main\n");
		client_main(ipstr,port,draw_event_callback, xml_event_callback, bin_packet_callback);
	}

	exit(0);
}

