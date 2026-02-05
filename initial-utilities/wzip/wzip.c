#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        printf("wzip: file1 [file2 ...]\n");
        exit(1);
    }

    int first_read = 1;
    char current_ch = 0;
    int count = 0;

    for (int i = 1; i < argc; i++)
    {
        FILE *fp = fopen(argv[i], "r");
        if (fp == NULL)
        {
            printf("wzip: cannot open file\n");
            exit(1);
        }

        // Only read first character on the VERY FIRST file
        if (first_read)
        {
            if (fread(&current_ch, 1, 1, fp) > 0)
            {
                count = 1;
                first_read = 0; // mark flag first as 0
            }
        }

        char next_ch;

        while (fread(&next_ch, 1, 1, fp) > 0)
        {
            if (current_ch == next_ch)
            {
                count++;
            }
            else
            {
                /* Read the next 4-bytes (size of int) as int from the opened file */
                fwrite(&count, sizeof(int), 1, stdout);
                fwrite(&current_ch, 1, 1, stdout);
                count = 1;
                current_ch = next_ch;
            }
        }

        fclose(fp);
    }

    fwrite(&count, sizeof(int), 1, stdout);
    fwrite(&current_ch, 1, 1, stdout);
    exit(0);
}
