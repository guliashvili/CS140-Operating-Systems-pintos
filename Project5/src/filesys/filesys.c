#include "filesys/filesys.h"
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include "filesys/file.h"
#include "filesys/free-map.h"
#include "filesys/inode.h"
#include "filesys/directory.h"
#include "cached_block.h"
#include "../filesys/directory.h"

/* Partition that contains the file system. */
struct cached_block *fs_device_cached;

static void do_format (void);

/* Initializes the file system module.
   If FORMAT is true, reformats the file system. */
void
filesys_init (bool format) 
{
  fs_device_cached = cached_block_init(block_get_role (BLOCK_FILESYS), 64);
  if (fs_device_cached == NULL)
    PANIC ("No file system device found, can't initialize file system.");

  free_map_init ();

  if (format) 
    do_format ();

  free_map_open ();
}

/* Shuts down the file system module, writing any unwritten data
   to disk. */
void
filesys_done (void) 
{
  fflush_all(fs_device_cached);
  free_map_close ();
}

/* Creates a file named NAME with the given INITIAL_SIZE.
   Returns true if successful, false otherwise.
   Fails if a file named NAME already exists,
   or if internal memory allocation fails. */
bool
filesys_create (const char *name,struct dir *dir, off_t initial_size, bool is_dir)
{
  if(strlen(name) == 1 && *name == '.')
    return false;
  if(strlen(name) == 2 && *name == '.' && name[1] == '.')
    return false;
  block_sector_t inode_sector = 0;
  bool success = (dir != NULL
                  && free_map_allocate (&inode_sector)
                  && inode_create (inode_sector, initial_size)
                  && dir_add (dir, name, inode_sector, is_dir));
  if (!success && inode_sector != 0) 
    free_map_release (inode_sector);

  return success;
}

/* Opens the file with the given NAME.
   Returns the new file if successful or a null pointer
   otherwise.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
struct file *
filesys_open (struct dir *dir, const char *name, bool *is_dir)
{
  struct inode *inode = NULL;

  if (dir != NULL) {
    dir_lookup(dir, name, &inode, is_dir);
  }
  if(!inode && (strlen(name) == 0 || (strlen(name) == 1 && name[0] == '.') )){
    *is_dir = true;
    inode = dir_get_inode(dir);
  }

  return file_open (inode);
}

/* Deletes the file named NAME.
   Returns true if successful, false on failure.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
bool
filesys_remove (struct dir *dir,const char *name)
{
  bool success = dir != NULL && dir_remove (dir, name);

  return success;
}

/* Formats the file system. */
static void
do_format (void)
{
  printf ("Formatting file system...");
  free_map_create ();
  if (!dir_create (ROOT_DIR_SECTOR, 16))
    PANIC ("root directory creation failed");
  free_map_close ();
  printf ("done.\n");
}
