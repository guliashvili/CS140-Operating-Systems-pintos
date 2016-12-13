#ifndef PROJECT5_CACHE_H
#define PROJECT5_CACHE_H
#include "../devices/block.h"

struct cache_entry{
  int z;
};
struct cached_block{
    struct block *block;
    int buffer_len;
    struct cache_entry *entries;
};

struct cached_block *cached_block_init(struct block *block, int buffer_elem);
void cached_block_read_segment(struct cached_block *cache, block_sector_t sector, int s, int e, void *buffer, int info);
void cached_block_read(struct cached_block *cache, block_sector_t sector, void *buffer, int info);

void cached_block_write_segment(struct cached_block *cache, block_sector_t sector, int s, int e, void *buffer, int info);
void cached_block_write(struct cached_block *cache, block_sector_t sector, void *buffer, int info);

block_sector_t cached_block_size (struct cached_block *cache);
#endif //PROJECT5_CACHE_H
