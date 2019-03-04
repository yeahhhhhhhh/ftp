#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <string.h>
#include <iostream>

#include <sys/syscall.h>

#include "PrivParent.h"
#include "Common.h"
#include "PrivSock.h"
#include "SysUtil.h"

using namespace std;

PrivParent::PrivParent(){}
PrivParent::~PrivParent(){}

void PrivParent::handleParent(Session *session){
	struct passwd *pw = getpwnam("nobody");
	if(pw == NULL)
		return;
	if(setegid(pw->pw_gid) < 0)
		ERR_EXIT("setegid");
	if(seteuid(pw->pw_uid) < 0)
		ERR_EXIT("seteuid");

	struct __user_cap_header_struct capHeader;
	struct __user_cap_data_struct capData;
	
	capHeader.version =  _LINUX_CAPABILITY_VERSION_2;
	capHeader.pid = 0;
	
	__u32 capMask = 0;
	capMask |= (1 << 10);
	
	capData.effective = capData.permitted = capMask;
	capData.inheritable = 0;
	
	capset(&capHeader, &capData);

	char cmd;
	PrivSock ps;
	while(1){
		cmd = ps.getCmd(session->parentFd);
		switch(cmd){
			case GET_DATA_SOCK:
				getDataSock(session);
				break;
			case PASV_ACTIVE:
				pasvActive(session);
				break;
			case PASV_LISTEN:
				pasvlisten(session);
				break;
			case PASV_ACCEPT:
				pasvAccept(session);
				break;
		}		
	}
}

void PrivParent::getDataSock(Session* session){
	PrivSock ps;
	unsigned short port = (unsigned short)ps.getInt(session->parentFd);
	char ip[16] = {0};
	ps.recvBuf(session->parentFd, ip, sizeof(ip));
	
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);
	
	SysUtil su;
	int fd = su.tcpClient(20);
	if(fd == -1){
		ps.sendResult(session->parentFd, RESULT_BAD);
		return;
	}
	if(connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0){
		close(fd);
		ps.sendResult(session->parentFd, RESULT_BAD);
		return;
	}
	ps.sendResult(session->parentFd, RESULT_OK);
	ps.sendFd(session->parentFd, fd);
	close(fd);
}
void PrivParent::pasvActive(Session* session){
	int active;
	if(session->pasvListenFd != -1){
		active = 1;
	}else{
		active = 0;
	}
	PrivSock ps;
	ps.sendInt(session->parentFd, active);
}
void PrivParent::pasvlisten(Session* session){
	SysUtil su;
	char ip[16] = {0};
	su.getLocalIp(ip);
	session->pasvListenFd = su.tcpServer(ip, 0);
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	if((getsockname(session->pasvListenFd, (struct sockaddr*)&addr, &addrlen)) < 0){
		ERR_EXIT("getsockname");
	}
	unsigned short port = ntohs(addr.sin_port);
	PrivSock ps;
	ps.sendInt(session->parentFd, (int)port);

}
void PrivParent::pasvAccept(Session* session){
	PrivSock ps;
	int fd = accept(session->pasvListenFd, NULL, NULL);
	close(session->pasvListenFd);
	if(fd < 0){
		ps.sendResult(session->parentFd, RESULT_BAD);
		return;
	}
	ps.sendResult(session->parentFd, RESULT_OK);
	ps.sendFd(session->parentFd, fd);
	close(fd);
}
int PrivParent::capset(cap_user_header_t hdrp, const cap_user_data_t datap){
	return syscall(__NR_capset, hdrp, datap);
}
