#pragma once

#include "../include/SocketHandler.hpp"
#include <deque>
#include <mutex>

enum class MessageType{
    AUTOMATE,
    RESOURCE
};

class Sender{
    std::deque<std::vector<unsigned char>> *commandQueue;
    std::deque<const char *> *resourceQueue;
    std::mutex &mutexQueue;
    SocketHandler *sock;
    MessageType type;
    public:
    Sender(std::deque<std::vector<unsigned char>> *queue, std::mutex &_mutexQueue,  SocketHandler * sockt,MessageType socketType);
    Sender(std::deque<const char *> *queue, std::mutex &_mutexQueue, SocketHandler *sockt ,MessageType socketType);
    void start();
};