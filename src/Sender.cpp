#include "../include/Sender.hpp"

#include <deque>
#include <mutex>
#include <unistd.h>


Sender::Sender(std::deque<std::vector<unsigned char>> *queue, std::mutex &_mutexQueue, SocketHandler *sockt, MessageType socketType) : commandQueue(queue), resourceQueue(nullptr), mutexQueue(_mutexQueue), sock(sockt), type(socketType){

};

Sender::Sender(std::deque<const char *> *queue, std::mutex &_mutexQueue,SocketHandler *sockt,MessageType socketType) : commandQueue(nullptr), resourceQueue(queue), mutexQueue(_mutexQueue), sock(sockt), type(socketType){
};


void Sender::start(){
    switch(type){
        case MessageType::AUTOMATE:

            while (true){
                mutexQueue.lock();
                if (commandQueue->empty()){
                    mutexQueue.unlock();
                    usleep(300 * 1000);
                    continue;
                }
                std::vector<unsigned char> nextCommand = commandQueue->front();
                commandQueue->pop_front();
                mutexQueue.unlock();

                this->sock->sendData(nextCommand.data(),sizeof(nextCommand.data()));
            }
            break;
        case MessageType::RESOURCE:
            while (true){
                mutexQueue.lock();
                if (resourceQueue->empty()){
                    mutexQueue.unlock();
                    usleep(300 * 1000);
                    continue;
                }
                const char * nextCommand = resourceQueue->front();
                resourceQueue->pop_front();
                mutexQueue.unlock();

                this->sock->sendData(nextCommand,sizeof(nextCommand));
            }
            break;
    }

}



