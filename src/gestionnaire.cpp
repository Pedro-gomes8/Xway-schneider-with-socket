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
#include <csignal> 

#include <nlohmann/json.hpp>
using json = nlohmann::json;

using namespace std;

#define CHECKERROR(var, val, msg) \
  if (var == val) {               \
    perror(msg);                  \
    exit(1);                      \
  }

std::mutex R1,R2,R3,R4,R5; 

std::mutex ownersMutex;
std::map<std::string, int> resourceOwners = {
    {"R1", 0},
    {"R2", 0},
    {"R3", 0},
    {"R4", 0},
    {"R5", 0}
};

vector<string> split(const string& str, char delimiter){
    vector<string> result;
    stringstream ss(str);
    string token;

    while(getline(ss,token,delimiter)){
        result.push_back(token);
    }

    return result;


}

void help_command(char name[]) {
    fprintf(stderr, "Usage: %s [local IP] [port]\n", name);
}

void printMenu(int signal_num) {

    for (const auto &entry : resourceOwners) {
        cout << "Ressource " << entry.first << " -> ";
        if (entry.second == 0) {
            cout << "Available" << endl;
        } else {
            cout << "Busy by train " << entry.second << endl;
        }
    }
    cout << "=================== " << endl;

    
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

    //================ 1) Try to accept connexion 

    cout << "Thread: " << std::this_thread::get_id() << " Try to accept:" << endl;
            
    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientLen);
    CHECKERROR(clientSocket, -1, "Accept failed \n");
    
    cout << "Thread: " << std::this_thread::get_id()  << " Accept done" << endl;

    while (true) {
       
       
        //================= 2) Read msg from client

        //cout << "Thread: " << std::this_thread::get_id() << " try to read " << endl;
        n = read(clientSocket, buffer, sizeof(buffer) - 1);
        //cout << "Thread: " << std::this_thread::get_id() << "serverSocket: " << serverSocket << " clientSocket: " << clientSocket << endl;
        //cout << "Thread: " << std::this_thread::get_id() << " n: " << n << endl;

        CHECKERROR(n, -1, "Read failed \n");
        
        //cout << "Thread: " << std::this_thread::get_id() << " read done: " << buffer << endl;

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
            
            int trainId_debug;

            switch (trainId)
            {
            case 39:
                /* code */
                trainId_debug = 1;
                break;
            case 42:
                /* code */
                trainId_debug = 2;
                break;
            case 49:
                /* code */
                trainId_debug = 3;
                break;
            case 52:
                /* code */
                trainId_debug = 4;
                break;
            
            default:
                break;
            }


            cout << "Thread " << std::this_thread::get_id()
                 << " - Train " << trainId_debug
                 << " askef for ressource: " << resource
                 << " action: " << action << endl;
            
            // Treat ressource message
            vector<string> resource_treated;
            char delimiter = ',';
            resource_treated = split(resource,delimiter);

            // =============== 3.1) Ressource Logic BEGIN

            if (action == "Lock" || action == "lock") {
                bool isOwner = false;
                {
                    std::lock_guard<std::mutex> lock(ownersMutex);
                    for (const auto ressource_splited : resource_treated){
                        // Only unlock if the train who sends lock is the owner
                        if (resourceOwners[ressource_splited] == trainId_debug) {
                            isOwner = true;
                        }
                        else{
                            isOwner = false;
                            break;
                        }
                    }
                
                    raise(SIGUSR1);

                }
                if(isOwner == true){
                    //ack = "Train has already all the ressources";
                    ack = "1";

                }else{
                    // Try to lock the ressource indicated in "ressource"
                    if (resource == "R1") {
                        R1.lock();
                    } else if (resource == "R2") {
                        R2.lock();

                    } else if (resource == "R3") {
                        R3.lock();

                    } else if (resource == "R4") {
                        R4.lock();

                    } else if (resource == "R5") {
                        R5.lock();

                    } else if (resource == "R1,R2") {
                        std::lock(R1,R2);

                    } else if (resource == "R2,R3") {
                        std::lock(R2,R3);

                    } else if (resource == "R3,R4") {
                        std::lock(R3,R4);

                    } else if (resource == "R1,R2,R3") {
                        std::lock(R1,R2,R3);

                    } else if (resource == "R1,R2,R3,R4") {
                        std::lock(R1,R2,R3,R4);

                    }  
                    else {
                        cerr << "Resource not found: " << resource << endl;
                    }
                    // After lock complet, save the train as ressource owner
              
                    {
                        std::lock_guard<std::mutex> lock(ownersMutex);

                        for (const auto ressource_splited : resource_treated){
                            resourceOwners[ressource_splited] = trainId_debug;
                        }

                        raise(SIGUSR1);

                    }
                    //ack = "Resource locked successfully";
                    ack = "1";

                }              
            }
            
            else if (action == "Unlock" || action == "unlock") {
                bool isOwner = false;
                
                // Check if its train has the semaphore he wants to unlock
                {
                    std::lock_guard<std::mutex> lock(ownersMutex);
                    // Only unlock if the train who sends unlock is the owner

                    for (const auto ressource_splited : resource_treated){
                        // Only unlock if the train who sends lock is the owner
                        if (resourceOwners[ressource_splited] == trainId_debug) {
                            isOwner = true;
                            resourceOwners[ressource_splited] = 0;
                        }
                        else{
                            isOwner = false;
                            break;
                        }
                    }
                    raise(SIGUSR1);

                }

                if (isOwner) {
                    if (resource == "R1") {
                        R1.unlock();
                    } else if (resource == "R2") {
                        R2.unlock();

                    } else if (resource == "R3") {
                        R3.unlock();

                    } else if (resource == "R4") {
                        R4.unlock();

                    } else if (resource == "R5") {
                        R5.unlock();

                    }
                    else if (resource == "R1,R2") {
                        R1.unlock();
                        R2.unlock();

                    }else if (resource == "R2,R3") {
                        R2.unlock();
                        R3.unlock();

                    }
                    else if (resource == "R3,R4") {
                        R3.unlock();
                        R4.unlock();

                    }
                    else if (resource == "R1,R2,R3") {
                        R1.unlock();
                        R2.unlock();
                        R3.unlock();


                    }else if (resource == "R1,R2,R3,R4") {
                        R1.unlock();
                        R2.unlock();
                        R3.unlock();
                        R4.unlock();


                    } else {
                        cerr << "Resource not found: " << resource << endl;
                    }
                    //ack = "Resource unlocked successfully";
                    ack = "1";

                } else {
                    //ack = "Unlock ignored: train is not owner of resource";
                    ack = "0";

                }
            }
            else {
                //ack = "No action taken";
                ack = "0";

            }
            // =============== Ressource Logic END ====================


        }
        catch (json::exception &e) {
            cerr << "Erro ao parsear JSON: " << e.what() << endl;
            exit(1);
        }

        // SEND THE ACK TO CLIENT
        CHECKERROR(send(clientSocket, ack, strlen(ack), 0), -1, "Send ACK failed \n");
 
    }
    close(clientSocket);
    cout << "Client socket closed." << endl;
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


    // Cria e configura sockets para PC1 a PC4
    int socketPC1 = socket(AF_INET, SOCK_STREAM, 0);
    CHECKERROR(socketPC1, -1, "Creation socket PC1 fail !!!\n");
    int bindPC1 = bind(socketPC1, (struct sockaddr*)&pc1Address, sizeof(pc1Address));
    CHECKERROR(bindPC1, -1, "Error binding socket PC1 !!!\n");

    int socketPC2 = socket(AF_INET, SOCK_STREAM, 0);
    CHECKERROR(socketPC2, -1, "Creation socket PC2 fail !!!\n");
    int bindPC2 = bind(socketPC2, (struct sockaddr*)&pc2Address, sizeof(pc2Address));
    CHECKERROR(bindPC2, -1, "Error binding socket PC2 !!!\n");

    // =================== Signal treatment setup

    struct sigaction sigbreak;
    sigbreak.sa_handler = &printMenu;
    sigemptyset(&sigbreak.sa_mask);
    sigbreak.sa_flags = SA_RESTART;

    CHECKERROR(sigaction(SIGUSR1,&sigbreak,NULL),-1,"Sigaction problem");


    // =================== Create train threads
    thread threadPC1(watchTrain, socketPC1);
    thread threadPC2(watchTrain, socketPC2);
    // ======================== Wait for trains threads

    threadPC1.join();
    cout << "Thread 1 finished" << endl;
    threadPC2.join();
    cout << "Thread 2 finished" << endl;

    close(socketPC1);
    close(socketPC2);
    
    return 0;
}
