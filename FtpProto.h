#ifndef __FTPPROTO_H__
#define __FTPPROTO_H__

#include <iostream>
#include "Session.h"

using namespace std;

//struct FtpCmd{
//    const char* cmd;
//    void (*cmdHandle)(Session*);
//};

class FtpProto {
    public:
//	  FtpCmd* ftpCmds_;
	  
    public:
	  FtpProto ();
	  virtual ~FtpProto ();
	
	public:
	  void doUser(Session* session);
	  void doPass(Session* session);
	  void doSyst(Session* session);
	  void doFeat(Session* session);
	  void doPwd(Session* session);
	  void doType(Session* session);
	  void doPasv(Session* session);
	  void doPort(Session* session);
	  void doList(Session* session);
	  void doCwd(Session* session);
	  void doRetr(Session* session);
	  void doStor(Session* session);
	public:
	  void handleChild(Session *session);
	  int listCommon(Session* session);
	  int getTransferFd(Session* session);
	  int portActive(Session* session);
	  int pasvActive(Session* session);
	  int getPortFd(Session* session);
	  int getPasvFd(Session* session);
	  
};

#endif // __FTPPROTO_H__

