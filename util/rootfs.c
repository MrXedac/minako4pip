#include <stdint.h>
#include "minako.h"

extern void* _rootfs;

/* ASCII octal to int */
int oct2bin(unsigned char *str, int size)
{
    int n = 0;
    unsigned char *c = str;
    while (size-- > 0) {
        n *= 8;
        n += *c - '0';
        c++;
    }
    return n;
}

/* Returns file size, pointer to data in **out 
 * Format : USTAR */
int rootfs_lookup(char *filename, unsigned char **out)
{
    unsigned char *ptr = (unsigned char*)&_rootfs;

    while (!memcmp(ptr + 257, "ustar", 5)) {
        int filesize = oct2bin(ptr + 0x7c, 11);
        if (!memcmp(ptr, filename, strlen(filename) + 1)) {    
            *out = ptr + 512;
            return filesize;
        }
        ptr += (((filesize + 511) / 512) + 1) * 512;
    }
    return 0;
}
