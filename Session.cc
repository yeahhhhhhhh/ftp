#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#include "Session.h"
#include "FtpProto.h"
#include "PrivParent.h"
#include "ConfParseUtil.h"
#include "PrivSock.h"

using namespace std;

Session::Session(){
	// control connect
	uid = 0;
	ctrlFd = -1;
	memset(cmdline, 0, sizeof(cmdline));
	memset(cmd, 0, sizeof(cmd));
	memset(arg, 0, sizeof(arg));
	
	// data connection
	portAddr = NULL;
	dataFd = -1;
	pasvListenFd = -1;

	// ipc
	parentFd = -1;
	childFd = -1;
	
	// status
	isASCII = 0;
}
Session::~Session(){}

void Session::beginSession(Session *session){
	PrivSock ps;
	ps.init(session);

	int pid;
	pid = fork();
	if(pid < 0)
		ERR_EXIT("beginSession-fork");
		
	if(pid == 0){
		// ftp server process
		ps.setChildContext(session);
		FtpProto ftpProto;
		ftpProto.handleChild(session);
	}else{
		// nobody process
		ps.setParentContext(session);
		PrivParent privParent;
		privParent.handleParent(session);
	}
}
