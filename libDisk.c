#include "libDisk.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int openDisk(char *filename, int nBytes) {
    int new_nBytes;
    int disk = -1; // default file open failure value

    if (nBytes < BLOCKSIZE && nBytes != 0) {
        printf("Line 10 error failure: openDisk\n");
        return -1;
    } else if (nBytes % BLOCKSIZE != 0) {
        new_nBytes = (nBytes / BLOCKSIZE) * BLOCKSIZE; // closest multiple to BLOCKSIZE < nBytes
    } else {
        new_nBytes = nBytes;
    }

    // open disk (unix file)
    if (nBytes == 0) { // disk already exists
        disk = open(filename, O_RDWR, 0666);
    } else {
        disk = open(filename, O_RDWR | O_CREAT, 0666);
    }

    if (disk < 0) {
        printf("File Open failure: openDisk\n");
        return -1; // file open failure
    }
    
    // if new_nBytes is set, truncate the file
    if (new_nBytes != 0) {
        int sizeSet = ftruncate(disk, new_nBytes);
        if (sizeSet != 0) {
            closeDisk(disk);
            printf("size set failure: openDisk\n");
            return -1;
        }
    }

    return disk;
}

int closeDisk(int disk) {
    return close(disk);
}

int readBlock(int disk, int bNum, void *block) {
    off_t offset = bNum * BLOCKSIZE;
    off_t diskOffset = lseek(disk, offset, SEEK_SET);

    if (diskOffset < 0) {
        printf("Disk Offset Error: readBlock\n");
        return -1;
    }

    ssize_t bytesRead = read(disk, block, BLOCKSIZE);
    if (bytesRead < 0 || bytesRead != BLOCKSIZE) {
        printf("Cannot read block: readBlock\n");
        return -1; // unable to read
    }

    return 0; // read
}

int writeBlock(int disk, int bNum, void *block) {
    off_t blockOffset = bNum * BLOCKSIZE;
    off_t diskOffset = lseek(disk, blockOffset, SEEK_SET);

    if (diskOffset < 0) {
        printf("Disk Offset Error: writeBlock\n");
        return -1;
    }

    ssize_t bytesWritten = write(disk, block, BLOCKSIZE);
    if (bytesWritten != BLOCKSIZE) {
        printf("Cannot write block: writeBlock\n");
        return -1; // unable to write
    }

    return 0; // written
}

