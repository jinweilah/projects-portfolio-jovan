/*
A C application program named chardevclient.c, to access the 
“character device” via the new read&write device driver, chardev.c in the kernel. 
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE 512    // Max length of buffer size in power of 2

int main() {
    int fd, c, retval, n;
    char writeSentence[MAX_SIZE]={0};
    char readSentence[MAX_SIZE]={0};

    fd = open("/dev/chardev", O_RDWR);  // open the chardev driver, O_RDWR = open for reading and writing
    if (fd < 0)   // if fail to open the driver (the drive is not in kernel)
    {
        perror("open /dev/chardev");
        exit(EXIT_FAILURE);
    }
    n = 0;
    printf("Writing to /dev/chardev: \n");  
    while(n<11){   //loop more than 10 times
        fgets(writeSentence, MAX_SIZE, stdin);   //user input sentence
        retval = write(fd, writeSentence, strlen(writeSentence)); // write sentence into the driver
        if (retval < 0) // fail to write info
        {
            perror("writing /dev/chardev");
        }

        retval = read(fd, readSentence, MAX_SIZE);
        if (retval < 0)  // fail to read info
        {
            perror("reading /dev/chardev");
        } 
        else 
        {
            printf("%s(%d letters)\n", readSentence, retval); //user space app print message and length
        }
        n++;
    }
    close(fd);  //close character device file
    exit(EXIT_SUCCESS);
}