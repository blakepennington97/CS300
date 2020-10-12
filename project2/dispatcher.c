// Pennington.Jesse
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include "./sigtrap.c"

int job_dispatch_list[1000][1000];
int job_dispatch_list_length = 0;

// A linked list (LL) node to store a queue entry 
struct QNode { 
	int arrival_time;
    int priority;
    int processor_time; 
	struct QNode* next; 
}; 

// The queue, front stores the front node of LL and rear stores the 
// last node of LL 
struct Queue { 
	struct QNode *front, *rear; 
}; 

// A utility function to create a new linked list node. 
struct QNode* newNode(int arrival_time, int priority, int processor_time) 
{ 
	struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode)); 
	temp->arrival_time = arrival_time; 
	temp->priority = priority; 
	temp->processor_time = processor_time; 
	return temp; 
} 

// A utility function to create an empty queue 
struct Queue* createQueue() 
{ 
	struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue)); 
	q->front = q->rear = NULL; 
	return q; 
} 

// The function to add a key k to q 
void enQueue(struct Queue* q, int arrival_time, int priority, int processor_time) 
{ 
	// Create a new LL node 
	struct QNode* temp = newNode(arrival_time, priority, processor_time); 

	// If queue is empty, then new node is front and rear both 
	if (q->rear == NULL) { 
		q->front = q->rear = temp; 
		return; 
	} 

	// Add the new node at the end of queue and change rear 
	q->rear->next = temp; 
	q->rear = temp; 
} 

// Function to remove a key from given queue q 
void deQueue(struct Queue* q) 
{ 
	// If queue is empty, return NULL. 
	if (q->front == NULL) 
		return; 

	// Store previous front and move front one node ahead 
	struct QNode* temp = q->front; 

	q->front = q->front->next; 

	// If front becomes NULL, then change rear also as NULL 
	if (q->front == NULL) 
		q->rear = NULL; 

	free(temp); 
}

struct pcb {
    pid_t pid;
    char *args[10];
    int arrival_time;
    int remaining_time;
    struct pcb *next;
};

typedef struct pcb PCB;
typedef PCB *PcbPtr;

struct {
    int arrival_time;
    int priority;
    int processor_time;
} Process;


void printQueue(struct Queue *queue) {
    printf("\n\nJob Queue:\n");
    struct QNode *temp = (struct QNode*)malloc(sizeof(struct QNode));
    temp = queue->front;
    while (temp != NULL) {
        printf("%d ", temp->arrival_time);
        printf("%d ", temp->priority);
        printf("%d\n", temp->processor_time);
        temp = temp->next;
    }
    printf("Finished bitches.\n");
}

void FCFS() {
    //Intialize dispatcher queue
    struct Queue *dispatcher_queue = createQueue();
    //Fill dispatcher queue from dispatch list file
    int i;


    for (i = 0; i < job_dispatch_list_length; i++) {
        enQueue(dispatcher_queue, job_dispatch_list[i][0], job_dispatch_list[i][1], job_dispatch_list[i][2]);
    }
    printQueue(dispatcher_queue);
    //Start dispatcher timer (dispatcher timer = 0)
    clock_t start_time = clock();
    //While there's anything in the queue or there is a currently running process:
    pid_t pid = fork();
    //while(dispatcher_queue != NULL) {
        //If a process is currently running; 

    execvp("./process", dispatcher_queue->front->processor_time);
    //}
}

void readFromInputFile() {
    FILE *fp;
    char *line;
    size_t len = 0;
    ssize_t read;
    char *token;
    int i, j;

    fp = fopen("input.txt", "r");

    if (fp == NULL) {
        exit(0);
    }
    //parse
    for (i = 0; (read = getline(&line, &len, fp)) != -1; i++) {
        printf("Retrieved line of length %zu:\n", read);
        printf("%s", line);
        token = strtok(line, ",");
        j = 0;
        while (token != NULL) {
            job_dispatch_list[i][j] = atoi(token);
            token = strtok(NULL, ",");
            j++;
        }
    }
    job_dispatch_list_length = i;

    fclose(fp);
    if (line) {
        free(line);
    }
}

void printJobList() {
    printf("\n\nJob Dispatch List:\n");
    int i;
    for (i = 0; i < job_dispatch_list_length; i++) {
        printf("%d ", job_dispatch_list[i][0]);
        printf("%d ", job_dispatch_list[i][1]);
        printf("%d\n", job_dispatch_list[i][2]);
    }
}




void startProcess() {

}

void suspendProcess() {

}

void restartProcess() {

}

void terminateProcess() {

}



int main() {
    readFromInputFile();
    printJobList();
    FCFS();
    return 0;
}