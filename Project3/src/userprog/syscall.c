#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
//#include <sched.h>
#include "threads/interrupt.h"
#include "devices/shutdown.h"
#include "threads/thread.h"
#include "../threads/thread.h"
#include "process.h"

static void syscall_handler (struct intr_frame *);
static void halt();
static void exit (int status);
static tid_t exec (const char * cmd_line );
static bool create (const char * file , unsigned initial_size );
static bool remove (const char * file);
static int read (int fd , void * buffer , unsigned size );
static int write (int fd , const void * buffer , unsigned size );
static void seek (int fd , unsigned position );
static unsigned tell (int fd);
static void close (int fd );


void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  printf ("system call!\n");
  int sys_call_id = *((int*)f->esp);
  uint32_t ret = 23464464;

  switch (sys_call_id){
    /* Projects 2 and later. */
    case SYS_HALT:                   /* Halt the operating system. */
      halt();
      break;
    case SYS_EXIT:                   /* Terminate this process. */
      break;
    case SYS_EXEC:                   /* Start another process. */
      break;
    case SYS_WAIT:                   /* Wait for a child process to die. */
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
      break;
    case SYS_SEEK:                   /* Change position in a file. */
      break;
    case SYS_TELL:                   /* Report current position in a file. */
      break;
    case SYS_CLOSE:                  /* Close a file. */
      break;
  }
  if(ret != 23464464){
    f->eax = ret;
  }

  thread_exit ();
}

static void exit (int status){
    thread_current()->exit_status = status;
    thread_exit();

}

static void halt(){
	shutdown_power_off();
}

static tid_t exec (const char * cmd_line ){
    tid_t processId = process_execute(cmd_line);
    if(processId != TID_ERROR) return processId;
    return -1;
}

int wait (int pid ){
   return process_wait(pid);
}

static bool create (const char * file , unsigned initial_size ){
    return filesys_create(file, initial_size);
}
static bool remove (const char * file ) {
    return filesys_remove(file);
}
static int filesize (int fd ){

}

static int read (int fd , void * buffer , unsigned size ){

}
static int write (int fd , const void * buffer , unsigned size ){

}

static void seek (int fd , unsigned position ){

}

static unsigned tell (int fd){


}

void close (int fd ){


}