#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <tuple>

#include "../include/mapping.h"
#include "../include/Tram.h"
#include "../include/Train.h"

#define GREEN "\033[1;32m"
#define NOCOLOR "\033[1;0m"

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
  
    Tram tram(37,10);

    std::vector<std::tuple<unsigned char, int>> path1 = {{0x07,0},{0x07,1},{0x21,0},{0x1D,1},{0x31,1},{0x09,1}};

    Train train1(&tram,path1);

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
  