/*
	csci4211 Fall 2011
	Programming Assignment: Simple File Sharing System
*/
#define JOIN_PORT 8009
#define MAXMSGLEN  1024
#define MAXNAMELEN 128
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <errno.h>
#include <dirent.h>
#include "sockcomm.h"
/*
  User by a new peer host to join the system
  throught host 'peerhost'.
*/
int join(char *peerhost, int peerport)
{
  int sd;
  struct sockaddr_in sin;
  socklen_t len = sizeof(sin);
  sd = ConnectToServer(peerhost, peerport);
  if(getsockname(sd, (struct sockaddr*)&sin, &len) == -1)
    perror("getsockname");
  else
    printf("admin: connected to peer host on '%s' at '%hu' thru '%hu'\n", peerhost, peerport, ntohs(sin.sin_port));
  return(sd);
}

int main(int argc, char *argv[])
{
  int hJoinSock;
  int hListenSock;
  int hDataSock;
  int sock, newsock;
  int livesdmax ; /* maximum active descriptor */
  fd_set readset; /* used for select() */
  fd_set livesdset; /* used to keep track of active descriptors */
  char szDir[256]; 
  DIR *dp;
  struct dirent *dirp;
  char filelist[2048]; /* contains all file names in the shared directory, separated by '\r\n' */
  char szBuffer[2048];
  struct hostent *myhostent;
  struct timeval tv;
  struct sockaddr_in sa;
  socklen_t salen = sizeof(sa);
  if(argc!=2 && argc!=3)
  {
    printf("Usage: %s <directory pathname> [<peer name>]\n", argv[0]);
    exit(1);
  }
  FD_ZERO(&readset);
  FD_ZERO(&livesdset);
  /* argv[1] contains the pathname of the directory that is shared */
  strcpy(szDir,argv[1]);
  if((dp=opendir(szDir))==NULL)
  {
    printf("Can not open directory '%s'\n", szDir);
    exit(1);
  }
  else
  {
    filelist[0]='\0';
    while((dirp=readdir(dp))!=NULL)
    {
      if((strcmp(dirp->d_name,".")==0) || (strcmp(dirp->d_name,"..")==0)) continue;
      strcat(filelist,dirp->d_name);
      strcat(filelist,"\r\n");
    }
    closedir(dp);
  }

  /* start accepting join request of new peers */
  if((hJoinSock=SocketInit(JOIN_PORT))==-1)
  {
    perror("SocketInit");
    exit(1);
  }
  if(gethostname(szDir, sizeof(szDir))){return 1;} 
  if((myhostent = gethostbyname(szDir)) == NULL){
    printf("Error, no such host\n");
    return 1;
  }
  
  /*
    Use gethostname() and gethostbyname() to figure the full name of the host
  */
  printf("admin: started server on '%s' at '%hu'\n", myhostent->h_name, JOIN_PORT);

  /* if address is specified, join the system from the specified node */
  if(argc>2)
  {
    // connect to a known peer in the system
    sock=join(argv[2], JOIN_PORT);
    if(sock==-1) exit(0);
  }
  /*
    Prepare parameters for the select()
    Note: 'sock' is not set for the first member since it doesn't join others
  */
  tv.tv_sec = 30*6;
  tv.tv_usec = 0;
  FD_SET(hJoinSock, &livesdset);
  FD_SET(sock, &livesdset);
  livesdmax = hJoinSock;
  while(1)
  {
    int frsock;
    readset = livesdset;
    /* watch for stdin, hJoinSock and other peer sockets */
    if((select(livesdmax+1, &readset, NULL, NULL,&tv)) == -1){
      perror("select");
      exit(0);
    }
    for(frsock=3; frsock<=livesdmax; frsock++)
    {
      /* 
         frsock starts from 3 since descriptor 0, 1 and 2 are
         stdin, stdout and stderr
      */
      if(frsock==hJoinSock) continue;

      if(FD_ISSET(frsock,&readset))
      {
/*
	while(sizeof(szBuffer) > 0){
	  ReadMsg(frsock, szBuffer, sizeof(szBuffer));
	}
*/	
	//while(recv(frsock, szBuffer, sizeof(szBuffer), 0)){}
	if(recv(frsock, szBuffer, sizeof(szBuffer), 0) == 0){
	  printf("admin: disconnected from \"%s(%hu)\"\n",inet_ntoa(sa.sin_addr), ntohs(sa.sin_port));
	  close(frsock);
	  FD_CLR(frsock, &readset);
	}
        /*
          FILL HERE:
          1. Read message from socket 'frsock';
           2. If message size is 0, the peer host has disconnected. Print out message
             like "admin: disconnected from 'venus.cs.umn.edu(42453)'"
          3. If message size is >0, inspect whehter it is a 'GET' message
          3.1. Extract file name, IP address and port number in the 'GET' message
          3.2. Do local lookup on 'filelist' to check whether requested file is in 
               this peerhost
          3.3  If requested file presents, make connection to originating host and send data
          3.4  If requested file is not here, forward 'GET' message to all neighbors except
               the incoming one
        */
      }
    }

    /* input message from stdin */
    if(FD_ISSET(0,&readset))
    {
      if(!fgets(szBuffer,MAXMSGLEN,stdin)) exit(0);

      /*
        FILL HERE:
        1. Inspect whether input command is a valid 'get' command with file name
        2. create new listen socket for future data channel setup
        3. User fork() to generate a child process
        4. In the child process, use select() to monitor listen socket. Set a timout
           period for the select(). If no connection after timeout, close the listen
           socket. If select() returns with connection, use AcceptConnection() on listen
           socket to setup data connection. Then, download the file from remote peer.
        5. In the parent process, construct "GET" message and send it to all neighbor peers.
      */
    }

    /* join request on hJoinSock from a new peer */
    if(FD_ISSET(hJoinSock,&readset))
    {
      newsock = AcceptConnection(hJoinSock);
      getsockname(newsock, (struct sockaddr *)&sa, &salen);
      FD_SET(newsock, &livesdset);
      if(newsock >livesdmax)
	livesdmax = newsock;
      else
	livesdmax++;
      printf("admin: join from \"%s\" at \"%hu\"\n",inet_ntoa(sa.sin_addr), ntohs(sa.sin_port) );
      /*
        FILL HERE:
        1. Accept connection request for new joining peer host.
        2. Update 'livesdset' and 'livesdmax'
        3. print out message like "admin: join from 'venus.cs.umn.edu' at '34234'"
      */
    }
  }
}
