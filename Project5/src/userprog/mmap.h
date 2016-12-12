
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
    int fd;
};

bool mmap_discard(struct supp_pagedir_entry *e);
void mmap_init(void);
void munmap_sys(int map_id);
int mmap_sys(int fd, void *vaddr, int s, int len, int flags);

#endif //PROJECT4_MMAP_H
