//
// Created by a on 11/20/16.
//

#ifndef PROJECT4_PAGING_H
#define PROJECT4_PAGING_H

#include "frame.h"
#include "threads/pte.h"
#include "threads/palloc.h"

#define PAGING_MAGIC 432432232
#define PATH_MAX_LENGTH 30

struct supp_pagedir_entry{
    struct file_info file_info;
    enum palloc_flags flags;
    bool writable;
    int MAGIC;
};

struct supp_page_tabl2{
    struct supp_pagedir_entry *entries[1<<PDBITS];
};

struct supp_page_table{
    struct supp_page_tabl2 *entries[1<<PTBITS];
};

struct supp_page_table* init_supp_pagedir(void);
void supp_pagedir_destroy(struct supp_page_table *table);
bool paging_pagedir_exists(uint32_t *pagedir, const void *uaddr);
bool virtually_create_page(struct supp_page_table *table, void *upage,  bool writable, enum palloc_flags flag,
                           struct file_info *file_info);
struct supp_pagedir_entry **
lookup_supp_page (struct supp_page_table *table, const void *vaddr, bool create);
void supp_page_destroy(struct supp_page_table *table, void *upage);
bool paging_supp_pagedir_exists(uint32_t *pagedir, const void *uaddr);
bool really_create_page(struct supp_page_table *table, uint32_t *pd, void *upage);

#endif //PROJECT4_PAGING_H
