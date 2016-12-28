#include "files.h"
#include "../lib/string.h"
#include "../filesys/inode.h"

static int FD_C = 2;
static int add_file(struct file *f, struct dir *dir);
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
struct dir *merge_dir(struct dir *active_dir,const char *req){
  char *request = malloc(strlen(req) + 1);
  strlcpy(request, req, strlen(req) + 1);
  struct dir *work;
  if(request[0] == '/') {
    //printf("merge_dir: dir_open_root()\n");
    work = dir_open_root();
  }
  else{
    //printf("merge_dir: dir_reopen(active_dir)\n");
    work = dir_reopen(active_dir);
  }
  bool is_it = strlen(request) > 2;
  char *token, *save_ptr;
  for (token = strtok_r (request, "/", &save_ptr); token != NULL;
       token = strtok_r (NULL, "/", &save_ptr)){

    struct dir *next = NULL;
    if(strlen(token) == 2 && token[0] == '.' && token[1] == '.'){
      //printf("merge_dir: back ..\n");
      next = dir_get_parent_dir(work);
    }else if(strlen(token) == 1 && token[0] == '.') {
      //printf("merge_dir: same .\n");
      next = dir_reopen(work);
    }else{
      struct inode *tmp = NULL;
      bool is_dir;
      dir_lookup(work, token, &tmp, &is_dir);
      if(tmp != NULL && is_dir){
        //printf("found dir with name %s\n", token);
        next = dir_open(tmp);
      }else{
        if(tmp) {
          //printf("could find this(%s) name but was file\n",token);
          inode_close(tmp);
        }else{
          //printf("could not find this(%s) name\n", token);
        }
      }
    }
    dir_close(work);
    work = next;
    if(work == NULL){
      //printf("was null \n");
      free(request);
      return NULL;
    }
  }
  free(request);
  return work;
}
/* Obtain a file's size. */
int filesize_sys (int fd){
  int ans;
  struct user_file_info *f= find_open_file(fd);
  if(f == NULL) ans = -1;
  else ans = file_length(f->f);
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
    struct user_file_info *f= find_open_file(fd);
    if(f == NULL) ans = -1;
    else{
      ans = file_read(f->f, buffer, size);
    }
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
    struct user_file_info *f= find_open_file(fd);
    if(!f || f->dir) ans = -1;
    else ans = file_write(f->f, buffer, size);
    return ans;
  }
}
/* Change position in a file. */
void seek_sys (int fd, unsigned position){
  struct user_file_info *f= find_open_file(fd);
  if(f != NULL) file_seek(f->f, position);
}
/* Report current position in a file. */
unsigned tell_sys (int fd){
  int ans = 0;
  struct user_file_info *f= find_open_file(fd);
  if(f != NULL) ans = file_tell(f->f);
  return ans;
}
/* Close a file. */
void close_sys (int fd){
  struct user_file_info *f = find_open_file(fd);
  if (f != NULL) {
    file_close(f->f);
    if(f->dir) dir_close(f->dir);
    list_remove(&f->link);
    free(f);
  }
}

static int add_file(struct file *f, struct dir *dir){
  struct user_file_info *info = malloc(sizeof(struct user_file_info));
  info->f = f;
  info->fd = __sync_fetch_and_add(&FD_C, 1);
  info->dir = dir;
  list_push_back(&thread_current()->open_files, &info->link);

  return info->fd;
}

/* Open a file. */
int open_sys (const char *path, bool readonly){
  //printf("%s active inode %d\n","open_sys: ", dir_get_inode(thread_current()->active_dir)->sector);
  if(path == NULL)
    return -1;
  //printf("open_sys:  %s %d\n", path, readonly);
  int ret_FDC;
  bool is_dir;






  struct file *f;
  const char *prefix = "open_sys: ";
  //printf("%s active inode %d\n",prefix, dir_get_inode(thread_current()->active_dir)->sector);
  char *dir_non_c = malloc(strlen(path) + 1);
  strlcpy(dir_non_c, path, strlen(path) + 1);
  //printf("%s: full %s %s\n",prefix, dir_non_c, path);
  int i;
  for(i = strlen(dir_non_c) - 1; i >= 0 && dir_non_c[i] == '/'; dir_non_c[i] = 0, i--);
  for(; i >= 0 && dir_non_c[i] != '/'; dir_non_c[i] = 0, i--);
  for(int j = i; j >= 0 && dir_non_c[j] == '/'; dir_non_c[j] = 0, j--);
  //printf("%s: the rest %s\n",prefix, dir_non_c);
  struct dir * res = merge_dir(thread_current()->active_dir, dir_non_c);
  //printf("%s: res %d\n",prefix, res);
  if(!res) f = NULL;
  else{
    strlcpy(dir_non_c, path + i + 1, strlen(path) - i - 1 + 1);
    //printf("%s: %s name %s\n",prefix, is_dir?"folder":"file", dir_non_c);
    f = filesys_open(res, dir_non_c, &is_dir);
    dir_close(res);
  }
  free(dir_non_c);





  struct dir *dir = NULL;
  //printf("open_sys: file %d dir %d\n",f, dir);
  if(is_dir && f) dir = dir_open(file_get_inode(f));
  //if(f == NULL) PANIC("%d %d %d", f, (int)is_dir, dir);
  if(f == NULL) ret_FDC = -1;
  else {
    if(readonly) file_deny_write(f);
    ret_FDC = add_file(f, dir);
  }
  return ret_FDC;
}
static bool create(const char *prefix, const char *path, unsigned initial_size, bool is_dir){
  //printf("%s active inode %d\n",prefix, dir_get_inode(thread_current()->active_dir)->sector);
  bool ret;
  char *dir_non_c = malloc(strlen(path) + 1);
  strlcpy(dir_non_c, path, strlen(path) + 1);
  //printf("%s: full %s %s\n",prefix, dir_non_c, path);
  int i;
  for(i = strlen(dir_non_c) - 1; i >= 0 && dir_non_c[i] == '/'; dir_non_c[i] = 0, i--);
  for(; i >= 0 && dir_non_c[i] != '/'; dir_non_c[i] = 0, i--);
  for(int j = i; j >= 0 && dir_non_c[j] == '/'; dir_non_c[j] = 0, j--);
  //printf("%s: the rest %s\n",prefix, dir_non_c);
  struct dir * res = merge_dir(thread_current()->active_dir, dir_non_c);
  //printf("%s: res %d\n",prefix, res);
  if(!res) ret = false;
  else{
    strlcpy(dir_non_c, path + i + 1, strlen(path) - i - 1 + 1);
    //printf("%s: %s name %s\n",prefix, is_dir?"folder":"file", dir_non_c);
    ret = filesys_create(dir_non_c, res, initial_size, is_dir);
    dir_close(res);
  }
  free(dir_non_c);
  return ret;
}

/* Create a file. */
bool create_sys (const char * file , unsigned initial_size ){
  return create("create_sys", file, initial_size, false);
}
/* Delete a file. */
bool remove_sys (const char * file) {
  bool ans;
  ans = filesys_remove(file);
  
  return ans;
}

int file_reopen_sys(int fd){
  int ans;
  
  struct user_file_info *f = find_open_file(fd);
  if(f == NULL) ans = -1;
  else{
    struct file *file = file_reopen(f->f);
    file_seek(file, 0);
    ans = add_file(file, f->dir ? dir_reopen(f->dir) : NULL);
  }
  
  return ans;
}

bool chdir (const char * dir){
  bool ret;
  //printf("chdir %s\n",dir);
  char *dir_non_c = malloc(strlen(dir) + 1);
  strlcpy(dir_non_c, dir, strlen(dir) + 1);
  struct dir *res = merge_dir(thread_current()->active_dir, dir_non_c);
  if(!res) ret = false;
  else{
    dir_close(thread_current()->active_dir);
    thread_current()->active_dir = res;
    ret = true;
  }
  free(dir_non_c);
  return ret;
}
bool mkdir (const char * dir){
  return create("mkdir: ", dir, 100, true);
}

bool readdir (int fd , char * name){
  struct user_file_info *f= find_open_file(fd);
  if(f == NULL) return false;
  if(!f->dir) return false;

  dir_readdir(f->dir, name);
  return true;
}

bool isdir (int fd){
  struct user_file_info *f= find_open_file(fd);
  return f && f->dir;
}
int inumber (int fd){
  struct user_file_info *f= find_open_file(fd);
  return f && !f->dir;
}