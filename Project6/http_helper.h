//
// Created by a on 1/19/17.
//

#ifndef PROJECT6_HTTP_HELPER_H
#define PROJECT6_HTTP_HELPER_H

#include "map_entry.h"

#define HTTP_METHOD "http_method"
#define HTTP_VERSION "http_version"
#define HTTP_URI "http_uri"
#define HTTP_CONTENT "http_content"
#define HTTP_TRIMMED_DOMAIN "http_trimmed_domain"

void http_put_val(http_map_entry *root, char *key, char *value);

http_map_entry *http_parse(int fd);

void http_destroy(http_map_entry *http);

const char *http_get_val(http_map_entry *root, const char *key);

#endif //PROJECT6_HTTP_HELPER_H
