#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>



/* The default size of the disk and file system block */
#define BLOCKSIZE 256
#define MAGIC_NUMBER 0x44
/* Your program should use a 10240 Byte disk size giving you 40 blocks
total. This is a default size. You must be able to support different
possible values */
#define DEFAULT_DISK_SIZE 10240
/* use this name for a default emulated disk file name */
#define DEFAULT_DISK_NAME “tinyFSDisk”
/* use as a special type to keep track of files */
typedef int fileDescriptor;




int tfs_mkfs(char *filename, int nBytes);
int tfs_mount(char *diskname);
int tfs_unmount(void);
fileDescriptor tfs_openFile(char *name);
int tfs_closeFile(fileDescriptor FD);
int tfs_writeFile(fileDescriptor FD,char *buffer, int size);
int tfs_deleteFile(fileDescriptor FD);
int tfs_readByte(fileDescriptor FD, char *buffer);
int tfs_seek(fileDescriptor FD, int offset);


int tfs_mkfs(char *filename, int nBytes){
    /* Makes a blank TinyFS file system of size nBytes on the unix file
specified by ‘filename’. This function should use the emulated disk
library to open the specified unix file, and upon success, format the
file to be a mountable disk. This includes initializing all data to 0x00,
setting magic numbers, initializing and writing the superblock and
inodes, etc. Must return a specified success/error code. */

//Basic Error Checking
    if (openDisk(filename, nBytes) != 0) {
        return -1;
    }

    FILE *disk_file = fopen(filename, "r+b");
    if (!disk_file) {
        perror("Failed to open file");
        return -1;
    }

    char superblock[BLOCKSIZE] = {0};
    superblock[1] = MAGIC_NUMBER;
    superblock[2] = 0; //Do not Know
    superblock[3] = NULL;
    superblock[4] = 2;
    //complete initialization
    //Need help with this function

    //need to write superblocks and inodes
    //return specified code


}

int tfs_mount(char *diskname){


}


int tfs_unmount(void){

}

fileDescriptor tfs_openFile(char *name){





}

int tfs_closeFile(fileDescriptor FD){
    if(FD < 0){
        return -1;
    }

    int flags = fcntl(FD, F_GETFD);
    if (flags & FD_CLOEXEC){
        // The file descriptor is not in use - NOTE this was found by generative AI when searched on google
        printf("tfs_closeFile: File is not open\n");
        return -1;



    }
    else{
        close(FD);
        return 0;

    }
}

int tfs_writeFile(fileDescriptor FD,char *buffer, int size){
    

}

int tfs_deleteFile(fileDescriptor FD){

}

int tfs_readByte(fileDescriptor FD, char *buffer){
    
}


int tfs_seek(fileDescriptor FD, int offset){

}
