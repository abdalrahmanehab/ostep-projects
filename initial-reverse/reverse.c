#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef struct Node
{
    char *line;
    struct Node *next;
} Node;

Node *head = NULL;

void createNode(char *st)
{
    Node *newnode = malloc(sizeof(Node));

    if (newnode == NULL)
    {
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }

    newnode->line = strdup(st);

    if (newnode->line == NULL)
    {
        fprintf(stderr, "malloc failed\n");
        exit(1);
    }

    newnode->next = head;

    head = newnode;
}

void printall()
{
    Node *current = head;

    while (current != NULL)
    {
        printf("%s", current->line);
        current = current->next;
    }
}

void save_database(char *fileName)
{
    FILE *fp = fopen(fileName, "w");

    if (fp == NULL)
    {
        fprintf(stderr, "error: cannot open file '%s'\n", fileName);
        exit(1);
    }

    Node *current = head;
    while (current != NULL)
    {
        fprintf(fp, "%s", current->line);
        current = current->next;
    }

    fclose(fp);
}

int main(int argc, char *argv[])
{
    if (argc > 3)
    {
        fprintf(stderr, "usage: reverse <input> <output>\n");
        exit(1);
    }

    if (argc == 3)
    {
        struct stat stat1, stat2;
        if (stat(argv[1], &stat1) == 0 && stat(argv[2], &stat2) == 0)
        {
            if (stat1.st_ino == stat2.st_ino && stat1.st_dev == stat2.st_dev)
            {
                fprintf(stderr, "reverse: input and output file must differ\n");
                exit(1);
            }
        }
    }

    // deafault to stdin
    FILE *fp = stdin;

    // in case there is an input file
    if (argc >= 2)
    {
        fp = fopen(argv[1], "r");
        if (fp == NULL)
        {
            fprintf(stderr, "reverse: cannot open file '%s'\n", argv[1]);
            exit(1);
        }
    }

    char *line = NULL;
    size_t n = 0;

    while (getline(&line, &n, fp) != EOF)
    {
        createNode(line);
    }

    if (argc == 3)
    {
        save_database(argv[2]);
    }
    else
    {
        printall();
    }

    if (argc >= 2)
    {
        fclose(fp);
    }
    free(line);

    return 0;
}