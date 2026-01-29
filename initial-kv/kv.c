#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct Node
{
    int key;
    char *value;

    struct Node *next;
} Node;

Node *head = NULL;

// functions prototypes
Node *createNode(int key, char *value);
void put(int key, char *value);
char *get(int key);
int delete_key(int key);
void clear();
void print_all();

// creates a node that stores values
Node *createNode(int key, char *value)
{
    Node *new_Node = malloc(sizeof(Node));
    new_Node->key = key;
    new_Node->value = strdup(value);

    new_Node->next = NULL;

    return new_Node;
}

// Store a key-value pair  ,If key exists, UPDATE the value , If key is new, ADD it

void put(int key, char *value)
{
    delete_key(key);
    Node *new_node = createNode(key, value);
    new_node->next = head;
    head = new_node;
}

// Find and return the value for a key
char *get(int key)
{
    Node *current;
    current = head;

    while (current != NULL)
    {
        if (current->key == key)
        {
            return current->value;
        }
        current = current->next;
    }

    return NULL;
}

// Remove a key-value pair from storage
int delete_key(int key)
{
    if (head == NULL)
        return 0;
    // case 1 delete the head
    if (head->key == key)
    {
        Node *temp;
        temp = head;
        head = head->next;
        free(temp->value); // Free the string first as it is also a pointer
        free(temp);
        return 1;
    }

    // Case 2: Delete middle or last node

    Node *current = head;
    while (current->next != NULL)
    {

        if (current->next->key == key)
        {
            Node *temp;
            temp = current->next;
            current->next = temp->next;
            free(temp->value);
            free(temp);
            return 1;
        }

        current = current->next;
    }

    return 0;
}

// remove all
void clear()
{
    Node *current = head;

    while (current != NULL)
    {
        Node *temp = current;
        current = current->next;
        free(temp->value);
        free(temp);
    }

    head = NULL;
}

// print all in database.txt

void print_all()
{
    Node *current = head;
    while (current != NULL)
    {
        printf("%i,%s\n", current->key, current->value);
        current = current->next;
    }
}

void save_database()
{
    FILE *fp = fopen("database.txt", "w");

    if (fp == NULL)
        return;
    Node *current = head;
    while (current != NULL)
    {
        fprintf(fp, "%d,%s\n", current->key, current->value);
        current = current->next;
    }

    fclose(fp);
}

void load_database()
{
    FILE *fp = fopen("database.txt", "r");
    if (fp == NULL)
        return;

    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL)
    {
        line[strcspn(line, "\n")] = '\0';

        char *key_s = strtok(line, ",");
        char *value = strtok(NULL, ",");

        if (key_s && value)
        {
            int key = atoi(key_s);
            put(key, value);
        }
    }
    fclose(fp);
}
int main(int argc, char *argv[])
{
    load_database();
    for (int i = 1; i < argc; i++)
    {

        char *cmd = strtok(argv[i], ",");
        if (cmd == NULL)
            continue;

        // commands that only require only charchter
        if (strcmp(cmd, "c") == 0)
        {
            clear();
        }
        else if (strcmp(cmd, "a") == 0)
        {
            print_all();
        }
        else
        {
            char *key_s = strtok(NULL, ",");
            // check if key exists before conversion to avoid NULL pointer dereference in atoi()
            if (key_s != NULL)
            {
                int key = atoi(key_s);

                if (strcmp(cmd, "p") == 0)
                {
                    char *value = strtok(NULL, ",");
                    put(key, value);
                }
                else if (strcmp(cmd, "g") == 0)
                {
                    char *str = get(key);
                    if (str == NULL)
                    {
                        printf("%d not found\n", key);
                    }
                    else
                    {
                        printf("%s\n", str);
                    }
                }
                else if (strcmp(cmd, "d") == 0)
                {
                    if (!delete_key(key))
                    {
                        printf("%d not found\n", key);
                    }
                }
                else
                {
                    printf("bad command\n");
                }
            }
        }
    }

    save_database();
    return 0;
}