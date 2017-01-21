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
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include "stdbool.h"
#include <netinet/tcp.h>
#include "processor.h"
#include "http_helper.h"
#include "config.h"

void processor_inner_routine(struct processor_state *aux, http_map_entry *http){
  static const char *msg200 = "HTTP/1.0 200 OK\r\n"
          "Content-Type: text/html\r\n"
          "Content-Length: 93\r\n"
          "\r\n"
          "<html>\n"
          "<body>\n"
          "<h1>Happy New Millennium!</h1>\n"
          "(more file contents)\n"
          "  .\n"
          "  .\n"
          "  .\n"
          "</body>\n"
          "</html>";
  static const char *msg400 = "HTTP/1.1 404 Not Found\r\n"
          "Content-Type: text/html\r\n"
          "Content-Length: 207\r\n"
          "\r\n"
          "<html><head>\n"
          "<title>Error response</title>\n"
          "</head>\n"
          "<body>\n"
          "<h1>Error response</h1>\n"
          "<p>Error code 404.\n"
          "<p>Message: File not found.\n"
          "<p>Error code explanation: 404 = Nothing matches the given URI.\n"
          "</body></html>";



  char *domain_port = strdup(http_get_val(http, "host"));
  char *domain = NULL;
  char *port = NULL;

  bool is404 = false;
  int i = 0;
  for (char *save_ptr, *token = strtok_r (domain_port, ":", &save_ptr); token != NULL;
       token = strtok_r (NULL, ":", &save_ptr), i++){
      if(i == 0){ // domain
        domain = strdup(token);
      }else if(i == 1){ // port
        port = strdup(token);
      }else if(i == 2){
        is404 = true;
      }
  }
  free(domain_port);
  if(port == NULL) port = strdup("80");

  if(domain == NULL || !vhost_exists(domain) || !config_value_exists(domain, "port")
    || atoi(config_get_value(domain, "port")) != aux->port || atoi(port) != aux->port){
    is404 = true;
  }
  free(domain);
  free(port);
  if(is404){
    int err = write(aux->fd, msg400, strlen(msg400));
    if (err < 0)
      fprintf(stderr, " Error in send");

    return;
  }

  int err = write(aux->fd, msg200, strlen(msg200));
  if (err < 0)
    fprintf(stderr, " Error in send");




}

long long processor_state_routine (struct processor_state *aux){

  time_t end_t = time(0);
  bool first = true;
  while(first || time(0) < end_t){
    first = false;

    http_map_entry *http = http_parse(aux->fd);

    http_map_entry *entry = NULL;
    HASH_FIND_STR(http, "connection", entry);
    bool keep_alive = (entry != NULL) && (strcmp(entry->value, "keep_alive") == 0);

    //process
    processor_inner_routine(aux, http);


    http_destroy(http);

    if(keep_alive)
      end_t = time(0) + 5;

  }

  close(aux->fd);
}

void *one_port_listener(void *aux) {
  int port = (long) aux;


  int server_fd, err;
  struct sockaddr_in server, client;

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0){
    fprintf(stderr, "could not create socket \n");
    exit(1);
  }

  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  int opt_val = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

  err = bind(server_fd, (struct sockaddr *) &server, sizeof(server));
  if (err < 0){
    fprintf(stderr, "error in binding \n");
    exit(1);
  }

  err = listen(server_fd, 128);

  if (err < 0) {
    fprintf(stderr, "Could not listen on socket\n");
    exit(1);
  }

  printf("Server is listening on %d\n", port);
  unsigned clilen = sizeof(client);
  while(1) {
    fflush(stdout);
    int newsockfd = accept(server_fd,
                       (struct sockaddr *) &client,
                       &clilen);
    fflush(stdout);
    if (newsockfd < 0)
      return NULL;

    processor_state *data = malloc(sizeof(processor_state));
    data->fd = newsockfd;
    data->port = port;
    data->start_routine = processor_state_routine;
    processor_add(newsockfd, 1, data);
  }
}


void start_server(config_map_entry *root) {
  processor_init();

  pthread_t *trd[1 << 16];
  memset(trd, 0, sizeof(trd));

  config_map_entry *item1, *tmp1;
  HASH_ITER(hh, root, item1, tmp1) {
    config_map_entry *e = NULL;
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