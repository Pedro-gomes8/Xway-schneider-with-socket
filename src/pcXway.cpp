#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <tuple>
#include <iostream>
#include "../include/mapping.h"
#include "../include/Tram.h"
#include "../include/Train.h"
#include "../include/SocketHandler.hpp"


#define BUFSIZE 256

#define CHECKERROR(var, val, msg) \
  if (var == val)                 \
  {                               \
    perror(msg);                  \
    exit(1);                      \
  }
void help_command(char name[])
{
fprintf(stderr, "Usage: %s [PC_IP Automate_IP ResourceManager_IP ResourceManager_Port TRAIN_ID XwayAddr Port\n", name);
}



int main(int argc, char *argv[]){
    if ((argc > 1 && strncmp(argv[1], "help", 4) == 0) || argc < 3)
    {
      help_command(argv[0]);
      exit(EXIT_SUCCESS);
    }
    // Example bin/main PC_IP Automate_IP ResourceManager_IP ResourceManager_Port TRAIN_ID XwayAddr Port
    SocketHandler sockAutomate(argv[1], argv[2], 502);


    SocketHandler resourceManager(argv[1], argv[3], atoi(argv[4]));
    int nbcar;
  
    std::cout << "Connecting socket automate\n";
    sockAutomate.connectSocket();

    std::cout << "Connecting socket resource manager\n";
    resourceManager.connectSocket();

    char buff[BUFSIZE + 1];

    std::vector<std::tuple<unsigned char, int>> path = {{0x0A,0},{0x07,1},{0x21,0},{0x1D,1},{0x0D,0},{0x31,1},{0x09,1},{0x1C,1},{0x17,0},{0x1B,1},{0x25,1},{0x2F,1}};

  
    // 37 adresse xway
  

    // Tram tram(37,16,52);

    // train 1
    // in ints {{20, 0}, {3, 1}, {22, 0}, {14, 0}, {23, 1}, {33, 0}, {10, 1}, {7, 0}, {10, 0}, {29, 1}, {19, 1}}
    std::vector<std::tuple<unsigned char, int>> path1 = {{0x14,0},{0x03,1},{0x16,0},{0x0E,0},{0x17,1},{0x21,0},{0x0A,1},{0x03,0},{0x1D,1},{0x13,1}};
    //Train train1(&tram,path1);

    // train 2
    // in ints {{7, 0}, {4, 1}, {22, 1}, {27, 1}, {13, 0}, {28, 1}, {12, 0}, {9, 1}, {20, 0}, {24, 1}}
    std::vector<std::tuple<unsigned char, int>> path2 = {{0x04,1},{0x07,0},{0x16,1},{0x1B,1},{0x0D,0},{0x1C,1},{0x0C,0},{0x09,1},{0x14,0},{0x18,1}};
    //Train train1(&tram,path2);

    // train 3
    // in ints {{0, 0}, {0, 1}, {13, 1}, {22, 0}, {20, 1}, {14, 0}, {30, 1}, {13, 0}, {9, 1}, {31, 1}, {21, 0}, {26, 1}, {1, 0}, {15, 1}, {12, 1}}
    std::vector<std::tuple<unsigned char, int>> path3 = {{0x00,0},{0x00,1},{0x0D,1},{0x16,0},{0x14,1},{0x0E,0},{0x1E,1},{0x0D,0},{0x09,1},{0x1F,1},{0x15,0},{0x1A,1},{0x01,0},{0x0F,1},{0x0C,1}};
    //Train train1(&tram,path3);

    // train 4
    // TODO: Fix the path, problem when inversing the troncon in the Ti07 at the end of the path (or the beginning when restarting the path)
    // in ints {{10, 0}, {7, 1}, {33, 0}, {29, 1}, {13, 0}, {49, 1}, {9, 1}, {28, 1}, {23, 0}, {27, 1}, {47, 1}}
    std::vector<std::tuple<unsigned char, int>> path4 = {{0x0A,0},{0x07,1},{0x21,0},{0x1D,1},{0x0D,0},{0x31,1},{0x09,1},{0x1C,1},{0x17,0},{0x1B,1},{0x25,1},{0x2F,1}};
        // change last number depending on the train to test
    // 39 -> train 1
    // 42 -> train 2
    // 49 -> train 3
    // 52 -> train 4

    int trainId = atoi(argv[5]);

    switch (trainId)
    {
      case 39:
        path = path1;
        break;
      case 42:
        path = path2;
        break;
      case 49:
        path = path3;
        break;
      case 52:
        path = path4;
        break;
      default:
        break;
    }

    int xwayAddr = atoi(argv[6]);
    int port = atoi(argv[7]);


    Train train(trainId, xwayAddr, port, path, &resourceManager);

    train.followPath();

    int isTramVar = 0;
    do
    {
  
      sockAutomate.sendData(train.tram.tramVar, sizeof(train.tram.tramVar));
      usleep(300 * 1000);
      isTramVar = 1;
  
      printf("Tram sent\n");
      sockAutomate.receiveData(train.tram.tramReceived, sizeof(train.tram.tramReceived));
      printf("Response received\n");
      printf("Response: ");
      for (int i = 0; i < sizeof(train.tram.tramReceived); i++)
      {
        printf("%02X ", train.tram.tramReceived[i]);
      }
      printf("\n");
  
      if (isTramVar)
      {
        sockAutomate.receiveData(train.tram.tramReceived, sizeof(train.tram.tramReceived));
          printf("Response received\n");
        printf("Response: ");
        for (int i = 0; i < sizeof(train.tram.tramReceived); i++)
        {
          printf("%02X ", train.tram.tramReceived[i]);
        }
        train.tram.ack[13] = train.tram.tramReceived[13];
  
        isTramVar = 0;
      }
      sockAutomate.sendData(train.tram.ack, sizeof(train.tram.ack));
      train.followPath();
  
    } while (strcmp(buff, "FIN") && strcmp(buff, "fin"));
    return EXIT_SUCCESS;
  }
  