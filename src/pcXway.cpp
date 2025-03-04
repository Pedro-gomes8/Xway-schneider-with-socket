#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../include/mapping.h"
#include "../include/Tram.h"

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
    // CHECKERROR(bind(sd1, (const struct sockaddr *)&addrCli, sizeof(addrCli)), -1,
    //            "Erreur de bind !!!\n");
  
    // CHECKERROR(bind(sd1, (const struct sockaddr *)&addrCli, sizeof(addrCli)), -1,
    //            "Erreur de bind !!!\n");
  
    CHECKERROR(connect(sd1, (const struct sockaddr *)&addrServ, sizeof(struct sockaddr_in)), -1,
               "Erreur de connect !!!\n");
  
    // build tram with hexa values bit by bit
    // my station 30 -> 0x1E in the red 1 -> 0x10 (because is the 4 first bits), then his station 14 -> 0x0E in the red 1 -> 0x10
    // so it gives the bit 0x1E 10 0E 10 after the 0xF1
    
  
    // 37 adresse xway
  
    Tram tram(37,10);
    // char tramVar[] = {0x00, 0x00, 0x00, 0x01, 0x00, 0x15, 0x00, 0xF1, 0x25, 0x10, 0x0E, 0x10,  0x37, 0x10,0x06, 0x68, 0x07, 0x34, 0x00, 0x03, 0x00, 0x25, 0x00, 0x07, 0x00, 0xFF, 0xFF};
  
    // char tramResponse[] = {0x00, 0x00, 0x00, 0x01, 0x00, 0x14, 0x00, 0xF1, 0x25, 0x10, 0x0E, 0x10, 0x37, 0x06, 0x68, 0x07, 0x34, 0x00, 0x03, 0x00, 0x25, 0x00, 0x12, 0x00, 0x13, 0x00};
   
    // APPEND THIS BELOW TO UNDERSTAND THE TRAM
    // 00 00 00 01 00 -> Fixed
    // [xx] -> Longeur du octet de requete.
    // 00 f1 -> 5way address 
    // 25 -> Xway address my machine 
    // 10 -> reseau e porte
    // 0E 10-> Fixed
    // [09] XX -> [09] adresse de tache XX: porte ATTENTION. 09 doit etre superieur a F
    // 
  
    // char ackNowledge[] = {0x00, 0x00, 0x00, 0x01, 0x00, 0x09, 0x00, 0xF1, 0x25, 0x10, 0x0E, 0x10, 0x19, 0x18, 0xFE};
  
    int isTramVar = 0;
    do
    {
  
      printf(GREEN "CLIENT '%d'>" NOCOLOR, getpid());
      // write(sd1, send, sizeof(send));
  
      fgets(buff, BUFSIZE, stdin);
  
      if (strncmp(buff, "run", 3) == 0)
      {
        // tram[12] = 0x24;
        // write(sd1, tram, sizeof(tram));
      }
      else if (strncmp(buff, "stop", 4) == 0)
      {
        // tram[12] = 0x25;
        // write(sd1, tram, sizeof(tram));
      }
      else if (strncmp(buff, "data", 4) == 0)
      {
        // fgets(buff, BUFSIZE, stdin);
  
        write(sd1, tram.tramVar, sizeof(tram.tramVar));
        isTramVar = 1;
      }
      else
      {
        // write(sd1, tram, sizeof(tram));
      }
  
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
  
    } while (strcmp(buff, "FIN") && strcmp(buff, "fin"));
  
    close(sd1);
  
    return EXIT_SUCCESS;
  }
  