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

static struct user_file_info *find_open_file(int fd);
static void syscall_handler (struct intr_frame *);
static bool equals_fd(const struct list_elem *elem, void *fd);

/* Projects 2 and later. */
static void halt (void);
static int exec (const char *file);
static int wait (int);
static bool create (const char *file, unsigned initial_size);
static bool remove (const char *file);
static int open (const char *file);
static int filesize (int fd);
static int read (int fd, void *buffer, unsigned length);
static int write (int fd, const void *buffer, unsigned length);
static void seek (int fd, unsigned position);
static unsigned tell (int fd);
static void close (int fd);
static void check_pointer(void *s);

static int FD_C = 2;
bool check_pointer_nonsastik(void *s){
  if((unsigned int)s >= (unsigned  int)PHYS_BASE)
    return 0;
  if(pagedir_get_page(thread_current()->pagedir, s) == (void*)0)
    return 0;
  return 1;
}
static void check_pointer(void *s){
  if(!check_pointer_nonsastik(s)) exit(-1);
}

static void *get_arg_pointer(struct intr_frame *f, int i, int len){
  void **p = (((void**)(f)->esp) + (i));
  check_pointer(p);
  check_pointer((char*)p + 3);
  check_pointer(*p);
  //check_pointer((char*)*p + 3);

  char *ret = (char*)*p;
  check_pointer(ret);
  if(len == -1){
    for(;*ret;check_pointer(++ret));
  }else{
    check_pointer(ret + len);
    for(;len >= 0; len--, check_pointer(ret++));
  }

  return *p;
}
static int get_arg(struct intr_frame *f, int i){
  int *p = (((int*)(f)->esp) + (i));
  check_pointer(p);
  check_pointer((char*)p + 3);

  return *p;
}

#define ITH_ARG_POINTER(f, i, TYPE, len) ((TYPE)get_arg_pointer(f, i, len))

#define ITH_ARG(f, i, TYPE) ((TYPE)(get_arg(f, i)))



void
syscall_init (void) {
  lock_init(&fileSystem);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}
static const char*getname(int sys_call_id);
static const char*getname(int sys_call_id){
  switch (sys_call_id){
    /* Projects 2 and later. */
    case SYS_HALT:                   /* Halt the operating system. */
      return "HALT";
    case SYS_EXIT:                   /* Terminate this process. */
      return "EXIT";
    case SYS_EXEC:                   /* Start another process. */
      return "EXEC";
    case SYS_WAIT:                   /* Wait for a child process to die. */
      return "WAIT";
    case SYS_CREATE:                 /* Create a file. */
      return "CREATE";
    case SYS_REMOVE:                 /* Delete a file. */
      return "REMOVE";
    case SYS_OPEN:                   /* Open a file. */
      return "OPEN";
    case SYS_FILESIZE:               /* Obtain a file's size. */
      return "FILESIZE";
    case SYS_READ:                   /* Read from a file. */
      return "READ";
    case SYS_WRITE:                  /* Write to a file. */
      return "WRITE";
    case SYS_SEEK:                   /* Change position in a file. */
      return "SEEK";
    case SYS_TELL:                   /* Report current position in a file. */
      return "TELL";
    case SYS_CLOSE:                  /* Close a file. */
      return "CLOSE";
    default:
      return "NONE WTF";

  }
}

static void
syscall_handler (struct intr_frame *f)
{
  int sys_call_id = ITH_ARG(f, 0, int);
  uint32_t ret = 23464464;
  switch (sys_call_id){
    /* Projects 2 and later. */
    case SYS_HALT:                   /* Halt the operating system. */
      halt();
      break;
    case SYS_EXIT:                   /* Terminate this process. */
      exit(ITH_ARG(f, 1, int));
      break;
    case SYS_EXEC:                   /* Start another process. */
      ret = exec(ITH_ARG_POINTER(f, 1, const char *, -1));
      break;
    case SYS_WAIT:                   /* Wait for a child process to die. */
      ret = wait(ITH_ARG(f, 1, int));
      break;
    case SYS_CREATE:                 /* Create a file. */
      ret = create(ITH_ARG_POINTER(f, 1, char *, -1), ITH_ARG(f, 2, unsigned int));
      break;
    case SYS_REMOVE:                 /* Delete a file. */
      ret = remove(ITH_ARG_POINTER(f, 1, char *, -1));
      break;
    case SYS_OPEN:                   /* Open a file. */
      ret = open(ITH_ARG_POINTER(f, 1, char *, -1));
      break;
    case SYS_FILESIZE:               /* Obtain a file's size. */
      ret = filesize(ITH_ARG(f, 1, int));
      break;
    case SYS_READ:                   /* Read from a file. */
      ret = read(ITH_ARG(f, 1, int), ITH_ARG_POINTER(f, 2, void*, ITH_ARG(f, 3, unsigned int)), ITH_ARG(f, 3, unsigned int));
      break;
    case SYS_WRITE:                  /* Write to a file. */
      ret = write(ITH_ARG(f, 1, int), ITH_ARG_POINTER(f, 2,const void *, ITH_ARG(f, 3, unsigned int)), ITH_ARG(f, 3, unsigned int));
      break;
    case SYS_SEEK:                   /* Change position in a file. */
      seek(ITH_ARG(f, 1, int), ITH_ARG(f, 2, unsigned int));
      break;
    case SYS_TELL:                   /* Report current position in a file. */
      ret = tell(ITH_ARG(f, 1, int));
      break;
    case SYS_CLOSE:                  /* Close a file. */
      close(ITH_ARG(f, 1, int));
      break;
    default:
      exit(-1);
  }
  if(ret != 23464464){
    f->eax = ret;
  }
}
/* Terminate this process. */
void exit (int status){
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
/* Open a file. */
static int open (const char *file_name){
  int ret_FDC;
  lock_acquire(&fileSystem);
  struct file *f = filesys_open(file_name);
  if(f == NULL) ret_FDC = -1;
  else {
    struct user_file_info *info = malloc(sizeof(struct user_file_info));
    info->f = f;
    ret_FDC = info->fd = FD_C++;
    list_push_back(&thread_current()->open_files, &info->link);
  }
  lock_release((&fileSystem));
  return ret_FDC;
}
/* Wait for a child process to die. */
static int wait (int pid){
   return process_wait(pid);
}
/* Create a file. */
static bool create (const char * file , unsigned initial_size ){
  bool ans;
  lock_acquire(&fileSystem);
  ans = filesys_create(file, initial_size);
  lock_release((&fileSystem));
  return ans;
}
/* Delete a file. */
static bool remove (const char * file) {
  bool ans;
  lock_acquire(&fileSystem);
  ans = filesys_remove(file);;
  lock_release((&fileSystem));
  return ans;
}


static bool equals_fd(const struct list_elem *elem, void *fd){
  return list_entry (elem, struct user_file_info, link)->fd == *(int*)fd;
}
static struct user_file_info *find_open_file(int fd){
  struct list_elem *e =  list_find(&thread_current()->open_files, equals_fd, (void*)&fd);
  if(e == NULL) return NULL;
  else return list_entry(e, struct user_file_info, link);
}
/* Obtain a file's size. */
static int filesize (int fd){
  int ans;
  lock_acquire(&fileSystem);
  struct user_file_info *f= find_open_file(fd);
  if(f == NULL) ans = -1;
  else ans = file_length(f->f);
  lock_release((&fileSystem));
  return ans;
}
/* Read from a file. */
static int read (int fd, void * buffer, unsigned size){
  if(fd == 0){
    unsigned i;
    char *s = (char*)buffer;
    for(i = 0; i < size; i++, s++) *s = input_getc();
    return size;
  }else{
    int ans;
    lock_acquire(&fileSystem);
    struct user_file_info *f= find_open_file(fd);
    if(f == NULL) ans = -1;
    else ans = file_read(f->f, buffer, size);
    lock_release((&fileSystem));
    return ans;
  }
}
/* Write to a file. */
static int write (int fd , const void * buffer , unsigned size ){
  if(1 == fd){
    putbuf(buffer, size);
    return size;
  }else{
    int ans;
    lock_acquire(&fileSystem);
    struct user_file_info *f= find_open_file(fd);
    if(f == NULL) ans = -1;
    else ans = file_write(f->f, buffer, size);
    lock_release((&fileSystem));
    return ans;
  }
}
/* Change position in a file. */
static void seek (int fd, unsigned position){
  lock_acquire(&fileSystem);
  struct user_file_info *f= find_open_file(fd);
  if(f != NULL) file_seek(f->f, position);
  lock_release((&fileSystem));
}
/* Report current position in a file. */
static unsigned tell (int fd){
  int ans = 0;
  lock_acquire(&fileSystem);
  struct user_file_info *f= find_open_file(fd);
  if(f != NULL) file_tell(f->f);
  lock_release((&fileSystem));
  return ans;
}
/* Close a file. */
static void close (int fd){
  lock_acquire(&fileSystem);
  struct user_file_info *f= find_open_file(fd);
  if(f != NULL) {
    file_close(f->f);
    list_remove(&f->link);
    free(f);
  }
  lock_release((&fileSystem));

}