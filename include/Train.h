#ifndef _TRAIN_H
#define _TRAIN_H

#include <vector>
#include <mutex>
#include "Tram.h"
#include "SocketHandler.hpp"
#include "../include/Sender.hpp"
#include "../include/ResponseRegistry.hpp"



class Train {
    private:
    int pathStep;
    int trainId;
    int xwayAddr;
    int port;


    public:
    std::vector<std::tuple<unsigned char, int>> path;
    Tram tram;
    SocketHandler *resourceManager;
    Sender *sender;
    std::deque<std::vector<unsigned char>> *commandQueue;
    std::mutex &mutexQueue;
    std::string resourceTaken;

    
    Train(int _trainId, int _xwayAddr, int _port, std::vector<std::tuple<unsigned char, int>> _path, SocketHandler *sock, Sender *send, std::deque<std::vector<unsigned char>> *queue, std::mutex &_mutexQueue);

    ~Train();
    // Updates tram to send to automate.
    // Handles resources and sends request to resource manager.
    void followPath();


    void handleResource(unsigned char activate, int istrocon);
    void releaseResource(unsigned char activate);
};


#endif