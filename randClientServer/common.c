#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/random.h>
#include "common.h"

void die(char *issue) {
  perror(issue);
  exit(EXIT_FAILURE);
}

int sendRandData(int from, int to, int numBytesRequested) {
  static char buf[BUFF_SIZE];
  static int s = 0;
  static int flagg=0;
  // ssize_t nread;

  ssize_t numBytesWrittenToBuf = 0;
  ssize_t numBytesWrittenToClient = 0;

/*


*/

  if (flagg == 0) {
    numBytesWrittenToBuf = getrandom(buf, BUFF_SIZE, 0);
  if (numBytesWrittenToBuf < 0) {
    die("sendRandData: couldn't write random bytes to the buffer.");
  }
  flagg = 1;}

  // if( (numBytesRequested > (BUFF_SIZE-s)) || s == 0 ) {
  //   if(numBytesRequested > (BUFF_SIZE-s)) {
  //
  //   }
  //   //numBytesWrittenToBuf = getrandom(buf, BUFF_SIZE, 0);
  //   s = 0;
  //   // if(numBytesWrittenToBuf < 0) {
  //   //   die("sendRandData: couldn't write random bytes to the buffer.");
  //   // }
  // }

  // printf("Sending random data 1.\n");
  // numBytesWrittenToClient += write(to, buf, numBytesRequested-15);
  // printf("nb bytes written %ld\n", numBytesWrittenToClient);
  // if ( numBytesWrittenToClient < 0 ) {
  //   die("sendRandData: couldn't write the data to the client.");
  // }
  //
  // numBytesWrittenToClient += write(to, buf+15, numBytesRequested-15);
  // printf("nb bytes written %ld\n", numBytesWrittenToClient);
  // if ( numBytesWrittenToClient < 0 ) {
  //   die("sendRandData: couldn't write the data to the client.");
  // }

  while (numBytesWrittenToClient < numBytesRequested) {
    if ((numBytesRequested-numBytesWrittenToClient) < (BUFF_SIZE-s)) {
      printf("Sending random data 1.\n");
      numBytesWrittenToClient += write(to, buf+s, numBytesRequested-s);
      if ( numBytesWrittenToClient < 0 ) {
        die("sendRandData: couldn't write the data to the client.");
      }

      s += numBytesWrittenToClient;
    }
    else if ((numBytesRequested-numBytesWrittenToClient) > (BUFF_SIZE-s)) {
      printf("Sending random data.\n");
      numBytesWrittenToClient += write(to, buf+s, BUFF_SIZE-s);
      if ( numBytesWrittenToClient < 0 ) {
        die("sendRandData: couldn't write the data to the client.");
      }
      s = 0;

      numBytesWrittenToBuf = getrandom(buf, BUFF_SIZE, 0);
    if (numBytesWrittenToBuf < 0) {
      die("sendRandData: couldn't write random bytes to the buffer.");
    }
    }
  }


  return numBytesWrittenToClient;

  // while( nread = read(from, buf, sizeof buf), nread > 0 ) {
  //   char *out_ptr = buf;
  //   ssize_t nwritten;
  //   do {
  //     nwritten = write(to, out_ptr, nread);
  //     if (nwritten >= 0) {
  //       nread -= nwritten;
  //       out_ptr += nwritten;
  //     } else if (errno != EINTR) {
  //       return -1;
  //     }
  //
  //   } while (nread > 0);
  // }
  //
  // return nread;
}
