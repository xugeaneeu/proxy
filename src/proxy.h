#pragma once

#include "cache.h"
#include "config.h"
#include "net.h"
#include "threadpool.h"

#include <semaphore.h>
#include <stdatomic.h>
#include <stdint.h>


/*------------types-------------*/

typedef struct ProxyServer {
  LRU_Cache_t* cache;
  NetListener* listener;
  ThreadPool   pool;
} ProxyServer;

typedef struct ProxyConfig {
  char         cacheless;
  LRU_Cache_t* cache;

  uint16_t port;
} ProxyConfig;


/*-------------API--------------*/

int         InitProxy(ProxyServer* serv, ProxyConfig* cfg);
int         InitServerAndServe(ProxyServer* serv);
int         Shutdown(ProxyServer* serv);
ProxyConfig ProxyConfigDefault(void);