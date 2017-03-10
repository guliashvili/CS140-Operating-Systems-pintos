#include "filesys/directory.h"
#include <stdio.h>
#include <string.h>
#include <list.h>
#include "filesys/filesys.h"
#include "filesys/inode.h"
#include "threads/malloc.h"
#include "../threads/gio_synch.h"
#include "inode.h"
#include "../threads/malloc.h"
#include "../tests/filesys/base/syn-read.h"

#define REDUCEE 1

struct rw_lock dir_locks_list[SECTOR_NUM / REDUCEE];
static struct rw_lock *get_lock(int sector);
static struct rw_lock *get_lock(int sector){
  ASSERT(sector / REDUCEE < SECTOR_NUM / REDUCEE);
  return dir_locks_list + sector / REDUCEE;
}

void dir_init(void){
  for(int i = 0; i < SECTOR_NUM / REDUCEE; i++) rw_lock_init(dir_locks_list + i);
}
/* A directory. */
struct dir 
  {
    struct inode *inode;                /* Backing store. */
    off_t pos;                          /* Current position. */
  };

/* A single directory entry. */
struct dir_entry 
  {
    block_sector_t inode_sector;        /* Sector number of header. */
    char name[NAME_MAX + 1];            /* Null terminated file name. */
    bool in_use;                        /* In use or free? */
    bool is_dir;
  };

/* Creates a directory with space for ENTRY_CNT entries in the
   given SECTOR.  Returns true if successful, false on failure. */
bool
dir_create (block_sector_t sector, size_t entry_cnt UNUSED) {
  return inode_create(sector, 0);
}

/* Opens and returns the directory for the given INODE, of which
   it takes ownership.  Returns a null pointer on failure. */
struct dir *
dir_open (struct inode *inode)
{
  struct dir *dir = calloc (1, sizeof *dir);
  if (inode != NULL && dir != NULL)
    {
      dir->inode = inode;
      dir->pos = sizeof(block_sector_t);

      return dir;
    }
  else
    {
      inode_close (inode);
      free (dir);
      return NULL;
    }
}

/* Opens the root directory and returns a directory for it.
   Return true if successful, false on failure. */
struct dir *
dir_open_root (void)
{
  return dir_open (inode_open (ROOT_DIR_SECTOR));
}

/* Opens and returns a new directory for the same inode as DIR.
   Returns a null pointer on failure. */
struct dir *
dir_reopen (struct dir *dir)
{
  return dir_open (inode_reopen (dir->inode));
}

/* Destroys DIR and frees associated resources. */
void
dir_close (struct dir *dir)
{
  if (dir != NULL)
    {
      inode_close (dir->inode);
      free (dir);
    }
}

/* Returns the inode encapsulated by DIR. */
struct inode *
dir_get_inode (struct dir *dir) 
{
  return dir->inode;
}

/* Searches DIR for a file with the given NAME.
   If successful, returns true, sets *EP to the directory entry
   if EP is non-null, and sets *OFSP to the byte offset of the
   directory entry if OFSP is non-null.
   otherwise, returns false and ignores EP and OFSP. */
static bool
lookup (const struct dir *dir, const char *name,
        struct dir_entry *ep, off_t *ofsp) 
{
  struct dir_entry e;
  size_t ofs;
  
  ASSERT (dir != NULL);
  ASSERT (name != NULL);

  for (ofs = sizeof(block_sector_t); (int)ofs < inode_length(dir->inode);
       ofs += sizeof e) {
    if(inode_read_at (dir->inode, &e, sizeof e, ofs) != sizeof e) break;
    if (e.in_use && !strcmp(name, e.name)) {
      if (ep != NULL)
        *ep = e;
      if (ofsp != NULL)
        *ofsp = ofs;
      return true;
    }
  }
  return false;
}

/* Searches DIR for a file with the given NAME
   and returns true if one exists, false otherwise.
   On success, sets *INODE to an inode for the file, otherwise to
   a null pointer.  The caller must close *INODE. */
bool
dir_lookup (const struct dir *dir, const char *name,
            struct inode **inode, bool *is_dir)
{
  struct dir_entry e;

  ASSERT (dir != NULL);
  ASSERT (name != NULL);
  r_lock_acquire(get_lock(dir->inode->sector));
  if (lookup (dir, name, &e, NULL)) {
    *inode = inode_open(e.inode_sector);
    if(is_dir) *is_dir = e.is_dir;
  } else
    *inode = NULL;
  r_lock_release(get_lock(dir->inode->sector));
  return *inode != NULL;
}

/* Adds a file named NAME to DIR, which must not already contain a
   file by that name.  The file's inode is in sector
   INODE_SECTOR.
   Returns true if successful, false on failure.
   Fails if NAME is invalid (i.e. too long) or a disk or memory
   error occurs. */
bool
dir_add (struct dir *dir, const char *name, block_sector_t inode_sector, bool is_dir)
{
  struct dir_entry e;
  off_t ofs;
  bool success = false;
  ASSERT (dir != NULL);
  ASSERT (name != NULL);

  /* Check NAME for validity. */
  if (*name == '\0' || strlen (name) > NAME_MAX)
    return false;

  w_lock_acquire(get_lock(dir->inode->sector));

  /* Check that NAME is not in use. */
  if (lookup (dir, name, NULL, NULL))
    goto done;

  /* Set OFS to offset of free slot.
     If there are no free slots, then it will be set to the
     current end-of-file.
     
     inode_read_at() will only return a short read at end of file.
     Otherwise, we'd need to verify that we didn't get a short
     read due to something intermittent such as low memory. */

  for (ofs = sizeof(block_sector_t); inode_read_at (dir->inode, &e, sizeof e, ofs) == sizeof e;
       ofs += sizeof e) {
    if (!e.in_use)
      break;
  }

  /* Write slot. */
  e.in_use = true;
  e.is_dir = is_dir;
  strlcpy (e.name, name, sizeof e.name);
  e.inode_sector = inode_sector;
  success = inode_write_at (dir->inode, &e, sizeof e, ofs) == sizeof e;
  ASSERT(success);

  if(success && is_dir){
    struct inode *inode = inode_open(e.inode_sector);
    ASSERT(inode);
    if(inode_write_at(inode, &dir->inode->sector, sizeof(dir->inode->sector), 0) != sizeof(dir->inode->sector)){
      success = false;
    }
    inode_close(inode);
  }

 done:
  w_lock_release(get_lock(dir->inode->sector));
  return success;
}

/* Removes any entry for NAME in DIR.
   Returns true if successful, false on failure,
   which occurs only if there is no file with the given NAME. */
bool
dir_remove (struct dir *dir, const char *name) 
{
  struct dir_entry e;
  struct inode *inode = NULL;
  bool success = false;
  off_t ofs;

  ASSERT (dir != NULL);
  ASSERT (name != NULL);

  w_lock_acquire(get_lock(dir->inode->sector));

  /* Find directory entry. */
  if (!lookup (dir, name, &e, &ofs))
    goto done;

  /* Open inode. */
  inode = inode_open (e.inode_sector);
  if (inode == NULL || (e.is_dir && inode->open_cnt > 1))
    goto done;
  /* Erase directory entry. */
  e.in_use = false;
  if (inode_write_at (dir->inode, &e, sizeof e, ofs) != sizeof e) {
    goto done;
  }

  /* Remove inode. */
  inode_remove (inode);
  success = true;

 done:
  inode_close (inode);
  w_lock_release(get_lock(dir->inode->sector));
  return success;
}

/* Reads the next directory entry in DIR and stores the name in
   NAME.  Returns true if successful, false if the directory
   contains no more entries. */
bool
dir_readdir (struct dir *dir, char name[NAME_MAX + 1])
{
  r_lock_acquire(get_lock(dir->inode->sector));
  struct dir_entry e;
  while (inode_read_at (dir->inode, &e, sizeof e, dir->pos) == sizeof e){
    dir->pos += sizeof e;
    if (e.in_use)
    {
      strlcpy (name, e.name, NAME_MAX + 1);
      r_lock_release(get_lock(dir->inode->sector));
      return true;
    }
  }
  r_lock_release(get_lock(dir->inode->sector));
  return false;
}

struct dir * dir_get_parent_dir(struct dir *dir){
  block_sector_t sector;
  if(dir->inode->sector == ROOT_DIR_SECTOR) sector = ROOT_DIR_SECTOR;
  else
    ASSERT(inode_read_at(dir->inode, &sector, sizeof(block_sector_t), 0) == sizeof(block_sector_t));

  return dir_open(inode_open(sector));
}