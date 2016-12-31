#include "cached_block.h"
#include "../threads/malloc.h"
#include "../lib/string.h"
#include "../lib/stddef.h"
#include "../lib/random.h"
#include "../devices/timer.h"
#include "../threads/vaddr.h"

static int evict(struct cached_block *cache);

static struct cached_block *cached_block_g;
#define QUEUE_N PGSIZE / 16
#define READ_AHEAD

#ifdef READ_AHEAD
static uint64_t queue_e = 0;
static uint64_t queue_s = 0;
static uint16_t queue[QUEUE_N];
#endif

#define CACHED_BLOCK_SLEEP_S 10
#define READ_AHEAD
#undef READ_AHEAD
static void read_ahead(int sector){
  struct cached_block *cache = cached_block_g;

  struct rw_lock *l = cache->locks + sector;
  bool upgraded = false;
  r_lock_acquire(l);

  int in_ram = cache->addr[sector];
  if(in_ram != -1){ // is in cache

  }else {
    r_lock_upgrade_to_w(l);
    upgraded = true;
    in_ram = cache->addr[sector]; // until I got write lock someone else(on the same sector)might got write lock and got the buffer
    if(in_ram != -1) goto finish;

    in_ram = evict(cache);

    if(in_ram != -1){
      ASSERT(cache->addr[sector] == -1);
      ASSERT(cache->entries[in_ram].holder == -1);
      cache->addr[sector] = in_ram;
      cache->entries[in_ram].holder = sector;
      ASSERT(cache->entries[in_ram].lock.holder == thread_current());
      lock_release(&cache->entries[in_ram].lock);

      block_read(cache->block, sector, cache->entries[in_ram].data);
      goto finish;
    }
  }

  finish:
  if(upgraded) w_lock_release(l);
  else r_lock_release(l);
}

static void fflusher (void *cached_block)
{
  while(1){
    for(int i = 0; i < CACHED_BLOCK_SLEEP_S; i++) {
      timer_msleep(1000);
#ifdef READ_AHEAD
      if(queue_s < __sync_fetch(&queue_e)){
        int get = __sync_fetch(&queue[queue_s++]);
        if(get >= 0 && get < cached_block_size(cached_block_g))
          read_ahead(get % QUEUE_N), queue_s++;
      }
#endif
    }
    fflush_all((struct cached_block *)cached_block);
  }
}

struct cached_block *cached_block_init(struct block *block, int buffer_elem){
  ASSERT(block);
  ASSERT(buffer_elem >= 0);

  struct cached_block *cache = (struct cached_block *)malloc(sizeof(struct cached_block));
  ASSERT(cache);
  cache->block = block;
  cache->buffer_len = buffer_elem;
  cache->entries = calloc(buffer_elem, sizeof(struct cache_entry));
  for(int i = 0; i < buffer_elem; i++)
    lock_init(&cache->entries[i].lock), cache->entries[i].holder = -1, cache->entries[i].dirty = 0, cache->entries[i].accessed = 0;
  cache->locks = malloc(sizeof(struct rw_lock) * SECTOR_NUM);
  cache->addr = malloc(sizeof(int8_t) * SECTOR_NUM);
  ASSERT(cache->locks);
  ASSERT(cache->entries);
  for(int i = 0; i < SECTOR_NUM; i++) rw_lock_init(cache->locks + i);
  memset(cache->addr, -1, sizeof(int8_t) * SECTOR_NUM);

  thread_create("fflusher", 0, fflusher, cache);
  return cached_block_g = cache;
}

static void fflush_single(struct cached_block *cache, int in_ram_index) {
  int dirty_num;
  while(!__sync_bool_compare_and_swap(&cache->entries[in_ram_index].dirty, dirty_num = __sync_fetch(&cache->entries[in_ram_index].dirty), 0));

  if(dirty_num) {
    block_write(cache->block, cache->entries[in_ram_index].holder, cache->entries[in_ram_index].data);
    cache->entries[in_ram_index].dirty = true;
  }
}
void fflush_all(struct cached_block *cache){
  for(int i = 0; i < cache->buffer_len; i++){
    lock_acquire(&cache->entries[i].lock);

    if(cache->entries[i].holder != -1) fflush_single(cache, i);

    lock_release(&cache->entries[i].lock);
  }
}

int evict_I = 0;
static int evict(struct cached_block *cache){
  for(int try_hard = 0; try_hard < 2; try_hard++)
  for(int i = 0; i < cache->buffer_len; i++){
    int rnd = __sync_fetch_and_add(&evict_I, 1) % cache->buffer_len;
    if(rnd < 0) rnd += cache->buffer_len;
    if(lock_try_acquire(&cache->entries[rnd].lock)){
      int holder;
      if((holder = cache->entries[rnd].holder) != -1){
        if(w_try_lock_acquire(&cache->locks[holder])) {
          if(!cache->entries[rnd].accessed) {
            fflush_single(cache, rnd);
            cache->addr[holder] = -1;
            cache->entries[rnd].holder = -1;
            w_lock_release(&cache->locks[holder]);
          }else{
            cache->entries[rnd].accessed = 0;
            w_lock_release(&cache->locks[holder]);
            lock_release(&cache->entries[rnd].lock);
            continue;
          }
        }else{
          lock_release(&cache->entries[rnd].lock);
          continue;
        }
      }

      return rnd;
    }
  }

  return -1;
}

void cached_block_read(struct cached_block *cache, block_sector_t sector, void *buffer, int info){
  ASSERT(buffer);
  ASSERT(cache);
  ASSERT(sector >= 0);
  ASSERT(sector < 8 * 1024 * 1024 / BLOCK_SECTOR_SIZE);

  cached_block_read_segment(cache, sector, 0, BLOCK_SECTOR_SIZE, buffer, info);
}

void cached_block_read_segment(struct cached_block *cache, block_sector_t sector, int s, int e, void *buffer, int info){
  ASSERT(buffer);
  ASSERT(cache);
  ASSERT(s < e);

  struct rw_lock *l = cache->locks + sector;
  bool upgraded = false;
  r_lock_acquire(l);

  int in_ram = cache->addr[sector];
  if(in_ram != -1){ // is in cache
    sorry_goto_read_from_cache:
    atomic_gio_memcpy(buffer, cache->entries[in_ram].data + s, e - s);
    __sync_add_and_fetch(&cache->entries[in_ram].accessed, 1);
  }else {
    r_lock_upgrade_to_w(l);
    upgraded = true;
    in_ram = cache->addr[sector]; // until I got write lock someone else(on the same sector)might got write lock and got the buffer
    if(in_ram != -1) goto sorry_goto_read_from_cache;

    in_ram = evict(cache);

    if(in_ram != -1){
      ASSERT(cache->addr[sector] == -1);
      ASSERT(cache->entries[in_ram].holder == -1);
      cache->addr[sector] = in_ram;
      cache->entries[in_ram].holder = sector;
      ASSERT(cache->entries[in_ram].lock.holder == thread_current());
      lock_release(&cache->entries[in_ram].lock);

      block_read(cache->block, sector, cache->entries[in_ram].data);
      goto sorry_goto_read_from_cache;
    }

    if (s == 0 && e == BLOCK_SECTOR_SIZE) {
        block_read(cache->block, sector, buffer);
    } else {
      void *buf = malloc(BLOCK_SECTOR_SIZE);
      block_read(cache->block, sector, buf);
      memcpy(buffer, buf + s, e - s);
      free(buf);
    }
  }

  if(upgraded) w_lock_release(l);
  else r_lock_release(l);

#ifdef READ_AHEAD
  if(sector + 1 < cached_block_size(cache))
    __sync_lock_test_and_set(queue + __sync_fetch_and_add(&queue_e,1) % QUEUE_N, sector + 1);
#endif
}

void cached_block_write(struct cached_block *cache, block_sector_t sector,const void *buffer, int info){
  ASSERT(cache);
  ASSERT(buffer);

  ASSERT(sector >= 0);
  ASSERT(sector < 8 * 1024 * 1024 / BLOCK_SECTOR_SIZE);

  cached_block_write_segment(cache, sector, 0, BLOCK_SECTOR_SIZE, buffer, NULL, info);
}
struct inode_disk
{
    block_sector_t lvl1;               /* First data sector. */
    off_t length;                       /* File size in bytes. */
    unsigned magic;                     /* Magic number. */
    uint32_t unused[125];               /* Not used. */
} PACKED;

void cached_block_write_segment(struct cached_block *cache, block_sector_t sector, int s, int e,
                                const void *buffer,const void *full_buffer, int info){
  ASSERT(cache);
  ASSERT(buffer);
  if(s == 0 && e == BLOCK_SECTOR_SIZE) full_buffer = buffer;

  struct rw_lock *l = cache->locks + sector;

  bool upgraded = false;
  r_lock_acquire(l);
  int in_ram = cache->addr[sector];

  if(in_ram != -1){
    sorry_goto_write_in_cache:
    atomic_gio_memcpy(cache->entries[in_ram].data + s, buffer, e - s);
    __sync_add_and_fetch(&cache->entries[in_ram].dirty, 1);
    __sync_add_and_fetch(&cache->entries[in_ram].accessed, 1);
  }else {
    r_lock_upgrade_to_w(l);
    upgraded = true;

    in_ram = cache->addr[sector];
    if(in_ram != -1) goto sorry_goto_write_in_cache;

    in_ram = evict(cache);
    if(in_ram != -1){
      ASSERT(cache->addr[sector] == -1);
      ASSERT(cache->entries[in_ram].holder == -1);
      cache->addr[sector] = in_ram;
      cache->entries[in_ram].holder = sector;
      ASSERT(cache->entries[in_ram].lock.holder == thread_current());
      lock_release(&cache->entries[in_ram].lock);
      block_read(cache->block, sector, cache->entries[in_ram].data);

      goto sorry_goto_write_in_cache;
    }

    if (full_buffer) {
      block_write(cache->block, sector, full_buffer);
    } else {
      void *buf = malloc(BLOCK_SECTOR_SIZE);

      block_read(cache->block, sector, buf);
      atomic_gio_memcpy(buf + s, buffer, e - s);
      block_write(cache->block, sector, buf);

      free(buf);
    }
  }
  if(upgraded) w_lock_release(l);
  else r_lock_release(l);

}

block_sector_t cached_block_size (struct cached_block *cache){
  ASSERT(cache);
  return block_size(cache->block);
}