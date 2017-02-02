//
// Created by a on 1/30/17.
//

#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <assert.h>
#include "cgi_bin.h"
#include "map_entry.h"
#include "http_helper.h"
#include "config.h"


bool is_cgi_bin(const char *url){
  const char *question = strstr(url, "?");
  const char *cgi = strstr(url, ".cgi");
  return cgi && (!question || cgi < question);
}

int cgi_bin_execute(http_map_entry *root){
  const char *url = http_get_val(root, HTTP_URI);

  const char *CONTENT_LENGTH = NULL;
  const char *CONTENT = NULL;

  const char *QUERY_STRING = strstr(url, "?");
  if(QUERY_STRING) QUERY_STRING++;
  else QUERY_STRING = url + strlen(url);

  const char *METHOD = http_get_val(root, HTTP_METHOD);
  if(!strcmp(METHOD, "get")) METHOD = "GET";
  else if(!strcmp(METHOD, "post")){
    METHOD = "POST";
    CONTENT_LENGTH = http_get_val(root, "content-length");
    CONTENT = http_get_val(root, HTTP_CONTENT);
  }

  const int READ = 0;
  const int WRITE = 1;

  int pipe_in[2] = {-1, -1};
  int pipe_out[2] = {-1, -1};
  if(pipe(pipe_in)) {
    fprintf(stderr, "could not create pipe");
    return -1;
  }
  if(pipe(pipe_out)){
    close(pipe_in[0]);
    close(pipe_in[1]);

    fprintf(stderr, "could not create second pipe");
    return -1;
  }

  pid_t fork_id = fork();
  if(fork_id < 0){
    close(pipe_in[0]);
    close(pipe_in[1]);
    close(pipe_out[0]);
    close(pipe_out[1]);

    fprintf(stderr, "server is fucked, can not fork\n");

    return -1;
  }else if(fork_id == 0){
    close(pipe_in[WRITE]);
    close(pipe_out[READ]);

    dup2(pipe_in[READ], STDIN_FILENO);
    dup2(pipe_out[WRITE], STDOUT_FILENO);

    setenv("QUERY_STRING", QUERY_STRING, 1);
    setenv("METHOD", METHOD, 1);

    if(CONTENT_LENGTH) setenv("CONTENT_LENGTH", CONTENT_LENGTH, 1);
    if(CONTENT) setenv("CONTENT", CONTENT, 1);

    char *tmp = (char*)strstr(url, "?");
    if(tmp) *tmp = 0;
    const char *CGI = config_get_value(http_get_val(root, HTTP_TRIMMED_DOMAIN), "cgi-bin");
    char *path = malloc(strlen(url) + strlen(CGI) + 10);
    assert(path);
    char *const argv[] = {path, NULL};
    sprintf(path, "%s/%s", CGI, url);

    exit(execv(path, argv));

  }else{
    close(pipe_in[READ]);
    close(pipe_out[WRITE]);

    if(CONTENT) assert(write(pipe_in[WRITE], CONTENT, atoi(CONTENT_LENGTH)) == atoi(CONTENT_LENGTH));
    close(pipe_in[WRITE]);

    int status;
    if (wait(&status)) {
      return pipe_out[READ];
    }else{
      fprintf(stderr, "Could not wait for child");

      close(pipe_out[READ]);
      return -1;
    }

  }

}