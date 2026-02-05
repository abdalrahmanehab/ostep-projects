#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void searchstream(FILE *fp, char *srch)
{
    char *line = NULL;
    __ssize_t n = 0;

    while (getline(&line, &n, fp) != EOF)
    {
        if (strstr(line, srch) != NULL)
        {
            printf("%s", line);
        }
    }
    free(line);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("wgrep: searchterm [file ...]\n");
        exit(1);
    }

    /* in case no file specified mygrep reads from stdin */

    if (argc == 2)
    {
        searchstream(stdin, argv[1]);
    }
    else
    {
        for (int i = 2; i < argc; i++)
        {
            FILE *fp = fopen(argv[i], "r");
            if (fp == NULL)
            {
                printf("wgrep: cannot open file\n");
                exit(1);
            }

            searchstream(fp, argv[1]);
            fclose(fp);
        }
    }
    return 0;
}