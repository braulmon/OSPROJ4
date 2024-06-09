#ifndef LIBTINYFS_H
#define LIBTINYFS_H
#include <time.h>

/* The default size of the disk and file system block */
#define BLOCKSIZE 256
#define MAGIC_NUMBER 0x44
#define SUPERBLOCK_TYPE 1
#define INODE_TYPE 2 // metadata for a single file, could tell where the next file extent is
#define FILE_EXTENT_TYPE 3
#define FREE_BLOCK_TYPE 4
#define MAX_FILES 50

/* Your program should use a 10240 Byte disk size giving you 40 blocks total. 
This is a default size. You must be able to support different possible values */
#define DEFAULT_DISK_SIZE 10240
/* Use this name for a default emulated disk file name */
#define DEFAULT_DISK_NAME "tinyFSDisk"
/* Use as a special type to keep track of files */
typedef int fileDescriptor;

typedef struct {
    int block_type;
    int magic_number;
    int next_block;
    int free;
    int data[BLOCKSIZE - 4];
} Block;

typedef struct {
    int root_inode;
    int free_block_list;
} Superblock;

typedef struct {
    char file_name[8];     // File name
    int file_size;         // Size of the file in bytes
    int start_block;       // Starting block of the file
    int block_count;       // Number of blocks allocated to the file
    int read_only;         // Read-only flag
    time_t creation_time;  // File creation time
    time_t modification_time; // File modification time
    time_t access_time;    // File access time
} Inode;

int fd_table[MAX_FILES];
int current_mounted_disk = -1;

int tfs_mkfs(char *filename, int nBytes);
int tfs_mount(char *diskname);
int tfs_unmount(void);
fileDescriptor tfs_openFile(char *name);
int tfs_closeFile(fileDescriptor FD);

int tfs_writeFile(fileDescriptor FD, char *buffer, int size);
/* Writes buffer ‘buffer’ of size ‘size’, which represents an entire
file’s content, to the file system. Previous content (if any) will be
completely lost. Sets the file pointer to 0 (the start of file) when
done. Returns success/error codes. */

int tfs_deleteFile(fileDescriptor FD);
int tfs_readByte(fileDescriptor FD, char *buffer);
int tfs_seek(fileDescriptor FD, int offset);

#endif // LIBTINYFS_H
