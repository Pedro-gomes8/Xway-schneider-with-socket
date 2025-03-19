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

std::mutex parole, R1, R2, R3, R4, R5;

void watchTrain(int serverSocket){

    const char *ack;
    //unique_lock<mutex> ParoleLock(parole);

    // std::cout << "ID thread: " << std::this_thread::get_id() << std::endl;
    
    // Here we set a queue with length 2 
    CHECKERROR(listen(serverSocket, 2),-1," Listen Socket failed \n");

    char buffer[1024];
    ssize_t n;

    struct sockaddr_in clientAddress;
    socklen_t clientLen = sizeof(clientAddress);
    int clientSocket;

    while(true){

        std::cout << "Try to accept:" << std::endl; 
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientLen);
        CHECKERROR(clientSocket,-1," Listen Socket failed \n");
        std::cout << "Accept done" << std::endl; 

        
        std::cout << "Try to read:" << std::endl; 
        n = read(clientSocket, buffer, sizeof(buffer) - 1);
        CHECKERROR(n,-1," Listen Socket failed \n");
        std::cout << "read done:" << std::endl; 
        
        buffer[n] = '\0';
        
        try {
            // Converte a string recebida para um objeto JSON
            json receivedMsg = json::parse(buffer);
            int trainId = receivedMsg["train_id"];
            std::string resource = receivedMsg["resource"];

            std::cout << "Thread " << std::this_thread::get_id()
                      << " - Trem " << trainId 
                      << " solicitou o recurso: " << resource << std::endl;

            // Se for mensagem de finalização, interrompe o loop
            if (resource == "fim") {
                std::cout << "Finalizando watchTrain" << std::endl;
                close(clientSocket);
                break;
            }
    
            // Seleciona o mutex correspondente com base na string resource
            std::mutex* resourceMutex = nullptr;
            if (resource == "R1") {
                resourceMutex = &R1;
            } else if (resource == "R2") {
                resourceMutex = &R2;
            } else if (resource == "R3") {
                resourceMutex = &R3;
            } else if (resource == "R4") {
                resourceMutex = &R4;
            } else if (resource == "R5") {
                resourceMutex = &R5;
            } else {
                std::cerr << "Recurso desconhecido: " << resource << std::endl;
            }
    
            if (resourceMutex != nullptr) {
                // Cria um lock no mutex correspondente. Esse lock será automaticamente liberado 
                // quando o objeto lock sair de escopo.
                std::cout << "Train " << trainId << " tenta travar mutex " << resource << std::endl;
                std::unique_lock<std::mutex> lock(*resourceMutex);
                std::cout << "Mutex " << resource << " travado para trem " << trainId << std::endl;

                ack = "1";

                CHECKERROR(send(clientSocket, ack, strlen(ack), 0),-1," send ACK failed \n");
                
                /*
                if (send(clientSocket, ack, strlen(ack), 0) < 0) {
                    perror("send ACK");
                }
                */

                // Aqui você pode inserir a lógica de processamento enquanto o recurso estiver travado.
                // Por exemplo: acessar o recurso, atualizar status, etc.
                // O lock será liberado automaticamente ao sair deste bloco.
            }

        }
        catch (json::exception& e) {
            std::cerr << "Erro ao parsear JSON: " << e.what() << std::endl;
            ack = "0";
            
        }

        close(clientSocket);
        std::cout << "Client socket closed: " << std::endl;
       // ParoleLock.unlock();

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
