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


http_map_entry* http_parse(int fd);
void http_destroy(http_map_entry *http);

#endif //PROJECT6_HTTP_HELPER_H
