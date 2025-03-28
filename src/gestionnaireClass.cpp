#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <csignal>

#include <chrono>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <algorithm>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

using namespace std;

#define CHECKERROR(var, val, msg) \
  if (var == val) {               \
    perror(msg);                  \
    exit(1);                      \
  }

// Função auxiliar para dividir uma string com base em um delimitador.
vector<string> split(const string& str, char delimiter) {
    vector<string> result;
    stringstream ss(str);
    string token;
    while(getline(ss, token, delimiter)) {
        result.push_back(token);
    }
    return result;
}

// Classe que gerencia o bloqueio e desbloqueio dos recursos.
class ResourceManager {
public:
    ResourceManager() {
        // Inicializa os estados dos recursos: 0 significa "disponível"
        resourceOwners["R1"] = 0;
        resourceOwners["R2"] = 0;
        resourceOwners["R3"] = 0;
        resourceOwners["R4"] = 0;
        resourceOwners["R5"] = 0;
        // Cria os mutexes para cada recurso
        resourceMutexes["R1"] = new mutex();
        resourceMutexes["R2"] = new mutex();
        resourceMutexes["R3"] = new mutex();
        resourceMutexes["R4"] = new mutex();
        resourceMutexes["R5"] = new mutex();
    }
    
    ~ResourceManager() {
        for (auto& p : resourceMutexes) {
            delete p.second;
        }
    }
    
    // Tenta bloquear os recursos indicados para o trem (trainId).
    // Retorna true se o bloqueio for bem-sucedido ou se o trem já for o dono.
    bool lockResources(const vector<string>& resources, int trainId) {
        // Verifica se o trem já possui todos os recursos
        bool alreadyOwner = true;
        {
            lock_guard<mutex> lock(ownersMutex);
            for (auto& res : resources) {
                if (resourceOwners[res] != trainId) {
                    alreadyOwner = false;
                    break;
                }
            }
        }
        if (alreadyOwner)
            return true;
        
        // Para evitar deadlock, ordenamos os recursos (todos os threads devem usar a mesma ordem)
        vector<string> sortedResources = resources;
        sort(sortedResources.begin(), sortedResources.end());
        // Bloqueia cada recurso sequencialmente
        for (auto& res : sortedResources) {
            resourceMutexes[res]->lock();
        }
        // Atualiza os donos dos recursos
        {
            lock_guard<mutex> lock(ownersMutex);
            for (auto& res : sortedResources) {
                resourceOwners[res] = trainId;
            }
        }
        return true;
    }
    
    // Desbloqueia os recursos para o trem (trainId).
    // Retorna true se o trem for o dono de todos os recursos.
    bool unlockResources(const vector<string>& resources, int trainId) {
        bool isOwner = true;
        {
            lock_guard<mutex> lock(ownersMutex);
            for (auto& res : resources) {
                if (resourceOwners[res] != trainId) {
                    isOwner = false;
                    break;
                }
            }
        }
        if (!isOwner)
            return false;
        
        // Ordena os recursos para desbloquear na mesma ordem
        vector<string> sortedResources = resources;
        sort(sortedResources.begin(), sortedResources.end());
        for (auto& res : sortedResources) {
            resourceMutexes[res]->unlock();
        }
        {
            lock_guard<mutex> lock(ownersMutex);
            for (auto& res : sortedResources) {
                resourceOwners[res] = 0;
            }
        }
        return true;
    }
    
    // Exibe o status atual dos recursos.
    void printStatus() {
        lock_guard<mutex> lock(ownersMutex);
        for (auto& entry : resourceOwners) {
            cout << "Resource " << entry.first << " -> ";
            if (entry.second == 0)
                cout << "Available" << endl;
            else
                cout << "Busy by train " << entry.second << endl;
        }
        cout << "===================" << endl;
    }
    
private:
    mutex ownersMutex;
    map<string, int> resourceOwners;
    map<string, mutex*> resourceMutexes;
};

// Ponteiro global para ResourceManager, para uso no tratador de sinal.
ResourceManager* gResourceManager = nullptr;

// Função de tratamento de sinal para imprimir o status dos recursos.
void signalHandler(int signal_num) {
    if (gResourceManager != nullptr) {
        gResourceManager->printStatus();
    }
}

// Classe que gerencia as conexões de um servidor (um socket) e processa as mensagens.
class ResourceServer {
public:
    ResourceServer(int serverSocket, ResourceManager& manager) 
        : serverSocket(serverSocket), manager(manager) {}
    
    // Função que fica em loop aceitando conexões, lendo mensagens, processando e enviando respostas.
    void run() {
        CHECKERROR(listen(serverSocket, 5), -1, "Listen Socket failed \n");
        struct sockaddr_in clientAddress;
        socklen_t clientLen = sizeof(clientAddress);
        
        while (true) {
            cout << "Thread " << this_thread::get_id() << " waiting to accept connection..." << endl;
            int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientLen);
            if (clientSocket < 0) {
                perror("Accept failed");
                continue;
            }
            cout << "Thread " << this_thread::get_id() << " connection accepted." << endl;
            
            char buffer[1024];
            ssize_t n = read(clientSocket, buffer, sizeof(buffer) - 1);
            if(n <= 0) {
                cout << "Thread " << this_thread::get_id() << " client disconnected." << endl;
                close(clientSocket);
                continue;
            }
            buffer[n] = '\0';
            try {
                json receivedMsg = json::parse(buffer);
                int trainId = receivedMsg["train_id"];
                string resource = receivedMsg["resource"];
                string action = receivedMsg["action"];
                
                // Se o trainId for 0, usamos isso como condição de término para testes.
                if (trainId == 0) {
                    cout << "Break command received. Closing connection." << endl;
                    close(clientSocket);
                    break;
                }
                
                // Aqui você pode incluir a lógica de mapeamento do trainId (por exemplo, via switch-case)
                int trainId_debug = trainId; // Para fins de exemplo, usamos o próprio trainId.
                
                cout << "Thread " << this_thread::get_id() 
                     << " - Train " << trainId_debug
                     << " requested resource: " << resource
                     << " action: " << action << endl;
                
                vector<string> resourceList = split(resource, ',');
                const char* ack = nullptr;
                if (action == "Lock" || action == "lock") {
                    bool success = manager.lockResources(resourceList, trainId_debug);
                    ack = success ? "1" : "0";
                } else if (action == "Unlock" || action == "unlock") {
                    bool success = manager.unlockResources(resourceList, trainId_debug);
                    ack = success ? "1" : "0";
                } else {
                    ack = "0";
                }
                
                CHECKERROR(send(clientSocket, ack, strlen(ack), 0), -1, "Send ACK failed \n");
            }
            catch (json::exception &e) {
                cerr << "JSON parse error: " << e.what() << endl;
                close(clientSocket);
                continue;
            }
            close(clientSocket);
            cout << "Thread " << this_thread::get_id() << " closed client connection." << endl;
        }
    }
    
private:
    int serverSocket;
    ResourceManager& manager;
};

void printHelp(char* name) {
    fprintf(stderr, "Usage: %s [local IP] [port]\n", name);
}

int main(int argc, char* argv[]) {
    if (argc < 3 || (argc > 1 && strncmp(argv[1], "help", 4) == 0)) {
        printHelp(argv[0]);
        exit(EXIT_SUCCESS);
    }
    
    // Configura o endereço para duas portas (ex.: dois trens)
    sockaddr_in addr1, addr2;
    addr1.sin_family = AF_INET;
    addr1.sin_port = htons(atoi(argv[2]));
    addr1.sin_addr.s_addr = inet_addr(argv[1]);
    
    addr2.sin_family = AF_INET;
    addr2.sin_port = htons(atoi(argv[2]) + 1);
    addr2.sin_addr.s_addr = inet_addr(argv[1]);
    
    int socket1 = socket(AF_INET, SOCK_STREAM, 0);
    CHECKERROR(socket1, -1, "Creation socket1 failed\n");
    int bind1 = bind(socket1, (struct sockaddr*)&addr1, sizeof(addr1));
    CHECKERROR(bind1, -1, "Bind socket1 failed\n");
    
    int socket2 = socket(AF_INET, SOCK_STREAM, 0);
    CHECKERROR(socket2, -1, "Creation socket2 failed\n");
    int bind2 = bind(socket2, (struct sockaddr*)&addr2, sizeof(addr2));
    CHECKERROR(bind2, -1, "Bind socket2 failed\n");
    
    // Cria o gerenciador de recursos e configura o ponteiro global para debug
    ResourceManager manager;
    gResourceManager = &manager;
    
    // Configura o tratamento de sinal para imprimir o status dos recursos
    struct sigaction sigAct;
    sigAct.sa_handler = signalHandler;
    sigemptyset(&sigAct.sa_mask);
    sigAct.sa_flags = SA_RESTART;
    CHECKERROR(sigaction(SIGUSR1, &sigAct, NULL), -1, "Sigaction failed\n");
    
    // Cria objetos ResourceServer para cada socket
    ResourceServer server1(socket1, manager);
    ResourceServer server2(socket2, manager);
    
    // Cria threads para executar os servidores
    thread t1(&ResourceServer::run, &server1);
    thread t2(&ResourceServer::run, &server2);
    
    t1.join();
    t2.join();
    
    close(socket1);
    close(socket2);
    
    return 0;
}
