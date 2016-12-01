#ifndef PROJECT4_PAGING_H
#define PROJECT4_PAGING_H

#include "threads/pte.h"
#include "threads/palloc.h"
#include "lib/stdbool.h"
#include "lib/stdint.h"
#include "devices/block.h"
#include "../lib/stdbool.h"
#include "../lib/stdint.h"
#include "../devices/block.h"
#include "../threads/pte.h"

#define PAGING_MAGIC 432432232

struct supp_pagedir_entry{
    enum palloc_flags flags;
    uint32_t **pagedir;

    block_sector_t sector_t;
    int fd;
    int s;
    int e;

    void *upage;
    int MAGIC;
};
struct supp_pagedir2{
    struct supp_pagedir_entry *entries[1<<PTBITS];
};

struct supp_pagedir{
    struct supp_pagedir2 *entries[1<<PDBITS];
};

struct supp_pagedir_entry **supp_pagedir_lookup(struct supp_pagedir *table, const void *vaddfr, bool create);
void supp_pagedir_virtual_create(void *upage, enum palloc_flags flag);
struct supp_pagedir* supp_pagedir_init(void);
void supp_pagedir_destroy(struct supp_pagedir *spd, uint32_t *pd);
void supp_pagedir_destroy_page(struct supp_pagedir *spd, uint32_t *pd, void *upage);
void paging_activate(void *upage);
void supp_pagedir_set_prohibit(void *upage, bool prohibit);
void supp_pagedir_set_readfile(void *vaddr, int fd, int s, int e, int flags);


#endif //PROJECT4_PAGING_H
