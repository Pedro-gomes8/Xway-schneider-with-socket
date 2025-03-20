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
#include <semaphore>

//#include "../include/mapping.h"
//#include "../include/tram.h"

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
fprintf(stderr, "Usage: %s [local IP] [port]\n", name);
}

int treatSemaphores(std::vector<std::binary_semaphore*>& semaphores, std::string action) {
    if (action == "Lock" || action == "lock") {
        // Bloqueia (espera) até conseguir adquirir o semáforo
        for (auto sem : semaphores) {
            sem->acquire();  // Essa chamada bloqueia até que o semáforo esteja disponível
        }
        return 0;
    }
    else if (action == "Unlock" || action == "unlock") {
        for (auto sem : semaphores) {
            sem->release();
        }
        return 0;
    }
    else {
        // Ação "nothing" ou inválida
        return 1;
    }
}



std::mutex parole, R1, R2, R3, R4, R5;
std::binary_semaphore semR1(1),semR2(1),semR3(1),semR4(1),semR5(1); // Inicializa com 1, funcionando como um mutex




void watchTrain(int serverSocket) {
    const char *ack = nullptr;

    // Coloca o socket em modo de escuta
    CHECKERROR(listen(serverSocket, 1), -1, "Listen Socket failed \n");

    char buffer[1024];
    ssize_t n;

    struct sockaddr_in clientAddress;
    socklen_t clientLen = sizeof(clientAddress);
    int clientSocket;

    while (true) {
        std::cout << "Try to accept:" << std::endl;
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientLen);
        CHECKERROR(clientSocket, -1, "Accept failed \n");
        std::cout << "Accept done" << std::endl;

        std::cout << "Try to read:" << std::endl;
        n = read(clientSocket, buffer, sizeof(buffer) - 1);
        CHECKERROR(n, -1, "Read failed \n");
        std::cout << "Read done:" << std::endl;

        buffer[n] = '\0';

        try {
            // Converte a string recebida para um objeto JSON
            json receivedMsg = json::parse(buffer);
            int trainId = receivedMsg["train_id"];
            std::string resource = receivedMsg["resource"];
            std::string action   = receivedMsg["action"];

            if(trainId == 0){
                std::cout << "Break " << std::endl;
                break;
            }

            std::cout << "Thread " << std::this_thread::get_id()
                      << " - Trem " << trainId
                      << " solicitou o recurso: " << resource
                      << " ação: " << action << std::endl;

            
            std::vector<std::binary_semaphore*> selectedSemaphores;
            
            if (resource == "R1") {
                selectedSemaphores.push_back(&semR1);
            } else if (resource == "R2") {
                selectedSemaphores.push_back(&semR2);
            } else if (resource == "R3") {
                selectedSemaphores.push_back(&semR3);
            } else if (resource == "R4") {
                selectedSemaphores.push_back(&semR4);
            } else if (resource == "R5") {
                selectedSemaphores.push_back(&semR5);
            } else {
                std::cerr << "Resource not found: " << resource << std::endl;
            }
            
            int result = treatSemaphores(selectedSemaphores, action);
            if (result == 0) {
                ack = "Resource processed successfully";
            } else {
                ack = "Failed to process resource";
            }
        }
        catch (json::exception &e) {
            std::cerr << "Erro ao parsear JSON: " << e.what() << std::endl;
            exit(1);
        }

        // Envia o ACK para o cliente
        if (send(clientSocket, ack, strlen(ack), 0) < 0) {
            perror("Send ACK failed");
        }
        close(clientSocket);
        std::cout << "Client socket closed." << std::endl;
    }
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


    // Create Socket for PC1 (TRAIN 1 ET TRAIN 2)

    int socketPC3 = socket(AF_INET,SOCK_STREAM,0);

    CHECKERROR(socketPC3, -1, "Creation socket PC1 fail !!!\n");

    sockaddr_in pc3Address;
    pc3Address.sin_family = AF_INET;
    pc3Address.sin_port = htons(atoi(argv[2]) + 2);
    pc3Address.sin_addr.s_addr = inet_addr(argv[1]);

    // binding pc 1 socket 

    int bindPC3 = bind(socketPC3, (struct sockaddr*)&pc3Address,sizeof(pc3Address) );
    CHECKERROR(bindPC3, -1, "Error binding socket PC3 !!!\n");

    // Create Socket for PC2 (TRAIN 3 ET TRAIN 4)

    int socketPC4 = socket(AF_INET,SOCK_STREAM,0);
    CHECKERROR(socketPC4, -1, "Creation socket PC4 fail !!!\n");

    sockaddr_in pc4Address;
    pc4Address.sin_family = AF_INET;
    pc4Address.sin_port = htons(atoi(argv[2]) + 3);
    pc4Address.sin_addr.s_addr = inet_addr(argv[1]);

    int bindPC4 = bind(socketPC4, (struct sockaddr*)&pc4Address,sizeof(pc4Address) );
    CHECKERROR(bindPC4, -1, "Error binding socket PC4 !!!\n");



    std::thread threadPC3(watchTrain,socketPC3);
    std::thread threadPC4(watchTrain,socketPC4);

    threadPC1.join();
    std::cout << "Thread 1 finished" << std::endl; 

    threadPC2.join();
    std::cout << "Thread 2 finished" << std::endl; 

    threadPC3.join();
    std::cout << "Thread 3 finished" << std::endl; 

    threadPC4.join();
    std::cout << "Thread 4 finished" << std::endl; 



    close(socketPC1);
    close(socketPC2);
    close(socketPC3);
    close(socketPC4);
    

    return 0;
    
}
