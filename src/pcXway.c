/****************************************************/
/* Application : clientUdp                            */
/* Date : 12/01/2023                                */
/* Version : 2                                      */
/* Compilation : gcc clientUdp.c -o clientUdpc          */
/* Usage : ./clientUdpc [adrIPserv] [portServ]   [adrIPcli] */
/****************************************************/
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
// #include <time.h>

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

void help_cl()
{
  fprintf(stdout, "Available operations:\n"
                  "add\n"
                  "report [location]\n"
                  "eoa\n"
                  "fin\n");
}

int sd1;

int main(int argc, char *argv[])
{
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
  char npdu[] = {0x00, 0x00, 0x00, 0x01, 0x00, 0x0A, 0x00, 0xF0, 0x1E, 0x10, 0x0E, 0x10};
  char apdu[] = {0x09, 0x1E, 0x24, 0x06};

  char tram[] = {0x00, 0x00, 0x00, 0x01, 0x00, 0x08, 0x00, 0xF0, 0x25, 0x10, 0x0E, 0x10, 0x24, 0x06};

  char received[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  // 37 adresse xway

  char tramVar[] = {0x00, 0x00, 0x00 ,0x01,0x00, // Fixed
                    0x17, // SIZE OF TRAM
                    0x00, // FIXED
                    0xF1, // 5 way extension
                    0x25, // Mon Adresse Xway = 37
                    0x10, // Reseau et porte fixed to 01
                    0x0E, // Fixed
                    0x10, // Fixed
                    0x09, // 17 Task number -> Customizable
                    0x10, // Num porte -> Customizable
                    0x37, 0x10,0x06, 0x68, 0x07, 0x34, 0x00, 0x03, 0x00, 0x25, 0x00, 0x07, 0x00, 0xFF, 0xFF // Request

                    };
  char anotherTram[] = {0x00, 0x00, 0x00 ,0x01,0x00, // Fixed
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
  // char tramVar[] = {0x00, 0x00, 0x00, 0x01, 0x00, 0x15, 0x00, 0xF1, 0x25, 0x10, 0x0E, 0x10,  0x37, 0x10,0x06, 0x68, 0x07, 0x34, 0x00, 0x03, 0x00, 0x25, 0x00, 0x07, 0x00, 0xFF, 0xFF};

  // unsigned char tramResponse[] = {0x00, 0x00, 0x00, 0x01, 0x00, 0x12, 0x00, 0xF1, 0x25, 0x10, 0x25, 0x10, 0x37, 0x06, 0x68, 0x07, 0x34, 0x00, 0x03, 0x00, 0x25, 0x00, 0x12, 0x00, 0x13, 0x00};
  unsigned char tramResponse[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
 
  // APPEND THIS BELOW TO UNDERSTAND THE TRAM
  // 00 00 00 01 00 -> Fixed
  // [xx] -> Longeur du octet de requete.
  // 00 f1 -> 5way address 
  // 25 -> Xway address my machine 
  // 10 -> reseau e porte
  // 0E 10-> Fixed
  // [09] XX -> [09] adresse de tache XX: porte ATTENTION. 09 doit etre superieur a F
  // 

  char ackNowledge[] = {0x00, 0x00, 0x00, 0x01, 0x00, 0x09, 0x00, 0xF1, 0x25, 0x10, 0x0E, 0x10, 0x19, 0x10, 0xFE};

  int trajectory = 0;
  int isTramVar = 0;
  int sizeTram = sizeof(anotherTram)/sizeof(char);
  do
  {

    switch (trajectory){
      case 0:
        // 7

        anotherTram[sizeTram-1] = 0x00;
        anotherTram[sizeTram-2] = 0x07;

        anotherTram[sizeTram-3] = 0xFF;
        anotherTram[sizeTram-4] = 0xFF;
        break;

      case 1:
        anotherTram[sizeTram-1] = 0xFF;
        anotherTram[sizeTram-2] = 0xFF;

        anotherTram[sizeTram-3] = 0x00;
        anotherTram[sizeTram-4] = 0x07;
        break;
      
      case 2:
        anotherTram[sizeTram-1] = 0x00;
        anotherTram[sizeTram-2] = 0x21;

        anotherTram[sizeTram-3] = 0xFF;
        anotherTram[sizeTram-4] = 0xFF;
      
        break;
      
      case 3:
        anotherTram[sizeTram-1] = 0xFF;
        anotherTram[sizeTram-2] = 0xFF;

        anotherTram[sizeTram-3] = 0x00;
        anotherTram[sizeTram-4] = 0x1d;
      break;

      case 4:
        anotherTram[sizeTram-1] = 0xFF;
        anotherTram[sizeTram-2] = 0xFF;

        anotherTram[sizeTram-3] = 0x00;
        anotherTram[sizeTram-4] = 0x31;
        break;
      case 5:
        anotherTram[sizeTram-1] = 0xFF;
        anotherTram[sizeTram-2] = 0xFF;

        anotherTram[sizeTram-3] = 0x00;
        anotherTram[sizeTram-4] = 0x09;
        break;

    }
    // printf(GREEN "CLIENT '%d'>" NOCOLOR, getpid());
    // // write(sd1, send, sizeof(send));

    // fgets(buff, BUFSIZE, stdin);

    // if (strncmp(buff, "run", 3) == 0)
    // {
    //   tram[12] = 0x24;
    //   write(sd1, tram, sizeof(tram));
    // }
    // else if (strncmp(buff, "stop", 4) == 0)
    // {
    //   tram[12] = 0x25;
    //   write(sd1, tram, sizeof(tram));
    // }
    // else if (strncmp(buff, "data", 4) == 0)
    // {
    //   // fgets(buff, BUFSIZE, stdin);

    //   write(sd1, anotherTram, sizeof(anotherTram));
    //   isTramVar = 1;
    // }
    // else
    // {
    write(sd1, anotherTram, sizeof(anotherTram));
    isTramVar = 1;
    usleep(300 * 1000);
    // }

    printf("Tram sent\n");
    read(sd1, received, sizeof(received));
    printf("Response received\n");
    printf("Response: ");
    for (int i = 0; i < sizeof(received); i++)
    {
      printf("%02X ", received[i]);
    }
    printf("\n");

    if (isTramVar)
    {
      printf("Waiting capteur\n");
      read(sd1, tramResponse, sizeof(tramResponse));
      printf("Response received Nice\n");
      printf("Response: ");
      for (int i = 0; i < sizeof(tramResponse); i++)
      {
        printf("%02X ", tramResponse[i]);
      }
      ackNowledge[13] = tramResponse[13];

      isTramVar = 0;
    }

    write(sd1, ackNowledge, sizeof(ackNowledge));
    trajectory++;
  } while (trajectory < 6);

  close(sd1);

  return EXIT_SUCCESS;
}
