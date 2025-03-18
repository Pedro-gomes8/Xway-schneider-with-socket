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
//#include "../include/Tram.h"

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
fprintf(stderr, "Usage: %s [server IP] [server port]\n", name);
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
   
    int connectClient = connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    CHECKERROR(connectClient, -1, "connectClientfail !!!\n");
    
    const char* message = "Hello, server!";
    send(clientSocket, message, strlen(message), 0);

    close(clientSocket);

    return 0;
    
}
