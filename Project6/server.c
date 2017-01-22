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
#include <dirent.h>
#include "processor.h"
#include "http_helper.h"
#include "config.h"
#include <sys/sendfile.h>
#include <fcntl.h>
#include <libut.h>

static bool push404(struct processor_state *aux){
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

  int err = write(aux->fd, msg400, strlen(msg400));
  if (err < 0)
    fprintf(stderr, " Error in send");
}

static bool was404_error(struct processor_state *aux, http_map_entry *http, char **domaain){

  char *domain_port = strdup(http_get_val(http, "host"));
  char *port = NULL;

  bool is404 = false;
  int i = 0;
  *domaain = NULL;
  for (char *save_ptr, *token = strtok_r (domain_port, ":", &save_ptr); token != NULL;
       token = strtok_r (NULL, ":", &save_ptr), i++){
    if(i == 0){ // domain
      *domaain = strdup(token);
    }else if(i == 1){ // port
      port = strdup(token);
    }else if(i == 2){
      is404 = true;
    }
  }
  free(domain_port);
  if(port == NULL) port = strdup("80");

  if(*domaain == NULL || !vhost_exists(*domaain) || !config_value_exists(*domaain, "port")
     || atoi(config_get_value(*domaain, "port")) != aux->port || atoi(port) != aux->port){
    is404 = true;
  }
  free(port);
  if(is404)
    push404(aux);


  return is404;
}

void push_construct_dir(struct processor_state *aux, DIR *dir){
  UT_string *header, *body;
  utstring_new(body);
  utstring_new(header);

  utstring_printf(body, "<html><title>I love gio</title><body>"
          "<h2>Directory listing for /</h2>"
          "<hr>"
          "<ul>");
  struct dirent *d;
  while(d = readdir(dir))
    if(strcmp(d->d_name, ".") && strcmp(d->d_name, "..")) {
      if(strstr(d->d_name, "."))
        utstring_printf(body, "<li><a href=\"%s\">%s/</a>\n", d->d_name, d->d_name);
      else
        utstring_printf(body, "<li><a href=\"%s/\">%s/</a>\n", d->d_name, d->d_name);
    }


  utstring_printf(body, "</ul>"
          "<hr>"
          "</body>"
          "</html>");


  utstring_printf(header, "HTTP/1.0 200 OK\r\n"
          "Content-Type: text/html\r\n"
          "Content-Length: %d\r\n"
          "\r\n"
          "%s", (int)strlen(utstring_body(body)), utstring_body(body));

  if(write(aux->fd, utstring_body(header), strlen(utstring_body(header))) < 0){
    fprintf(stderr, "error in sending\n");
  }

  utstring_free(body);
  utstring_free(header);
}

static void send_file_gio(int fd, int file_fd, const char *type){
  FILE* fp = fdopen(file_fd, "r");
  fseek(fp, 0, SEEK_END);
  int length = ftell(fp) - 1;
  rewind(fp);

  char s[100];

  if(strstr(type, ".html"))
    type = "text/html";
  else if(strstr(type, ".mp4"))
    type = "video/mp4";
  else if(strstr(type, ".jpg"))
    type = "image/jpeg";
  else{
    fprintf(stderr, "could not detect the type %s\n",type);
  }
  sprintf(s, "HTTP/1.1 200 OK\r\n"
          "Content-Length: %d\r\n"
          "Content-Type: %s\r\n\r\n", length, type);

  if(write(fd, s , strlen(s)) < 0){
    fprintf(stderr, "Could not send info %d %s", fd, s);
  }

  lseek(file_fd, 0, SEEK_SET);

  sendfile(fd, file_fd, NULL, length);

  fclose(fp);
}
void processor_inner_routine(struct processor_state *aux, http_map_entry *http){
  char *domain;
  if(was404_error(aux, http, &domain)) {
    free(domain);
    return;
  }
  char *main_folder = config_get_value(domain, "documentroot");
  const char *relative_folder = http_get_val(http, HTTP_URI);
  if(strstr(relative_folder, "..")){
    fprintf(stderr, "dots(..) are prohibited for safety reasons\n");
    push404(aux);
    free(domain);
    return;
  }

  char *help_folder = strcat(calloc(strlen(relative_folder) + strlen(main_folder) + 20,1), main_folder);
  help_folder = strcat(help_folder, "/");
  help_folder = strcat(help_folder, relative_folder);
  DIR *dir = opendir(help_folder);

  int file_fd = open(help_folder, O_RDONLY);
  help_folder = strcat(help_folder, "/index.html");
  int index_html_file_fd = open(help_folder, O_RDONLY);

  if(index_html_file_fd >= 0){
    send_file_gio(aux->fd, index_html_file_fd, ".html");
  }else if(dir){
    push_construct_dir(aux, dir);
  }else if(file_fd >= 0){
    help_folder[strlen(help_folder) - strlen("/index.html")] = 0;
    int ind = strlen(help_folder) - 6;
    if(ind < 0) ind = 0;
    send_file_gio(aux->fd, file_fd, help_folder + ind);
  }else {
    fprintf(stderr, "Could not chdir to relative folder\n");
    push404(aux);
  }

  free(help_folder);
  closedir(dir);
  close(index_html_file_fd);

  free(domain);
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