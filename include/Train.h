#ifndef _TRAIN_H
#define _TRAIN_H

#include <vector>
#include <mutex>
#include "../include/Tram.h"
#include "../include/SocketHandler.hpp"




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
    
    Train(int _trainId, int _xwayAddr, int _port, std::vector<std::tuple<unsigned char, int>> _path, SocketHandler *sock);
    void followPath();
};


#endif