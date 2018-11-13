#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/random.h>
#include "common.h"

#define MAX_PENDING 256


/* Helper functions to sendRandData() [see below]. */

ssize_t fillBufWithRandomBytes(char *buf) {
  /* fillBufWithRandomBytes initializes a buffer with random bytes. */

  ssize_t numBytesWrittenToBuf = 0;

  numBytesWrittenToBuf = getrandom(buf, BUFF_SIZE, 0); // suitable for cryptographic purposes

  if (numBytesWrittenToBuf < 0) {
    die("fillBufWithRandomBytes: couldn't write random bytes to the buffer.");
  }
  return numBytesWrittenToBuf;
}

ssize_t writeBytesToClient(int to, char *buf, ssize_t numBytesToSend) {
  /* writeBytesToClient writes numBytesToSend bytes to the client. */

  ssize_t numBytesJustSent = 0;
  numBytesJustSent = write(to, buf, numBytesToSend);
  printf("\t%ld bytes sent.\n", numBytesJustSent);

  if ( numBytesJustSent < 0 ) {
    die("sendRandData: couldn't write the data to the client. aaa");
  }

  return numBytesJustSent;
}
/* END -- Helper functions to sendRandData() */


ssize_t sendRandData(int to, ssize_t numBytesRequested) {
  /* sendRandData sends the client the number of bytes requested. */

  char buf[BUFF_SIZE];
  ssize_t numBytesLeftToSend = numBytesRequested;
  ssize_t numBytesWrittenToClient = 0;
  ssize_t numBytesJustWritten = 0;

  fillBufWithRandomBytes(buf); // initialize the buffer with random bytes

  printf("Sending random bytes.\n");

  while (numBytesLeftToSend > 0) {
    if (numBytesLeftToSend > BUFF_SIZE) { // if you need more than a buffer of bytes
      numBytesJustWritten = writeBytesToClient(to, buf, BUFF_SIZE); // send a whole one
      fillBufWithRandomBytes(buf); // and reinitialize it
    }
    else { // otherwise only send what you need from the buffer
      numBytesJustWritten = writeBytesToClient(to, buf, numBytesLeftToSend);
    }
    numBytesWrittenToClient += numBytesJustWritten;
    numBytesLeftToSend -= numBytesJustWritten;
  }

  return numBytesWrittenToClient;
}

int getClientNumBytes( int clientSock ) {
  /* getClientNumBytes gets the number of bytes from the client. */

  int numBytes;

  if( read( clientSock, &numBytes, sizeof(numBytes)) < sizeof(numBytes) ) {
    perror("getClientNumBytes: couldn't get the number of bytes requested by the client.");
  }

  return numBytes;
}

void handleClient( int clientSock ) {
  /* handleClient gets from the client the number of bytes
      it requested and then sends it the (same number of) random bytes.*/

  ssize_t numBytesRequested;

  numBytesRequested = getClientNumBytes(clientSock);
  sendRandData( clientSock, numBytesRequested );

  close( clientSock );
}


void run( int serverSock ) {
  while( 1 ) {
    struct sockaddr_in clientAddress;
    unsigned int clientLength = sizeof(clientAddress);
    int clientSock;
    printf( "Waiting for incoming connections\n");
    clientSock =
      accept(serverSock, (struct sockaddr *) &clientAddress, &clientLength );
    if( clientSock < 0 ) {
      die("Failed to accept client connection");
    }
    printf( "Client connected: %s\n", inet_ntoa(clientAddress.sin_addr));
    handleClient(clientSock);
  }
}

void prepare_address( struct sockaddr_in *address,  int port ) {
  size_t addrSize = sizeof( address );
  memset(address, 0, addrSize);
  address->sin_family = AF_INET;
  address->sin_addr.s_addr = htonl(INADDR_ANY);
  address->sin_port = htons(port);
}

int makeSocket( int port ) {
  struct sockaddr_in address;
  int sock = socket(PF_INET, SOCK_STREAM, 0);
  if( sock < 0 ) {
    die("Failed to create socket");
  }
  prepare_address( &address, port );
  if( bind( sock,
	    (struct sockaddr *) &address,
	    sizeof(address)
	    ) < 0  )
    {
      die("Failed to bind the server socket");
    }
  if (listen(sock, MAX_PENDING) < 0) {
    die("Failed to listen on server socket");
  }
  return sock;
}

int main( int argc, char **argv ) {
  int servSock;
  int port;

  if (argc != 2) {
    fprintf(stderr, "USAGE: %s <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  port = atoi(argv[1]);

  servSock = makeSocket( port );

  printf( "Server running on port %d\n", port);

  run(servSock);

  close(servSock);

  return EXIT_SUCCESS;
}
