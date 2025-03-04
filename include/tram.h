#ifndef _TRAM_H

#define _TRAM_H


#include <vector>

class Tram{
    public:
    Tram(int xWayAddress, int portNumber);
    int clientStation;
    int clientPortNumber;

    
    unsigned char tramVar[28];
    unsigned char tramOk[15];

    unsigned char tramReceived[24];
    unsigned char ack[15];
};


// unsigned char aguillagesMap[] = {
    
// };

// unsigned char sectionMap[] = {


// };


#endif // _TRAM_H