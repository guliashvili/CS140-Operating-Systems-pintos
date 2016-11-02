#include "userprog/syscall.h"
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

static void syscall_handler (struct intr_frame *);
static void halt(void);
static void exit (int status);
static tid_t exec (const char * cmd_line );
static bool create (const char * file , unsigned initial_size );
static bool remove_remove (const char * file);
static int read (int fd , void * buffer , unsigned size );
static int write (int fd , const void * buffer , unsigned size );
static void seek (int fd , unsigned position );
static unsigned tell (int fd);
static void close (int fd );
static int get_arg(struct intr_frame *f, int i);
static void *get_arg_pointer(struct intr_frame *f, int i);
static void check_pointer(void *s);
static int wait (int pid);

static void check_pointer(void *s){
  if((unsigned int)s < (unsigned  int)0x08048000)
    exit(-1);
  if((unsigned int)s >= (unsigned  int)PHYS_BASE)
    exit(-1);
  if(pagedir_get_page(thread_current()->pagedir, s) == NULL)
    exit(-1);
}

static void *get_arg_pointer(struct intr_frame *f, int i){
  void **p = (((void**)(f)->esp) + (i));
  check_pointer(p);
  check_pointer((char*)p + 3);
  check_pointer(*p);
  check_pointer((char*)*p + 3);

  return *p;
}
static int get_arg(struct intr_frame *f, int i){
  int *p = (((int*)(f)->esp) + (i));
  check_pointer(p);
  check_pointer((char*)p + 3);

  return *p;
}

#define ITH_ARG_POINTER(f, i, TYPE) ((TYPE)get_arg_pointer(f, i))

#define ITH_ARG(f, i, TYPE) ((TYPE)(get_arg(f, i)))


struct lock fileSystem;
struct fileInfo{
    int fileId;
    struct list_elem elem;
    struct lock fileLock;
};

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
  char *file_name;char *tmp;
  switch (sys_call_id){
    /* Projects 2 and later. */
    case SYS_HALT:                   /* Halt the operating system. */
      halt();
      break;
    case SYS_EXIT:                   /* Terminate this process. */
      exit(ITH_ARG(f, 1, int));
      break;
    case SYS_EXEC:                   /* Start another process. */
      ret = exec(ITH_ARG_POINTER(f, 1, const char *));
      break;
    case SYS_WAIT:                   /* Wait for a child process to die. */
      ret = wait(ITH_ARG(f, 1, int));
      break;
    case SYS_CREATE:                 /* Create a file. */
      break;
    case SYS_REMOVE:                 /* Delete a file. */
      break;
    case SYS_OPEN:                   /* Open a file. */
      break;
    case SYS_FILESIZE:               /* Obtain a file's size. */
      break;
    case SYS_READ:                   /* Read from a file. */
      break;
    case SYS_WRITE:                  /* Write to a file. */
      ret = write(ITH_ARG(f, 1, int), ITH_ARG_POINTER(f, 2,const void *), ITH_ARG(f, 3, unsigned int));
      break;
    case SYS_SEEK:                   /* Change position in a file. */
      break;
    case SYS_TELL:                   /* Report current position in a file. */
      break;
    case SYS_CLOSE:                  /* Close a file. */
      break;
    default:
      exit(-1);
  }
  if(ret != 23464464){
    f->eax = ret;
  }
}

static void exit (int status){
  struct thread *t = thread_current()->parent_thread;
  if(t != NULL){
    struct thread_child* tc = thread_set_child_exit_status(t, thread_tid(), status);
    ASSERT(tc);
    sema_up(&tc->semaphore);
  }

  printf("%s: exit(%d)\n", thread_current()->name, status);

  thread_exit();
}

static void halt(){
	shutdown_power_off();
}

static tid_t exec (const char * cmd_line ){
  tid_t processId = process_execute(cmd_line);


  return processId;
}

static int wait (int pid){
   return process_wait(pid);
}

static bool create (const char * file , unsigned initial_size ){
    return filesys_create(file, initial_size);
}
static bool remove_remove (const char * file ) {
  lock_acquire(&fileSystem);
  struct fileInfo * foundFile;
  //foundFile = findFile()
  if(foundFile == NULL){

  }
  else{
    lock_acquire(&foundFile->fileLock);
  }
  lock_release((&fileSystem));
  return filesys_remove(file);
}
static int filesize (int fd UNUSED){

}

static int read (int fd UNUSED, void * buffer UNUSED, unsigned size UNUSED){

}
static int write (int fd , const void * buffer , unsigned size ){
  if(1 == fd){
    putbuf(buffer, size);
    return size;
  }
  return -1;

}

static void seek (int fd UNUSED, unsigned position UNUSED){

}

static unsigned tell (int fd UNUSED){


}

static void close (int fd UNUSED){


}