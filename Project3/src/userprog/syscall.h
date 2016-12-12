#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

struct lock fileSystem;
void syscall_init (void);
void exit (int status);
#endif /* userprog/syscall.h */
