#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "common.h"

void die(char *issue) {
  perror(issue);
  exit(EXIT_FAILURE);
}
