#include "connection.h"
#include "test01.h"

#define DBG_PR(fmt, args...) \
	printf("%s  [%s]:%d " fmt, (server_flg ? "server":"client"), __FUNCTION__,__LINE__, ## args)

// ���
#define MSGSIZE 1024
#define BUFSIZE (MSGSIZE + 1)

typedef struct _tagCHILD_DAEMON_INFO {
	unsigned int	nID;		// �̤��ֹ�					// add by mms takashima 2007.12.13
	unsigned int	ClientID;	// WM �� ID or IP���ɥ쥹	// add by mms takashima 2007.12.13
	int				realSock;	// ID�б� wmrd �Υ����å�	// add by mms takashima 2007.12.13
	int		ChildQId;           // �ҥǡ����Υ�å��������塼ID
	int		RealQId;			// �ꥢ�륿����ǡ����Υ�å��������塼ID
	int		DataQId;            // �ǡ��������ǡ����Υ�å��������塼ID
	//pid_t	pid_wmcd;			// wmcd�Υץ���ID
	//pid_t	pid_wmrd;			// wmrd�Υץ���ID
	//unsigned int ClientIP;
	struct in_addr ClientIP;
	unsigned char state;
} CHILD_DAEMON_INFO;
static CHILD_DAEMON_INFO taskTbl[MAX_CLIENT];        /* �ҥǡ������Ͽ�ơ��֥� */


bool server_main(unsigned short servPort, bool (*event_callback)(void *), bool (*xml_callback)(unsigned char*, int, int), bool (*bin_callback)(unsigned char*, int, int))
{

	bool ret = true;
	int i;
	struct timeval	tm;

    char recvBuffer[BUFSIZE]; //receive temporary buffer
    int byteRcvd; //received buffer size

    int servSock; //server socket descripter
    struct sockaddr_in servSockAddr; //server internet socket address
    struct sockaddr_in clitSockAddr; //client internet socket address
    unsigned int clitLen; // client internet socket address length

	// �ѿ������
	for (i=0; i<MAX_CLIENT; i++) {
		memset(&(taskTbl[i]),0,sizeof(CHILD_DAEMON_INFO));
		taskTbl[i].realSock = -1;
	}

	// port number �����å�
    if (servPort == 0) {
        fprintf(stderr, "invalid port number.\n");
        exit(EXIT_FAILURE);
    }

	// �����С��Ԥ����������åȤ�����
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 ){
        perror("socket() failed.");
        exit(EXIT_FAILURE);
    }

	// �����С��Ԥ����������åȤ���Ͽ
    memset(&servSockAddr, 0, sizeof(servSockAddr));
    servSockAddr.sin_family      = AF_INET;
    servSockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servSockAddr.sin_port        = htons(servPort);

    if (bind(servSock, (struct sockaddr *) &servSockAddr, sizeof(servSockAddr) ) < 0 ) {
        perror("bind() failed.");
        exit(EXIT_FAILURE);
    }

	// �����С��Ԥ���������
    if (listen(servSock, 2/*accept�Ԥ��ο�(!=������³��)*/) < 0) {
        perror("listen() failed.");
        exit(EXIT_FAILURE);
    }
	DBG_PR("SERVER LISTEN START at PORT=%d MAX_CLIENT=%d\n", servPort, MAX_CLIENT);

	// ���٥���Ԥ��롼��
    while(1) {
	    fd_set rfds;
		FD_ZERO(&rfds);

		FD_SET(servSock, &rfds); // listen socket
		for (i=0; i<MAX_CLIENT; i++) {
			if (taskTbl[i].realSock != -1) {
				FD_SET(taskTbl[i].realSock, &rfds); // client socket (MAX 5 client)
			}
		}

		tm.tv_sec  = 0 ;
		tm.tv_usec = 100*1000 ;		 // 100msec
		select(FD_SETSIZE, &rfds, 0, 0, &tm);	/* �ե�����ǥ�������ץ������å� */

		// ��³�Ԥ�����
		if(FD_ISSET (servSock, &rfds)) {
			// ��³�򸡽�
			dbgxwin::common_flags |= COMMONFLG_NEED_MENU_REDRAW;

			// ������¸�ΰ�򥵡���
			for (i=0; i<MAX_CLIENT; i++) {
				if (taskTbl[i].realSock == -1) {
					break;
				}
			}
			if (i >= MAX_CLIENT) {
				// �����С�¦�Υ꥽�����ζ������ʤ�
				//   ���ꡧ��ö�����Ĥ��ơ��������ڤꤹ��
				//   ������ɤ��Ȥ�٤�����Ĭ��
				int tmpsock;
				if ((tmpsock = accept(servSock, (struct sockaddr *) &clitSockAddr, &clitLen)) >= 0) {
					close(tmpsock);
				}
				continue;
			}

			// �������¸
			clitLen = sizeof(clitSockAddr);
			if ((taskTbl[i].realSock = accept(servSock, (struct sockaddr *) &clitSockAddr, &clitLen)) < 0) {
				perror("accept() failed.");
				exit(EXIT_FAILURE);
			}
			taskTbl[i].ClientIP	= clitSockAddr.sin_addr;
			DBG_PR("CONNECT taskNo=%d ip=%s\n", i, inet_ntoa(taskTbl[i].ClientIP));
		}
		// ���饤����Ȥ���Υǡ�������
		for (i=0; i<MAX_CLIENT; i++) {
			if(taskTbl[i].realSock != -1
			   && FD_ISSET (taskTbl[i].realSock, &rfds)) {  /* client ��������� */

				if ((byteRcvd = recv(taskTbl[i].realSock, recvBuffer, MSGSIZE, 0)) > 0) {
					recvBuffer[byteRcvd] = '\0';
					//DBG_PR("RECV [%s]\n", recvBuffer);
					if (strncmp(recvBuffer,"<?xml",strlen("<?xml")) == 0) {
						// XML ���٥�Ƚ���
						if (xml_callback) {
							if ((*xml_callback)((unsigned char*)recvBuffer,byteRcvd,i) == false) {
								goto EXIT_COMMON;
							}
						}
					} else {
						// BIN/��XML
						if (bin_callback) {
							if ((*bin_callback)((unsigned char*)recvBuffer,byteRcvd,i) == false) {
								goto EXIT_COMMON;
							}
						}
					}
				} else if(byteRcvd == 0){
					DBG_PR("CONNECTION CLOSE by CLIENT\n");
					taskTbl[i].realSock = -1;
					// MENU ������ɬ�ץե饰�򤿤Ƥ�
					dbgxwin::common_flags |= COMMONFLG_NEED_MENU_REDRAW;
				} else {
					perror("recv() failed.");
					taskTbl[i].realSock = -1;
					//exit(EXIT_FAILURE);
					// MENU ������ɬ�ץե饰�򤿤Ƥ�
					dbgxwin::common_flags |= COMMONFLG_NEED_MENU_REDRAW;
				}
			}
		}
		// X-Window ���٥�Ƚ���
		if (event_callback) {
			if ((*event_callback)(NULL) == false) {
			EXIT_COMMON:
				DBG_PR("detect exit\n");

				// ���饤����Ȥ�����
				for (i=0; i<MAX_CLIENT; i++) {
					if(taskTbl[i].realSock != -1) {
						DBG_PR("DISCONNECT taskNo=%d ip=%s\n", i, inet_ntoa(taskTbl[i].ClientIP));
						close(taskTbl[i].realSock);
						taskTbl[i].realSock = -1;
					}
				}

				// �����С��Ԥ����������
				DBG_PR("SERVER LISTEN END\n");
				close(servSock);

				ret = false;
				break;
			}
		}
    }

    return ret;
}

struct in_addr *server_chk_connection(int taskNo) {
	if(taskTbl[taskNo].realSock != -1) {
		//DBG_PR("return taskNo=%d ip=%s\n", taskNo, inet_ntoa(taskTbl[taskNo].ClientIP));
		return &(taskTbl[taskNo].ClientIP);
	}
	//DBG_PR("return NULL\n");
	return NULL;
};

void server_connection_close(int taskNo) {
	if(taskTbl[taskNo].realSock != -1) {
		close(taskTbl[taskNo].realSock);
		DBG_PR("CONNECTION CLOSE by USER\n");
		taskTbl[taskNo].realSock = -1;
	}
}

void server_sendbin(unsigned char *p, int len, int taskNo) {
	if(taskTbl[taskNo].realSock != -1) {
		// ��������
		if (p && len != 0) {
			if (send(taskTbl[taskNo].realSock, p, len, 0) <= 0) {
				perror("send() failed.");
				exit(EXIT_FAILURE);
			}
			//DBG_PR("SEND [%dbyte]\n", len);
		}
	}
}
void server_sendstr(char *p, int taskNo) {
	if(taskTbl[taskNo].realSock != -1) {
		server_sendbin((unsigned char*)p, strlen(p), taskNo);
	}
}
void server_sendstr_broadcast(char *p) {
	int i;
	for (i=0; i<MAX_CLIENT; i++) {
		server_sendstr(p, i);
	}
}

static int client_sock = -1; //local socket descripter
static struct sockaddr_in client_servSockAddr; //server internet socket address

bool client_main(char *ipstr, unsigned short servPort, bool (*event_callback)(void *), bool (*xml_callback)(unsigned char*, int, int), bool (*bin_callback)(unsigned char*, int, int))
{
	bool ret = true;
	struct timeval	tm;

    char recvBuffer[BUFSIZE]; //receive temporary buffer
    int byteRcvd; //received buffer size

    memset(&client_servSockAddr, 0, sizeof(client_servSockAddr));

    client_servSockAddr.sin_family = AF_INET;

	// �����С� IP ADDRESS ������
    if (ipstr != NULL
		&& inet_aton(ipstr, &client_servSockAddr.sin_addr) == 0) {
        fprintf(stderr, "Invalid IP Address.\n");
        exit(EXIT_FAILURE);
    }

	// �����С� PORT ������
    if (servPort == 0) {
        fprintf(stderr, "invalid port number.\n");
        exit(EXIT_FAILURE);
    }
    client_servSockAddr.sin_port = htons(servPort);

	// ���٥���Ԥ��롼��
    while(1) {
	    fd_set rfds;

		if (client_sock == -1) {
			usleep(100*1000);

		} else {

			FD_ZERO(&rfds);

			FD_SET(client_sock, &rfds); // listen socket

			tm.tv_sec  = 0 ;
			tm.tv_usec = 100*1000 ;		 // 100msec
			select(FD_SETSIZE, &rfds, 0, 0, &tm);	/* �ե�����ǥ�������ץ������å� */

			// ��³�Ԥ�����
			if(FD_ISSET (client_sock, &rfds)) {
				if ((byteRcvd = recv(client_sock, recvBuffer, MSGSIZE, 0)) > 0) {
					recvBuffer[byteRcvd] = '\0';
					//DBG_PR("RECV [%s]\n", recvBuffer);
					if (strncmp(recvBuffer,"<?xml",strlen("<?xml")) == 0) {
						// XML ���٥�Ƚ���
						if (xml_callback) {
							if ((*xml_callback)((unsigned char*)recvBuffer,byteRcvd,0) == false) {
								goto EXIT_COMMON;
							}
						}
					} else {
						// BIN/��XML
						if (bin_callback) {
							if ((*bin_callback)((unsigned char*)recvBuffer,byteRcvd,0) == false) {
								goto EXIT_COMMON;
							}
						}
					}
				} else if(byteRcvd == 0){
					DBG_PR("CONNECTION CLOSE by SERVER\n");
					client_sock = -1;
					// MENU ������ɬ�ץե饰�򤿤Ƥ�
					dbgxwin::common_flags |= COMMONFLG_NEED_MENU_REDRAW;
				} else {
					perror("recv() failed.");
					client_sock = -1;
					//exit(EXIT_FAILURE);
					// MENU ������ɬ�ץե饰�򤿤Ƥ�
					dbgxwin::common_flags |= COMMONFLG_NEED_MENU_REDRAW;
				}
			}
		}

		// X-Window ���٥�Ƚ���
		if (event_callback) {
			if ((*event_callback)(NULL) == false) {
			EXIT_COMMON:
				DBG_PR("detect exit\n");

				// ����
				client_connection_close();
				ret = false;
				break;
			}
		}
    }
	
    return ret;
}

void client_sendstr(char *p) {
	if(client_sock != -1) {
		int len;
		// ��������
		if (p && (len=strlen(p)) != 0) {
			if (send(client_sock, p, len, 0) <= 0) {
				perror("send() failed.");
				exit(EXIT_FAILURE);
			}
		}
	}
}
void client_connection_open(void) {
	if(client_sock == -1) {
		// �����åȤ�����
		if ((client_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 ){
			perror("socket() failed.");
			exit(EXIT_FAILURE);
		}
		// �����åȤ�Ȥäƥ����С�����³
		if (connect(client_sock, (struct sockaddr*) &client_servSockAddr, sizeof(client_servSockAddr)) < 0) {
			perror("connect() failed.");
			exit(EXIT_FAILURE);
		}
		DBG_PR("CONNECT ip=%s\n", inet_ntoa(client_servSockAddr.sin_addr));
	}
}
void client_connection_close(void) {
	if(client_sock != -1) {
		close(client_sock);
		DBG_PR("CONNECTION CLOSE by USER\n");
		client_sock = -1;
	}
}

struct in_addr *client_chk_connection(void) {
	if(client_sock != -1) {
		//DBG_PR("return ip=%s\n", inet_ntoa(client_servSockAddr.sin_addr));
		return &(client_servSockAddr.sin_addr);
	}
	//DBG_PR("return NULL");
	return NULL;
}
