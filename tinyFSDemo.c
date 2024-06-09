#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libTinyFS.h"
#include <time.h>

// Simple helper function to fill buffer with as many inPhrase strings as possible before reaching size
int fillBufferWithPhrase(char *inPhrase, char *Buffer, int size)
{
    int index = 0, i;
    if (!inPhrase || !Buffer || size <= 0 || size < strlen(inPhrase))
        return -1;

    while (index < size)
    {
        for (i = 0; inPhrase[i] != '\0' && (i + index < size); i++)
            Buffer[i + index] = inPhrase[i];
        index += i;
    }
    Buffer[size - 1] = '\0'; // explicit null termination
    return 0;
}

// This program will create 2 files (of sizes 200 and 1000) to be read from or stored in the TinyFS file system.
int main()
{

    char readBuffer;
    char *afileContent, *bfileContent, *cfileContent, *dfileContent; // buffers to store file content
    int afileSize = 200;                              // sizes in bytes
    int bfileSize = 1000;
    int cfileSize = 1000;
    int dfileSize = 1000;

    char phrase1[] = "hello world from (a) file ";
    char phrase2[] = "(b) file content ";
    char phrase3[] = "c file content";
    fileDescriptor aFD, bFD, cFD, dFD;

    // Try to mount the disk
    printf("Attempting to mount the disk\n");
    if (tfs_mount(DEFAULT_DISK_NAME) < 0)
    { // if mount fails
        printf("Mount failed, trying to create a new file system\n");
        if (tfs_mkfs(DEFAULT_DISK_NAME, DEFAULT_DISK_SIZE) < 0)
        { // then make a new disk
            perror("Failed to create file system");
            return -1;
        }
        if (tfs_mount(DEFAULT_DISK_NAME) < 0)
        { // if we still can't open it...
            perror("Failed to mount disk after creating file system");
            return -1;
        }
    }

    afileContent = (char *)malloc(afileSize * sizeof(char));
    if (fillBufferWithPhrase(phrase1, afileContent, afileSize) < 0)
    {
        perror("Failed to fill buffer for afile");
        return -1;
    }

    bfileContent = (char *)malloc(bfileSize * sizeof(char));
    if (fillBufferWithPhrase(phrase2, bfileContent, bfileSize) < 0)
    {
        perror("Failed to fill buffer for bfile");
        return -1;
    }

    cfileContent = (char *)malloc(bfileSize * sizeof(char));
    if (fillBufferWithPhrase(phrase3, cfileContent, cfileSize) < 0)
    {
        perror("Failed to fill buffer for cfile");
        return -1;
    }

    
    dfileContent = (char *)malloc(dfileSize * sizeof(char));
    if (fillBufferWithPhrase(phrase3, dfileContent, dfileSize) < 0)
    {
        perror("Failed to fill buffer for dfile");
        return -1;
    }

    // Print content of files for debugging
    printf("(a) File content: %s\n(b) File content: %s\nReady to store in TinyFS\n (c) File Content: %s\n", afileContent, bfileContent, cfileContent);

    // Read or write files to TinyFS
    aFD = tfs_openFile("afile");
    if (aFD < 0)
    {
        perror("tfs_openFile failed on afile");
    }
    else
    {
        if (tfs_readByte(aFD, &readBuffer) < 0)
        { // if readByte() fails, there was no afile, so we write to it
            if (tfs_writeFile(aFD, afileContent, afileSize) < 0)
            {
                perror("tfs_writeFile failed on afile");
            }
            else
            {
                printf("Successfully written to afile\n");
            }
        }
    }

    bFD = tfs_openFile("bfile");
    if (bFD < 0)
    {
        perror("tfs_openFile failed on bfile");
    }
    else
    {
        if (tfs_readByte(bFD, &readBuffer) < 0)
        { // if readByte() fails, there was no bfile, so we write to it
            if (tfs_writeFile(bFD, bfileContent, bfileSize) < 0)
            {
                perror("tfs_writeFile failed on bfile");
            }
            else
            {
                printf("Successfully written to bfile\n");
            }
        }
    }

    cFD = tfs_openFile("cfile");
    tfs_makeRO(cFD);

    if (cFD < 0)
    {
        perror("tfs_openFile failed on cfile");
    }
    else
    {
        if (tfs_readByte(cFD, &readBuffer) < 0)
        { // if readByte() fails, there was no bfile, so we write to it
            if (tfs_writeFile(cFD, cfileContent, cfileSize) < 0)
            {
                printf("RO, now making RW......\n");
                if(tfs_makeRW(cFD) == 0){
                    printf("File Succesfully RW enabled\n");
                }
                else{
                    perror("R/W function problem");
                }
                if(tfs_writeFile(cFD, cfileContent, cfileSize) < 0){
                    perror("tfs_writeFile failed on cfile");
                }
                else{
                    printf("Successfully written to cfile\n");
                }
            }
            else
            {
                printf("Successfully written to cfile\n");
            }
        }
    }

    dFD = tfs_openFile("dfile");
    if(tfs_writeByte(dFD, 0, (int)cfileContent) == -1){
        perror("Write Byte Error");

    }
    else{
        printf("Succesfully Transferred Contents onto file...byte by byte \n");
        if(strcmp(cfileContent, dfileContent) == 0){
            printf("Both Functions are Identical, writeByte Works\n");
        }
        else{
            printf("Function Write Byte Error, Not identical\n");
        }

    }

    tfs_readFileInfo(aFD);
    tfs_readFileInfo(bFD);
    tfs_readFileInfo(cFD);

    tfs_readdir();
    tfs_rename(aFD, "NEW NAM");
    tfs_readdir();


    // Free both content buffers
    free(afileContent);
    free(bfileContent);
    free(cfileContent);
    free(cfileContent);
    if (tfs_unmount() < 0)
    {
        perror("tfs_unmount failed");
    }
    printf("%d", current_mounted_disk);
    printf("\nend of demo\n\n");
    return 0;
}
