//
// Created by a on 1/11/17.
//
//http://www.linuxhowtos.org/data/6/server.c
#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "stdbool.h"
#include "processor.h"

long long processor_state_routine (struct processor_state *aux){
  static const char *msg = "HTTP/1.0 200 OK\n"
          "Date: Fri, 31 Dec 1999 23:59:59 GMT\n"
          "Content-Type: text/html\n"
          "Content-Length: 1354\n"
          "\n"
          "<html>\n"
          "<body>\n"
          "<h1>Happy New Millennium!</h1>\n"
          "(more file contents)\n"
          "  .\n"
          "  .\n"
          "  .\n"
          "</body>\n"
          "</html>";
  int err = send(aux->fd, msg, strlen(msg), 0);
  if (err < 0){
    fprintf(stderr, " Error in send");
    exit(1);
  }
}

void *one_port_listener(void *aux) {
  int sockfd, newsockfd, portno;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    fprintf(stderr, "ERROR opening socket");
    return NULL;
  }
  bzero((char *) &serv_addr, sizeof(serv_addr));
  portno = (long) aux;
  printf("listening to port %d\n", portno);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  if (bind(sockfd, (struct sockaddr *) &serv_addr,
           sizeof(serv_addr)) < 0) {
    fprintf(stderr, "ERROR on binding");
    close(sockfd);
    return NULL;
  }
  listen(sockfd, 5);
  clilen = sizeof(cli_addr);
  newsockfd = accept(sockfd,
                     (struct sockaddr *) &cli_addr,
                     &clilen);
  if (newsockfd < 0)
    return NULL;

  processor_state *data = malloc(sizeof(processor_state));
  data->fd = newsockfd;
  data->start_routine = processor_state_routine;
  processor_add(newsockfd, 1, data);
}


void start_server(map_entry *root) {
  processor_init();

  pthread_t *trd[1 << 16];
  memset(trd, 0, sizeof(trd));

  map_entry *item1, *tmp1;
  HASH_ITER(hh, root, item1, tmp1) {
    map_entry *e = NULL;
    HASH_FIND_STR(item1->sub, "port", e);
    if (e) {
      long port = atoi(e->value);
      if (trd[port]) continue;
      pthread_create(trd[port] = malloc(sizeof(pthread_t)), NULL, one_port_listener, (void *) port);
    }
  }

  for (int i = 0; i < (1 << 16); i++)
    if (trd[i])
      pthread_join(*trd[i], NULL), free(trd[i]);

}