#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include <stdio.h> //printf(), fprintf(), perror()
#include <sys/socket.h> //socket(), bind(), accept(), listen()
#include <arpa/inet.h> // struct sockaddr_in, struct sockaddr, inet_ntoa()
#include <stdlib.h> //atoi(), exit(), EXIT_FAILURE, EXIT_SUCCESS
#include <string.h> //memset()
#include <unistd.h> //close()

#define MAX_CLIENT 5

bool server_main(unsigned short servPort, bool (*event_callback)(void *), bool (*xml_callback)(unsigned char*, int, int), bool (*bin_callback)(unsigned char*, int, int));
struct in_addr *server_chk_connection(int taskNo);
void server_connection_close(int taskNo);
void server_sendbin(unsigned char *p, int len, int taskNo);
void server_sendstr(char *p, int taskNo);
void server_sendstr_broadcast(char *p);


bool client_main(char *ipstr, unsigned short servPort, bool (*event_callback)(void *), bool (*xml_callback)(unsigned char*, int, int), bool (*bin_callback)(unsigned char*, int, int));
void client_sendstr(char *p);
void client_connection_open(void);
void client_connection_close(void);
struct in_addr *client_chk_connection(void);

#endif // __CONNECTION_H__
