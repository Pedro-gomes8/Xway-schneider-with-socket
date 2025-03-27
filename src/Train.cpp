#include "../include/Train.h"
#include "../include/Tram.h"
#include "../include/SocketHandler.hpp"
#include <iostream>
#include <tuple>
#include <nlohmann/json.hpp>
#include <cstring>

Train::Train(int _trainId, int _xwayAddr, int _port, std::vector<std::tuple<unsigned char, int>> _path, SocketHandler *sock): pathStep(0), trainId(_trainId), xwayAddr(_xwayAddr), port(_port), path(_path),tram(_xwayAddr, _port, _trainId), resourceManager(sock){


}


void Train::handleResource(unsigned char activate, int istrocon){
    char managerResponse[256];
    int toRequest = 0;
    nlohmann::json message;
    message["train_id"] = this->trainId;
    message["action"] = "lock";
    switch(this->trainId){
        case 39:
            switch(activate){
                case 0x14:
                    this->resourceTaken = "R5";
                    message["resource"] = "R5";
                    toRequest = 1;
                    break;
                case 0x16:
                    this->resourceTaken = "R4";
                    message["resource"] = "R4";
                    toRequest = 1;
                    break;
                case 0x21:
                    this->resourceTaken = "R2,R3";
                    message["resource"] = "R2,R3";
                    toRequest = 1;
                    break;

                /*
                case 0x03:
                    if (istrocon) break;
                    this->resourceTaken = "R2";
                    message["resource"] = "R2";
                    toRequest = 1;
                    break;
                */
                default:
                    break;
            }
            break;
        case 42:
            switch(activate){
                case 0x07:
                    this->resourceTaken = "R1,R2,R3";
                    message["resource"] = "R1,R2,R3";
                    toRequest = 1;
                    break;
                case 0x0D:
                    this->resourceTaken = "R1,R2,R3,R4";
                    message["resource"] = "R4";
                    toRequest = 1;
                    break;
                    /*
                case 0x0C:
                    this->resourceTaken = "R4";
                    message["resource"] = "R4";
                    toRequest = 1;
                    break;
                    */
                
                case 0x14:
                    this->resourceTaken = "R5";
                    message["resource"] = "R5";
                    toRequest = 1;
                    break;
                default:
                    break;
            }
            break;
        case 49:
            switch(activate){
                case 0x0E:
                    this->resourceTaken = "R3,R4";
                    message["resource"] = "R3,R4";
                    toRequest = 1;
                    break;
                default:
                    break;
            }
            break;
        case 52:
            switch(activate){
                case 0x0A:
                    this->resourceTaken = "R1,R2,R3";
                    message["resource"] = "R1,R2,R3";
                    toRequest = 1;
                    break;
                case 0x21:
                    this->resourceTaken = "R1,R2,R3";
                    message["resource"] = "R1,R2,R3";
                    toRequest = 1;
                    break;
                default:
                    break;
            }
            break;
    }
    if (toRequest){
        std::string jsonStr = message.dump();
        std::cout << "Sending info to resource manager\n";
        std::cout << jsonStr.c_str() << std::endl;
        this->resourceManager->sendData(jsonStr.c_str(), jsonStr.size());
        this->resourceManager->receiveData(managerResponse, sizeof(managerResponse));
        std::cout << "Received data from manager" << managerResponse << "\n";

        toRequest = 0;

        // ACK . 1 . 0
        if (strcmp(managerResponse, "0") == 0){
            std::cout << "Error\n";
    }
}
}

void Train::releaseResource(unsigned char activate){
    char managerResponse[256];
    int toRequest = 0;
    nlohmann::json message;
    message["train_id"] = this->trainId;
    message["action"] = "unlock";

    switch(this->trainId){
        case 39:
            switch(activate){
                case 0x16:
                    toRequest = 1;
                    break;
                case 0x21:
                    toRequest = 1;
                    break;
                /*
                case 0x03:
                    toRequest = 1;
                    break;
                */
                case 0x13:
                    toRequest = 1;
                    break;
                default:
                    break;
            }
            break;
        case 42:
            switch(activate){
                case 0x04:
                    toRequest = 1;
                    break;
                case 0x14:
                    toRequest = 1;
                    break;
                    /*
                case 0x0C:
                    toRequest = 1;
                    break;]
                    */
                
                default:
                    break;
            }
            break;
        case 49:
            switch(activate){
                case 0x1F:
                    toRequest = 1;
                    break;
                default:
                    break;
            }
            break;
        case 52:
            switch(activate){
                /*case 0x21:
                    toRequest = 1;
                    break;
                    */
                case 0x25:
                    toRequest = 1;
                    break;
                case 0x1C:
                    toRequest = 1;
                    break;
                default:
                    break;
            }
            break;
    }
    if (toRequest && !(this->resourceTaken.empty())){
        std::cout << "Sending release\n";

        message["resource"] = this->resourceTaken;
        toRequest = 0;
        this->resourceTaken = "";
        if (activate == 0x1C){
            message["resource"] = "R3";
            this->resourceTaken = "R1,R2";
            toRequest = 1;
        }
        std::string jsonStr = message.dump();
        std::cout << jsonStr.c_str() << std::endl;

        this->resourceManager->sendData(jsonStr.c_str(), jsonStr.size());
        std::cout << "s" << std::endl;

        this->resourceManager->receiveData(managerResponse, sizeof(managerResponse));

        // ACK . 1 . 0
        if (strcmp(managerResponse, "0") == 0){
            std::cout << "Error\n";

    }
    std::cout << "Released\n";

    }
}

void Train::followPath(){
    unsigned char activate = std::get<0>(this->path[this->pathStep]);
    int isTrocon = std::get<1>(this->path[this->pathStep]);
    printf("Activate : %02x\n",activate );
    //
    releaseResource(activate);
    handleResource(activate,isTrocon);
    //
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