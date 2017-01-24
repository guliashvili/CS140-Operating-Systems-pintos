//
// Created by a on 1/23/17.
//

#include "logger.h"


void log_write_error(struct log_info *log_info, const char *s) {
  fprintf(stderr, "log_write_error: %s\n", s);
}

void log_write_info(struct log_info *log_info) {
  fprintf(stdout, "log_info: %d %d\n", log_info->sent_length, log_info->status_code);
}