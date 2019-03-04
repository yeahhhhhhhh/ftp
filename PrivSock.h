#ifndef __PRIVSOCK_H__
#define __PRIVSOCK_H__

#include "Session.h"

#define GET_DATA_SOCK 1
#define PASV_ACTIVE 2
#define PASV_LISTEN 3
#define PASV_ACCEPT 4

#define RESULT_OK 1
#define RESULT_BAD 2


class PrivSock {
    private:
	  /* data */
	  
    public:
	  PrivSock ();
	  virtual ~PrivSock ();
	 
	public:
	  void init(Session* session);
	  void close(Session* session);
	  void setParentContext(Session* session);
	  void setChildContext(Session* session);
	  
	  void sendCmd(int fd, char cmd);
	  char getCmd(int fd);
	  void sendResult(int fd, char res);
	  char getResult(int fd);
	  
	  void sendInt(int fd, int theInt);
	  int getInt(int fd);
	  void senfBuf(int fd, const char* buf, unsigned int len);
	  void recvBuf(int fd, char* buf, unsigned int len);
	  void sendFd(int sockFd, int fd);
	  int recvFd(int sockFd);
	  
};

#endif // __PRIVSOCK_H__

