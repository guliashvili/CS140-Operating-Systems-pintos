#include "files.h"
#include "mmap.h"


static int FD_C = 2;
static int add_file(struct file *f);
static struct user_file_info *find_open_file(int fd);
static bool equals_fd(const struct list_elem *elem, void *fd);

static bool equals_fd(const struct list_elem *elem, void *fd){
  return list_entry (elem, struct user_file_info, link)->fd == *(int*)fd;
}
static struct user_file_info *find_open_file(int fd){
  struct list_elem *e =  list_find(&thread_current()->open_files, equals_fd, (void*)&fd);
  if(e == NULL) return NULL;
  else return list_entry(e, struct user_file_info, link);
}


/* Obtain a file's size. */
int filesize_sys (int fd){
  int ans;
  lock_acquire(&fileSystem);
  struct user_file_info *f= find_open_file(fd);
  if(f == NULL) ans = -1;
  else ans = file_length(f->f);
  lock_release((&fileSystem));
  return ans;
}
/* Read from a file. */
int read_sys (int fd, void * buffer, unsigned size){
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
    else{
      void *vaddr = buffer;
      ans = file_read(f->f, buffer, size);
    }
    lock_release((&fileSystem));
    return ans;
  }
}
/* Write to a file. */
int write_sys (int fd , const void * buffer , unsigned size ){
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
void seek_sys (int fd, unsigned position){
  lock_acquire(&fileSystem);
  struct user_file_info *f= find_open_file(fd);
  if(f != NULL) file_seek(f->f, position);
  lock_release((&fileSystem));
}
/* Report current position in a file. */
unsigned tell_sys (int fd){
  int ans = 0;
  lock_acquire(&fileSystem);
  struct user_file_info *f= find_open_file(fd);
  if(f != NULL) file_tell(f->f);
  lock_release((&fileSystem));
  return ans;
}
/* Close a file. */
void close_sys (int fd){
  lock_acquire(&fileSystem);
  struct user_file_info *f = find_open_file(fd);
  if (f != NULL) {
    file_close(f->f);
    list_remove(&f->link);
    free(f);
  }
  lock_release(&fileSystem);
}

static int add_file(struct file *f){
  ASSERT(fileSystem.holder == thread_current());

  struct user_file_info *info = malloc(sizeof(struct user_file_info));
  info->f = f;
  info->fd = FD_C++;
  list_push_back(&thread_current()->open_files, &info->link);

  return info->fd;
}

/* Open a file. */
int open_sys (const char *file_name, bool readonly){
  int ret_FDC;
  lock_acquire(&fileSystem);
  struct file *f = filesys_open(file_name);
  if(f == NULL) ret_FDC = -1;
  else {
    if(readonly) file_deny_write(f);
    ret_FDC = add_file(f);
  }
  lock_release((&fileSystem));
  return ret_FDC;
}

/* Create a file. */
bool create_sys (const char * file , unsigned initial_size ){
  bool ans;
  lock_acquire(&fileSystem);
  ans = filesys_create(file, initial_size);
  lock_release((&fileSystem));
  return ans;
}
/* Delete a file. */
bool remove_sys (const char * file) {
  bool ans;
  lock_acquire(&fileSystem);
  ans = filesys_remove(file);
  lock_release((&fileSystem));
  return ans;
}

int file_reopen_sys(int fd){
  int ans;
  lock_acquire(&fileSystem);
  struct user_file_info *f = find_open_file(fd);
  if(f == NULL) ans = -1;
  else{
    struct file *file = file_reopen(f->f);
    file_seek(file, 0);
    ans = add_file(file);
  }
  lock_release((&fileSystem));
  return ans;
}