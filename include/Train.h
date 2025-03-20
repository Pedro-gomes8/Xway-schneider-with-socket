#ifndef _TRAIN_H
#define _TRAIN_H

#include <vector>
#include <mutex>
#include "Tram.h"
#include "SocketHandler.hpp"




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
    std::string resourceTaken;
    
    Train(int _trainId, int _xwayAddr, int _port, std::vector<std::tuple<unsigned char, int>> _path, SocketHandler *sock);

    // Updates tram to send to automate.
    // Handles resources and sends request to resource manager.
    void followPath();


    void handleResource(unsigned char activate);
    void releaseResource(unsigned char activate);
};


#endif