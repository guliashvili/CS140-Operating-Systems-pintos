#ifndef PROJECT4_FRAME_H
#define PROJECT4_FRAME_H
#include "../lib/kernel/list.h"

#define FRAME_MAGIC 432432232
#define PATH_MAX_LENGTH 30
struct frame_statistics{
    int read;
    int write;
};
struct file_info{
    char path[PATH_MAX_LENGTH + 1];
    int page_id;
};

struct frame{
    struct list pointing_threads;
    struct file_info file_info;
    struct frame_statistics frame_statistics;

    int MAGIC;
};

struct frame_map{
  struct frame *frames;
};

size_t frame_buf_size(int pages_cnt);
struct frame_map * frame_map_create_in_buf(int pages_cnt, void *base, size_t len);

#endif //PROJECT4_FRAME_H
