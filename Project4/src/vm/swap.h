#ifndef PROJECT4_SWAP_H
#define PROJECT4_SWAP_H

#include "devices/block.h"
#include "../devices/block.h"
#include "../threads/vaddr.h"
#include "../lib/kernel/bitmap.h"
#include "../threads/synch.h"
#include "threads/synch.h"
struct swap_map{
    struct lock lock;
    struct bitmap *map;
};

void swap_init(void *p);
int swap_get_init_size(void);
void swap_read(block_sector_t t, void *vaddr_p);
block_sector_t swap_write(void *kpage);

#endif //PROJECT4_SWAP_H
