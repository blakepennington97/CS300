// Pennington.Jesse
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <wait.h>

#define UNDEFINED 0
#define INITIALIZED 1
#define READY 2
#define RUNNING 3
#define SUSPENDED 4
#define TERMINATED 5

// A linked list (LL) node to store a queue entry 
struct queue { 
    pid_t pid;
    char *args[3];
	int arrival_time;
    int priority;
    int processor_time; 
    int status;
	struct queue *next; 
}; 

typedef struct queue Queue;
typedef Queue *QueuePtr;

QueuePtr startProcess(QueuePtr);
QueuePtr suspendProcess(QueuePtr);
QueuePtr terminateProcess(QueuePtr);
int queueOrderTraversal(QueuePtr *);
QueuePtr createQueue();
QueuePtr enQueue(QueuePtr, QueuePtr);
QueuePtr deQueue(QueuePtr *);

// A utility function to create an empty queue 
QueuePtr createQueue() { 
	QueuePtr new_process = (QueuePtr)malloc(sizeof(Queue)); 
    new_process->pid = 0;
    new_process->args[0] = "./process";
    new_process->args[1] = NULL;
    new_process->arrival_time = 0;
    new_process->priority = 0;
    new_process->processor_time = 0;
    new_process->status = UNDEFINED;
    new_process->next = NULL;
	return new_process; 
} 

// Function to add process to the end of input_queue
QueuePtr enQueue(QueuePtr input_queue, QueuePtr process) { 
    QueuePtr temp = input_queue;
    process->next = NULL;
    //if queue contains entries already
    if (input_queue) {
        while (input_queue->next) {
            input_queue = input_queue->next;
        }
        input_queue->next = process;
        return temp;
    }
    return process;
} 

// Function to remove a process from front of input_queue
QueuePtr deQueue(QueuePtr *process) { 
	QueuePtr temp;
    //could i do temp = process, then && (temp)?
    if (process && (temp = *process)) {
        *process = temp->next;
        return temp;
    }
    return NULL;
}

//looks at multi-level queues and returns largest non-empty queue
int queueOrderTraversal(QueuePtr *temp) {
    int i;
    int selected_queue;

    for (i = 0; i < 4; i++) {
        if (temp[i]) {
            selected_queue = i;
            return selected_queue;
        }
    }
    //else return negative
    return -1;
}

void readFromInputFile(char *file_name, QueuePtr input_queue, QueuePtr process) {
    FILE *fp;
    fp = fopen(file_name, "r");

    if (fp == NULL) {
        printf("Error in opening input file.\n");
        exit(0);
    }

    //parse input file and put into the input_queue
    while (!feof(fp)) {
        process = createQueue();
        if (fscanf(fp, "%d, %d, %d", &(process->arrival_time), &(process->priority), &(process->processor_time)) != 3) {
                free(process);
                continue;
        }
        process->status = INITIALIZED;
        //printf("Just read: %d %d %d\n",process->arrival_time, process->priority, process->processor_time);
        input_queue = enQueue(input_queue, process);
    }
    fclose(fp);
    printf("finished reading file\n");
}

QueuePtr startProcess(QueuePtr processNode) {
    //if process isn't suspended, start process
    if (processNode->pid == 0) {
        // processNode->pid = fork();
        // if (processNode->pid == 0) {
        //     processNode->pid = getpid();
        //     processNode->status = RUNNING;
        //     execvp(processNode->args[0], processNode->args);
        //     perror(processNode->args[0]);
        //     exit(2);
        // }
        // else if (processNode->pid == -1) {
        //     printf("forking error\n");
        //     exit(1);
        // }
        switch (processNode->pid = fork()) {
            case -1:
                perror("startProcess");
                exit(1);
            case 0:
                processNode->pid = getpid();
                processNode->status = RUNNING;
                execvp (processNode->args[0], processNode->args);
                perror (processNode->args[0]);
                exit (2);
        }
    }
    //restart the process
    else {
        kill(processNode->pid, SIGCONT);
        //restartProcess() here?
    }
    processNode->status = RUNNING;
    return processNode;
}

QueuePtr suspendProcess(QueuePtr processNode) {
    int status;
    kill(processNode->pid, SIGTSTP);
    waitpid(processNode->pid, &status, WUNTRACED);
    processNode->status = SUSPENDED;
    return processNode;
}

QueuePtr terminateProcess(QueuePtr processNode) {
    int status;
    kill(processNode->pid, SIGINT);
    waitpid(processNode->pid, &status, WUNTRACED);
    processNode->status = TERMINATED;
    return processNode;
}

QueuePtr restartProcess(QueuePtr processNode) {
    int status;
    kill(processNode->pid, SIGCONT);
    processNode->status = RUNNING;
    return processNode;
}





int main(int argc, char **argv) {
    QueuePtr input_queue = NULL;
    QueuePtr user_queue[4];
    QueuePtr current_process = NULL;
    QueuePtr process = NULL;
    FILE *fp;
    int i;
    int timer = 0;

    //printf("file name: %s\n", argv[1]);
    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("Error in opening file.\n");
        exit(1);
    }

    if (argc < 2) {
        printf("Usage: ./dispatcher.c <sampleInput.txt>\n");
        exit(1);
    }

    //init array full of NULLs
    // for (i = 0; i < 4; i++) {
    //     user_queue[i] = NULL;
    // }
    for(i = 0; i < 4; user_queue[i++] = NULL);

    //readFromInputFile(argv[1], &input_queue, &process);
    while (!feof(fp)) {
        process = createQueue();
        if (fscanf(fp, "%d, %d, %d", &(process->arrival_time), &(process->priority), &(process->processor_time)) != 3) {
            free(process);
            continue;
        }
        process->status = INITIALIZED;
        //printf("Just read: %d %d %d\n",process->arrival_time, process->priority, process->processor_time);
        input_queue = enQueue(input_queue, process);
    }
    //while stuff in queue or process running
    while (input_queue || current_process || queueOrderTraversal(user_queue) >= 0) {
        //while stuff in input_queue and arrival and timer match
        while (input_queue && input_queue->arrival_time <= timer) {
            process = deQueue(&input_queue);
            process->status = READY;
            //process->priority = 0;
            //adjust
            user_queue[process->priority] = enQueue(user_queue[process->priority], process);
        }
        if (current_process && current_process->status == RUNNING) {
            current_process->processor_time--;
            //if process runs out of time, kill it :(
            if (current_process->processor_time <= 0) {
                current_process = terminateProcess(current_process);
                free(current_process);
                current_process = NULL;
            }
            //if anything waiting, suspend current process and reduce priority
            else if (queueOrderTraversal(user_queue) >= 0) {
                suspendProcess(current_process);
                //correction
                //CHANGES MADE HERE
                if ((current_process->priority) <= 2) {
                    current_process->priority++;
                }
                //place in correct priority order
                user_queue[current_process->priority] = enQueue(user_queue[current_process->priority], current_process);
                current_process = NULL;
            }
        }
        //if user queue NOT empty and no processes running, deQueue from user queue and run it
        if(!current_process && (i = queueOrderTraversal(user_queue)) >= 0) {
            // Dequeue a process from the highest priority feedback queue that is not empty
            //current_process not receiving correctly?
            current_process = deQueue(&user_queue[i]);
            // b. If already started but suspended, restart it (send SIGCONT to it)
            if (current_process->status == SUSPENDED) {
                current_process = restartProcess(current_process);
            }
            // else start it (fork & exec)
            else {
                current_process = startProcess(current_process);
            }
            // c. Set it as currently running process;
            //i = queueOrderTraversal(user_queue);
            // current_process = deQueue(&user_queue[i]);
            // startProcess(current_process);
        }
        //increase time and sleep
        //printf("me sleep\n");
        sleep(1);
        timer++;
    }

    exit(0);
}