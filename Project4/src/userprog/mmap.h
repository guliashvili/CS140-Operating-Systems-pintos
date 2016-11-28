#include <stdio.h>
#include <syscall-nr.h>
//#include <sched.h>
#include "threads/interrupt.h"
#include "devices/shutdown.h"
#include "threads/thread.h"
#include "../threads/thread.h"
#include "process.h"
#include "../filesys/filesys.h"
#include "../threads/interrupt.h"
#include "../lib/kernel/list.h"
#include "../threads/malloc.h"
#include "threads/vaddr.h"
#include "pagedir.h"
#include "../devices/input.h"
#include "../filesys/file.h"
#include "../lib/kernel/stdio.h"
#include "userprog/syscall.h"
#include "vm/paging.h"
#include "userprog/exception.h"
#include "exception.h"
#include "../vm/paging.h"
#include "../lib/syscall-nr.h"
#include "../threads/palloc.h"

#ifndef PROJECT4_MMAP_H
#define PROJECT4_MMAP_H

void munmap(int map_id);
int mmap(int fd, void *vaddr);

#endif //PROJECT4_MMAP_H
