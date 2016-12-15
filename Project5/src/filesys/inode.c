#include "filesys/inode.h"
#include <list.h>
#include <debug.h>
#include <round.h>
#include <string.h>
#include "filesys/filesys.h"
#include "filesys/free-map.h"
#include "threads/malloc.h"
#include "off_t.h"
#include "cached_block.h"
#include "filesys.h"
#include "../threads/thread.h"
#include "../threads/gio_synch.h"

/* Identifies an inode. */
#define INODE_MAGIC 0x494e4f44

#define SECTOR_NUM (8 * 1024 * 1024 / BLOCK_SECTOR_SIZE)

/* On-disk inode.
   Must be exactly BLOCK_SECTOR_SIZE bytes long. */
struct inode_disk
  {
    block_sector_t start;               /* First data sector. */
    off_t length;                       /* File size in bytes. */
    unsigned magic;                     /* Magic number. */
    uint32_t unused[125];               /* Not used. */
  } PACKED;

static off_t inode_length_meta (const struct inode_disk *inode_disk);

/* Returns the number of sectors to allocate for an inode SIZE
   bytes long. */
static inline size_t
bytes_to_sectors (off_t size)
{
  return DIV_ROUND_UP (size, BLOCK_SECTOR_SIZE);
}

/* In-memory inode. */
struct inode 
  {
    block_sector_t sector;              /* Sector number of disk location. */
    int open_cnt;                       /* Number of openers. */
    bool removed;                       /* True if deleted, false otherwise. */
    int deny_write_cnt;                 /* 0: writes ok, >0: deny writes. */
  };


static struct inode *inode_reopen_no_lock (struct inode *inode);

/* Returns the block device sector that contains byte offset POS
   within INODE.
   Returns -1 if INODE does not contain data for a byte at offset
   POS. */
static block_sector_t
byte_to_sector (const struct inode_disk *inode_disk, off_t pos)
{
  ASSERT (inode_disk != NULL);
  if (pos < inode_disk->length)
    return inode_disk->start + pos / BLOCK_SECTOR_SIZE;
  else
    return -1;
}

/* List of open inodes, so that opening a single inode twice
   returns the same `struct inode'. */
static struct inode **inodes;
static struct lock *sectors_lock;
/* Initializes the inode module. */
void
inode_init (void) 
{
  inodes = calloc(SECTOR_NUM, sizeof(struct inode*));
  sectors_lock = malloc(SECTOR_NUM * sizeof(struct lock));
  for(int i = 0; i < SECTOR_NUM; i++){
    lock_init(sectors_lock + i);
  }
}

/* Initializes an inode with LENGTH bytes of data and
   writes the new inode to sector SECTOR on the file system
   device.
   Returns true if successful.
   Returns false if memory or disk allocation fails. */
bool
inode_create (block_sector_t sector, off_t length)
{
  ASSERT(sector < SECTOR_NUM);
  struct inode_disk *disk_inode = NULL;
  bool success = false;

  ASSERT (length >= 0);

  /* If this assertion fails, the inode structure is not exactly
     one sector in size, and you should fix that. */
  ASSERT (sizeof *disk_inode == BLOCK_SECTOR_SIZE);

  disk_inode = calloc (1, sizeof *disk_inode);
  if (disk_inode != NULL)
    {
      size_t sectors = bytes_to_sectors (length);
      disk_inode->length = length;
      disk_inode->magic = INODE_MAGIC;
      if (free_map_allocate (sectors, &disk_inode->start)) 
        {
          cached_block_write (fs_device_cached, sector, disk_inode, 0);
          if (sectors > 0) 
            {
              static char zeros[BLOCK_SECTOR_SIZE];
              size_t i;
              
              for (i = 0; i < sectors; i++) 
                cached_block_write (fs_device_cached, disk_inode->start + i, zeros, 0);
            }
          success = true; 
        } 
      free (disk_inode);
    }
  return success;
}

/* Reads an inode from SECTOR
   and returns a `struct inode' that contains it.
   Returns a null pointer if memory allocation fails. */
struct inode *
inode_open (block_sector_t sector)
{
  ASSERT(sector < SECTOR_NUM);
  struct inode *inode;

  lock_acquire(sectors_lock + sector);

  if(inodes[sector]){
    inode = inode_reopen_no_lock(inodes[sector]);
  }else {

    /* Allocate memory. */
    inode = malloc(sizeof *inode);
    if (inode == NULL)
      PANIC("Not enough memory for inode");

    /* Initialize. */
    inode->sector = sector;
    inode->open_cnt = 1;
    inode->deny_write_cnt = 0;
    inode->removed = false;

    inodes[sector] = inode;
  }

  lock_release(sectors_lock + sector);

  return inode;
}
/* Reopens and returns INODE. */
static struct inode *inode_reopen_no_lock (struct inode *inode)
{
  if (inode != NULL) {
    __sync_add_and_fetch(&inode->open_cnt, 1);
  }
  return inode;
}
/* Reopens and returns INODE. */
struct inode *
inode_reopen (struct inode *inode)
{
  lock_acquire(sectors_lock + inode->sector);
  struct inode *ret = inode_reopen_no_lock(inode);
  lock_release(sectors_lock + inode->sector);

  return ret;
}

/* Returns INODE's inode number. */
block_sector_t
inode_get_inumber (const struct inode *inode)
{
  return inode->sector;
}

/* Closes INODE and writes it to disk.
   If this was the last reference to INODE, frees its memory.
   If INODE was also a removed inode, frees its blocks. */
void
inode_close (struct inode *inode) 
{
  /* Ignore null pointer. */
  if (inode == NULL)
    return;
  int sector = inode->sector;
  lock_acquire(sectors_lock + inode->sector);
  bool closed = false;
  /* Release resources if this was the last opener. */
  if (__sync_sub_and_fetch(&inode->open_cnt, 1) == 0) // it has write lock, no need for __sync
    {
      inodes[inode->sector] = NULL;

      /* Deallocate blocks if removed. */
      if (__sync_fetch(&inode->removed))
        {
          free_map_release (inode->sector, 1);

          struct inode_disk meta_data;
          cached_block_read (fs_device_cached, inode->sector, &meta_data, 0);
          free_map_release (meta_data.start,
                            bytes_to_sectors (meta_data.length));

        }

      free (inode); 
    }
  lock_release(sectors_lock + sector);
}

/* Marks INODE to be deleted when it is closed by the last caller who
   has it open. */
void
inode_remove (struct inode *inode) 
{
  ASSERT (inode != NULL);
  __sync_bool_compare_and_swap(&inode->removed, 0, 1);
}

/* Reads SIZE bytes from INODE into BUFFER, starting at position OFFSET.
   Returns the number of bytes actually read, which may be less
   than SIZE if an error occurs or end of file is reached. */
off_t
inode_read_at (struct inode *inode, void *buffer_, off_t size, off_t offset) 
{
  uint8_t *buffer = buffer_;
  off_t bytes_read = 0;

  struct inode_disk meta_data;
  cached_block_read (fs_device_cached, inode->sector, &meta_data, 0);

  while (size > 0) 
    {
      /* Disk sector to read, starting byte offset within sector. */
      block_sector_t sector_idx = byte_to_sector (&meta_data, offset);
      int sector_ofs = offset % BLOCK_SECTOR_SIZE;

      /* Bytes left in inode, bytes left in sector, lesser of the two. */
      off_t inode_left = inode_length_meta (&meta_data) - offset;
      int sector_left = BLOCK_SECTOR_SIZE - sector_ofs;
      int min_left = inode_left < sector_left ? inode_left : sector_left;

      /* Number of bytes to actually copy out of this sector. */
      int chunk_size = size < min_left ? size : min_left;
      if (chunk_size <= 0)
        break;

      cached_block_read_segment(fs_device_cached, sector_idx, sector_ofs, sector_ofs + chunk_size,
                                buffer + bytes_read, 0);
      
      /* Advance. */
      size -= chunk_size;
      offset += chunk_size;
      bytes_read += chunk_size;
    }

  return bytes_read;
}

/* Writes SIZE bytes from BUFFER into INODE, starting at OFFSET.
   Returns the number of bytes actually written, which may be
   less than SIZE if end of file is reached or an error occurs.
   (Normally a write at end of file would extend the inode, but
   growth is not yet implemented.) */
off_t
inode_write_at (struct inode *inode, const void *buffer_, off_t size,
                off_t offset) 
{
  const uint8_t *buffer = buffer_;
  off_t bytes_written = 0;

  if (__sync_fetch(&inode->deny_write_cnt))
    return 0;

  struct inode_disk meta_data;
  cached_block_read (fs_device_cached, inode->sector, &meta_data, 0);
  while (size > 0) 
    {
      /* Sector to write, starting byte offset within sector. */
      block_sector_t sector_idx = byte_to_sector (&meta_data, offset);
      int sector_ofs = offset % BLOCK_SECTOR_SIZE;

      /* Bytes left in inode, bytes left in sector, lesser of the two. */
      off_t inode_left = inode_length_meta (&meta_data) - offset;
      int sector_left = BLOCK_SECTOR_SIZE - sector_ofs;
      int min_left = inode_left < sector_left ? inode_left : sector_left;

      /* Number of bytes to actually write into this sector. */
      int chunk_size = size < min_left ? size : min_left;
      if (chunk_size <= 0)
        break;

      cached_block_write_segment(fs_device_cached, sector_idx, sector_ofs, sector_ofs + chunk_size,
        buffer + bytes_written, NULL, 0);

      /* Advance. */
      size -= chunk_size;
      offset += chunk_size;
      bytes_written += chunk_size;
    }

  return bytes_written;
}

/* Disables writes to INODE.
   May be called at most once per inode opener. */
void
inode_deny_write (struct inode *inode) 
{
  __sync_add_and_fetch(&inode->deny_write_cnt, 1);
}

/* Re-enables writes to INODE.
   Must be called once by each inode opener who has called
   inode_deny_write() on the inode, before closing the inode. */
void
inode_allow_write (struct inode *inode) 
{
  __sync_sub_and_fetch(&inode->deny_write_cnt, 1);
}

/* Returns the length, in bytes, of INODE's data. */
static off_t inode_length_meta (const struct inode_disk *inode_disk)
{
  return inode_disk->length;
}


/* Returns the length, in bytes, of INODE's data. */
off_t
inode_length (const struct inode *inode)
{

  struct inode_disk meta_data;
  cached_block_read (fs_device_cached, inode->sector, &meta_data, 0);
  return meta_data.length;
}
