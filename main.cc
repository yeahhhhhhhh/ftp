#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "SysUtil.h"
#include "Common.h"
#include "Session.h"
#include "ConfParseUtil.h"
#include "FtpProto.h"

using namespace std;

int main (int argc, char const* argv[]){
//	FtpProto f;
//	f.listCommon();
//	return 0;

//	SysUtil su2;
//	char ip[16] = {0};
//	su2.getLocalIp(ip);

//	return 0;

	Session session;

	SysUtil su;
	int listenfd = su.tcpServer(NULL, 5188);
	int conn;
	int pid;
	
	struct sockaddr_in cliaddr;
	memset(&cliaddr, 0, sizeof(cliaddr));
	socklen_t clilen;
	
	
	ConfParseUtil confParseUtil;
	cout << "pasvEnable = " << ConfParseUtil::pasvEnable << endl;
	cout << "portEnable = " << ConfParseUtil::portEnable << endl;
	cout << "listenPort = " << ConfParseUtil::listenPort << endl;
	cout << "maxClients = " << ConfParseUtil::maxClients << endl;
	cout << "maxPerIp = " << ConfParseUtil::maxPerIp << endl;
	cout << "connectTimeout = " << ConfParseUtil::connectTimeout << endl;
	cout << "idleSessionTime = " << ConfParseUtil::idleSessionTime << endl;
	cout << "dataConnectionTimeout = " << ConfParseUtil::dataConnectionTimeout << endl;
	cout << "localUmask = " << ConfParseUtil::localUmask << endl;
	cout << "uploadMaxRate = " << ConfParseUtil::uploadMaxRate << endl;
	cout << "downloadMaxRate = " << ConfParseUtil::downloadMaxRate << endl;
	
	while(1){
		cout << "waiting connection..." << endl;
		conn = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
		cout << "congratulation connected!" << endl;
		if(conn == -1)
			ERR_EXIT("accept");
		
		pid = fork();
		if(pid == 0){
			close(listenfd);
			Session session;
			session.ctrlFd = conn;
			session.beginSession(&session);
		}else{
			close(conn);
		}
			
	}
	return 0;
}
