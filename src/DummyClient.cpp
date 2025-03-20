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
fprintf(stderr, "Usage: %s [server IP] [server port] \n", name);
}

int main(int argc, char *argv[]){


    if ((argc > 1 && strncmp(argv[1], "help", 4) == 0) || argc < 3)
    {
      help_command(argv[0]);
      exit(EXIT_SUCCESS);
    }

    // Create Socket for PC1 (TRAIN 1 ET TRAIN 2)

  
    
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(atoi(argv[2]));
    serverAddress.sin_addr.s_addr = inet_addr(argv[1]);
   
  

    while(1){

    // Cria um novo socket para cada requisição
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    CHECKERROR(clientSocket, -1, "Creation socket client fail !!!\n");

    // Conecta ao servidor
    int connectClient = connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    CHECKERROR(connectClient, -1, "connectClient fail !!!\n");

    int ID_user;
    string ressource_user;
    string action_user;
    
    cout << "ID do train [1,2,3,4,...]" << endl;
    cin >> ID_user;
    cout << "resource [R1,R2,R3,R4,...]" << endl;
    cin >> ressource_user;
    cout << "action [lock, unlock, nothing]" << endl;
    cin >> action_user;
    
    if (ID_user == -1) {
        cout << "Sair do programa" << endl;
        close(clientSocket);
        break;
    }

    // Cria a mensagem JSON
    json message;
    message["train_id"] = ID_user;
    message["resource"] = ressource_user;
    message["action"] = action_user;

    std::string jsonStr = message.dump();
    send(clientSocket, jsonStr.c_str(), jsonStr.size(), 0);

    char ack[1024];
    int bytesReceived = recv(clientSocket, ack, sizeof(ack) - 1, 0);
    CHECKERROR(bytesReceived, -1, "acknowledge problem !!!\n");

    ack[bytesReceived] = '\0';
    cout << "Acknowledge recebido do servidor: " << ack << endl;

    close(clientSocket);
    }


    return 0;
    
}
