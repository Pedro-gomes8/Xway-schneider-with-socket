#ifndef _TRAIN_H
#define _TRAIN_H

#include <vector>
#include <mutex>
#include "../include/Tram.h"



class Train {
    private:
    int pathStep;
    int trainId;
    int xwayAddr;
    int port;


    public:
    std::vector<std::tuple<unsigned char, int>> path;
    Tram tram;
    
    Train(int _trainId, int _xwayAddr, int _port, std::vector<std::tuple<unsigned char, int>> _path);
    void followPath();
};


#endif