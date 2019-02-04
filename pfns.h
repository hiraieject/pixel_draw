
// PjFpNetworkService

#define PTNS_TAG_COMMAND "connect"
#define PTNS_TAG_COMMAND "command"
#define PTNS_TAG_DRAWCOM "drawcom"

typedef enum PTNS_packet_command_t {
	PTNS_COM_CONNECT,
	PTNS_COM_DISCONNECT,
	PTNS_COM_GET_IMAGE,
	PTNS_COM_DRAW_LINE,
	PTNS_COM_DRAW_RECTANGLE,
	PTNS_COM_DRAW_MARKER,
	PTNS_COM_DRAW_ERASE,
	PTNS_COM_IMG_RAW,
	PTNS_COM_IMG_JPG,
	PTNS_COM_IMG_X11PIXMAP,
} PTNS_packet_command;

typedef struct PTNS_packet_header_t {
	unsigned char command;
	unsigned short datasize;
} PTNS_packet_header;

typedef struct PTNS_packet_connect_t {
	unsigned char name[10];
	unsigned char friendly_name[10];
	unsigned short draw_scr_size_x;
	unsigned short draw_scr_size_y;
	unsigned short draw_scr_mode;
	unsigned long capability;
} PTNS_packet_connect;

typedef struct PTNS_packet_connect_rply_t {
	unsigned char name[10];
	unsigned char friendly_name[10];
	unsigned short draw_scr_size_x;
	unsigned short draw_scr_size_y;
	unsigned short draw_scr_mode;
	unsigned long capability;
} PTNS_packet_reply_bool;

	unsigned char datatop[1];
	} basic;
	struct {
		unsigned char command;
		unsigned short datasize;
	} command;
	struct {
		unsigned char command;
		unsigned short datasize;
		
	} connect;
}

typedef struct PTNS_packet_connect {
	unsigned char command;
	unsigned short datasize;
	
};
