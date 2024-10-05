#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    // Two pipes: fd1[] for P1 -> P2 and fd2[] for P2 -> P1 communication
    int fd1[2], fd2[2]; 
    char fixed_str[] = "howard.edu";
    char fixed_str2[] = "gobison.org";
    char input_str[100];
    char second_input[100];
    char concat_str[200];
    pid_t p;

    // Create pipes
    if (pipe(fd1) == -1 || pipe(fd2) == -1) {
        fprintf(stderr, "Pipe creation failed\n");
        return 1;
    }

    // Create child process
    p = fork();

    if (p < 0) {
        fprintf(stderr, "Fork failed\n");
        return 1;
    }

    // Parent process: P1
    else if (p > 0) {
        close(fd1[0]); // Close reading end of first pipe (P1 -> P2)
        close(fd2[1]); // Close writing end of second pipe (P2 -> P1)

        printf("Enter a string to concatenate: ");
        scanf("%s", input_str);

        // Write input string to pipe and close writing end
        write(fd1[1], input_str, strlen(input_str) + 1);
        close(fd1[1]);

        // Wait for child to send concatenated string
        wait(NULL);

        // Read the second string from P2
        read(fd2[0], second_input, 100);

        // Concatenate second string with "gobison.org"
        int k = strlen(second_input);
        int i;  // Declare variable outside the loop
        for (i = 0; i < strlen(fixed_str2); i++) {
            second_input[k++] = fixed_str2[i];
        }
        second_input[k] = '\0'; // null-terminate the final string

        printf("Final concatenated string: %s\n", second_input);
        close(fd2[0]); // Close reading end of second pipe
    }

    // Child process: P2
    else {
        close(fd1[1]); // Close writing end of first pipe (P1 -> P2)
        close(fd2[0]); // Close reading end of second pipe (P2 -> P1)

        // Read string from P1
        read(fd1[0], concat_str, 100);
        close(fd1[0]);

        // Concatenate "howard.edu" to the string received from P1
        int k = strlen(concat_str);
        int i;  // Declare variable outside the loop
        for (i = 0; i < strlen(fixed_str); i++) {
            concat_str[k++] = fixed_str[i];
        }
        concat_str[k] = '\0'; // null-terminate the final string

        printf("Concatenated string: %s\n", concat_str);

        // Ask for a second input to send back to P1
        printf("Enter a string to send back to P1: ");
        scanf("%s", second_input);

        // Send second string to P1 and close writing end of second pipe
        write(fd2[1], second_input, strlen(second_input) + 1);
        close(fd2[1]);

        exit(0);
    }

    return 0;
}
