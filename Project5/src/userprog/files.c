#include "files.h"
#include "../lib/string.h"
#include "../filesys/inode.h"

static int FD_C = 2;
static struct dir *merge_dir(struct dir *active_dir,const char *req);
static void split_str(const char *path, char **s, char **e);
static void destruct_split(char *s, char *e);
static int add_file(struct file *f, struct dir *dir);
static struct user_file_info *find_open_file(int fd);
static bool equals_fd(const struct list_elem *elem, void *fd);
static bool equals_sector(const struct list_elem *elem, void *sector);
static struct user_file_info *find_open_file_with_sector(int sector);
static bool equals_fd(const struct list_elem *elem, void *fd){
  return list_entry (elem, struct user_file_info, link)->fd == *(int*)fd;
}

static struct user_file_info *find_open_file(int fd){
  struct list_elem *e =  list_find(&thread_current()->open_files, equals_fd, (void*)&fd);
  if(e == NULL) return NULL;
  else return list_entry(e, struct user_file_info, link);
}

static bool equals_sector(const struct list_elem *elem, void *sector){
  return file_get_inode(list_entry (elem, struct user_file_info, link)->f)->sector == *(int*)sector;
}

static struct user_file_info *find_open_file_with_sector(int sector){
  struct list_elem *e =  list_find(&thread_current()->open_files, equals_sector, (void*)&sector);
  if(e == NULL) return NULL;
  else return list_entry(e, struct user_file_info, link);
}


static void split_str(const char *path, char **s, char **e){
  ASSERT(s);
  ASSERT(e);

  int i = strlen(path);
  char *A = *s = malloc(i + 1);
  char *B = *e = malloc(i + 1);
  strlcpy(A, path, strlen(path) + 1);
  for(i = i - 1; i >= 0 && A[i] == '/'; A[i] = 0, i--);
  int k = 0;
  for(; i >= 0 && A[i] != '/'; A[i] = 0, i--){
    B[k++] = A[i];
  }
  B[k] = 0;
  for(int i = 0, j = strlen(B) - 1; i < j; i++, j-- ){
    B[i] ^= B[j];
    B[j] ^= B[i];
    B[i] ^= B[j];
  }
  for(i = i - 1; i >= 0 && A[i] == '/'; A[i] = 0, i--);
}

static void destruct_split(char *s, char *e){
  free(s);
  free(e);
}

static struct dir *merge_dir(struct dir *active_dir,const char *req){
  char *request = malloc(strlen(req) + 1);
  strlcpy(request, req, strlen(req) + 1);
  struct dir *work;
  if(request[0] == '/') {
    work = dir_open_root();
  }
  else{
    work = dir_reopen(active_dir);
  }
  char *token, *save_ptr;
  for (token = strtok_r (request, "/", &save_ptr); token != NULL;
       token = strtok_r (NULL, "/", &save_ptr)){

    struct dir *next = NULL;
    if(strlen(token) == 2 && token[0] == '.' && token[1] == '.'){
      next = dir_get_parent_dir(work);
    }else if(strlen(token) == 1 && token[0] == '.') {
      next = dir_reopen(work);
    }else{
      struct inode *tmp = NULL;
      bool is_dir;
      dir_lookup(work, token, &tmp, &is_dir);
      if(tmp != NULL && is_dir){
        next = dir_open(tmp);
      }else{
        if(tmp) {
          inode_close(tmp);
        }
      }
    }
    dir_close(work);
    work = next;
    if(work == NULL){
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
    if(!f || f->dir) ans = -1;
    else ans = file_read(f->f, buffer, size);

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
  if(path == NULL)
    return -1;
  if(strlen(path) == 0)
    return -1;
  int ret_FDC;
  bool is_dir;



  char *A, *B;
  split_str(path, &A, &B);


  struct file *f;
  struct dir * res = merge_dir(thread_current()->active_dir, A);
  if(!res) f = NULL;
  else{
    f = filesys_open(res, B, &is_dir);
    dir_close(res);
  }
  destruct_split(A, B);





  struct dir *dir = NULL;
  if(is_dir && f) dir = dir_open(inode_reopen(file_get_inode(f)));
  if(f == NULL) ret_FDC = -1;
  else {
    if(readonly) file_deny_write(f);
    ret_FDC = add_file(f, dir);
  }
  return ret_FDC;
}
static bool create(const char *prefix, const char *path, unsigned initial_size, bool is_dir){
  bool ret;

  char *A, *B;
  split_str(path, &A, &B);

  struct dir * res = merge_dir(thread_current()->active_dir, A);
  if(!res) ret = false;
  else{
    ret = filesys_create(B, res, initial_size, is_dir);
    dir_close(res);
  }
  destruct_split(A, B);
  return ret;
}

/* Create a file. */
bool create_sys (const char * file , unsigned initial_size ){
  return create("create_sys", file, initial_size, false);
}
/* Delete a file. */
bool remove_sys (const char * path) {
  bool ret;
  char *A, *B;
  split_str(path, &A, &B);

  struct dir * res = merge_dir(thread_current()->active_dir, A);

  if(!res) ret = false;
  else{
    bool is_dir;
    struct inode *inode;
    dir_lookup(res, B, &inode, &is_dir);
    if(!inode) {
      ret = false;
      inode_close(inode);
    }else {
      if(is_dir && find_open_file_with_sector(inode->sector)){
        inode_close(inode);
        ret = false;
      }else if (!is_dir) {
        ret = filesys_remove(res, B);
        inode_close(inode);
      } else {
        char s[NAME_MAX + 1];
        s[0] = 0;
        struct dir *d = dir_open(inode);
        dir_readdir(d, s);
        bool allow = !s[0] && dir_get_inode(d)->sector != dir_get_inode(thread_current()->active_dir)->sector;
        dir_close(d);
        if (allow) {
          ret = filesys_remove(res, B);
        } else {
          ret = false;
        }

      }
    }
  }
  if(res) dir_close(res);
  destruct_split(A, B);
  return ret;
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

  return dir_readdir(f->dir, name);
}

bool isdir (int fd){
  struct user_file_info *f= find_open_file(fd);
  return f && f->dir;
}
int inumber (int fd){
  struct user_file_info *f= find_open_file(fd);
  return file_get_inode(f->f)->sector;
}