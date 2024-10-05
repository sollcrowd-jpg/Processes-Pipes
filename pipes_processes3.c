#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <search_term>\n", argv[0]);
        return 1;
    }

    int pipefd1[2], pipefd2[2];
    pid_t pid1, pid2;

    // Create the first pipe (for cat -> grep)
    if (pipe(pipefd1) == -1) {
        perror("pipe1");
        exit(1);
    }

    // Fork to create the first child (grep process)
    pid1 = fork();
    if (pid1 < 0) {
        perror("fork1");
        exit(1);
    }

    if (pid1 == 0) {
        // We're in the first child (P2: grep <search_term>)

        // Create the second pipe (for grep -> sort)
        if (pipe(pipefd2) == -1) {
            perror("pipe2");
            exit(1);
        }

        // Fork to create the second child (sort process)
        pid2 = fork();
        if (pid2 < 0) {
            perror("fork2");
            exit(1);
        }

        if (pid2 == 0) {
            // We're in the second child (P3: sort)

            // Replace standard input with the read end of pipe 2 (grep -> sort)
            dup2(pipefd2[0], 0);

            // Close all pipe file descriptors
            close(pipefd1[0]);
            close(pipefd1[1]);
            close(pipefd2[1]);
            close(pipefd2[0]);

            // Execute sort
            execlp("sort", "sort", NULL);
            perror("exec sort failed");
            exit(1);
        } else {
            // We're in the first child (P2: grep)

            // Replace standard input with the read end of pipe 1 (cat -> grep)
            dup2(pipefd1[0], 0);

            // Replace standard output with the write end of pipe 2 (grep -> sort)
            dup2(pipefd2[1], 1);

            // Close all pipe file descriptors
            close(pipefd1[0]);
            close(pipefd1[1]);
            close(pipefd2[0]);
            close(pipefd2[1]);

            // Execute grep with the search term passed from argv[1]
            execlp("grep", "grep", argv[1], NULL);
            perror("exec grep failed");
            exit(1);
        }
    } else {
        // We're in the parent process (P1: cat scores)

        // Replace standard output with the write end of pipe 1 (cat -> grep)
        dup2(pipefd1[1], 1);

        // Close all pipe file descriptors
        close(pipefd1[0]);
        close(pipefd1[1]);

        // Execute cat scores
        execlp("cat", "cat", "scores", NULL);
        perror("exec cat failed");
        exit(1);
    }

    return 0;
}
