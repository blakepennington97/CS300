#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int collatz_algo(int n) {
    if (n%2 == 0) {
        n = n / 2;
    }
    else {
        n = n * 3 + 1;
    }
    return n;
}

void input_check(int n) {
    if (n <= 0) {
        printf("Please provide an integer greater than zero.\n");
        exit(0);
    }
}

void arg_check(int n) {
    if (n != 2) {
        printf("Please enter one and only one argument.\n");
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    arg_check(argc);
    int starting_num = atoi(argv[1]);
    int n = starting_num;
    input_check(starting_num);

    pid_t pid;
    /* fork a child process */
    pid = fork();

    if (pid < 0) { /* error occurred */
        fprintf(stderr, "Fork Failed");
        return 1;
    }
    else if (pid == 0) { /* child process */
        while (n != 1) {
            printf("%d, ", n);
            n = collatz_algo(n);
        }
        printf("%d\n", n);
    }
    else { /* parent process */
        /* parent will wait for the child to complete */
        wait(NULL);
    }
    return 0;
}

