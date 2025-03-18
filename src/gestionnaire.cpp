#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <thread>

//#include "../include/mapping.h"
//#include "../include/Tram.h"

using namespace std;

std::mutex R1, R2, R3, R4, R5;

void watchTrain(){
    
    // increment number by 1 for 1000000 times
    for(int i=0; i<1000000; i++){
        number++;
    }
}

int main(nt argc, char *argv[]){

    thread pc1(watchTrain);

    thread pc2(watchTrain);


    return 0;
    
}
