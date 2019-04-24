#include "socket/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *util_readfile(const char *filename)
{
    FILE *f = fopen(filename, "rb");
    if(!f)
        return NULL;

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *content = calloc(1, fsize + 1);
    if(!content)
        goto out_err;

    fread(content, fsize, 1, f);
    fclose(f);

    content[fsize] = 0;

    return content;

out_err:
    fclose(f);
    return NULL;
}

void util_writefile(const char *filename, const char *content)
{
    FILE *f = fopen(filename, "wb");
    fwrite(content, strlen(content), 1, f);
    fclose(f);
}
