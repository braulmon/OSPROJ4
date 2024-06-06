#include "libDisk.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int openDisk(char *filename, int nBytes) {
    // Implementation here
    if (nBytes % BLOCKSIZE != 0) {
        return -1; // nBytes not a multiple of BLOCKSIZE
    }
    return 0;
}

int closeDisk(int disk) {
    return close(disk);
}

int readBlock(int disk, int bNum, void *block) {
    // Implementation here
    return 0;
}

int writeBlock(int disk, int bNum, void *block) {
    // Implementation here
    return 0;
}
