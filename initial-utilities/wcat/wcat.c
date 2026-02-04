#include <stdio.h>
#include <stdlib.h>

#define MAX_BUFFER_SIZE 256

int main(int argc, char *argv[])
{

    // chekcs at least one file exist
    if (argc < 2)
    {
        exit(0);
    }

    for (int i = 1; i < argc; i++)
    {
        char *file = argv[i];
        FILE *fp = fopen(file, "r");

        if (fp == NULL)
        {
            printf("wcat: cannot open file\n");
            exit(1);
        }

        char my_buffer[MAX_BUFFER_SIZE];
        // If fgets returns NULL, the file is empty or finished
        while (fgets(my_buffer, MAX_BUFFER_SIZE, fp) != NULL)
        {
            printf("%s", my_buffer);
        }

        fclose(fp);
    }
}