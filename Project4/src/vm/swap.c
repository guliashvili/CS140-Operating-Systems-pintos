#include "vm/swap.h"
#include "swap.h"
#include "../lib/debug.h"
#include "threads/malloc.h"
#include "../threads/malloc.h"
#include "paging.h"
#include "../userprog/pagedir.h"
#include "../threads/thread.h"

static struct swap_map *s_map = NULL;
#define NUM_OF_HARD_DISK_SEGMENT (PGSIZE / BLOCK_SECTOR_SIZE)

void swap_init(void){
  struct block *swap = block_get_role(BLOCK_SWAP);
  ASSERT(PGSIZE % BLOCK_SECTOR_SIZE == 0);
  s_map = malloc(sizeof(struct swap_map));
  lock_init(&s_map->lock);
  s_map->map = bitmap_create(block_size(swap) * BLOCK_SECTOR_SIZE / PGSIZE);
}

block_sector_t swap_write(void *kpage){
  ASSERT(s_map);
  ASSERT(s_map->map);
  ASSERT(kpage);
  ASSERT(is_kernel_vaddr(kpage));

  lock_acquire(&s_map->lock);

  size_t t = bitmap_scan_and_flip(s_map->map, 0, 1, 0);
  if(t == BITMAP_ERROR) {
    lock_release(&s_map->lock);
    return BLOCK_SECTOR_T_ERROR;
  }
  block_sector_t start = t * NUM_OF_HARD_DISK_SEGMENT;
  struct block *swap = block_get_role(BLOCK_SWAP);

  block_sector_t i;
  for(i = start; i < start + NUM_OF_HARD_DISK_SEGMENT; i++, kpage += BLOCK_SECTOR_SIZE) {
    block_write(swap, i, kpage);
  }

  lock_release(&s_map->lock);
  return start;
}

void swap_read(block_sector_t t, void *vaddr_p){
  ASSERT(s_map);
  ASSERT(s_map->map);
  ASSERT(vaddr_p == NULL || is_user_vaddr(vaddr_p));

  ASSERT(t % NUM_OF_HARD_DISK_SEGMENT == 0);
  ASSERT(bitmap_all(s_map->map, t / NUM_OF_HARD_DISK_SEGMENT, 1));

  struct block *swap = block_get_role(BLOCK_SWAP);
  block_sector_t i;
  if(vaddr_p != NULL) {

    lock_acquire(&s_map->lock);
    for (i = t; i < t + NUM_OF_HARD_DISK_SEGMENT; i++, vaddr_p += BLOCK_SECTOR_SIZE) {
      ASSERT(thread_current()->pagedir);
      block_read(swap, i, pagedir_get_page(thread_current()->pagedir, vaddr_p));
    }
    lock_release(&s_map->lock);
  }

  bitmap_set(s_map->map, t / NUM_OF_HARD_DISK_SEGMENT, 0);
}