//
// Created by a on 1/23/17.
//

#include "logger.h"
#include <arpa/inet.h>

//https://en.wikipedia.org/wiki/C_date_and_time_functions
void print_time(FILE *f){
  time_t current_time = time(NULL);

  if (current_time == ((time_t)-1)) {
    fprintf(stderr, "Failure to obtain the current time.\n");
  }

  /* Convert to local time format. */
  char c_time_string[200];
  ctime_r(&current_time, c_time_string);

  if (c_time_string == NULL) {
    fprintf(stderr, "Failure to convert the current time.\n");
  }
  if(c_time_string[strlen(c_time_string) - 1] == '\n') c_time_string[strlen(c_time_string) - 1] = 0;
  fprintf(f, "[%s] ", c_time_string);
}

void log_write_error(struct log_info *log_info, const char *s) {
  const char *domain = http_get_val(log_info->root, HTTP_TRIMMED_DOMAIN);
  if(!vhost_exists(domain)) return;
  pthread_mutex_lock(config_get_value(domain, LOG_INFO_KEY));


  FILE *f = fopen(config_get_value(domain, "log"), "a+");
  if(f) {
    print_time(f);
    fprintf(f, "%s\n", s);
    fclose(f);
  }else{
    fprintf(stderr, "Could not open log file\n");
  }

  pthread_mutex_unlock(config_get_value(domain, LOG_INFO_KEY));
}

void log_write_info(struct log_info *log_info) {
  const char *domain = http_get_val(log_info->root, HTTP_TRIMMED_DOMAIN);
  if(!vhost_exists(domain)) return;

  char ip[(3+2)*4];

  inet_ntop(AF_INET, &(log_info->ipAddr), ip, INET_ADDRSTRLEN);
  pthread_mutex_lock(config_get_value(domain, LOG_INFO_KEY));

  FILE *f = fopen(config_get_value(domain, "log"), "a+");
  if(f){
    print_time(f);
    fprintf(f, "%s ", ip);
    fprintf(f, "%s ", http_get_val(log_info->root, "host"));
    fprintf(f, "%s ", http_get_val(log_info->root, HTTP_URI));
    fprintf(f, "%d %d ", log_info->status_code, log_info->sent_length);
    fprintf(f, "%s\n", http_get_val(log_info->root, "user-agent"));


    fclose(f);
  }else{
    fprintf(stderr, "Could not open log file\n");
  }

  fprintf(stdout, "log_info: %d %d %s\n", log_info->sent_length, log_info->status_code, ip);
  pthread_mutex_unlock(config_get_value(domain, LOG_INFO_KEY));
}