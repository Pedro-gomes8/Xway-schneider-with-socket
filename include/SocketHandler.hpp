#pragma once

#include <arpa/inet.h>
#include <sys/socket.h>

class SocketHandler{
    public:
    int sd1;
    struct sockaddr_in addrServ, addrCli;
    SocketHandler(const char* clientAddress, const char* servAddress,int port);
    //~SocketHandler();

    int connectSocket();

    int sendData(const void *buf,size_t size);

    int receiveData(void *buf, size_t size);
};