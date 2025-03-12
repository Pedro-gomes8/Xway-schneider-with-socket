#include "../include/Tram.h"
#include <cstring>
#include <vector>

Tram::Tram(int xWayAddress, int portNumber, int trainHex) {
    this->clientStation = xWayAddress;
    this->clientPortNumber = portNumber;
    this->trainHex = trainHex;
    unsigned char temp[] = {0x00, 0x00, 0x00 ,0x01,0x00, // Fixed
        0x16, // SIZE OF TRAM index [5]
        0x00, // FIXED
        0xF1, // 5 way extension
        0x25, // Mon Adresse Xway = 37 index [8]
        0x10, // Reseau et porte fixed to 01  
        0x0E, // Fixed
        0x10, // Fixed
        0x09, // 09 Task number
        0x10, // Num porte -> Customizable index[13]
        0x37, // Code Request -> Write Object index[14]
        // 0x10, // Why is it here?
        0x06, // Code categorie
        0x68, // Object Segment 68 : Espace données internes
        0x07, // Type du segment 7: word, 8: double word
        0x34, 0x00, // Instant du premier objet Ex: %MW10 (10 -> 0A 00) 52-> trem 4
        0x03, 0x00, // Nombre d'objet à ecrire (3 dans ce cas)
        0x25, 0x00, // Valeur a écrire dans %MW10 par example. 37. Me!
        0x07, 0x00, // Valeur a écrire dans %MW11. 7 -> ID of troncon
        0xFF, 0xFF // Valeur a écrire dans %MW12. FF FF  -> FF FF in the aguillage.
    };

    unsigned char ackTemp[] = {0x00, 0x00, 0x00 ,0x01,0x00,
    0x09,
    0x00,
    0xF1,
    0x25, 0x10,
    0x0E, 0x10,
    0x19, 0x00,
    0xFE};
    
    memcpy(this->tramVar,temp,sizeof(temp));
    memcpy(this->ack,ackTemp, sizeof(ackTemp));
    this->tramVar[8] = this->clientStation;
    this->tramVar[22] = this->clientStation;

    this->ack[8] = this->clientStation;

    this->tramVar[13] = this->clientPortNumber;
    this->ack[13] = this->clientPortNumber;

    this->tramVar[18] = this->trainHex;
    
    
    this->tramVarSize = sizeof(this->tramVar)/sizeof(unsigned char);
    }



