#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
int HIDDEN_MESSAGE;

void syscall_init (void);
void exit (int status, const char *caller);
#endif /* userprog/syscall.h */
