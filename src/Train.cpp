#include "../include/Train.h"
#include "../include/Tram.h"
#include "../include/SocketHandler.hpp"
#include <iostream>
#include <tuple>

Train::Train(int _trainId, int _xwayAddr, int _port, std::vector<std::tuple<unsigned char, int>> _path, SocketHandler *sock): pathStep(0), trainId(_trainId), xwayAddr(_xwayAddr), port(_port), path(_path),tram(_xwayAddr, _port, _trainId), resourceManager(sock){


}




void Train::followPath(){
    unsigned char activate = std::get<0>(this->path[this->pathStep]);
    int isTrocon = std::get<1>(this->path[this->pathStep]);
    printf("Activate : %02x\n",activate );
    std::cout << "istrocon: "<<  isTrocon << "\n";
    
    size_t tramsize = this->tram.tramVarSize;
    // std::cout << "path step " << this->pathStep << "\n";
    if (isTrocon){
        this->tram.tramVar[tramsize - 1] = 0xFF;
        this->tram.tramVar[tramsize - 2] = 0xFF;

        this->tram.tramVar[tramsize - 3] = 0x00;
        this->tram.tramVar[tramsize - 4] = activate;
    }else {
        this->tram.tramVar[tramsize - 1] = 0x00;
        this->tram.tramVar[tramsize - 2] = activate;

        this->tram.tramVar[tramsize - 3] = 0xFF;
        this->tram.tramVar[tramsize - 4] = 0xFF;
    }
    
    this->pathStep++;
    if (this->pathStep >= this->path.size()){
        this->pathStep = 0;
    }
    // std::cout << "Path Step" << this->pathStep << "\n";


}