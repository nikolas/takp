#include "tail.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

ssize_t fdSize(int fd)
{
    struct stat buf;
    int err = fstat(fd, &buf);
    if (err < 0)
        return -1;
    return buf.st_size;
}

void tail(const char* filename, tailfn callback)
{
    int fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        fprintf(stderr, "Error opening file: [%d] %s\n", errno, filename);
        return;
    }
    size_t pos = 0; //fdSize(fd);
    if(pos < 0)
    {
        fprintf(stderr, "Error checking file size: [%d] %s\n", errno, filename);
        return;
    }
    printf("tail %s\n", filename);
    char line[64 * 1024];
    while(1)
    {
        ssize_t fileSize = fdSize(fd);
        ssize_t newPos = lseek(fd, pos, SEEK_SET);
        if(newPos > fileSize)
        {
            pos = fileSize;
            fprintf(stderr, "File truncation detected. New size: %zd\n", pos);
        }
        ssize_t readSize = read(fd, &line, sizeof(line));
        if(readSize == 0)
        {
            printf("Waiting for more data to read.\n");
            sleep(1);
        }
        ssize_t last = 0;
        for (ssize_t here = last; here < readSize; here++)
        {
            if (line[here] == '\n')
            {
                line[here] = '\0';
                callback(&line[last]);
                last = here + 1;
            }
        }
        pos += last;
    }
}
