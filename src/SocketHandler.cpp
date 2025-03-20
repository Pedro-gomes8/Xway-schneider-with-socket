#include "../include/SocketHandler.hpp"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string>


SocketHandler::SocketHandler(const char* clientAddress, const char* servAddress,int port){
    this->sd1 = socket(AF_INET, SOCK_STREAM, 0);

    memset(&this->addrCli, 0, sizeof(this->addrCli));
    addrCli.sin_family = AF_INET;
    addrCli.sin_port = 0;
    addrCli.sin_addr.s_addr = inet_addr(clientAddress);

    memset(&this->addrServ, 0, sizeof(this->addrServ));
    addrServ.sin_family = AF_INET;
    addrServ.sin_addr.s_addr = inet_addr(servAddress);
    addrServ.sin_port = htons(port);

}

SocketHandler::~SocketHandler(){
    close(this->sd1);
}

int SocketHandler::connectSocket(){
    if (connect(sd1, (const struct sockaddr *)&addrServ, sizeof(struct sockaddr_in)) == -1){
        return 0;
    }
    return 1;
}


int SocketHandler::sendData(const void *buf){
    write(this->sd1, buf, sizeof(buf));
    return 0;
}

int SocketHandler::receiveData(void *buf){
    read(this->sd1, buf, sizeof(buf));
    return 0;
}

