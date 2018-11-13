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


/* Helper functions of sendRandData() */
ssize_t fillBufWithRandomBytes(char *buf) {
  ssize_t numBytesWrittenToBuf;

  numBytesWrittenToBuf = getrandom(buf, BUFF_SIZE, 0);
  if (numBytesWrittenToBuf < 0) {
    die("fillBufWithRandomBytes: couldn't write random bytes to the buffer.");
  }
  return numBytesWrittenToBuf;
}


/* END -- Helper functions of sendRandData() */

int sendRandData(int to, int numBytesRequested) {
  char buf[BUFF_SIZE];
  int s = 0;
  ssize_t numBytesWrittenToBuf = 0;
  ssize_t numBytesWrittenToClient = 0;

  numBytesWrittenToBuf = fillBufWithRandomBytes(buf);

/* Pourquoi buffer parfois > 1024 ?? */

  while (numBytesWrittenToClient < numBytesRequested) {
    if ((numBytesRequested-numBytesWrittenToClient) < (BUFF_SIZE-s)) {
      printf("Sending random data 1.\n");
      numBytesWrittenToClient += write(to, buf+s, numBytesRequested-s);
      if ( numBytesWrittenToClient < 0 ) {
        die("sendRandData: couldn't write the data to the client. aaa");
      }

      s += numBytesWrittenToClient;
    }
    else if ((numBytesRequested-numBytesWrittenToClient) > (BUFF_SIZE-s)) {
      printf("Sending random data.\n");
      numBytesWrittenToClient += write(to, buf+s, BUFF_SIZE-s); /* HERE ?? E_INTR? E_WOULDBLOCK ??*/
      if ( numBytesWrittenToClient < 0 ) {
        die("sendRandData: couldn't write the data to the client. bbb ");
      }
      s = 0;

      numBytesWrittenToBuf = getrandom(buf, BUFF_SIZE, 0);
    if (numBytesWrittenToBuf < 0) {
      die("sendRandData: couldn't write random bytes to the buffer. cccc");
    }
    }
  }

  return numBytesWrittenToClient;
}

int getClientNumBytes( int clientSock ) {
  int numBytes;

  if( read( clientSock, &numBytes, sizeof(numBytes)) < sizeof(numBytes) ) {
    perror("getClientNumBytes: couldn't get the number of bytes requested by the client.");
  }

  return numBytes;
}

void handleClient( int clientSock ) {
  int numBytesRequested;

  numBytesRequested = getClientNumBytes(clientSock);
  sendRandData( clientSock, numBytesRequested ); // enlever premier argument (inutile ?)

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
