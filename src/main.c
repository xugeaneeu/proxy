#include "config.h"
#include "proxy.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>


int main(void) {
  ProxyConfig cfg;
  ProxyServer serv;

  cfg.port = PORT;
  cfg.cache = NULL;
  cfg.cacheless = 0;

  if (InitProxy(&serv, &cfg)) {
    fprintf(stderr, "error when creating server");
    return EXIT_FAILURE;
  }

  if (InitServerAndServe(&serv)) {
    Shutdown(&serv);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
