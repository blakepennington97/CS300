// Pennington.Jesse
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <wait.h>

// ****************************************************************************
// This is a multiprogramming system that runs and manages a process(s) (sigtrap.c)
// in the form of a multi-level priority queue.
// 
// The below code is a representation of creating a dispatcher for both user and
// system processes
//
// The dispatcher operates at four priority levels, with the highest priority level
// being the system level implemented as a FCFS queue. The remaining levels operate
// at decreasing priorities in a RR (round robin) fashion. Time quantum is 1 second.
//
// To execute this program, run make then >dispatcher.c sampleInput.txt
// ****************************************************************************
struct queue { 
    int arrival_time;
    int priority;
    int processor_time;
    int status;
    char *args[3];
	struct queue *next;
    pid_t pid;
}; 

typedef struct queue Queue;
typedef Queue *QueuePtr;

#define READY 0
#define RUNNING 1
#define SUSPENDED 2
#define TERMINATED 3


//inits queue node
QueuePtr createQueue() { 
	QueuePtr new_process = (QueuePtr)malloc(sizeof(Queue)); 
    new_process->pid = 0;
    new_process->args[0] = "./process";
    new_process->args[1] = NULL;
    new_process->arrival_time = 0;
    new_process->priority = 0;
    new_process->processor_time = 0;
    new_process->status = 0;
    new_process->next = NULL;
	return new_process; 
} 

//adds process to end of input_queue
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

//removes a process from front of input_queue
QueuePtr deQueue(QueuePtr *process) { 
	QueuePtr temp = *process;
    if (process && temp) {
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
        process->status = 0;
        input_queue = enQueue(input_queue, process);
    }
    fclose(fp);
    printf("finished reading file\n");
}


//starts a process
QueuePtr startProcess(QueuePtr processNode) {
    //if process isn't suspended, start process
    if (processNode->pid == 0) {
        processNode->pid = fork();
        if (processNode->pid == 0) {
            processNode->pid = getpid();
            processNode->status = RUNNING;
            execvp(processNode->args[0], processNode->args);
            perror(processNode->args[0]);
            exit(2);
        }
        else if (processNode->pid == -1) {
            printf("forking error\n");
            exit(1);
        }
    }
    processNode->status = RUNNING;
    return processNode;
}

//suspends a process
QueuePtr suspendProcess(QueuePtr processNode) {
    int status;
    kill(processNode->pid, SIGTSTP);
    waitpid(processNode->pid, &status, WUNTRACED);
    processNode->status = SUSPENDED;
    return processNode;
}


//terminates a process
QueuePtr terminateProcess(QueuePtr processNode) {
    int status;
    kill(processNode->pid, SIGINT);
    waitpid(processNode->pid, &status, WUNTRACED);
    processNode->status = TERMINATED;
    return processNode;
}


//restarts a process
QueuePtr restartProcess(QueuePtr processNode) {
    kill(processNode->pid, SIGCONT);
    processNode->status = RUNNING;
    return processNode;
}


int main(int argc, char **argv) {
    QueuePtr input_queue = NULL;
    QueuePtr dispatcher_queues[4];
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
    for (i = 0; i < 4; i++) {
        dispatcher_queues[i] = NULL;
    }

    //readFromInputFile(argv[1], &input_queue, &process);

    //read from file and fill input_queue 
    while (!feof(fp)) {
        process = createQueue();
        if (fscanf(fp, "%d, %d, %d", &(process->arrival_time), &(process->priority), &(process->processor_time)) != 3) {
            free(process);
            continue;
        }
        process->status = 0;
        //printf("Just read: %d %d %d\n",process->arrival_time, process->priority, process->processor_time);
        input_queue = enQueue(input_queue, process);
    }

    //while stuff in queue or process running
    while (input_queue || current_process || queueOrderTraversal(dispatcher_queues) >= 0) {
        //Unload pending processes from the input queue
        while (input_queue && input_queue->arrival_time <= timer) {
            process = deQueue(&input_queue);
            process->status = READY;
            //process->priority = 0;

            //adjust
            dispatcher_queues[process->priority] = enQueue(dispatcher_queues[process->priority], process);
        }

        //If a process is currently running
        if (current_process && current_process->status == RUNNING) {
            current_process->processor_time--;
            //if process runs out of time, kill it :(
            if (current_process->processor_time <= 0) {
                current_process = terminateProcess(current_process);
                free(current_process);
                current_process = NULL;
            }
            //if anything waiting, suspend current process and reduce priority
            else if (queueOrderTraversal(dispatcher_queues) >= 0) {
                suspendProcess(current_process);

                //reduce priority but don't make it greater than 3, and if priority is system level DO NOT change
                if ((current_process->priority <= 2) && (current_process->priority != 0)) {
                    current_process->priority++;
                }

                //place in correct priority order
                dispatcher_queues[current_process->priority] = enQueue(dispatcher_queues[current_process->priority], current_process);
                current_process = NULL;
            }
        }
        //if user queue NOT empty and no processes running, deQueue from user queue and run it
        if(!current_process && (i = queueOrderTraversal(dispatcher_queues)) >= 0) {
            // Dequeue a process from the highest priority feedback queue that is not empty
            current_process = deQueue(&dispatcher_queues[i]);

            if (current_process->status == SUSPENDED) {
                current_process = restartProcess(current_process);
            }
            else {
                current_process = startProcess(current_process);
            }
        }
        //increase timer and sleep
        sleep(1);
        timer++;
    }

    exit(0);
}