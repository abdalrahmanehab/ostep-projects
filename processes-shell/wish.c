#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#define ERR write(STDERR_FILENO, "An error has occurred\n", 22)

char *args[100];
char *path[100];
int path_count = 3;

char *findpath(char *command)
{
    char fullpath[256];
    for (int i = 0; i < path_count; i++)
    {
        sprintf(fullpath, "%s/%s", path[i], command);
        if (access(fullpath, X_OK) == 0)
        {
            return strdup(fullpath);
        }
    }

    return NULL;
}

pid_t spawncommand(char **cmd, char *outputfile, int redirect)
{
    int rc = fork();
    if (rc == 0)
    {
        if (redirect)
        {
            int fd = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0)
            {
                ERR;
                exit(1);
            }
            dup2(fd, 1);
            dup2(fd, 2);
            close(fd);
        }
        char *fullpath = findpath(cmd[0]);
        if (fullpath == NULL)
        {
            ERR;
            exit(1);
        }
        execv(fullpath, cmd);
        ERR;
        exit(1);
    }
    return rc;
}

void runcommand(char **cmd, char *outputfile, int redirect)
{
    // fork() creates another child process the run the command
    int rc = fork();

    // if rc = 0 it means it is the child process
    if (rc == 0)
    {
        if (redirect)
        {
            int fd = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0)
            {
                ERR;
                exit(1);
            }
            dup2(fd, 1); // stdout file
            dup2(fd, 2); // stderr file
            close(fd);
        }

        char *fullpath = findpath(cmd[0]);
        if (fullpath == NULL)
        {
            ERR;
            exit(1);
        }
        execv(fullpath, cmd);
        // the program continues if execv have not reached the path of the command
        ERR;
        exit(1);
    }
    else if (rc > 0)
    {
        // the parent process waits for the child to continue the shell
        wait(NULL);
    }
    else
    {
        ERR;
    }
}

int main(int arg_counter, char *argv[])
{
    FILE *in;
    if (arg_counter == 1)
    {
        in = stdin;
    }
    else if (arg_counter == 2)
    {
        in = fopen(argv[1], "r");
        if (in == NULL)
        {
            ERR;
            exit(1);
        }
    }
    else
    {
        ERR;
        exit(1);
    }

    char *line = NULL;
    size_t buf = 0;

    int argc;

    path[0] = strdup("/bin");
    path[1] = strdup("/usr/bin");
    path[2] = strdup("/usr/local/bin");

    while (1)
    {
        argc = 0;

        if (in == stdin)
        {
            printf("wish> ");
            fflush(stdout);
        }

        // quit the shell if it is used in batch mode
        if (getline(&line, &buf, in) == -1)
        {
            exit(0);
        }

        // add spaces around > and & so strtok can find them without spaces
        char newline[1024];
        int k = 0;
        for (int i = 0; line[i] != '\0'; i++)
        {
            if (line[i] == '>' || line[i] == '&')
            {
                newline[k++] = ' ';
                newline[k++] = line[i];
                newline[k++] = ' ';
            }
            else
            {
                newline[k++] = line[i];
            }
        }
        newline[k] = '\0';

        char *token = strtok(newline, " \t\n");
        while (token != NULL)
        {
            args[argc] = token;
            argc++;
            token = strtok(NULL, " \t\n");
        }
        args[argc] = NULL;

        // split into parallel commands at &
        char **cmds[100];
        int cmd_count = 0;
        cmds[0] = &args[0];
        cmd_count = 1;

        for (int i = 0; i < argc; i++)
        {
            if (strcmp(args[i], "&") == 0)
            {
                args[i] = NULL;
                cmds[cmd_count] = &args[i + 1];
                cmd_count++;
                if (i == 0)
                    argc = 0; // first command was empty
            }
        }

        pid_t pids[100];
        for (int i = 0; i < 100; i++)
            pids[i] = -1; // Initialize PIDs

        // Process each command independently
        for (int i = 0; i < cmd_count; i++)
        {
            if (cmds[i][0] == NULL)
                continue; // skip empty commands

            char *cmd_outputfile = NULL;
            int cmd_redirect = 0;
            int err_flag = 0;
            int redirect_count = 0;
            int redirect_idx = -1;

            // Check for redirection in THIS specific command
            for (int j = 0; cmds[i][j] != NULL; j++)
            {
                if (strcmp(cmds[i][j], ">") == 0)
                {
                    redirect_count++;
                    if (redirect_idx == -1) redirect_idx = j;
                }
            }

            if (redirect_count > 1)
            {
                err_flag = 1; // More than one > is an error
            }
            else if (redirect_count == 1)
            {
                cmd_redirect = 1;
                if (cmds[i][redirect_idx + 1] != NULL)
                {
                    cmd_outputfile = cmds[i][redirect_idx + 1];
                    // If there's another token after the filename, it's an error
                    if (cmds[i][redirect_idx + 2] != NULL)
                    {
                        err_flag = 1;
                    }
                }
                else
                {
                    err_flag = 1; // Nothing after >
                }
                cmds[i][redirect_idx] = NULL; // Cut off arguments before >
            }

            if (err_flag || cmds[i][0] == NULL)
            {
                ERR;
                continue; // Skip this command, but let others run
            }

            // built-in commands evaluated PER parallel block
            if (strcmp(cmds[i][0], "exit") == 0)
            {
                int exit_args = 0;
                while (cmds[i][exit_args] != NULL) exit_args++;
                
                if (exit_args != 1)
                {
                    ERR;
                }
                else
                {
                    exit(0);
                }
            }
            else if (strcmp(cmds[i][0], "cd") == 0)
            {
                int cd_args = 0;
                while (cmds[i][cd_args] != NULL) cd_args++;

                if (cd_args != 2)
                {
                    ERR;
                }
                else
                {
                    if (chdir(cmds[i][1]) != 0)
                    {
                        ERR;
                    }
                }
            }
            else if (strcmp(cmds[i][0], "path") == 0)
            {
                path_count = 0;
                int p_idx = 1;
                while (cmds[i][p_idx] != NULL)
                {
                    path[path_count] = strdup(cmds[i][p_idx]);
                    path_count++;
                    p_idx++;
                }
            }
            else if (strcmp(cmds[i][0], "loop") == 0)
            {
                int loop_args = 0;
                while (cmds[i][loop_args] != NULL) loop_args++;

                if (loop_args < 3)
                {
                    ERR;
                    continue;
                }
                
                int num = atoi(cmds[i][1]);

                // Loop 1 to N to prevent overwriting strings incorrectly
                for (int iter = 1; iter <= num; iter++)
                {
                    char *loop_cmd[100];
                    int k = 0;
                    for (int j = 2; cmds[i][j] != NULL; j++)
                    {
                        if (strcmp(cmds[i][j], "%loop%") == 0)
                        {
                            char num_str[20];
                            sprintf(num_str, "%d", iter);
                            loop_cmd[k++] = strdup(num_str);
                        }
                        else
                        {
                            loop_cmd[k++] = cmds[i][j]; // keep original
                        }
                    }
                    loop_cmd[k] = NULL;

                    pid_t loop_pid = spawncommand(loop_cmd, cmd_outputfile, cmd_redirect);
                    if (loop_pid > 0) {
                        waitpid(loop_pid, NULL, 0); 
                    }
                }
            }
            else
            {
                // External Command
                pids[i] = spawncommand(cmds[i], cmd_outputfile, cmd_redirect);
            }
        }

        // Wait only for the processes we actually spawned
        for (int i = 0; i < cmd_count; i++)
        {
            if (pids[i] > 0)
            {
                waitpid(pids[i], NULL, 0);
            }
        }
    }
    return 0; // standard practice to return 0 at the end of main
}