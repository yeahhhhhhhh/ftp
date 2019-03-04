#ifndef __SESSION_H__
#define __SESSION_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Common.h"

class Session {
    public:
	  // control connect
	  int uid;
	  int ctrlFd;
	  char cmdline[MAX_COMMAND_LINE];
	  char cmd[MAX_COMMAND];
	  char arg[MAX_ARG];
	  
	  // data connection
	  struct sockaddr_in* portAddr;
	  int dataFd;
	  int pasvListenFd;
	  
	  // ipc
	  int parentFd;
	  int childFd;
	  
	  // status
	  int isASCII;
	  
    public:
	  Session ();
	  virtual ~Session ();
	 
	public:
	  void beginSession(Session *session);
	  
};

#endif // __SESSION_H__

