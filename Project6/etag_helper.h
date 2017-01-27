//
// Created by a on 1/26/17.
//

#ifndef PROJECT6_ETAG_HELPER_H
#define PROJECT6_ETAG_HELPER_H

// find's all the pritable chars, rounds it down to the nearest 2s power
void etag_init();

/*
 * generates hash for file. In the end file offset is 0
 */
void etag_generate(char *buf, int buf_size, int file_fd);
/*
 * generates hash for c string
 */
void etag_generate_str(char *buf, int buf_size, char *in, int size);

#endif //PROJECT6_ETAG_HELPER_H
