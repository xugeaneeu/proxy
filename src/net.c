#include "net.h"
#include "logger.h"

#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

struct NetListener {
  int sockfd;
};

int NetListener_create(NetListener** out, uint16_t port, int backlog) {
  int ls = socket(AF_INET, SOCK_STREAM, 0);
  if (ls < 0) {
    log_error("socket");
    return -1;
  }

  int opt = 1;
  if (setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    log_error("setsockopt");
    close(ls);
    return -1;
  }

  struct sockaddr_in sa = {.sin_family = AF_INET,
                           .sin_addr.s_addr = INADDR_ANY,
                           .sin_port = htons(port)};

  if (bind(ls, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
    log_error("bind");
    close(ls);
    return -1;
  }
  if (listen(ls, backlog) < 0) {
    log_error("listen");
    close(ls);
    return -1;
  }

  NetListener* lst = malloc(sizeof(*lst));
  if (!lst) {
    close(ls);
    return -1;
  }
  lst->sockfd = ls;
  *out = lst;
  return 0;
}

void NetListener_destroy(NetListener* lst) {
  if (!lst)
    return;
  close(lst->sockfd);
  free(lst);
}

int NetListener_accept(NetListener* lst) {
  if (!lst)
    return -1;
  return accept(lst->sockfd, NULL, NULL);
}


int GetConnToOrigin(const char* host, const char* port) {
  struct addrinfo  hints = {hints.ai_socktype = SOCK_STREAM};
  struct addrinfo* res = NULL;
  int              sock = -1;

  if (getaddrinfo(host, port, &hints, &res) != 0) {
    log_error("getaddrinfo");
    return -1;
  }

  sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (sock < 0) {
    log_error("socket");
    freeaddrinfo(res);
    return -1;
  }

  if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
    log_error("connect");
    close(sock);
    freeaddrinfo(res);
    return -1;
  }

  freeaddrinfo(res);
  return sock;
}