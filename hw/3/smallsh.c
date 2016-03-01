/***
 * Smallsh
 *
 * Written by Nick McComb | www.nickmccomb.net
 * For CS344 (OS I) in February 2016
 *
 * This program is a very simple shell program that was used to demonstrate knowledge
 * of the fork() and exec() family in c. There are three built in commandds (cd, status, 
 * and exit), and everything else is passed onto the exec command. The shell also supports
 * file input redirection, file output redirection, and background processes, among other
 * things. 
 *
 * USAGE: ./smallsh
 *
 * The  program acceps no command line arguments, and functions as a normal shell when launched
 *
 * To type a command in the shell, use the following format:
 *
 * command [arg1 arg2 ...] [< input_file] [> output_file] [&]
 *
 * COMPILATION
 *
 * To compile the program, use the following line (also found in the README file):
 *
 * gcc -std=c99 -o smallsh smallsh.c
 *
 * PROGRAM ALGORITHM
 *
 * 1. CHECK IF ANY PROCESSES NEED TO BE CLEANED UP
 * 2. GET USER INPUT
 * 3. CHECK USER INPUT FOR SPECIAL CONDITIONS (background, I/O redirection, etc.)
 * 4. PROCESS USER INPUT
 *
 ***/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

#define USER_INPUT_MAX 2048
#define USER_NUM_ENTRIES 513

//Define our signal handler functions
void signalSIGINTHandler(int signum);
void signalSIGTERMHandler(int signum);

int main(){
  //**PROGRAM INIT**
  char userInput[USER_INPUT_MAX];

  static int status = -3;
  int backgroundProcessCount = 0;
  int backgroundPIDs[500];

  //Register signal handlers
  signal(SIGINT, signalSIGINTHandler);
  signal(SIGTERM, signalSIGTERMHandler);

  printf("[Welcome to Small Shell!] \n");
  

  //**MAIN PROGRAM LOOP**
  do{
    int cpid = 0; //Holds misc. process IDs when needed

    short timeout = 0;
    short rep = 0;

    //This weird set of do..while() loops just checks for processes an arbritrary amount of times.
    //It could definitely be optimized, but it works, so I left it
    do{
    do{
      cpid = waitpid(-1, &status, WNOHANG);  //Check for a process
      if(cpid > 0){ //Then we found a process
	if(WIFEXITED(status)){    //If the process exited, then we want to say so
	                          //Note: only a background process will trip this, because of how the 
			          //program flow was designed
          printf("process %d exited",  cpid);
	}
	if(WIFSIGNALED(status)){  //Print the terminated signal output (also only bg process)
          printf("process %d terminated by signal %d", cpid, WTERMSIG(status));
	}
	printf("\n");

      }
      ++timeout;
      fflush(stdout);
    }while(!cpid && timeout < 1000);
    }while(rep++ < 5);


    printf(": ");
    fflush(stdout);

    fgets(userInput, USER_INPUT_MAX, stdin);
     
    //Strip the newline character
    userInput[strlen(userInput)-1] = '\0';

    //Check for a blank line first

    //Check for blank line
    short inputLen = strlen(userInput);
    char  isBlank = 1; //Assume the line is blank
    for (int i = 0; i < inputLen; ++i){
      if(userInput[i] != ' ' && userInput[i] != '\0'){
	isBlank = 0;
	break;
      }
    }
    if(isBlank){
      continue;
    }


    //We want to get the first space deliminated substring
    //To do this we will first split it into a vector similar
    //to argv using strtok

    char  * tokResult = strtok(userInput, " "); 
    char  ** userInputVect = malloc(USER_NUM_ENTRIES * sizeof(char*));
    short userInputCount = 0;

    do{
      if(userInputVect[userInputCount]){
	free(userInputVect[userInputCount]);
	userInputVect[userInputCount] = NULL;
      }
      
      userInputVect[userInputCount] = malloc( (strlen(tokResult) + 1) * sizeof(char) );
      strcpy( userInputVect[userInputCount++] , tokResult );

    }while(tokResult = strtok(NULL, " "));

    userInputVect[userInputCount] = NULL;

    //Check for built in command
    //Note: there are a lot of program flow breaking 'continues' used in this program
    //and especially in this part. I deemed it the most elegant way to solve the problem

    if(!strcmp(userInputVect[0], "exit")){
      //We need to do some memory cleanup before we exit
      int tempCounter = 0;
      do{
        free(userInputVect[tempCounter]);
      }while(userInputVect[++tempCounter] != NULL);
      free(userInputVect);
      exit(0);
    }
    if(!strcmp(userInputVect[0], "cd")){
      //For the next line, it is important to check for the first check first,
      //otherwise you could SEGMENTATION FAULT
      if(userInputVect[1] == NULL || strcmp(userInputVect[1], "~/") == 0){
      //Then we want to change to the home directory
        chdir(getenv("HOME"));
      }
      else {  //We want to use the user's message
        if(chdir(userInputVect[1]) == -1){
          printf("Cannot open directory %s \n", userInputVect[1]);
	  fflush(stdout);
	}
      }
      //printf("Target path: _%s_\n", userInputVect[1]);
      continue;
    }
    if(!strcmp(userInputVect[0], "status")){
      //printf("[#Executing status#]\n"); 
      if(status != -3){
        printf("exit value %d \n", WEXITSTATUS(status));
        fflush(stdout);
      }
      continue;
    }

    //Check for comment line
    if(userInputVect[0][0] == '#'){
       //Then we want to do nothing
       continue;
    }

    //Check to see if we are piping the process anything
    //(this was not implemented in the final program)
    for(int i = 0; i < userInputCount; i++){
      if(!strcmp(userInputVect[i], "|")){
       // printf("  #Detected pipe\n");
      }
      //printf("Counted input: %d \n", userInputCount);
    }

    char setBackground = 0;
    //Check for background process
    for (int k = 0; k < userInputCount; k++){
      if(!strcmp(userInputVect[k], "&")){
        setBackground = 1;
	free(userInputVect[k]);
	userInputVect[k] = NULL;  //End the vector earlier (we don't want to pass the & to the exec
	userInputCount = k - 1;   //We do the same to the count, so the program doesn't iterate to NULL
	//printf(" ##background this process \n");
      }
    }

    //Check for file redirectionm
    char * outputFilename = NULL;
    for(int j = 0; j < userInputCount; j++){
      if(!strcmp(userInputVect[j], ">")){
        //printf("  #Writing to file! Seperator located at %d \n", j);
	//put the filename in the output name, also makes the name pointer != zero
        outputFilename = malloc(sizeof(char) * strlen(userInputVect[j + 1]));
	strcpy(outputFilename, userInputVect[j + 1]);
	//Cut the input vector short so that we only output the command to be output to file
	free(userInputVect[j]); //because otherwise we'd lose this memory
        userInputVect[j] = NULL;
	userInputCount = j - 1;
	break; //Break out of the for loop
      }
    }

    char * inputFilename = NULL;
    for(int lol = 0; lol < userInputCount; lol++){
      if(!strcmp(userInputVect[lol], "<")){//Then we need to do some stuff and things (read from a file)
	inputFilename = malloc(sizeof(char) * strlen(userInputVect[lol + 1]));
	strcpy(inputFilename, userInputVect[lol + 1]);

	free(userInputVect[lol]);
	userInputVect[lol] = NULL;
	userInputCount = lol - 1;
        break;
      }
    }


    //Here is where we actually fork off the child process (if we've reached this point)

    cpid = fork();

    if (cpid == 0){  //Then we are the child
       int fd;

      if(setBackground){  //Then we have a background process on our hands
        int null = open("/dev/null", O_RDWR);

        int readNull = open("/dev/null", O_RDONLY);

        dup2(null, 1);
	dup2(readNull, 0);
	//dup2(null, 2);

      }
      else if(inputFilename){ 
        fd = open( inputFilename, O_RDONLY, 0644);
        
        if(fd < 0){
          printf("Cannot open %s for input \n", inputFilename);
	  //free(inputFilename);
	  continue;
	}
        //free(inputFilename);
	dup2(fd, 0);

      }
      //Check if we need to change the output to a file
      else if(outputFilename){  //Check if the filename pointer is not null (only if not a background) TODO: fix, bug?
        fd = open( outputFilename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

	if(fd < 0){
          printf("Cannot open %s for output \n", outputFilename);
	  //free(outputFilename);
	  continue;
	}

	//free(outputFilename);

        dup2(fd , 1);
	
      }

      //This is the line that executes the desired command ('the core')
      execvp(userInputVect[0], userInputVect);

      //If we've reached this spot, that means the exec command failed.

      fprintf(stderr, "smallsh: cannot open %s for input\n", userInputVect[0]);
      fflush(stdout);

      exit(1); //We definitely want to kill the child here
    }
    else{  //Then we are the parent.
      if(setBackground)
        printf("background pid is %d \n", cpid);
      else{
        cpid = waitpid(cpid, &status, 0);  //we want to wait for the process we just made to exit (because it's fg)
      }
    }

  }while(1); //We want to loop forever, when an exit is desired, then the exit() command will cause it
}

void signalSIGINTHandler(int signum){
  printf("terminated by signal %d\n", signum);

  //char error [] = "Caught SIGINT\n";
  //puts(error);

  //Setup signal handler again
  signal(SIGINT, signalSIGINTHandler);
}

void signalSIGTERMHandler(int signum){
  printf("terminated by signal %d\n", signum);
  perror("hi there\n");
  exit(1);
  signal(SIGTERM, signalSIGTERMHandler);
}

