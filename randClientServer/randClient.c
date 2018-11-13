#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#include "common.h"

/* Prépare l'adresse du serveur */
void prepare_address( struct sockaddr_in *address, const char *host, int port ) {
  size_t addrSize = sizeof( address );
  memset(address, 0, addrSize);
  address->sin_family = AF_INET;
  inet_pton( AF_INET, (char*) host, &(address->sin_addr) );
  address->sin_port = htons(port);
}

/* Construit le socket client */
int makeSocket( const char *host, int port ) {
  struct sockaddr_in address;
  int sock = socket(PF_INET, SOCK_STREAM, 0);
  if( sock < 0 ) {
    die("Failed to create socket");
  }
  prepare_address( &address, host, port );
  if( connect(sock, (struct sockaddr *) &address, sizeof(address)) < 0) {
    die("Failed to connect with server");
  }
  return sock;
}

int main(int argc, char *argv[]) {
  int sock;    /* Socket */
  char *host;  /* Adresse IP du serveur */
  int port;    /* Port du service */
  size_t numBytes; /* Nombre de bytes aléatoires demandés */
  char *data; /* Buffer de reception */
  size_t rcvd=0; /* Bytes reçus */
  ssize_t n=0;

  /* Initialisation */


  if (argc != 4) {
    fprintf(stderr, "USAGE: %s <host> <port> <numBytes>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  host = argv[1];
  port = atoi(argv[2]);
  numBytes = atoi(argv[3]);

  /* Connection */

  sock = makeSocket( host, port );

  /* Envoi de la requête */

  if( write(sock,&numBytes,sizeof(numBytes)) < sizeof(numBytes) ) {
    die( "Cannot send the number of bytes requested." );
  }

  /* Reception de la réponse */

  data = (char*) malloc( numBytes );

  while( rcvd < numBytes ) {
    n = read( sock, data+rcvd, numBytes-rcvd );
    if( n < 0 ) {                  /* IMPORTANT !!!! CHECKER errno == EINTR EAGAIN ou EWOULBLOCK pour renvoyer CLIENT
                                        avec 3000 bytes : sendRandData: couldn't write the data to the client.: Bad address
                                        avec 300 bytes : boucle infinie dans sendRandData je pense
                                        utilise strace pour débugger*/
      die( "Cannot receive data" );
    }
    rcvd += n;
    printf( "Received %ld bytes.\n", n );
  }

  printf( "\nReceived: " );
  for( int i=0; i < numBytes; i++ ) {
    printf("%x", data[i] & 0xff );
  }
  printf("\n");


  /* Libération des resources */
  free( data );
  close(sock);

  exit(EXIT_SUCCESS);
}
