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

#include <nlohmann/json.hpp>
using json = nlohmann::json;

using namespace std;

#define CHECKERROR(var, val, msg) \
  if (var == val) {               \
    perror(msg);                  \
    exit(1);                      \
  }

void help_command(char name[]) {
    fprintf(stderr, "Usage: %s [local IP] [port]\n", name);
}

// Declaração global dos semáforos para cada recurso
std::binary_semaphore semR1(1), semR2(1), semR3(1), semR4(1), semR5(1);

// Mapa para rastrear o trem que possui o lock de cada recurso
std::mutex ownersMutex;
std::map<std::string, int> resourceOwners = {
    {"R1", 0},
    {"R2", 0},
    {"R3", 0},
    {"R4", 0},
    {"R5", 0}
};
void debugMenu() {
    while (true) {
        cout << "\n--- Menu de Debug ---" << endl;
        cout << "Type 'status' to view ressources states." << endl;
        cout << "Type 'exit' to quit." << endl;
        cout << "Option: ";
        
        string command;
        getline(cin, command);
        
        if (command == "status") {
            lock_guard<mutex> lock(ownersMutex);
            cout << "\nEstado atual dos recursos:" << endl;
            for (const auto &entry : resourceOwners) {
                cout << "Recurso " << entry.first << " -> ";
                if (entry.second == 0) {
                    cout << "Disponível" << endl;
                } else {
                    cout << "Ocupado pelo trem " << entry.second << endl;
                }
            }
        } else if (command == "exit") {
            cout << "Saindo do debug menu..." << endl;
            break;
        } else {
            cout << "Comando não reconhecido." << endl;
        }
    }
}

void watchTrain(int serverSocket) {

    // listen the serverSocket

    CHECKERROR(listen(serverSocket, 1), -1, "Listen Socket failed \n");

    // create useful variables

    const char *ack = nullptr;
    char buffer[1024];
    ssize_t n;

    //create clientAddress and client Socket

    struct sockaddr_in clientAddress;
    socklen_t clientLen = sizeof(clientAddress);
    int clientSocket;



    while (true) {
        //================ 1) Try to accept connexion 

        //cout << "Try to accept:" << endl;
        
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientLen);
        CHECKERROR(clientSocket, -1, "Accept failed \n");
        
        //cout << "Accept done" << endl;

        //================= 2) Read msg from client

        //cout << "Try to read:" << endl;
        
        n = read(clientSocket, buffer, sizeof(buffer) - 1);
        CHECKERROR(n, -1, "Read failed \n");
        
        //cout << "Read done:" << endl;

        buffer[n] = '\0';

        //================= 3) Convert msg to json object and do logic ressource treatment


        try {
            json receivedMsg = json::parse(buffer);
            int trainId = receivedMsg["train_id"];
            std::string resource = receivedMsg["resource"];
            std::string action   = receivedMsg["action"];

            // ================ Condition to break the loop (only for tests)
            if (trainId == 0) {
                cout << "Break" << endl;
                break;
            }

            // =============== PRINT DEBUG
            /*
            cout << "Thread " << std::this_thread::get_id()
                 << " - Train " << trainId
                 << " askef for ressource: " << resource
                 << " action: " << action << endl;
            */
            
            // =============== 3.1) Ressource Logic BEGIN

            if (action == "Lock" || action == "lock") {
                bool isOwner = false;
                {
                    std::lock_guard<std::mutex> lock(ownersMutex);
                    // Only unlock if the train who sends unlock is the owner
                    if (resourceOwners[resource] == trainId) {
                        isOwner = true;
                    }
                }
                if(isOwner == true){
                    ack = "Train has already the ressource";
                }else{
                    // Try to lock the ressource indicated in "ressource"
                    if (resource == "R1") {
                        semR1.acquire();
                    } else if (resource == "R2") {
                        semR2.acquire();
                    } else if (resource == "R3") {
                        semR3.acquire();
                    } else if (resource == "R4") {
                        semR4.acquire();
                    } else if (resource == "R5") {
                        semR5.acquire();
                    } else {
                        cerr << "Resource not found: " << resource << endl;
                    }
                    // After lock complet, save the train as ressource owner
              
                    {
                        std::lock_guard<std::mutex> lock(ownersMutex);
                        resourceOwners[resource] = trainId;
                    }
                    ack = "Resource locked successfully";
                }              
            }
            
            else if (action == "Unlock" || action == "unlock") {
                bool isOwner = false;
                
                // Check if its train has the semaphore he wants to unlock
                {
                    std::lock_guard<std::mutex> lock(ownersMutex);
                    // Only unlock if the train who sends unlock is the owner
                    if (resourceOwners[resource] == trainId) {
                        isOwner = true;
                        resourceOwners[resource] = 0; // clear owner in map
                    }
                }

                if (isOwner) {
                    if (resource == "R1") {
                        semR1.release();
                    } else if (resource == "R2") {
                        semR2.release();
                    } else if (resource == "R3") {
                        semR3.release();
                    } else if (resource == "R4") {
                        semR4.release();
                    } else if (resource == "R5") {
                        semR5.release();
                    } else {
                        cerr << "Resource not found: " << resource << endl;
                    }
                    ack = "Resource unlocked successfully";
                } else {
                    ack = "Unlock ignored: train is not owner of resource";
                }
            }
            else {
                ack = "No action taken";
            }
            // =============== Ressource Logic END ====================

        }
        catch (json::exception &e) {
            cerr << "Erro ao parsear JSON: " << e.what() << endl;
            exit(1);
        }

        // SEND THE ACK TO CLIENT
        CHECKERROR(send(clientSocket, ack, strlen(ack), 0), -1, "Send ACK failed \n");

        close(clientSocket);
        //cout << "Client socket closed." << endl;
    }
}

int main(int argc, char *argv[]) {
    if ((argc > 1 && strncmp(argv[1], "help", 4) == 0) || argc < 3) {
        help_command(argv[0]);
        exit(EXIT_SUCCESS);
    }

    sockaddr_in pc1Address;
    pc1Address.sin_family = AF_INET;
    pc1Address.sin_port = htons(atoi(argv[2]));
    pc1Address.sin_addr.s_addr = inet_addr(argv[1]);

    sockaddr_in pc2Address;
    pc2Address.sin_family = AF_INET;
    pc2Address.sin_port = htons(atoi(argv[2]) + 1);
    pc2Address.sin_addr.s_addr = inet_addr(argv[1]);

    sockaddr_in pc3Address;
    pc3Address.sin_family = AF_INET;
    pc3Address.sin_port = htons(atoi(argv[2]) + 2);
    pc3Address.sin_addr.s_addr = inet_addr(argv[1]);

    sockaddr_in pc4Address;
    pc4Address.sin_family = AF_INET;
    pc4Address.sin_port = htons(atoi(argv[2]) + 3);
    pc4Address.sin_addr.s_addr = inet_addr(argv[1]);


    // Cria e configura sockets para PC1 a PC4
    int socketPC1 = socket(AF_INET, SOCK_STREAM, 0);
    CHECKERROR(socketPC1, -1, "Creation socket PC1 fail !!!\n");
    int bindPC1 = bind(socketPC1, (struct sockaddr*)&pc1Address, sizeof(pc1Address));
    CHECKERROR(bindPC1, -1, "Error binding socket PC1 !!!\n");

    int socketPC2 = socket(AF_INET, SOCK_STREAM, 0);
    CHECKERROR(socketPC2, -1, "Creation socket PC2 fail !!!\n");
    int bindPC2 = bind(socketPC2, (struct sockaddr*)&pc2Address, sizeof(pc2Address));
    CHECKERROR(bindPC2, -1, "Error binding socket PC2 !!!\n");

    int socketPC3 = socket(AF_INET, SOCK_STREAM, 0);
    CHECKERROR(socketPC3, -1, "Creation socket PC3 fail !!!\n");
    int bindPC3 = bind(socketPC3, (struct sockaddr*)&pc3Address, sizeof(pc3Address));
    CHECKERROR(bindPC3, -1, "Error binding socket PC3 !!!\n");

    int socketPC4 = socket(AF_INET, SOCK_STREAM, 0);
    CHECKERROR(socketPC4, -1, "Creation socket PC4 fail !!!\n");
    int bindPC4 = bind(socketPC4, (struct sockaddr*)&pc4Address, sizeof(pc4Address));
    CHECKERROR(bindPC4, -1, "Error binding socket PC4 !!!\n");

    // Cria threads para cada socket (simulando PCs responsáveis por diferentes trens)
    thread threadPC1(watchTrain, socketPC1);
    thread threadPC2(watchTrain, socketPC2);
    thread threadPC3(watchTrain, socketPC3);
    thread threadPC4(watchTrain, socketPC4);

    thread debugThread(debugMenu);

    // Aguarda a saída do debug menu
    debugThread.join();
    cout << "debug menu finished" << endl;

    threadPC1.join();
    cout << "Thread 1 finished" << endl;
    threadPC2.join();
    cout << "Thread 2 finished" << endl;
    threadPC3.join();
    cout << "Thread 3 finished" << endl;
    threadPC4.join();
    cout << "Thread 4 finished" << endl;
    

    close(socketPC1);
    close(socketPC2);
    close(socketPC3);
    close(socketPC4);

    return 0;
}
