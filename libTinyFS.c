#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include "libDisk.h"
#include "libTinyFS.h"

void initialize_fd_table() {
    for (int i = 0; i < MAX_FILES; i++) {
        fd_table[i] = -1;
    }
}


    /* Makes a blank TinyFS file system of size nBytes on the unix file
        specified by ‘filename’. This function should use the emulated disk
        library to open the specified unix file, and upon success, format the
        file to be a mountable disk. This includes initializing all data to 0x00,
        setting magic numbers, initializing and writing the superblock and
        inodes, etc. Must return a specified success/error code.
//Basic Error Checking
//complete initialization
//Need help with this function
//need to write superblocks and inodes
//return specified code
//Super Block points to inodes that store information that points to another inode
//Initialize and then openDisk*/

int tfs_mkfs(char *filename, int nBytes)
{
    // openDisk has its own error checks already
    int disk = openDisk(filename, nBytes);

    // Initialize superblock
    Block superblock;
    superblock.block_type = SUPERBLOCK_TYPE;
    superblock.magic_number = MAGIC_NUMBER;
    superblock.next_block = 0;
    superblock.padding = 0;
    memset(superblock.data, 0, sizeof(superblock.data));

    Superblock *sb = (Superblock *)superblock.data;
    sb->root_inode = 1; // Assuming inode starts at block 1
    sb->free_block_list = 2; // Assuming free block list starts at block 2

    // Write the superblock to the disk at block 0
    if (writeBlock(disk, 0, &superblock) < 0) {
        perror("Failed to write superblock");
        return -1;
    }

    // Initialize free blocks
    for (int i = 1; i < nBytes / BLOCKSIZE; i++) {
        block.block_type = FREE_BLOCK_TYPE;
        block.magic_number = MAGIC_NUMBER;
        supeblock.next_block = 0;
        superblock.padding = 0;
        memset(superblock.data, 0, sizeof(superblock.data));
        if (i == nBytes / BLOCKSIZE - 1) {
            block.next_block = 0;
        } else {
            block.next_block = i + 1;
        }

        // Write the free block to the disk at block index i
        if (writeBlock(disk, i, &block) < 0) {
            return -1;
        }
    }

    return 0;
}

int tfs_mount(char *diskname) {
    // openDisk with 0 nBytes input to parameter means disk exists
    int disk = openDisk(diskname, 0);

    if (disk < 0) {
        perror("Failed to open disk");
        return -1;
    }

    Block superblock;
    if (readBlock(disk, 0, &superblock) < 0) {
        perror("Failed to read superblock");
        return -1;
    }

    if (superblock.magic_number != MAGIC_NUMBER || superblock.block_type != SUPERBLOCK_TYPE) {
        perror("Invalid superblock");
        return -1;
    }

    return 0;
}


int tfs_unmount(void) {
    if (current_mounted_disk < 0) {
        perror("No disk is currently mounted: tfs_unmount");
        return -1;
    }

    if (closeDisk(current_mounted_disk) < 0) {
        perror("Failed to unmount fs: tfs_unmount");
        return -1;
    }

    current_mounted_disk = -1;
    return 0;
}

fileDescriptor tfs_openFile(char *name)
{
    /* Creates or Opens a file for reading and writing on the currently
    mounted file system. Creates a dynamic resource table entry for the file,
    and returns a file descriptor (integer) that can be used to reference
    this entry while the filesystem is mounted. */
    int fd = -1;
    for (int i = 0; i < 50; i++) {
        if (fd_table[i] == -1) {
            // Find and initialize an available inode
            Inode inode;
            strcpy(inode.file_name, name);
            inode.file_size = 0;
            inode.start_block = -1;
            inode.block_count = 0;
            inode.read_only = 0;
            inode.creation_time = time(NULL);
            inode.modification_time = time(NULL);
            inode.access_time = time(NULL);

            int inode_block = allocate_inode();
            if (inode_block < 0) {
                perror("Failed to allocate inode block: tfs_openFile");
                return -1;
            }

            if (writeBlock(current_mounted_disk, inode_block, &inode) < 0) {
                perror("Failed to write inode block: tfs_openFile");
                return -1;
            }

            fd_table[i] = inode_block;  // Store inode index in fd_table
            return i;  // Return file descriptor (array index)
        }
    }

    perror("Max file descriptors reached");
    return -1;

    }

//Remainder math
//take the offset
//divide by how much is left
//


int tfs_closeFile(fileDescriptor FD)
{
    if (FD < 0)
    {
        return -1;
    }

    int flags = fcntl(FD, F_GETFD);
    if (flags & FD_CLOEXEC)
    {
        // The file descriptor is not in use - NOTE this was found by generative AI when searched on google
        printf("tfs_closeFile: File is not open\n");
        return -1;
    }
    else
    {
        close(FD);
        return 0;
    }
}

int tfs_writeFile(fileDescriptor FD, char *buffer, int size)
{

    current_inode = fd_table[FD];
    for(int i = 0; i < size; i++){
        readBlock(current_mounted_disk, current_inode)





    }
    





}

int tfs_deleteFile(fileDescriptor FD)
{

    int index = -1;
    for (int i = 0; i < 50; i++)
    {
        if (fd_table[i] == FD)
        {
            fd_table[i] = NULL;
            break;
        }

        else
        {
            print("FD DOES NOT EXIST IN FD TABLE: tfs_closeFile\n");
        }
    }





}

int tfs_readByte(fileDescriptor FD, char *buffer)
{
    /* reads one byte from the file and copies it to buffer, using the
    current file pointer location and incrementing it by one upon success.
    If the file pointer is already past the end of the file then
    tfs_readByte() should return an error and not increment the file pointer.
    */
    
    
   

}

int tfs_seek(fileDescriptor FD, int offset)
{

    if(fseek(FD,0, offset) == -1){
        perror("Fseek Error: tfs_seek");
        return -1;
    }


}
