
#include "threads/thread.h"
#include "../threads/thread.h"
#include "../filesys/filesys.h"
#include "../lib/kernel/list.h"
#include "../filesys/file.h"

#ifndef PROJECT4_MMAP_H
#define PROJECT4_MMAP_H

struct mmap_info{
    struct list_elem link;
    void *vaddr;
    int num_of_pages;
    int id;
};

void munmap(int map_id);
int mmap(int fd, void *vaddr);

#endif //PROJECT4_MMAP_H
