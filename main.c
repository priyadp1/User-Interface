#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <termios.h>
#include <fcntl.h>
#define MAX_LINE 80
#define HISTORY_SIZE 5

//Global Variables
char *args[MAX_LINE / 2 + 1];
char input[MAX_LINE];
char commandHistory[HISTORY_SIZE][MAX_LINE];
int historyIndex = 0;
int countHistory = 0;
int currIndex = -1;
char lastCommand[MAX_LINE] = "";
int size = 1000000;
char haystack[1000000];
char *inFile;
char *outFile;
int background = 0;
int isPipe = 0;
int startCommand = 0;
struct termios old;
struct termios new;
//Reset terminal back to normal state (For everything but arrow keys)
void resetRaw(void){
    tcsetattr(STDIN_FILENO , TCSANOW, &old);
}
//Set terminal to raw state (For arrow keys)
void setRaw(void){
    tcgetattr (STDIN_FILENO, &old);
    new = old;
    new.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new);
    
}
//Add command to history for arrow key implementation. Basically adds the command to the commandHistory array which is the history buffer
void addHistory(const char *command){
if(strlen(command) == 0){
    return;
}
strcpy(commandHistory[historyIndex] , command);
commandHistory[historyIndex][MAX_LINE - 1] = '\0';
historyIndex = (historyIndex + 1) % HISTORY_SIZE;
if(countHistory < HISTORY_SIZE){
    countHistory++;
}
currIndex = historyIndex;
}
//Reads Arrow Key Implementation.
void readCommands(char* input){
    memset(input, 0, MAX_LINE);
    int i = 0;

    while (1) {
        char c;
        int notRead = read(STDIN_FILENO, &c, 1);

        if (notRead <= 0) {
            perror("read");
            exit(1);
        }

        if (c == '\n') {//Newline
            input[i] = '\0';
            printf("\n");
            break;
        } 
        else if (c == 127) {//Backspace
            if (i > 0) {
                i--;
                input[i] = '\0';
                printf("\r\033[Kosh> %s", input); 
                fflush(stdout);
            }
        } 
        else if (c == '\033') {//Escape Sequence
            char seq[2];
            if (read(STDIN_FILENO, &seq[0], 1) == 1 && read(STDIN_FILENO, &seq[1], 1) == 1) {
                if (seq[0] == '[') { //Arrow Key Implementation
                    if (seq[1] == 'A' && countHistory > 0) { //Up Arrow
                        if(currIndex == 0){
                            currIndex = countHistory - 1;
                        } 
                        else{
                            currIndex -= 1;
                        }
                        strcpy(input, commandHistory[currIndex]);
                        i = strlen(input);
                        printf("\r\033[Kosh> %s", input);
                        fflush(stdout);
                    } 
                    else if (seq[1] == 'B' && countHistory > 0) { //Down Arrow
                        if (currIndex != historyIndex) {
                            currIndex = (currIndex + 1) % HISTORY_SIZE;
                            strcpy(input, commandHistory[currIndex]);
                            i = strlen(input);
                        } 
                        else {
                            input[0] = '\0';
                            i = 0;
                        }
                        printf("\r\033[Kosh> %s", input);
                        fflush(stdout);
                    }
                }
            }
        } 
        else if (i < MAX_LINE - 1) { 
            input[i++] = c;
            putchar(c);
            fflush(stdout); 
        }
    }
}
//Tokenize the string that the user inputs in the command line
void tokenize(char *array, char *args[], char **inFile, char **outFile, int* background, int *isPipe, int *pipeIndex) {
    char *string = strtok(array, " ");
    int i = 0;
    *inFile = NULL;
    *outFile = NULL;
    *background = 0;
    *isPipe = 0;
    int startCommand = 0;
    while (string != NULL) {
        //Checks for background process
        if(strcmp(string, "&") == 0){
            *background = 1;
        }
        else if(strcmp(string, "|") == 0){
            //Checks for pipes
           *isPipe = 1;
           args[i] = NULL;
           *pipeIndex = i + 1;
           startCommand = i + 1;
           i++;
        }
        else if(strcmp(string, "<") == 0){
            //Checks for input redirection
            *inFile = strtok(NULL, " ");
        }
        else if(strcmp(string, ">" ) == 0){
            //Checks for output redirection
            *outFile = strtok(NULL, " ");
        }
        else{
            //Continues going through the user input
        args[i++] = string;
    }
        string = strtok(NULL, " ");
    }
    args[i] = NULL;
}
void pipeCommand(char *first[] , char *second[]){
    //Pipe steps:
    //1. Detect Pipes (tokenize) DONE
    //2. Create global variable of pipeffd[2] DONE
    //3.Create Pipe in child
    //Set flag = 1/0 (MAIN)
    //3a. Open pipe with write end (write smth)
    //4. Close both ends of pipe
    //5. In parent, use an integer flag to see if a pipe exists
    //6.If a pipe exists, fork another child process, and that child process reads from the pipe using pipefd
    //Close both read and write pipes
    //7.Reset flag
    //8. Wait on child
    //Close both read and write pipes again
    //9.Done
int pipefd[2];//Create Pipe
pid_t pid1, pid2; // Create 2 pids
if(pipe(pipefd) == -1){//Error checking
    perror("Failed to pipe");
    exit(EXIT_FAILURE);
}
pid1 = fork();//Fork pid 1
if(pid1 == 0){
    close(pipefd[0]);
    dup2(pipefd[1] , STDOUT_FILENO);
    close(pipefd[1]);

        execvp(first[0], first);
        perror("execvp");
        exit(EXIT_FAILURE);
}
pid2 = fork();//Fork pid 2
if(pid2 == 0){
    close(pipefd[1]);
    dup2(pipefd[0] , STDIN_FILENO);
    close(pipefd[0]);

        execvp(second[0], second);
        perror("execvp");
        exit(EXIT_FAILURE);
}
//Close everything and waitpid for everything
close(pipefd[0]);
close(pipefd[1]);
waitpid(pid1, NULL, 0);
waitpid(pid2, NULL, 0);

}
int main(void) {
    //For & command don't call wait
    
    printf("Welcome to Prisha's shell!\n");
    memset(commandHistory, 0, sizeof(commandHistory));

    //Use loop to make terminal commands
    setRaw();
    char c;
    while (1) {
        //Get working directory using strrchr
        if (getcwd(haystack, size) != NULL) {
            char *needle = strrchr(haystack, '/');
            if (needle != NULL) {
                needle++;
            } 
            else {
                needle = haystack;
            }
            printf("osh>%s> ", needle);
        } 
        else {
            perror("Error in changing directory");
            continue;
        }
        fflush(stdout);

        //read Commands in user interface
        readCommands(input);
        resetRaw();
        if(strlen(input) == 0){
            continue;
        }
        //Check if input is equal to exit
        if (strcmp(input, "exit") == 0) {
            fprintf(stderr, "%s", "Bye!\n");
            exit(0);
        }
        addHistory(input);
        //!! command
        if (strcmp(input, "!!") == 0) {
            if (strlen(lastCommand) == 0) {
                printf("No commands in history\n");
                continue;
            }
            printf("%s\n", lastCommand);
            strcpy(input, lastCommand);
        } 
        else {
            strcpy(lastCommand, input);
        }
        //Flag pipe = 0 for now
        int pipeIndex = 0;
        tokenize(input, args, &inFile , &outFile, &background, &isPipe, &pipeIndex);

        if (args[0] == NULL) {
            continue;
        }


        //cd command
        if (strcmp(args[0], "cd") == 0) {
            if (args[1] == NULL) {
                fprintf(stderr, "Missing second argment\n");
            } 
            else {
                if (chdir(args[1]) != 0) {
                    perror("cd Failed");
                }
            }
            continue;
        }

         //Pipe
        if(isPipe){
            pipeCommand(args, &args[pipeIndex]);
        }
                else{
                //Normal Parent-child process
                    pid_t pid = fork();
                    if(pid < 0){
                        perror("Failed to fork");
                    }
                    else if(pid == 0){
                        if(inFile){
                            int fd = open(inFile, O_RDONLY);
                            if(fd == -1){
                                perror("Cannot open file");
                                exit(1);
                            }
                            dup2(fd, STDIN_FILENO);
                            close(fd);
                        }
                        if(outFile){
                            int fd = open(outFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                            dup2(fd, STDOUT_FILENO);
                            close(fd);
                        }
                        execvp(args[0] , args);
                        perror("Command not found!\n");
                        exit(1);
                    }
                    else{
                        resetRaw();
                        if(!background){
                            int status;
                            waitpid(pid, &status, 0);
                        }
                        else{
                            printf("Normal Process: %d\n" , pid);
                            waitpid(pid, NULL, WNOHANG);
                        }
                    }
                    setRaw();

                }
    }
     return 0;
}
    



