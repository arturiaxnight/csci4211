/*********************************************
 *
 * File name	    : sockcomm.c 
 * Description    : socket communication program
 *
 **********************************************/

#include "sockcomm.h"
#include <stdlib.h>
#include "sys/types.h"
#include "sys/uio.h"
#include "unistd.h"

/* Function: ConnectToServer
 * 1. Connect to specified server and port
 * 2. Return socket
 */
int ConnectToServer(const char* hostname, int port) {
  int sd;
  struct hostent *site;
  struct sockaddr_in me;
  site = gethostbyname(hostname);
  if(site == NULL)
    exit(1);
  sd = socket(AF_INET, SOCK_STREAM, 0);
  if(sd < 0)
    exit(1);
  memset(&me, 0, sizeof(struct sockaddr_in));
  memcpy(&me.sin_addr, site->h_addr_list[0], site->h_length);
  me.sin_family = AF_INET;
  me.sin_port = htons(port);
  return(connect(sd, (struct sockaddr*)&me, sizeof(struct sockaddr)) < 0 )? -1:sd;
}


/* Function: SocketInit
 * 1. Create a socket, bind to local IP and 'port', and listen to the socket
 * 2. Return the socket
 */
int SocketInit(int port) {
  int sd;
  sd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in ma;
  ma.sin_family = AF_INET;
  ma.sin_port = htons(port);
  ma.sin_addr.s_addr = INADDR_ANY;
  if(sd == -1){
    perror("socket");
    exit(1);
  }//end of testing socket
  bind(sd,(struct sockaddr*)&ma, sizeof(ma));
  listen(sd, 5);
  return(sd);
}


/* Function: AcceptCall
 * 1. Accept a connection request
 * 2. return the accepted socket
 */
int AcceptConnection(int sockfd) {
  int newsock;
  struct sockaddr_in ca;
  int addlen = sizeof(ca);
  newsock = accept(sockfd, (struct sockaddr*)&ca, &addlen);
  if(newsock < 0){
    perror("accept");
    exit(0);
  }
  return newsock;
}



/* Function: ReadMsg
 * Read a message
 */
int ReadMsg(int fd, char* buff, int size)
{
  int nread = read(fd, buff, size);
  buff[nread] = '\0';
  return(nread);
}

/* Function: SendMsg
 * receive a message
 */
int SendMsg(int fd, char* buff, int size)
{
  return(write(fd, buff, size));
}

