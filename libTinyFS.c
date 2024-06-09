#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "libDisk.h"
#include "libTinyFS.h"
#include "tinyFS_errno.h"

int fd_table[MAX_FILES];
int current_mounted_disk = -1;

int validate_fd(fileDescriptor FD)
{
    if (FD < 0 || FD >= MAX_FILES || fd_table[FD] == -1)
    {
        return EBADF;
    }
    return 0;
}

void initialize_fd_table()
{
    for (int i = 0; i < MAX_FILES; i++)
    {
        fd_table[i] = -1;
    }
}

void initialize_block(Block *block, char block_type, char magic_number, char next_block, char reserved)
{
    block->block_type = block_type;
    block->magic_number = magic_number;
    block->next_block = next_block;
    block->reserved = reserved;
    memset(block->data, 0, sizeof(block->data));
}

int allocate_inode()
{
    Block superblock;
    if (readBlock(current_mounted_disk, 0, &superblock) < 0)
    {
        return ERB;
    }

    Superblock *sb = (Superblock *)superblock.data;
    if (sb->free_block_list == 0)
    {
        return ENOB;
    }

    int free_block_index = sb->free_block_list;
    Block free_block;
    if (readBlock(current_mounted_disk, free_block_index, &free_block) < 0)
    {
        return ERB;
    }

    sb->free_block_list = free_block.next_block;
    if (writeBlock(current_mounted_disk, 0, &superblock) < 0)
    {
        return EWB;
    }

    return free_block_index;
}

int tfs_mkfs(char *filename, int nBytes)
{
    int disk = openDisk(filename, nBytes);
    if (disk < 0)
    {
        return ENODISK;
    }
    

    Block superblock;
    initialize_block(&superblock, SUPERBLOCK_TYPE, MAGIC_NUMBER, 0, 0);

    Superblock *sb = (Superblock *)superblock.data;
    sb->root_inode = 1;
    sb->free_block_list = 2;

    if (writeBlock(disk, 0, &superblock) < 0)
    {
        return EWB;
    }

    for (int i = 1; i < nBytes / BLOCKSIZE; i++)
    {
        Block block;
        if (i == nBytes / BLOCKSIZE - 1)
        {
            initialize_block(&block, FREE_BLOCK_TYPE, MAGIC_NUMBER, 0, 0);
        }
        else
        {
            initialize_block(&block, FREE_BLOCK_TYPE, MAGIC_NUMBER, i + 1, 0);
        }

        if (writeBlock(disk, i, &block) < 0)
        {
            return EWB;
        }
    }

    initialize_fd_table();

    return 0;
}

int tfs_mount(char *diskname)
{
    int disk = openDisk(diskname, 0);
    if (disk < 0)
    {
        return ENODISK;
    }

    Block superblock;
    if (readBlock(disk, 0, &superblock) < 0)
    {
        return ERB;
    }

    if (superblock.magic_number != MAGIC_NUMBER || superblock.block_type != SUPERBLOCK_TYPE)
    {
        return EBADSB;
    }

    current_mounted_disk = disk;
    initialize_fd_table();

    return 0;
}

int tfs_unmount(void)
{
    if (current_mounted_disk < 0 || closeDisk(current_mounted_disk) < 0)
    {
        return ENODISK;
    }

    current_mounted_disk = -1;

    return 0;
}

fileDescriptor tfs_openFile(char *name)
{
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (fd_table[i] == -1)
        {
            int inode_block = allocate_inode();
            if (inode_block < 0)
            {
                return inode_block;
            }
            

            Inode inode;
            strncpy(inode.file_name, name, sizeof(inode.file_name) - 1);
            inode.file_name[sizeof(inode.file_name) - 1] = '\0';
            inode.file_size = 0;
            inode.start_block = -1;
            inode.block_count = 0;
            inode.read_only = 0;
            inode.creation_time = time(NULL);
            inode.modification_time = time(NULL);
            inode.access_time = time(NULL);

            Block inode_data;
            initialize_block(&inode_data, INODE_TYPE, MAGIC_NUMBER, 0, 0);
            memcpy(inode_data.data, &inode, sizeof(Inode));

            if (writeBlock(current_mounted_disk, inode_block, &inode_data) < 0)
            {
                return EWB;
            }

            fd_table[i] = inode_block;
            return i;
        }
    }

    return ENOB;
}

int tfs_closeFile(fileDescriptor FD)
{
    int valFD = validate_fd(FD);
    if (valFD != 0)
    {
        return valFD;
    }

    fd_table[FD] = -1;

    return 0;
}

int tfs_writeFile(fileDescriptor FD, char *buffer, int size)
{
    int val_FD = validate_fd(FD);
    if (val_FD != 0)
    {
        printf("Invalid file descriptor: %d\n", val_FD);
        return val_FD;
    }

    int inode_block = fd_table[FD];
    Block inode_data;
    if (readBlock(current_mounted_disk, inode_block, &inode_data) < 0)
    {
        printf("Read Block Error: tfs_writeFile\n");
        return ERB;
    }

    Inode *inode = (Inode *)inode_data.data;


    if (inode->read_only)
    {
        printf("Error: File is read-only\n");
        return EROFS;
    }

    int current_block = inode->start_block;
    int bytes_written = 0;
    int prev_block = 0;

    while (bytes_written < size)
    {
        int block_num = allocate_inode();
        if (block_num < 0)
        {
            if (block_num == ENOB)
            {
                printf("Error: No free blocks available\n");
            }
            else
            {
                printf("Error: Failed to allocate inode, error code: %d\n", block_num);
            }
            return block_num;
        }

        Block data_block;
        initialize_block(&data_block, FILE_EXTENT_TYPE, MAGIC_NUMBER, 0, 0);

        int bytes_to_write = 0;
        if (size - bytes_written > BLOCKSIZE - 4)
        {
            bytes_to_write = BLOCKSIZE - 4;
        }
        else
        {
            bytes_to_write = size - bytes_written;
        }
        memcpy(data_block.data, buffer + bytes_written, bytes_to_write);
        bytes_written += bytes_to_write;

        if (prev_block != 0)
        {
            Block prev_data_block;
            if (readBlock(current_mounted_disk, prev_block, &prev_data_block) < 0)
            {
                printf("Read Block Error: tfs_writeFile (prev block)\n");
                return ERB;
            }
            prev_data_block.next_block = block_num;
            if (writeBlock(current_mounted_disk, prev_block, &prev_data_block) < 0)
            {
                printf("Write Block Error: tfs_writeFile (prev block)\n");
                return EWB;
            }
        }

        if (writeBlock(current_mounted_disk, block_num, &data_block) < 0)
        {
            printf("Write Block Error: tfs_writeFile (data block %d)\n", block_num);
            return EWB;
        }

        prev_block = block_num;
    }

    inode->file_size = size;
    if (current_block == -1)
    {
        inode->start_block = prev_block;
    }
    else
    {
        inode->start_block = current_block;
    }
    inode->modification_time = time(NULL);

    if (writeBlock(current_mounted_disk, inode_block, &inode_data) < 0)
    {
        printf("Write Block Error: tfs_writeFile (inode block)\n");
        return EWB;
    }

    return 0;
}
int tfs_deleteFile(fileDescriptor FD)
{
    int valid_FD = validate_fd(FD);
    if (valid_FD != 0)
    {
        return valid_FD;
    }

    int inode_block = fd_table[FD];
    Block inode_data;
    if (readBlock(current_mounted_disk, inode_block, &inode_data) < 0)
    {
        printf("Error reading inode block in tfs_deleteFile\n");
        return ERB;
    }

    Inode *inode = (Inode *)inode_data.data;

    if (inode->read_only)
    {
        printf("Error: File is read-only\n");
        return EROFS;
    }

    int current_block = inode->start_block;
    Block superblock;
    if (readBlock(current_mounted_disk, 0, &superblock) < 0) {
        printf("Error reading superblock in tfs_deleteFile\n");
        return ERB;
    }

    Superblock *sb = (Superblock *)superblock.data;

    while (current_block != 0)
    {
        Block block;
        if (readBlock(current_mounted_disk, current_block, &block) < 0)
        {
            printf("Error reading block %d in tfs_deleteFile\n", current_block);
            return ERB;
        }

        int next_block = block.next_block;

        Block free_block;
        initialize_block(&free_block, FREE_BLOCK_TYPE, MAGIC_NUMBER, 0, 0);
        if (writeBlock(current_mounted_disk, current_block, &free_block) < 0)
        {
            printf("Error writing block %d in tfs_deleteFile\n", current_block);
            return EWB;
        }

        block.next_block = sb->free_block_list;
        sb->free_block_list = current_block;

        current_block = next_block;
    }

    Block free_block;
    initialize_block(&free_block, FREE_BLOCK_TYPE, MAGIC_NUMBER, 0, 0);
    if (writeBlock(current_mounted_disk, inode_block, &free_block) < 0)
    {
        printf("Error writing inode block in tfs_deleteFile\n");
        return EWB;
    }

    if (writeBlock(current_mounted_disk, 0, &superblock) < 0) {
        printf("Error writing superblock in tfs_deleteFile\n");
        return EWB;
    }

    fd_table[FD] = -1;

    return 0;
}

int tfs_readByte(fileDescriptor FD, char *buffer)
{
    int valFD = validate_fd(FD);
    if (valFD != 0)
    {
        return valFD;
    }

    int inode_block = fd_table[FD];
    Block inode_data;
    if (readBlock(current_mounted_disk, inode_block, &inode_data) < 0)
    {
        return ERB;
    }

    Inode *inode = (Inode *)inode_data.data;
    if (inode->read_only)
    {
        printf("Error: File is read-only\n");
        return EROFS;
    }

    if (inode->start_block < 0 || inode->start_block >= inode->file_size)
    {
        return ENOENT;
    }

    int block_num = inode->start_block / (BLOCKSIZE - 4);
    int offset_within_block = inode->start_block % (BLOCKSIZE - 4);

    Block data_block;
    if (readBlock(current_mounted_disk, block_num, &data_block) < 0)
    {
        return ERB;
    }

    *buffer = data_block.data[offset_within_block];
    inode->start_block++;

    inode->access_time = time(NULL);
    if (writeBlock(current_mounted_disk, inode_block, &inode_data) < 0)
    {
        return EWB;
    }

    return 0;
}

int tfs_seek(fileDescriptor FD, int offset)
{
    int valFD = validate_fd(FD);
    if (valFD != 0)
    {
        return valFD;
    }

    int inode_block = fd_table[FD];
    Block inode_data;
    if (readBlock(current_mounted_disk, inode_block, &inode_data) < 0)
    {
        printf("Read Block Error: tfs_seek\n");
        return ERB;
    }

    Inode *inode = (Inode *)inode_data.data;

    if ((offset < 0) || (offset > inode->file_size))
    {
        printf("File is too Large: tfs_seek\n");
        return EFBIG;
    }

    inode->start_block = offset;

    inode->access_time = time(NULL);
    if (writeBlock(current_mounted_disk, inode_block, &inode_data) < 0)
    {
        printf("Write Block Error: tfs_seek\n");
        return EWB;
    }

    return 0;
}

int tfs_makeRO(fileDescriptor FD)
{
    int valFD = validate_fd(FD);
    if (valFD != 0)
    {
        return valFD;
    }

    int inode_block = fd_table[FD];
    Block inode_data;
    if (readBlock(current_mounted_disk, inode_block, &inode_data) < 0)
    {
        return ERB;
    }

    Inode *inode = (Inode *)inode_data.data;
    inode->read_only = 1;

    inode->modification_time = time(NULL);

    if (writeBlock(current_mounted_disk, inode_block, &inode_data) < 0)
    {
        return EWB;
    }

    return 0;
}

int tfs_makeRW(fileDescriptor FD)
{
    int valFD = validate_fd(FD);
    if (valFD != 0)
    {
        return valFD;
    }

    int inode_block = fd_table[FD];
    Block inode_data;
    if (readBlock(current_mounted_disk, inode_block, &inode_data) < 0)
    {
        return ERB;
    }

    Inode *inode = (Inode *)inode_data.data;
    inode->read_only = 0;

    inode->modification_time = time(NULL); // Update modification time

    if (writeBlock(current_mounted_disk, inode_block, &inode_data) < 0)
    {
        return EWB;
    }

    return 0;
}

int tfs_writeByte(fileDescriptor FD, int offset, unsigned int data)
{
    int valFD = validate_fd(FD);
    if (valFD != 0)
    {
        return valFD;
    }

    int inode_block = fd_table[FD];
    Block inode_data;
    if (readBlock(current_mounted_disk, inode_block, &inode_data) < 0)
    {
        return ERB;
    }

    Inode *inode = (Inode *)inode_data.data;
    if (inode->read_only)
    {
        return EROFS;
    }

    if (offset < 0 || inode->file_size <= offset)
    {
        return EFBIG;
    }

    int block_num = offset / (BLOCKSIZE - 4);
    int offset_within_block = offset % (BLOCKSIZE - 4);

    Block data_block;
    if (readBlock(current_mounted_disk, block_num, &data_block) < 0)
    {
        return ERB;
    }

    data_block.data[offset_within_block] = (char)data;

    if (writeBlock(current_mounted_disk, block_num, &data_block) < 0)
    {
        return EWB;
    }

    inode->modification_time = time(NULL);
    if (writeBlock(current_mounted_disk, inode_block, &inode_data) < 0)
    {
        return EWB;
    }

    return 0;
}

int tfs_readFileInfo(fileDescriptor FD)
{
    int valFD = validate_fd(FD);
    if (valFD != 0)
    {
        return valFD;
    }

    int inode_block = fd_table[FD];
    Block inode_data;
    if (readBlock(current_mounted_disk, inode_block, &inode_data) < 0)
    {
        return -1;
    }

    Inode *inode = (Inode *)inode_data.data;
    inode->access_time = time(NULL);
    printf("File: %s\n", inode->file_name);
    printf("Size: %d bytes\n", inode->file_size);
    printf("Creation Time %s\n", ctime(&inode->creation_time));
    printf("Last Modified %s\n", ctime(&inode->modification_time));
    printf("Last Accessed %s\n", ctime(&inode->access_time));


    return 0;
}

int tfs_rename(fileDescriptor FD, char* newName) {
    int valFD = validate_fd(FD);
    if (valFD != 0) {
        return valFD;
    }

    if (strlen(newName) > 8) {
        printf("New file name exceeds 8 characters: tfs_rename");
        return EINVAL;
    }

    int inode_block = fd_table[FD];
    Block inode_data;

    if (readBlock(current_mounted_disk, inode_block, &inode_data) < 0) {
        return ERB;
    }

    Inode *inode = (Inode *)inode_data.data;
    strcpy(inode->file_name, newName);
    inode->file_name[sizeof(inode->file_name) - 1] = '\0';

    inode->modification_time = time(NULL);
    inode->access_time = time(NULL);

    if (writeBlock(current_mounted_disk, inode_block, &inode_data) < 0) {
        return EWB;
    }

    return 0;
}

int tfs_readdir() {
    printf("Files on disk:\n");
    for (int i = 0; i < MAX_FILES; i++) {
        if (fd_table[i] != -1) {
            int inode_block = fd_table[i];
            Block inode_data;
            if (readBlock(current_mounted_disk, inode_block, &inode_data) < 0) {
                return ERB;
            }

            Inode *inode = (Inode *)inode_data.data;
            printf("%s\n", inode->file_name);
        }
    }

    return 0;
}
