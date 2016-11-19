//
// Created by a on 11/20/16.
//

#ifndef PROJECT4_PAGING_H
#define PROJECT4_PAGING_H

#include "frame.h"
#define PAGE_MAGIC 432432232
#define PATH_MAX_LENGTH 30

struct supp_pagedir_entry{
    struct file_info file_info;
    bool should_be_zeroed;
    int MAGIC;
};

struct supp_page_table{
    struct supp_pagedir_entry *entries;
};

struct supp_page_table* init_supp_pagedir(void);
void supp_pagedir_destroy(struct supp_page_table *table);


#endif //PROJECT4_PAGING_H
