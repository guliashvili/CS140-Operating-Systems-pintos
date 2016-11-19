//
// Created by a on 11/20/16.
//

#ifndef PROJECT4_PAGING_H
#define PROJECT4_PAGING_H

#include "threads/palloc.h"
#define PAGE_MAGIC 432432232
#define PATH_MAX_LENGTH 30

struct supplemental_page_dir_entry{
    struct file_info file_info;
    bool should_be_zeroed;
    int MAGIC;
};


#endif //PROJECT4_PAGING_H
