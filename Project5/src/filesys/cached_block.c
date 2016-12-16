#include "cached_block.h"
#include "../threads/malloc.h"
#include "../lib/string.h"
#include "../lib/stddef.h"


struct cached_block *cached_block_init(struct block *block, int buffer_elem){
  ASSERT(block);
  ASSERT(buffer_elem >= 0);

  struct cached_block *cache = (struct cached_block *)malloc(sizeof(struct cached_block));
  if(cache == NULL) PANIC("Not enought memory for cached_block");
  cache->block = block;
  cache->buffer_len = buffer_elem;
  cache->entries = calloc(buffer_elem, sizeof(struct cache_entry));

  return cache;
}

void cached_block_read(struct cached_block *cache, block_sector_t sector, void *buffer, int info){
  ASSERT(buffer);
  ASSERT(cache);

  cached_block_read_segment(cache, sector, 0, BLOCK_SECTOR_SIZE, buffer, info);
}

void cached_block_read_segment(struct cached_block *cache, block_sector_t sector, int s, int e, void *buffer, int info){
  ASSERT(buffer);
  ASSERT(cache);
  ASSERT(s < e);

  if(s == 0 && e == BLOCK_SECTOR_SIZE){
    block_read(cache->block, sector, buffer);
  }else {
    void *buf = malloc(BLOCK_SECTOR_SIZE);
    block_read(cache->block, sector, buf);
    atomic_gio_memcpy(buffer, buf + s, e - s);
    free(buf);
  }
}

void cached_block_write(struct cached_block *cache, block_sector_t sector,const void *buffer, int info){
  ASSERT(cache);
  ASSERT(buffer);

  cached_block_write_segment(cache, sector, 0, BLOCK_SECTOR_SIZE, buffer, NULL, info);
}

void cached_block_write_segment(struct cached_block *cache, block_sector_t sector, int s, int e,
                                const void *buffer,const void *full_buffer, int info){
  ASSERT(cache);
  ASSERT(buffer);

  if(s == 0 && e == BLOCK_SECTOR_SIZE) full_buffer = buffer;

  if(full_buffer){
    block_write(cache->block, sector, full_buffer);
  }else{
    void *buf = malloc(BLOCK_SECTOR_SIZE);

    block_read(cache->block, sector, buf);
    atomic_gio_memcpy(buf + s, buffer, e - s);
    block_write(cache->block, sector, buf);

    free(buf);
  }

}

block_sector_t cached_block_size (struct cached_block *cache){
  ASSERT(cache);
  return block_size(cache->block);
}