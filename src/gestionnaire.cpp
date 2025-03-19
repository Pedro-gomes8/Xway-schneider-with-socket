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
fprintf(stderr, "Usage: %s [local IP] [port]\n", name);
}

std::mutex R1, R2, R3, R4, R5;

void watchTrain(int serverSocket){


    std::cout << "ID thread: " << std::this_thread::get_id() << std::endl;
    
    // Here we set a queue with length 2 
    CHECKERROR(listen(serverSocket, 2),-1," Listen Socket failed \n");

    char buffer[1024];
    ssize_t n;

    while(strncmp(buffer, "fim", 3)!= 0){
        struct sockaddr_in clientAddress;
        socklen_t clientLen = sizeof(clientAddress);

        std::cout << "Try to accept:" << std::endl; 
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientLen);
        CHECKERROR(clientSocket,-1," Listen Socket failed \n");
        std::cout << "Accept done" << std::endl; 

        
        std::cout << "Try to read:" << std::endl; 
        n = read(clientSocket, buffer, sizeof(buffer) - 1);
        std::cout << "read done:" << std::endl; 
        
        buffer[n] = '\0';
        std::cout << "Received: " << buffer <<" from ID thread: " << std::this_thread::get_id() << std::endl;
       
        if(strncmp(buffer, "R1", 7) == 0){
            unique_lock<mutex> R1Lock(R1);
            cout << "R1 locked" << endl;
            this_thread::sleep_for(chrono::seconds(2));
            cout << "R1 unlocked by thread " << this_thread::get_id() << endl;
            R1Lock.unlock();
        }
    
        const char *ack = "ACK";
        if (send(clientSocket, ack, strlen(ack), 0) < 0) {
            perror("send ACK");
        }

        close(clientSocket);
        std::cout << "Client socket closed: " << std::endl;

    }
    
    // DONE: passar socket como argumento da função
    
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

    // Create Socket for PC1 (TRAIN 1 ET TRAIN 2)

    int socketPC1 = socket(AF_INET,SOCK_STREAM,0);

    CHECKERROR(socketPC1, -1, "Creation socket PC1 fail !!!\n");

    sockaddr_in pc1Address;
    pc1Address.sin_family = AF_INET;
    pc1Address.sin_port = htons(atoi(argv[2]));
    pc1Address.sin_addr.s_addr = inet_addr(argv[1]);

    // binding pc 1 socket 

    int bindPC1 = bind(socketPC1, (struct sockaddr*)&pc1Address,sizeof(pc1Address) );
    CHECKERROR(bindPC1, -1, "Error binding socket PC1 !!!\n");

    // Create Socket for PC2 (TRAIN 3 ET TRAIN 4)

    int socketPC2 = socket(AF_INET,SOCK_STREAM,0);
    CHECKERROR(socketPC2, -1, "Creation socket PC2 fail !!!\n");

    sockaddr_in pc2Address;
    pc2Address.sin_family = AF_INET;
    pc2Address.sin_port = htons(atoi(argv[2]) + 1);
    pc2Address.sin_addr.s_addr = inet_addr(argv[1]);

    int bindPC2 = bind(socketPC2, (struct sockaddr*)&pc2Address,sizeof(pc2Address) );
    CHECKERROR(bindPC2, -1, "Error binding socket PC2 !!!\n");


    //while(1);

    // create threads, one for each PC
    
    std::thread threadPC1(watchTrain,socketPC1);
    std::thread threadPC2(watchTrain,socketPC2);

    threadPC1.join();
    std::cout << "Thread 1 finished" << std::endl; 

    threadPC2.join();
    std::cout << "Thread 2 finished" << std::endl; 


    close(socketPC1);
    close(socketPC2);
    

    return 0;
    
}
