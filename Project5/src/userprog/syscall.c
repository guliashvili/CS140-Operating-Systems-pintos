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
#include "files.h"
#include "mmap.h"
#include "../lib/debug.h"
#include "syscall.h"


static void syscall_handler (struct intr_frame *);
static int read_sys_wrapper (int fd, void * buffer, unsigned size);
/* Projects 2 and later. */
static void halt (void);
static int exec (const char *file);
static int wait (int);
static void check_pointer(uint32_t esp, void *s, bool grow, bool prohibit, const char *name);

static void check_pointer(uint32_t esp, void *s, bool grow, bool prohibit, const char *name){
  if(!is_user_vaddr(s))
    exit(-1, "pointer is more then PHYS_BASE(check_pointer)");

  ASSERT(thread_current()->pagedir);
  if(!pagedir_get_page(thread_current()->pagedir, s)){
    if(!(grow && stack_resized(esp, s))) {
      if(!grow) {
        exit(-1, name);
      }
      exit(-1, "stack was not resized");
    }
  }
  supp_pagedir_set_prohibit(s, prohibit);
  if(prohibit) {
    ASSERT(thread_current()->pagedir);
    ASSERT(pagedir_get_page(thread_current()->pagedir, s));
  }
  if(prohibit) {
    ASSERT((*supp_pagedir_lookup(thread_current()->supp_pagedir, s, false))->flags & PAL_PROHIBIT_CACHE);
  }else {
    ASSERT(!((*supp_pagedir_lookup(thread_current()->supp_pagedir, s, false))->flags & PAL_PROHIBIT_CACHE));
  }
}

static void *get_arg_pointer(void *pointer, int i, int len, bool grow, bool prohibit, const char *name){

  void **p = (void**)pointer + i;
  check_pointer((uint32_t)pointer, p, grow, false, name);
  check_pointer((uint32_t)pointer, (char*)p + 3, grow, false, name);
  check_pointer((uint32_t)pointer, *p, grow, false, name);
  //check_pointer((char*)*p + 3);

  char *ret = (char*)*p;
  check_pointer((uint32_t)pointer, ret, grow, prohibit, name);
  if(len == -1){
    for(;*ret;check_pointer((uint32_t)pointer, ++ret, grow, prohibit, name));
  }else{
    check_pointer((uint32_t)pointer, ret + len, grow, prohibit, name);
    for(;len >= 0; len--, check_pointer((uint32_t)pointer, ret++, grow, prohibit, name));
  }
  void *rett = *p;
  return rett;
}
static int get_arg(void *pointer, int i, bool grow, bool prohibit, const char *name){
  int *p = (((int*)pointer) + (i));
  check_pointer((uint32_t)pointer, p, grow, prohibit, name);
  check_pointer((uint32_t)pointer, (char*)p + 3, grow, prohibit, name);

  return *p;
}

#define ITH_ARG_POINTER(f, i, TYPE, len, grow, prohibit, NAME) ((TYPE)get_arg_pointer((void*)f->esp, i, len, grow, prohibit, NAME))

#define ITH_ARG(f, i, TYPE, grow, prohibit, NAME) ((TYPE)(get_arg((void*)f->esp, i, grow, prohibit, NAME)))

void
syscall_init (void) {
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f)
{
  int sys_call_id = ITH_ARG(f, 0, int, false, true, "sys_call_id");
  uint32_t ret = 23464464;

  static int c = 0;
  switch (sys_call_id){
    /* Projects 2 and later. */
    case SYS_HALT:                   /* Halt the operating system. */
      HIDDEN_MESSAGE = SYS_HALT;
      halt();
      break;
    case SYS_EXIT:                   /* Terminate this process. */
      HIDDEN_MESSAGE = SYS_EXIT;
      exit(ITH_ARG(f, 1, int, false, false, "EXIT1"), "Exit syscall");
      break;
    case SYS_EXEC:                   /* Start another process. */
      HIDDEN_MESSAGE = SYS_EXEC;
      ret = exec(ITH_ARG_POINTER(f, 1, const char *, -1, false, false, "EXEC1"));
      break;
    case SYS_WAIT:                   /* Wait for a child process to die. */
      HIDDEN_MESSAGE = SYS_WAIT;
      ret = wait(ITH_ARG(f, 1, int, false, false,"WAIT1"));
      break;
    case SYS_CREATE:                 /* Create a file. */
      HIDDEN_MESSAGE = SYS_CREATE;
      ret = create_sys(ITH_ARG_POINTER(f, 1, char *, -1, false, true, "CREATE1"),
                   ITH_ARG(f, 2, unsigned int, false, false, "CREATE2"));
      ITH_ARG_POINTER(f, 1, char *, -1, false, false, "CREATE1*");
      break;
    case SYS_REMOVE:                 /* Delete a file. */
      HIDDEN_MESSAGE = SYS_REMOVE;
      ret = remove_sys(ITH_ARG_POINTER(f, 1, char *, -1, false, true, "REMOVE1"));
      ITH_ARG_POINTER(f, 1, char *, -1, false, false, "REMOVE1*");
      break;
    case SYS_OPEN:                   /* Open a file. */
      HIDDEN_MESSAGE = SYS_OPEN;
      ret = open_sys(ITH_ARG_POINTER(f, 1, char *, -1, false, true, "OPEN1"), false);
      ITH_ARG_POINTER(f, 1, char *, -1, false, false, "OPEN1*");
      break;
    case SYS_FILESIZE:               /* Obtain a file's size. */
      HIDDEN_MESSAGE = SYS_FILESIZE;
      ret = filesize_sys(ITH_ARG(f, 1, int, false, false, "FILESIZE1"));
      break;
    case SYS_READ:                   /* Read from a file. */
      HIDDEN_MESSAGE = SYS_READ;
      ret = read_sys_wrapper(ITH_ARG(f, 1, int, false, false,"READ1"),
                 ITH_ARG_POINTER(f, 2, void*, ITH_ARG(f, 3, unsigned int, false, false,"READ33"), true, true,"READ2"),
                 ITH_ARG(f, 3, unsigned int, false, false, "READ3"));
      ITH_ARG_POINTER(f, 2, void*, ITH_ARG(f, 3, unsigned int, false, false,"READ33*"), false, false,"READ2*");
      break;
    case SYS_WRITE:                  /* Write to a file. */
      HIDDEN_MESSAGE = SYS_WRITE;
      ret = write_sys(ITH_ARG(f, 1, int, false, false,"WRITE1"),
                  ITH_ARG_POINTER(f, 2,const void *, ITH_ARG(f, 3, unsigned int, false, false,"WRITE33"), true, true,"WRITE2"),
                  ITH_ARG(f, 3, unsigned int, false, false,"WRITE3"));

      ITH_ARG_POINTER(f, 2,const void *, ITH_ARG(f, 3, unsigned int, false, false,"WRITE33*"), false, false,"WRITE2*");

      break;
    case SYS_SEEK:                   /* Change position in a file. */
      HIDDEN_MESSAGE = SYS_SEEK;
      seek_sys(ITH_ARG(f, 1, int, false, false,"SEEK1"), ITH_ARG(f, 2, unsigned int, false, false,"SEEK2"));
      break;
    case SYS_TELL:                   /* Report current position in a file. */
      HIDDEN_MESSAGE = SYS_TELL;
      ret = tell_sys(ITH_ARG(f, 1, int, false, false,"TELL1"));
      break;
    case SYS_CLOSE:                  /* Close a file. */
      HIDDEN_MESSAGE = SYS_CLOSE;
      close_sys(ITH_ARG(f, 1, int, false, false, "close1"));
      break;
    case SYS_MMAP:
      HIDDEN_MESSAGE = SYS_MMAP;
      ret = mmap_sys(ITH_ARG(f, 1, int, false, false, "MMAP"),
                 (void*)ITH_ARG(f, 2, int, false, false, "MMAP2"), 0, -666, 0);
      break;
    case SYS_MUNMAP:
      HIDDEN_MESSAGE = SYS_MUNMAP;
      c++;
      munmap_sys(ITH_ARG(f, 1, int, false, false, "MUNMAP1"));
      break;
    case SYS_CHDIR:
      HIDDEN_MESSAGE = SYS_CHDIR;
      ret = chdir(ITH_ARG_POINTER(f, 1, char *, -1, false, true, "chdir 1"));
      ITH_ARG_POINTER(f, 1, char *, -1, false, false, "chdir 1*");
      break;
    case SYS_MKDIR:
      HIDDEN_MESSAGE = SYS_MKDIR;
      ret = mkdir(ITH_ARG_POINTER(f, 1, char *, -1, false, true, "mkdir 1"));
      ITH_ARG_POINTER(f, 1, char *, -1, false, false, "mkdir 1*");
      break;
    case SYS_READDIR:
      HIDDEN_MESSAGE = SYS_READDIR;
      ret = readdir(ITH_ARG(f, 1, int, false, false, "readir1"),
                    ITH_ARG_POINTER(f, 2, char *, -1, true, true, "readdir 2"));
      ITH_ARG_POINTER(f, 2, char *, -1, false, false, "readir 2*");
      break;
    case SYS_ISDIR:
      HIDDEN_MESSAGE = SYS_ISDIR;
      ret = isdir(ITH_ARG(f, 1, int, false, false, "isdir1"));
      break;
    case SYS_INUMBER:
      HIDDEN_MESSAGE = SYS_INUMBER;
      ret = inumber(ITH_ARG(f, 1, int, false, false, "isnumber1"));
      break;
    default:
      exit(-1, "Cold not find syscall id(syscall)");
  }

  if(ret != 23464464){
    f->eax = ret;
  }
  HIDDEN_MESSAGE = -1;
}


/* Terminate this process. */
void exit (int status, const char *caller){
  //if(status == -1) PANIC("%d %s %d",status, caller, HIDDEN_MESSAGE);
  struct thread *t = thread_current()->parent_thread;
  if(t != NULL) {
    struct thread_child *tc = thread_set_child_exit_status(t, thread_tid(), status);
    if (tc != NULL) {
      sema_up(&tc->semaphore);
    }
  }

  printf("%s: exit(%d)\n", thread_current()->name, status);
  thread_exit();
}
/* Halt the operating system. */
static void halt(){
  shutdown_power_off();
}
/* Start another process. */
static tid_t exec (const char * cmd_line ){
  tid_t processId = process_execute(cmd_line);

  return processId;
}

/* Wait for a child process to die. */
static int wait (int pid){
  return process_wait(pid);
}

static int read_sys_wrapper (int fd, void * buffer, unsigned size) {
  struct supp_pagedir_entry **ee = supp_pagedir_lookup(thread_current()->supp_pagedir, buffer, false);
  ASSERT(ee);
  ASSERT(*ee);
  if ((*ee)->flags & PAL_READONLY) exit(-1, "could not write in readonly page");
  int ret = read_sys(fd, buffer, size);
  return ret;
}