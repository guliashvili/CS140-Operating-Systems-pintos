#ifndef THREADS_PALLOC_H
#define THREADS_PALLOC_H

#include <stddef.h>
#include <stdint.h>

/* How to allocate pages. */
enum palloc_flags
  {
    PAL_ASSERT = 1,           /* Panic on failure. */
    PAL_ZERO = 2,             /* Zero page contents. */
    PAL_USER = 4,              /* User page. */
    PAL_READONLY = 8,
    PAL_THROUGH_FRAME = 16,
    PAL_PROHIBIT_CACHE = 32
};
uint32_t palloc_page_to_idx(enum palloc_flags flags, void *page);
void palloc_init (size_t user_page_limit);
void *palloc_get_page (enum palloc_flags);
void *palloc_get_multiple (enum palloc_flags, size_t page_cnt);
void palloc_free_page (void *);
void palloc_free_multiple (void *, size_t page_cnt);

#endif /* threads/palloc.h */
