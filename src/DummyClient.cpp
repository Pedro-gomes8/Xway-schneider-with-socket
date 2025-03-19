// C++ program to illustrate the client application in the
// socket programming
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
//#include "../include/tram.h"

// Inclua a biblioteca JSON (baixe e coloque o arquivo "json.hpp" no seu projeto)
#include <nlohmann/json.hpp>
using json = nlohmann::json;

using namespace std;

#define CHECKERROR(var, val, msg) \
  if (var == val)                 \
  {                               \
    perror(msg);                  \
    exit(1);                      \
  }

void help_command(char name[])
{
    // Mudar isso futuramente
fprintf(stderr, "Usage: %s [server IP] [server port] [msg]\n", name);
}

int main(int argc, char *argv[]){


    if ((argc > 1 && strncmp(argv[1], "help", 4) == 0) || argc < 3)
    {
      help_command(argv[0]);
      exit(EXIT_SUCCESS);
    }

    // Create Socket for PC1 (TRAIN 1 ET TRAIN 2)

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    CHECKERROR(clientSocket, -1, "Creation socket client fail !!!\n");

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(atoi(argv[2]));
    serverAddress.sin_addr.s_addr = inet_addr(argv[1]);
   

    while(1){

    int ID_user;
    char ressource_user[3];

    cout <<"ID do train [1,2,3,4,...]" <<endl ;
    cin >> ID_user;
    cout <<"ressource [R1,R2,R3,R4,...]" <<endl ;
    cin >> ressource_user;

    if(ID_user == -1){
      cout <<"Sair do programa" <<endl ;
      break;
    }

    int connectClient = connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    CHECKERROR(connectClient, -1, "connectClientfail !!!\n");
    
    // Cria a mensagem JSON com o ID do trem e o recurso solicitado
    json message;
    message["train_id"] = ID_user;       // Exemplo: trem de ID 1
    message["resource"] = ressource_user;    // Exemplo: recurso R1 (pode ser R1 a R5)

    // Converte o objeto JSON para string
    std::string jsonStr = message.dump();


    //const char* message = argv[3];
    send(clientSocket, jsonStr.c_str(), jsonStr.size(), 0);

    // Lendo o acknowledge enviado pelo servidor
    char ack[1024];
    int bytesReceived = recv(clientSocket, ack, sizeof(ack) - 1, 0);
    
    CHECKERROR(bytesReceived, -1, "acknowledge problem !!!\n");

    ack[bytesReceived] = '\0'; // Garante que a string esteja terminada em nulo
    cout << "Acknowledge recebido do servidor: " << ack << endl;

    }

    close(clientSocket);

    return 0;
    
}
