#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);
void exit (int status);
void seek_sys (int fd, unsigned position);
int read_sys (int fd, void * buffer, unsigned size);
int write_sys (int fd, const void *buffer, unsigned length);
#endif /* userprog/syscall.h */
