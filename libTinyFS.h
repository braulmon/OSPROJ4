#ifndef LIBTINYFS_H
#define LIBTINYFS_H

#include <time.h>

/* The default size of the disk and file system block */
#define BLOCKSIZE 256
#define MAGIC_NUMBER 0x44
#define SUPERBLOCK_TYPE 1
#define INODE_TYPE 2
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
    char block_type;
    char magic_number;
    char next_block;
    char reserved;
    char data[BLOCKSIZE - 4];
} Block;

typedef struct {
    int root_inode;
    int free_block_list;
} Superblock;

typedef struct {
    char file_name[8];
    int file_size;
    int start_block;
    int block_count;
    int read_only;
    time_t creation_time;
    time_t modification_time;
    time_t access_time;
} Inode;

extern int fd_table[MAX_FILES];
extern int current_mounted_disk;

int validate_fd(fileDescriptor FD);
int tfs_mkfs(char *filename, int nBytes);
int tfs_mount(char *diskname);
int tfs_unmount(void);
fileDescriptor tfs_openFile(char *name);
int tfs_closeFile(fileDescriptor FD);
int tfs_writeFile(fileDescriptor FD, char *buffer, int size);
int tfs_deleteFile(fileDescriptor FD);
int tfs_readByte(fileDescriptor FD, char *buffer);
int tfs_seek(fileDescriptor FD, int offset);

int tfs_makeRO(fileDescriptor FD);
int tfs_makeRW(fileDescriptor FD);
int tfs_writeByte(fileDescriptor FD, int offset, unsigned int data);
int tfs_readFileInfo(fileDescriptor FD);
int tfs_rename(fileDescriptor FD, char* newName);
int tfs_readdir();
#endif // LIBTINYFS_H
