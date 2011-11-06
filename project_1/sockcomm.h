#ifndef __SOCKCOMM_H
#define __SOCKCOMM_H

#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>

int ConnectToServer(const char* addr, int port);
int SocketInit(int port);
int AcceptConnection(int sockfd);
int ReadMsg(int fd, char* buff, int size);
int SendMsg(int fd, char* buff, int size);

#endif

  
