#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <thread>

//#include "../include/mapping.h"
//#include "../include/Tram.h"

using namespace std;

void help_command(char name[])
{
fprintf(stderr, "Usage: %s [local IP] [remote IP=localhost] [remote port = %d]\n", name,
        502);
}

std::mutex R1, R2, R3, R4, R5;

void watchTrain(){

    // fica lendo as mensagens do socket especifico
    // TODO: passar socket como argumento da função
    
    // Enquanto não receber msg de finalisar :
        // Ler o identificador para saber qual dos dois trens ele deve responder
        // Lê o codigo referente a qual recurso o trem quer
        // Solicita o recurso
            // Se o recurso estiver livre manda 1 como resposta via socket
            // Se não, espera ate que o recurso se libere
    // exit
    
}

int main(int argc, char *argv[]){


    if ((argc > 1 && strncmp(argv[1], "help", 4) == 0) || argc < 3)
    {
      help_command(argv[0]);
      exit(EXIT_SUCCESS);
    }

    int gest_socket = socket(AF_INET,SOCK_STREAM,0);
    sockaddr_in gestAddress;
    gestAddress.sin_family = AF_INET;
    gestAddress.sin_port = 0;
    gestAddress.sin_addr.s_addr = inet_addr(argv[1]);

    // binding gestionnaire socket
    bind(gest_socket, (struct sockaddr*)&gestAddress,sizeof(gestAddress) );

    listen(gest_socket, 2);

    int clientSocket = accept(serverSocket, nullptr, nullptr);

    char buffer[1024] = {0};
    recv(clientSocket, buffer, sizeof(buffer), 0);
    cout << "Message from client: " << buffer << endl;  
    

    // create threads, one for each PC
    /*
    thread pc1(watchTrain);
    thread pc2(watchTrain);
    */

    return 0;
    
}
