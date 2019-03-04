CC=g++
CFLAGS=-Wall -g
BIN=ftp_test
OBJS=main.o SysUtil.o Session.o FtpProto.o PrivParent.o StringUtil.o ConfParseUtil.o PrivSock.o
$(BIN):$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ -lcrypt
	rm -f *.o
%.o:%.cc
	$(CC) $(CFLAGS) -c $< -o $@
clear:
	rm -f *.o
