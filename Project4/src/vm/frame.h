#ifndef PROJECT4_FRAME_H
#define PROJECT4_FRAME_H
#include "../lib/kernel/list.h"

#define FRAME_MAGIC 432432232
#define PATH_MAX_LENGTH 30
struct frame_statistics{
    int read;
    int write;
};

struct frame{
    struct list pointing_threads;
    char path[PATH_MAX_LENGTH + 1];
    struct frame_statistics frame_statistics;

    int MAGIC;
};

#endif //PROJECT4_FRAME_H
