//
// Created by a on 1/26/17.
//

#ifndef PROJECT6_ETAG_HELPER_H
#define PROJECT6_ETAG_HELPER_H

void etag_init();
void etag_generate(char *buf, int buf_size, int file_fd);
void etag_generate_str(char *buf, int buf_size, char *in, int size);

#endif //PROJECT6_ETAG_HELPER_H
