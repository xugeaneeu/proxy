#include "logger.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

void log_error(const char* msg) {
  int errsv = errno;
  fprintf(stderr, "%s: %s\n", msg, strerror(errsv));
}