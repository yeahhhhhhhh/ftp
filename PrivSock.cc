#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <iostream>
#include <unistd.h>

#include "PrivSock.h"

using namespace std;

PrivSock::PrivSock(){}
PrivSock::~PrivSock(){}

void PrivSock::init(Session* session){
	int sockfds[2];
	if(socketpair(PF_UNIX, SOCK_STREAM, 0, sockfds) < 0)
		ERR_EXIT("socketpair");
	session->parentFd = sockfds[0];
	session->childFd = sockfds[1];
}
void PrivSock::close(Session* session){
	if(session->childFd != -1){
		::close(session->childFd);
		session->childFd = -1;
	}
	if(session->parentFd != -1){
		::close(session->parentFd);
		session->parentFd = -1;
	}
}
void PrivSock::setParentContext(Session* session){
	if(session->childFd != -1){
		::close(session->childFd);
		session->childFd = -1;
	}
}
void PrivSock::setChildContext(Session* session){
	if(session->parentFd != -1){
		::close(session->parentFd);
		session->parentFd = -1;
	}
}

void PrivSock::sendCmd(int fd, char cmd){
	int ret;
	ret = write(fd, &cmd, sizeof(cmd));
	if(ret != sizeof(cmd)){
		fprintf(stderr, "sendCmd error.\n");
		exit(EXIT_FAILURE);
	}
}
char PrivSock::getCmd(int fd){
	char res;
	int ret;
	ret = read(fd, &res, sizeof(res));
	if(ret != sizeof(res)){
		fprintf(stderr, "getCmd error.\n");
		exit(EXIT_FAILURE);
	}
	return res;
}
void PrivSock::sendResult(int fd, char res){
	int ret;
	ret = write(fd, &res, sizeof(res));
	if(ret != sizeof(res)){
		fprintf(stderr, "sendResult error.\n");
		exit(EXIT_FAILURE);
	}
}
char PrivSock::getResult(int fd){
	char res;
	int ret;
	ret = read(fd, &res, sizeof(res));
	if(ret != sizeof(res)){
		fprintf(stderr, "getResult error.\n");
		exit(EXIT_FAILURE);
	}
	return res;
}

void PrivSock::sendInt(int fd, int theInt){
	int ret;
	ret = write(fd, &theInt, sizeof(theInt));
	if(ret != sizeof(theInt)){
		fprintf(stderr, "sendInt error.\n");
		exit(EXIT_FAILURE);
	}
}
int PrivSock::getInt(int fd){
	int res;
	int ret;
	ret = read(fd, &res, sizeof(res));
	if(ret != sizeof(res)){
		fprintf(stderr, "getInt error.\n");
		exit(EXIT_FAILURE);
	}
	return res;
}
void PrivSock::senfBuf(int fd, const char* buf, unsigned int len){
	sendInt(fd, (int)len);
	int ret = write(fd, buf, len);
	if(ret != (int)len){
		fprintf(stderr, "senfBuf error.\n");
		exit(EXIT_FAILURE);
	}
}
void PrivSock::recvBuf(int fd, char* buf, unsigned int len){
	unsigned int recvLen = (unsigned int)getInt(fd);
	if(recvLen > len){
		fprintf(stderr, "recvBuf::getInt error.\n");
		exit(EXIT_FAILURE);
	}
	int ret = read(fd, buf, recvLen);
	if(ret != (int)recvLen){
		fprintf(stderr, "recvBuf::read error.\n");
		exit(EXIT_FAILURE);
	}
}
void PrivSock::sendFd(int sockFd, int fd){
	struct cmsghdr* cmsg;
	cmsg = (struct cmsghdr*)malloc(sizeof(struct cmsghdr) + sizeof(int));
	cmsg->cmsg_len = sizeof(struct cmsghdr) + sizeof(int);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	
	memcpy(CMSG_DATA(cmsg), &fd, sizeof(fd));
	
	struct msghdr msg;
	msg.msg_control = cmsg;
	msg.msg_controllen = cmsg->cmsg_len;
	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	struct iovec iov[1];
	char buf[100] = "hello";
	iov[0].iov_base = buf;
	iov[0].iov_len = strlen("hello");
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	
	if(sendmsg(sockFd, &msg, 0) < 0)
		cout << "sendmsg error. " << endl;
}
int PrivSock::recvFd(int sockFd){
	int fd = -1;
	struct cmsghdr* cmsg;
	cmsg = (struct cmsghdr*)malloc(sizeof(struct cmsghdr) + sizeof(int));
	cmsg->cmsg_len = sizeof(struct cmsghdr) + sizeof(unsigned int);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;

	struct msghdr msg;
	msg.msg_control = cmsg;
	msg.msg_controllen = cmsg->cmsg_len;
	msg.msg_name = NULL;
	msg.msg_namelen = 0;

	char buf[100] = {0};
	int len = 100;
	struct iovec iov[1];
	iov[0].iov_base = buf;
	iov[0].iov_len = len;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	if(recvmsg(sockFd, &msg, 0) < 0)
		cout << "recvmsg error." << endl;
	memcpy(&fd, CMSG_DATA(cmsg), sizeof(unsigned int));
	return fd;
}
