#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <pwd.h>
#include <shadow.h>
#include <unistd.h>
#include <crypt.h>
#include <dirent.h>
#include <time.h>
#include <fcntl.h>

#include "FtpProto.h"
#include "Common.h"
#include "StringUtil.h"
#include "SysUtil.h"
#include "PrivSock.h"

#include <iostream>
using namespace std;

FtpProto::FtpProto(){
//	FtpCmd ftpCmds[] = {{"USER",doUser},
//					    {"PASS",doPass},
//					    {NULL, NULL}};
//	ftpCmds_ = ftpCmds;
}
FtpProto::~FtpProto(){}

void FtpProto::handleChild(Session *session){
	write(session->ctrlFd, "220 (miniftp 0.1)\n", strlen("220 (miniftp 0.1)\n"));
	int ret;
	while(1){
		cout << "waiting for data..." << endl; 
		memset(session->cmdline, 0, sizeof(session->cmdline));
		ret = read(session->ctrlFd, session->cmdline, MAX_COMMAND_LINE);
		cout << "------session->cmdline = " << session->cmdline << endl;
		// handle ftp cmmand
		if(ret < 0)
			ERR_EXIT("read");
		else if(ret == 0)
			exit(EXIT_SUCCESS);
		else{
			StringUtil strUtil;
			strUtil.trimCtlf(session->cmdline);
			cout << "cmdline = " << session->cmdline << endl;
			strUtil.split(session->cmdline, session->cmd, session->arg, ' ');
			cout << "cmd = " << session->cmd << endl;
			cout << "arg = " << session->arg << endl;
			
//			FtpCmd* ftpCmds = ftpCmds_;
//			while(ftpCmds->cmd != NULL){
//				cout << "ftpCmds->cmd = " << ftpCmds->cmd << endl;
//				if(strcmp(ftpCmds->cmd, session->cmd) == 0){
//					cout << "--1" << endl;
//					if(ftpCmds->cmdHandle != NULL){
//						cout << "--2" << endl;
//						(ftpCmds->cmdHandle)(session);
//						cout << "--4" << endl;
//					}else{
//						write(session->ctrlFd, "502 Unimplements command.\r\n", strlen("502 Unimplements command.\r\n"));
//					}
//					break;
//				}
//				ftpCmds++;
//			}
//			write(session->ctrlFd, "500 Unknown command.\r\n", strlen("500 Unknown command.\r\n"));
			
			if(session->cmd != NULL){
				if(strcmp("USER",session->cmd) == 0){
					doUser(session);
				}else if(strcmp("PASS",session->cmd) == 0){
					doPass(session);
				}else if(strncmp("SYST", session->cmd, strlen("SYST")) == 0){
					doSyst(session);
				}else if(strncmp("FEAT", session->cmd, strlen("FEAT")) == 0){
					doFeat(session);
				}else if(strncmp("PWD", session->cmd, strlen("PWD")) == 0){
					doPwd(session);
				}else if(strncmp("TYPE", session->cmd, strlen("TYPE")) == 0){
					doType(session);
				}else if(strncmp("PASV", session->cmd, strlen("PASV")) == 0){
					doPasv(session);
				}else if(strncmp("PORT", session->cmd, strlen("PORT")) == 0){
					doPort(session);
				}else if(strncmp("LIST", session->cmd, strlen("LIST")) == 0){
					doList(session);
				}else if(strncmp("CWD", session->cmd, strlen("CWD")) == 0){
					doCwd(session);
				}else if(strncmp("RETR", session->cmd, strlen("RETR")) == 0){
					doRetr(session);			
				}else if(strncmp("STOR", session->cmd, strlen("STOR")) == 0){
					doStor(session);
				}else{
					write(session->ctrlFd, "500 Unknown command.\r\n", strlen("500 Unknown command.\r\n"));
				}
			}else{
				write(session->ctrlFd, "500 Unknown command.\r\n", strlen("500 Unknown command.\r\n"));
			}
			memset(session->cmd, 0, sizeof(session->cmd));
			memset(session->arg, 0, sizeof(session->arg));
		}
	}
}

int FtpProto::listCommon(Session* session){
	DIR* dir = opendir(".");
	if(dir == NULL){
		return 0;
	} 
	struct dirent* dt;
	struct stat sbuf;
	while((dt = readdir(dir)) != NULL){
		if(lstat(dt->d_name, &sbuf) < 0){
			continue;
		}
		if(dt->d_name[0] == '.'){
			continue;
		}	
		char perms[] = "----------";
		perms[0] = '?';
		mode_t mode = sbuf.st_mode;
		switch(mode & S_IFMT){
			case S_IFREG:
				perms[0] = '-';
				break;
			case S_IFDIR:
				perms[0] = 'd';
				break;
			case S_IFLNK:
				perms[0] = 'l';
				break;
			case S_IFIFO:
				perms[0] = 'p';
				break;
			case S_IFSOCK:
				perms[0] = 's';
				break;
			case S_IFCHR:
				perms[0] = 'c';
				break;
			case S_IFBLK:
				perms[0] = 'b';
				break;
		}
		
		if(mode & S_IRUSR){
			perms[1] = 'r';
		}
		if(mode & S_IWUSR){
			perms[2] = 'w';
		}
		if(mode & S_IXUSR){
			perms[3] = 'x';
		}
		if(mode & S_IRGRP){
			perms[4] = 'r';
		}
		if(mode & S_IWGRP){
			perms[5] = 'w';
		}
		if(mode & S_IXGRP){
			perms[6] = 'x';
		}
		if(mode & S_IROTH){
			perms[7] = 'r';
		}
		if(mode & S_IWOTH){
			perms[8] = 'w';
		}
		if(mode & S_IXOTH){
			perms[9] = 'x';
		}
		if(mode & S_ISUID){
			perms[3] = (perms[3] == 'x') ? 's' : 'S';
		}
		if(mode & S_ISGID){
			perms[6] = (perms[6] == 'x') ? 's' : 'S';
		}
		if(mode & S_ISVTX){
			perms[9] = (perms[9] == 'x') ? 's' : 'S';
		}
		
		char buf[1024] = {0};
		int off = 0;
		off += sprintf(buf, "%s", perms);
		off += sprintf(buf+off, " %3d %-8d %-8d ", (unsigned int)sbuf.st_nlink, sbuf.st_uid, sbuf.st_gid);
		off += sprintf(buf+off, "%8lu ", (unsigned long)sbuf.st_size);
		
		const char* pDateFormat = "%b %e %H:%H";
		struct timeval tv;
		gettimeofday(&tv, NULL);
		time_t localTime = tv.tv_sec;
		if(sbuf.st_mtime > localTime  || (localTime - sbuf.st_mtime) > 60*60*24*182){
			pDateFormat = "%b %e  %y";
		}
		char dateBuf[64] = {0};
		struct tm* pTm = localtime(&localTime);
		strftime(dateBuf, sizeof(dateBuf), pDateFormat, pTm);
		off += sprintf(buf+off, "%s ", dateBuf);
		if(S_ISLNK(sbuf.st_mode)){
			char temp[1024] = {0};
			readlink(dt->d_name, temp, sizeof(temp));
			sprintf(buf+off, "%s -> %s\r\n", dt->d_name, temp);
		}else{
			sprintf(buf+off, "%s\r\n", dt->d_name);
		}
		
//		cout << buf;
		write(session->dataFd, buf, strlen(buf));
	}
	closedir(dir);
	return 1;
}




void FtpProto::doUser(Session* session){
	cout << "3" << endl;
	struct passwd *pw = getpwnam(session->arg);
	if(pw == NULL){
		write(session->ctrlFd, "530 Login incorrect.\r\n", strlen("530 Login incorrect.\r\n"));
		return;
	}
	session->uid = pw->pw_uid;
	write(session->ctrlFd, "331 please specify the password.\r\n", strlen("331 please specify the password.\r\n"));
}

void FtpProto::doPass(Session* session){
	struct passwd* pw = getpwuid(session->uid);
	if(pw == NULL){
		write(session->ctrlFd, "530 Login incorrect.\r\n", strlen("530 Login incorrect.\r\n"));
		return;
	}
	struct spwd *sp = getspnam(pw->pw_name);
	if(sp == NULL){
		write(session->ctrlFd, "530 Login incorrect.\r\n", strlen("530 Login incorrect.\r\n"));
		return;
	}
	
	char* encryptedPass = crypt(session->arg, sp->sp_pwdp);
	if(strcmp(encryptedPass, sp->sp_pwdp) != 0){
		write(session->ctrlFd, "530 Login incorrect.\r\n", strlen("530 Login incorrect.\r\n"));
		return;
	}
	
	setegid(pw->pw_gid);
	seteuid(pw->pw_uid);
	chdir(pw->pw_dir);
	
	write(session->ctrlFd, "230 Login successful.\r\n", strlen("230 Login successful.\r\n"));
}

void FtpProto::doSyst(Session* session){
	write(session->ctrlFd, "215 UNIX Type:L8.\r\n", strlen("215 UNIX Type:L8.\r\n"));
}

void FtpProto::doFeat(Session* session){
	write(session->ctrlFd, "211-Features:\r\n", strlen("211-Features:\r\n"));
	write(session->ctrlFd, " EPRT\r\n", strlen(" EPRT\r\n"));
	write(session->ctrlFd, " EPSV\r\n", strlen(" EPSV\r\n"));
	write(session->ctrlFd, " MDTM\r\n", strlen(" MDTM\r\n"));
	write(session->ctrlFd, " PASV\r\n", strlen(" PASV\r\n"));
	write(session->ctrlFd, " REST STREAM\r\n", strlen(" REST STREAM\r\n"));
	write(session->ctrlFd, " SIZE\r\n", strlen(" SIZE\r\n")); 
	write(session->ctrlFd, " TVFS\r\n", strlen(" TVFS\r\n")); 
	write(session->ctrlFd, "211 End\r\n", strlen("211 End\r\n")); 
}

void FtpProto::doPwd(Session* session){
	char text[1024] = {0};
	char dir[1024+1] = {0};
	getcwd(dir, 1024);
	sprintf(text, "257 \"%s\"\r\n", dir);
	write(session->ctrlFd, text, strlen(text));
}

void FtpProto::doType(Session* session){
	if(strncmp(session->arg, "A", strlen("A")) == 0){
		session->isASCII = 1;
		write(session->ctrlFd, "200 Switching to ASCII mode.\r\n", strlen("200 Switching to ASCII mode.\r\n"));
	}else if(strcmp(session->arg, "I") == 0){
		session->isASCII = 0;
		write(session->ctrlFd, "200 Switching to Binary mode.\r\n", strlen("200 Switching to Binary mode.\r\n"));
	}else{
		write(session->ctrlFd, "500 Unrecognised TYPE command.\r\n", strlen("500 Unrecognised TYPE command.\r\n"));
	}
}

void FtpProto::doPasv(Session* session){
	char ip[16] = {0};
	SysUtil su;
	su.getLocalIp(ip);
	PrivSock ps;
	ps.sendCmd(session->childFd, PASV_LISTEN);
	unsigned short port = ps.getInt(session->childFd);
	
	unsigned int v[4];
	sscanf(ip, "%u.%u.%u.%u", &v[0], &v[1], &v[2], &v[3]);
	char text[1024] = {0};
	sprintf(text, "227 Entering Passive Mode (%u,%u,%u,%u,%u,%u).\r\n", v[0], v[1], v[2], v[3], port>>8, port&0xFF);
	write(session->ctrlFd, text, strlen(text));
}

void FtpProto::doPort(Session* session){
	unsigned int v[6];
	sscanf(session->arg, "%u,%u,%u,%u,%u,%u", &v[2],&v[3],&v[4],&v[5],&v[0],&v[1]);
	session->portAddr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
	memset(session->portAddr, 0, sizeof(struct sockaddr_in));
	session->portAddr->sin_family = AF_INET;
	unsigned char* p = (unsigned char*)&session->portAddr->sin_port;
	p[0] = v[0];
	p[1] = v[1];
	
	p = (unsigned char*)&session->portAddr->sin_addr;
	p[0] = v[2];
	p[1] = v[3];
	p[2] = v[4];
	p[3] = v[5];
	
	write(session->ctrlFd, "200 PORT command successful.\r\n", strlen("200 PORT command successful.\r\n"));
}

void FtpProto::doList(Session* session){
	// create data connection
	if(getTransferFd(session) == 0){
		return;
	}
	// 150
	write(session->ctrlFd, "150 Here comes the directory listing.\r\n", strlen("150 Here comes the directory listing.\r\n"));
	// transform list
	listCommon(session);
	// close 
	close(session->dataFd);
	session->dataFd = -1;
	// 226
	write(session->ctrlFd, "226 Directory send OK.\r\n", strlen("226 Directory send OK.\r\n"));
}

int FtpProto::getPortFd(Session* session){
	PrivSock ps;
	ps.sendCmd(session->childFd, GET_DATA_SOCK);
	unsigned short port = ntohs(session->portAddr->sin_port);
	char* ip = inet_ntoa(session->portAddr->sin_addr);
	ps.sendInt(session->childFd, (int)port);
	ps.senfBuf(session->childFd, ip, strlen(ip));
	
	char res = ps.getResult(session->childFd);
	if(res == RESULT_BAD){
		return 0;
	}else if(res == RESULT_OK){
		session->dataFd = ps.recvFd(session->childFd);
	}
	return 1;
}

int FtpProto::getPasvFd(Session* session){
	PrivSock ps;
	ps.sendCmd(session->childFd, PASV_ACCEPT);
	char res = ps.getResult(session->childFd);
	if(res == RESULT_BAD){
		return 0;
	}else if(res == RESULT_OK){
		session->dataFd = ps.recvFd(session->childFd);
	}
	return 1;
}

int FtpProto::getTransferFd(Session* session){
	if(!portActive(session) && !pasvActive(session)){
		write(session->ctrlFd, "425 Use PORT or PASV first.\r\n", strlen("425 Use PORT or PASV first.\r\n"));
		return 0;
	}
	int ret = 1;
	if(portActive(session)){
		if(getPortFd(session) == 0){
			ret = 0;
		}
	}
	
	if(pasvActive(session)){
		if(getPasvFd(session) == 0){
			ret = 0;
		}
	}
	
	if(session->portAddr){
		free(session->portAddr);
		session->portAddr = NULL;
	}
	return ret;
}

int FtpProto::portActive(Session* session){
	if(session->portAddr){
		if(pasvActive(session) == 1){
			cout << "both port and pasv are active." << endl;
			exit(EXIT_FAILURE);
		}
		return 1;
	}
	return 0;
}

int FtpProto::pasvActive(Session* session){
	PrivSock ps;
	ps.sendCmd(session->childFd, PASV_ACTIVE);
	int active = ps.getInt(session->childFd);
	if(active == 1){
		if(portActive(session) == 1){
			cout << "both port and pasv are active." << endl;
			exit(EXIT_FAILURE);
		}
		return 1;
	}

	return 0;
}

void FtpProto::doCwd(Session* session){
	if(chdir(session->arg) < 0){
		write(session->ctrlFd, "550 Failed to change director.\r\n", strlen("550 Failed to change director.\r\n"));
		return;
	}
	write(session->ctrlFd, "250 Directory successfully changed.\r\n", strlen("250 Directory successfully changed.\r\n"));
}

void FtpProto::doRetr(Session* session){
	// create data connection
	if(getTransferFd(session) == 0){
		return;
	}
	// 150
	write(session->ctrlFd, "150 Here comes the directory listing.\r\n", strlen("150 Here comes the directory listing.\r\n"));
	// open file
	int fd = open(session->arg, O_RDONLY);
	if(fd == -1){
		write(session->ctrlFd, "550 Could not create file.\r\n", strlen("550 Could not create file.\r\n"));
		return;
	}
	// add read lock
	SysUtil su;
	int ret;
	ret = su.lockFileRead(fd);
	if(ret == -1){
		write(session->ctrlFd, "550 Open file failed.\r\n", strlen("550 Open file failed.\r\n"));
		return;
	}
	struct stat sbuf;
	ret = fstat(fd, &sbuf);
	if(!S_ISREG(sbuf.st_mode)){
		write(session->ctrlFd, "550 Open file failed.\r\n", strlen("550 Open file failed.\r\n"));
		return;
	}
	char text[1024] = {0};
	if(session->isASCII){
		sprintf(text, "150 Opening ASCLL mode data connection for %s (%lld bytes).\r\n", session->arg, (long long)sbuf.st_size);
	}else{
		sprintf(text, "150 Opening BINARY mode data connection for %s (%lld bytes).\r\n", session->arg, (long long)sbuf.st_size);
	}
	write(session->ctrlFd, text, strlen(text));
	// download
	int flag;
	char buf[4096];
	while(1){
		ret = read(fd, buf, sizeof(buf));
		if(ret == -1){
			if(errno == EINTR){
				continue;
			}else{
				flag = -1;
				break;
			}
		}else if(ret == 0){
			flag = 0;
			break;
		}
		if(write(session->dataFd, buf, ret) != ret){
			flag = 2;
			break;
		}
	}
	// close 
	close(session->dataFd);
	session->dataFd = -1;
	close(fd);
	if(flag == 0){
		// 226
		write(session->ctrlFd, "226 Directory send OK.\r\n", strlen("226 Directory send OK.\r\n"));
	}else if(flag == 1){
		write(session->ctrlFd, "426 Failed reading from local file.\r\n", strlen("426 Failed reading from local file.\r\n"));
	}else if(flag == 2){
		write(session->ctrlFd, "451 Failed writting to network stream.\r\n", strlen("451 Failed writting to network stream.\r\n"));
	}
	
}

void FtpProto::doStor(Session* session){
	// create data connection
	if(getTransferFd(session) == 0){
		return;
	}
	// 150
	write(session->ctrlFd, "150 Here comes the directory listing.\r\n", strlen("150 Here comes the directory listing.\r\n"));
	// open file
	int fd = open(session->arg, O_CREAT | O_WRONLY, 0666);
	cout << "open fd = " << fd << endl;
	if(fd == -1){
		write(session->ctrlFd, "550 Open file failed.\r\n", strlen("550 Open file failed.\r\n"));
		return;
	}
	// add write lock
	SysUtil su;
	int ret;
	ret = su.lockFileWrite(fd);
	if(ret == -1){
		write(session->ctrlFd, "550 Lcok file failed.\r\n", strlen("550 Open file failed.\r\n"));
		return;
	}
	write(session->ctrlFd, "150 Ok to send data.\r\n", strlen("150 Ok to send data.\r\n"));
	
	int flag;
	char buf[1024];
	while(1){
		ret = read(session->dataFd, buf, sizeof(buf));
		if(ret == -1){
			if(errno == EINTR){
				continue;
			}else{
				flag = -1;
				break;
			}
		}else if(ret == 0){
			flag = 0;
			break;
		}
		if(write(fd, buf, ret) != ret){
			flag = 2;
			break;
		}
	}
	// close 
	close(session->dataFd);
	session->dataFd = -1;
	close(fd);
	if(flag == 0){
		// 226
		write(session->ctrlFd, "226 Directory send OK.\r\n", strlen("226 Directory send OK.\r\n"));
	}else if(flag == 1){
		write(session->ctrlFd, "426 Failed writting to local file.\r\n", strlen("426 Failed writting to local file.\r\n"));
	}else if(flag == 2){
		write(session->ctrlFd, "451 Failed reading from network stream.\r\n", strlen("451 Failed reading from network stream.\r\n"));
	}
}





