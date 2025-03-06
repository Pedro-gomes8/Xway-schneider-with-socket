#ifndef _TRAIN_H
#define _TRAIN_H

#include <vector>
#include "../include/Tram.h"


class Train {
    private:
    int pathStep;
    Tram *tram;

    public:
    std::vector<std::tuple<unsigned char, int>> path;
    
    Train(Tram *tram, std::vector<std::tuple<unsigned char, int>> path);
    void followPath();
};


#endif