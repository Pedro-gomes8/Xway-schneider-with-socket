#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <tuple>
#include <thread>
#include <mutex>


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
fprintf(stderr, "Usage: %s [local IP] [remote IP=localhost] [remote port = %d]\n", name,
        502);
}
int sd1;



int main(int argc, char *argv[]){
    if ((argc > 1 && strncmp(argv[1], "help", 4) == 0) || argc < 3)
    {
      help_command(argv[0]);
      exit(EXIT_SUCCESS);
    }
  
    int nbcar;
    struct sockaddr_in addrServ, addrCli;
  
    unsigned int adrlg = sizeof(struct sockaddr_in);
  
    // Etape 1 - Socket creation
  
    sd1 = socket(AF_INET, SOCK_STREAM, 0);
  
    CHECKERROR(sd1, -1, "Creation fail !!!\n");
  
    // Etape 2 - Socket address
  
    addrCli.sin_family = AF_INET;
    addrCli.sin_port = 0; // On laisse l'OS affecter le port au client
  
    addrCli.sin_addr.s_addr = inet_addr(argv[1]); // local IP
  
    // Etape 3 - Affectation de l'adresse a la socket avec controle d'erreur
  
    // Etape 4 - Adressage du destinataire
  
    addrServ.sin_family = AF_INET;
  
    if (argc > 3)
    {
      addrServ.sin_addr.s_addr = inet_addr(argv[2]); // should be the IP of the automate 10.31.125.14/24
    }
    else
    {
      addrServ.sin_addr.s_addr = inet_addr(argv[1]);
    }
  
    if (argc > 4)
    {
      addrServ.sin_port = htons(atoi(argv[3])); // should be the port 502
    }
    else
    {
      addrServ.sin_port = htons(502);
    }
  
    char buff[BUFSIZE + 1];

    CHECKERROR(connect(sd1, (const struct sockaddr *)&addrServ, sizeof(struct sockaddr_in)), -1,
               "Erreur de connect !!!\n");
  

  
    // 37 adresse xway
  
    // change last number depending on the train to test
    // 39 -> train 1
    // 42 -> train 2
    // 49 -> train 3
    // 52 -> train 4
    Tram tram(37,16,52);

    // train 1
    // in ints {{20, 0}, {3, 1}, {22, 0}, {14, 0}, {23, 1}, {33, 0}, {10, 1}, {7, 0}, {10, 0}, {29, 1}, {19, 1}}
    std::vector<std::tuple<unsigned char, int>> path1 = {{0x14,0},{0x03,1},{0x16,0},{0x0E,0},{0x17,1},{0x21,0},{0x0A,1},{0x03,0},{0x1D,1},{0x13,1}};
    //Train train1(&tram,path1);

    // train 2
    // in ints {{7, 0}, {4, 1}, {22, 1}, {27, 1}, {13, 0}, {28, 1}, {12, 0}, {9, 1}, {20, 0}, {24, 1}}
    std::vector<std::tuple<unsigned char, int>> path2 = {{0x07,0},{0x04,1},{0x16,1},{0x1B,1},{0x0D,0},{0x1C,1},{0x0C,0},{0x09,1},{0x14,0},{0x18,1}};
    //Train train1(&tram,path2);

    // train 3
    // in ints {{0, 0}, {0, 1}, {13, 1}, {22, 0}, {20, 1}, {14, 0}, {30, 1}, {13, 0}, {9, 1}, {31, 1}, {21, 0}, {26, 1}, {1, 0}, {15, 1}, {12, 1}}
    std::vector<std::tuple<unsigned char, int>> path3 = {{0x00,0},{0x00,1},{0x0D,1},{0x16,0},{0x14,1},{0x0E,0},{0x1E,1},{0x0D,0},{0x09,1},{0x1F,1},{0x15,0},{0x1A,1},{0x01,0},{0x0F,1},{0x0C,1}};
    //Train train1(&tram,path3);

    // train 4
    // TODO: Fix the path, problem when inversing the troncon in the Ti07 at the end of the path (or the beginning when restarting the path)
    // in ints {{10, 0}, {7, 1}, {33, 0}, {29, 1}, {13, 0}, {49, 1}, {9, 1}, {28, 1}, {23, 0}, {27, 1}, {47, 1}}
    std::vector<std::tuple<unsigned char, int>> path4 = {{0x0A,0},{0x07,1},{0x21,0},{0x1D,1},{0x0D,0},{0x31,1},{0x09,1},{0x1C,1},{0x17,0},{0x1B,1},{0x25,1},{0x2F,1},};
    Train train1(&tram, path4);

    train1.followPath();

    int isTramVar = 0;
    do
    {
  
    
      write(sd1, tram.tramVar, sizeof(tram.tramVar));
      usleep(300 * 1000);
      isTramVar = 1;
  
      printf("Tram sent\n");
      read(sd1, tram.tramReceived, sizeof(tram.tramReceived));
      printf("Response received\n");
      printf("Response: ");
      for (int i = 0; i < sizeof(tram.tramReceived); i++)
      {
        printf("%02X ", tram.tramReceived[i]);
      }
      printf("\n");
  
      if (isTramVar)
      {
          read(sd1, tram.tramReceived, sizeof(tram.tramReceived));
          printf("Response received\n");
        printf("Response: ");
        for (int i = 0; i < sizeof(tram.tramReceived); i++)
        {
          printf("%02X ", tram.tramReceived[i]);
        }
        tram.ack[13] = tram.tramReceived[13];
  
        isTramVar = 0;
      }
      write(sd1, tram.ack, sizeof(tram.ack));
      train1.followPath();
  
    } while (strcmp(buff, "FIN") && strcmp(buff, "fin"));
  
    close(sd1);
  
    return EXIT_SUCCESS;
  }
  