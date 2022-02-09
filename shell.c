#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>

// #include <sys/stat.h>
// #include <sys/types.h>

#define BUFFER_SIZE 2000
#define TOKENS_SIZE 2000
#define CWD_MAX 2000

int listDir(char *dir)
{
    DIR *dip;
    struct dirent *dit;

    int i = 0;

    /* check to see if user entered a directory name */
    if (dir == NULL)
    {
        printf("Usage: %s <directory>\n", dir);
        return 0;
    }

    /* DIR *opendir(const char *name);
     *
     * Open a directory stream to argv[1] and make sure
     * it's a readable and valid (directory) */
    if ((dip = opendir(dir)) == NULL)
    {

        dip = opendir(".");

        // perror("opendir");//prints out error
        // tests different errors.
        if (errno == ENOTDIR)
        {
            printf("Not Directory\n");
        }
        if (errno == EACCES)
        {
            printf("Permission Denied\n");
        }
        if (errno == ENOENT)
        {
            printf("File not Found\n");
        }
        return 0;
    }

    /*  struct dirent *readdir(DIR *dir);
     *
     * Read in the files from argv[1] and print */
    while ((dit = readdir(dip)) != NULL)
    {
        i++;
        if (dit->d_type == DT_DIR)
            printf("\nDirectory: %s", dit->d_name);
        else
            printf("\n%s", dit->d_name);
    }

    printf("\n\nreaddir() found a total of %i files\n", i);

    /* int closedir(DIR *dir);
     *
     * Close the stream to argv[1]. And check for errors. */
    if (closedir(dip) == -1)
    {
        perror("closedir");
        return 0;
    }

    return 1;
}

void execute_normal(char **argv)
{

    pid_t pid;
    int status;

    pid = fork();
    if (pid < 0)
    {
        printf("ERROR fork failed\n");
        exit(1);
    }
    else if (pid == 0) // child thread
    {
        if (execvp(*argv, argv) < 0)
        {
            perror("execvp");
            exit(1);
        }
    }
    else
    {
        while (wait(&status) != pid)
        {
        } // parent, waits for completion
    }
}

void execute_redirect(char **argv, char *outPath)
{

    pid_t pid;
    int status;
    int defout;
    int fd;

    pid = fork();
    if (pid < 0)
    {
        printf("ERROR fork failed\n");
        exit(1);
    }
    else if (pid == 0) // child thread
    {

        defout = dup(1);
        fd = open(outPath, O_RDWR | O_CREAT, 0644);

        dup2(fd, 1);
        if (execvp(*argv, argv) < 0)
        {
            perror("execvp");
            exit(1);
        }
        close(fd);
    }
    else
    {
        while (wait(&status) != pid)
        {
        } // parent, waits for completion
    }
}

int execute_detect(char **argv)
{
    int result = -1;
    int i = 0;

    while (argv[i] != 0)
    {
        if (strcmp(argv[i], ">") == 0)
        {
            result = i;
        }

        i++;
    }

    return result;
}

int main(int argc, char **argv)
{

    // variables that needed for the ls system call.

    while (1)
    {

        char buffer[BUFFER_SIZE];
        char *tokens[TOKENS_SIZE];
        char cwd[CWD_MAX];

        // set the token size to 0.
        int num_tokens = 0;

        char *token_ptr;

        printf("%s$$$ ", getcwd(cwd, sizeof(cwd)));

        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        // Tokenize
        token_ptr = strtok(buffer, " ");
        while (token_ptr != NULL)
        {
            tokens[num_tokens] = token_ptr;
            num_tokens += 1;

            token_ptr = strtok(NULL, " ");
        }

        tokens[num_tokens] = 0;

        if (strcmp(tokens[0], "echo") == 0)
        {
            // print rest of the tokens
            for (int i = 1; i < num_tokens; i++)
            {
                if (i == (num_tokens - 1))
                {
                    printf("%s\n", tokens[i]);
                }
                else
                {
                    printf("%s ", tokens[i]);
                }
            }
        }
        else if (strcmp(tokens[0], "exit") == 0)
        {
            printf("Exiting\n");
            break;
        }
        else if (strcmp(tokens[0], "ls") == 0)
        {

            if (tokens[1] == NULL)
            {
                listDir(".");
            }
            else
            {
                listDir(tokens[1]);
            }
        }
        else if (strcmp(tokens[0], "cd") == 0)
        {
            int result = chdir(tokens[1]);
            if (result == -1)
            {
                perror("Error");
            }
            // perror("There are no directroy."); How to deal with the serror.
        }
        else if (strcmp(tokens[0], "mkdir") == 0)
        {
            char *dirName = tokens[1];
            int result = mkdir(dirName, S_IRWXU);
            if (result == -1)
            {
                perror("Error");
            }
        }
        else if (strcmp(tokens[0], "rmdir") == 0)
        {
            char *dirName = tokens[1];
            int result = rmdir(dirName);
            if (result == -1)
            {
                perror("Error");
            }
        }
        else
        {

            int index = 0;

            int isThereSign = -1;

            while (tokens[index] != 0)
            {
                if (strcmp(tokens[index], ">") == 0)
                {
                    isThereSign = index;
                    break;
                }
                index++;
            }

            if (isThereSign != -1)
            {
                execute_redirect(tokens, tokens[isThereSign + 1]);
            }
            else
            {
                execute_normal(tokens);
            }
        }
    }
}