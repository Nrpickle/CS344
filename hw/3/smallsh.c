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
  
  do{


    pid_t cpid;

    short timeout = 0;
    short rep = 0;

    do{
    do{
      cpid = waitpid(-1, &status, WNOHANG);
      if(cpid > 1){
        printf("background pid %d is done: exit value %i\n", cpid, status);
      }
      ++timeout;
      //printf(".");
      //fflush(stdout);
    }while(!cpid && timeout < 1000);
    }while(rep++ < 5);


    printf(": ");
    fflush(stdout);

    fgets(userInput, USER_INPUT_MAX, stdin);
     
    //Strip the newline character
    userInput[strlen(userInput)-1] = '\0';

    //printf("I recieved the following: %s\n", userInput);
 
    
    //printf("characterCheck: %c \n", userInput[1]);
    
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
      //printf("Length of strtok: %d\n", strlen(tokResult));

      userInputVect[userInputCount] = malloc( (strlen(tokResult) + 1) * sizeof(char) );
      strcpy( userInputVect[userInputCount++] , tokResult );

      //printf("Recieved: %s \n", userInputVect[userInputCount - 1]);

    }while(tokResult = strtok(NULL, " "));

    //userInputVect[userInputCount] = malloc( sizeof(char) * 2);
    userInputVect[userInputCount] = NULL;

    //Check for built in command
    if(!strcmp(userInputVect[0], "exit"))
      exit(0);
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
	}
      }
      //printf("Target path: _%s_\n", userInputVect[1]);
      continue;
    }
    if(!strcmp(userInputVect[0], "status")){
      //printf("[#Executing status#]\n"); 
      printf("exit value %d \n", status);
      continue;
    }

    //Check for comment line
    if(userInputVect[0][0] == '#'){
       //Then we want to do nothing
       continue;
    }

    //Check to see if we are piping the process anything
    for(int i = 0; i < userInputCount; i++){
      if(!strcmp(userInputVect[i], "|")){
        printf("  #Detected pipe\n");
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

    cpid = fork();

    if (cpid == 0){  //Then we are the child
       int fd;

      if(setBackground){  //Then we have a background process on our hands
        int null = open("/dev/null", O_RDWR);

        int readNull = open("/dev/null", O_RDONLY);

        dup2(null, 1);
	dup2(readNull, 0);
	dup2(null, 2);

	//TODO: disable ctrl sequences

      }
      else if(inputFilename){ 
        fd = open( inputFilename, O_RDONLY, 0644);
        
        if(fd < 0){
          printf("Cannot open %s for input \n", inputFilename);
	  continue;
	}

	dup2(fd, 0);

      }
      //Check if we need to change the output to a file
      else if(outputFilename){  //Check if the filename pointer is not null (only if not a background) TODO: fix, bug?
        fd = open( outputFilename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

	if(fd < 0){
          printf("Cannot open %s for output \n", outputFilename);
	  continue;
	}

        dup2(fd , 1);
	
      }

      execvp(userInputVect[0], userInputVect);

      //printf("Failed stuffs");
      fprintf(stderr, "smallsh: cannot open %s for input\n", userInputVect[0]);
      
      //fputs("smallsh: cannot open for input. \n", stderr);
      exit(1);
    }
    else{  //Then we are the parent.
      if(setBackground)
        printf("background pid is %d \n", cpid);
      else{
        cpid = waitpid(-1, &status, 0);
      }
    }

  }while(1);
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

  signal(SIGTERM, signalSIGTERMHandler);
}

