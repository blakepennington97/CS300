#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 80 /* The maximum length of a command */
#define BUFFER_SIZE 50

char history[10][BUFFER_SIZE]; //stores history commands
int count = 0;


//displays history of commands
void display_history() {
    printf("Shell command history:\n");
    
    int i;
    int j = 0;
    int histCount = count;
    
    //loop for iterating through commands
    for (i = 0; i < 10; i++) {
        //command index
        printf("%d.  ", histCount);
        while (history[i][j] != '\n' && history[i][j] != '\0') {	
		//printing command
            printf("%c", history[i][j]);
            j++;
        }
        printf("\n");
        j = 0;
        histCount--;
        if (histCount == 0) {
            break;
        }
    }
    printf("\n");
} 



//Gets the command from shell, tokenizes it and set the args parameter
int format_command(char input_buffer[], char *args[], int *flag) {
   	int length; // # of chars in command line
    int i;     // loop index for input_buffer
    int start;  // index of beginning of next command
    int ct = 0; // index of where to place the next parameter into args[]
    int hist;
    
 	length = read(STDIN_FILENO, input_buffer, MAX_LINE);	
    start = -1;
    if (length == 0) {
        exit(0);   //end of command
    }
    if (length < 0) {
        printf("Command not read\n");
        exit(-1);  //terminate
    }
    
   //examines each character
    for (i = 0; i < length; i++) {
        switch (input_buffer[i]) {
            case ' ':
            case '\t':               // to seperate arguments
                if (start != -1) {
                    args[ct] = &input_buffer[start];    
                    ct++;
                }
                input_buffer[i] = '\0'; // add a null char at the end
                start = -1;
                break;
                
            case '\n': //final char 
                if (start != -1) {
                    args[ct] = &input_buffer[start];
                    ct++;
                }
                input_buffer[i] = '\0';
                args[ct] = NULL; // no more args
                break;
                
            default:           
                if (start == -1)
                    start = i;
                if (input_buffer[i] == '&') {
                    *flag = 1; //this flag is the differentiate whether the child process is invoked in background
                    input_buffer[i] = '\0';
                }
        }
    }
    
    args[ct] = NULL; // if the input line was > 80

    if(strcmp(args[0], "exit") == 0) {
        exit(0);
    }

    if (strcmp(args[0], "history") == 0) {		
        if (count > 0) {
		    display_history();
		}
		else {
		    printf("\nNo Commands in the history\n");
		}
		return -1;
    }

	else if (args[0][0] - '!' == 0) {	
        int x = args[0][1] - '0'; 
		int z = args[0][2] - '0'; 
		
		if (x > count) { // second letter check 
            printf("\nNo such command in the history\n");
            strcpy(input_buffer, "Nonexistent command");
		} 
		else if (z != -48) { // third letter check ('0' == 48)
            printf("\nNo such command in the history. History only extends to ten most recent.\n");
            strcpy(input_buffer, "Nonexistent command");
		}
		else {
			if (x == -15) { // Checking for '!!', ascii value of '!' is 33.
        	    strcpy(input_buffer, history[0]);  // this will be your 10th(last) command
			}
			else if (x == 0) { // Checking for '!0'
				printf("Enter proper command");
				strcpy(input_buffer, "Wrong command");
			}
			else if (x >= 1) { // Checking for '!n', n >=1
				strcpy(input_buffer, history[count - x]);
			}
			
		}
	}

    for (i = 9; i > 0; i--) { // Moving the history elements one step higher
        strcpy(history[i], history[i - 1]);
    }
        
    strcpy(history[0], input_buffer); // Updating the history array with input buffer
    count++;

    if (count > 10) {
        count = 10;
    }
}

int main(void) {
    char input_buffer[MAX_LINE]; // holds input command 
    int flag; // equals 1 if command is followed by &
    char *args[MAX_LINE/2 + 1]; // max arguments
    int should_run = 1;
    
    pid_t pid,tpid;
    int i;
   
    while (should_run) {            
        flag = 0; 
        printf("osh>");
        fflush(stdout);
        if (-1 != format_command(input_buffer, args, &flag)) { // get next command  
		    pid = fork();
        
        	if (pid < 0) { 
                printf("Fork failed.\n");
                exit(1);
        	}
       		else if (pid == 0) { 
           	 	// checks if command executed
                if (execvp(args[0], args) == -1) {
                    printf("Error executing command\n");
                }
       		}
       		// if flag == 0, parents waits, otherwise returns to the format_command()
        	else {
                i++;
           	 	if (flag == 0) {
                    i++;
                    wait(NULL);
           		}
            }
        }
    }
}
