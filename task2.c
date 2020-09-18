#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

//=======================================================================================
//     Author: Blake Pennington
//     This program imitates a shell in C.
//     To use, run and run normal shell commands. To see history, enter history.
//     To use the ! feature, !x to run command labeled x as seen in history.
//     To use the & feature, enter the command followed by a &. Ex: ls&
//     You can also enter !! to automatically run the last command.
//=======================================================================================

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
    
    //iterates through commands
    for (i = 0; i < 10; i++) {
        printf("%d.  ", histCount);
        while (history[i][j] != '\n' && history[i][j] != '\0') {	
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


int format_command(char input_buffer[], char *args[], int *flag) {
   	int length;
    int i;     
    int start;  // index of beginning of next command
    int xx = 0; // index of where to place the next parameter into args[]
    int hist;
    
 	length = read(STDIN_FILENO, input_buffer, MAX_LINE);	
    start = -1;
    if (length == 0) {
        exit(0);  
    }
    if (length < 0) {
        printf("Command not read\n");
        exit(-1); 
    }
    
   //examines each character
    for (i = 0; i < length; i++) {
        switch (input_buffer[i]) {
            case ' ':
            case '\t':
                if (start != -1) {
                    args[xx] = &input_buffer[start];    
                    xx++;
                }
                input_buffer[i] = '\0'; //null char
                start = -1;
                break;
                
            case '\n': //final char 
                if (start != -1) {
                    args[xx] = &input_buffer[start];
                    xx++;
                }
                input_buffer[i] = '\0';
                args[xx] = NULL; // no more args
                break;
                
            default:           
                if (start == -1)
                    start = i;
                if (input_buffer[i] == '&') {
                    *flag = 1; 
                    input_buffer[i] = '\0';
                }
        }
    }
    
    args[xx] = NULL; // if the input line was > 80

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
        char str[MAX_LINE/2 + 1];
        str[0] = args[0][1];
        str[1] = args[0][2];

		int actual = atoi(str);

        if (actual > count) {
            printf("\nNo such command in the history\n");
            strcpy(input_buffer, "Nonexistent command");
        }
        else {
			if (str[0] == '!') { // Checking for '!!'
                if (*history[0] == '\0') {
                    printf("No commands in history.\n");
                }
                else {  
        	        strcpy(input_buffer, history[0]);  // this will be your last command
                }
			}
			else if (str[0] == '0') { // Checking for '!0'
				printf("Enter proper command");
				strcpy(input_buffer, "Nonexistant command");
			}
			else if (actual >= 1) { // Checking for '!n'
				strcpy(input_buffer, history[count - actual]);
			}
			
		}
	}

    for (i = 9; i > 0; i--) { // Moving the history elements one step higher
        strcpy(history[i], history[i - 1]);
    }
        
    strcpy(history[0], input_buffer); // Updating the history array with input buffer
    count++;

    /* if (count > 10) {
        count = 10;
    } */
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
