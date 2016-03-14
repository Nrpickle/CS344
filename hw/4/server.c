/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

//#define ENCRYPT

char convert(char input);

void error(const char *msg)
{
  perror(msg);
  exit(1);
}

int main(int argc, char *argv[])
{
  int sockfd, newsockfd, portno;
  socklen_t clilen;
  char buffer[256];
  char plaintext[256];
  char key[256];
  struct sockaddr_in serv_addr, cli_addr;
  int n;
  if (argc < 2) {
    fprintf(stderr,"ERROR, no port provided\n");
    exit(1);
  }
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) 
    error("ERROR opening socket");
  bzero((char *) &serv_addr, sizeof(serv_addr));
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  if (bind(sockfd, (struct sockaddr *) &serv_addr,
	sizeof(serv_addr)) < 0) 
    error("ERROR on binding");
  do{
    //KILL ALL ZOMBIES
    static int status;
    waitpid(-1, &status, WNOHANG);

    fprintf(stdout, "[Server: Listening to packets on %d]\n", portno);
    listen(sockfd,5); //Listen to the port

    //At this point, we have a valid queued connection
    //
    //TODO: Insert fork to enable concurrency

    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, 
	(struct sockaddr *) &cli_addr,
	&clilen);
    if (newsockfd < 0)
      error("ERROR on accept");

    
    pid_t spawnpid;

    spawnpid = fork();

    if(spawnpid == 0){  //If we are the parent, we want to continue

/*
      clilen = sizeof(cli_addr);
      newsockfd = accept(sockfd, 
	  (struct sockaddr *) &cli_addr, 
	  &clilen);
      if (newsockfd < 0) 
	error("ERROR on accept");
*/  
      //We have accepted a connection
      bzero(buffer,256);

      //n = read(newsockfd,buffer,255);
      //if (n < 0) error("ERROR reading from socket");
      //printf("Here is the message: %s\n",buffer);

      bzero(plaintext, 256);
      n = read(newsockfd, plaintext, 255);
      //printf("[Server: here is the plaintext: %s]\n", plaintext);

      bzero(key, 256);
      n = read(newsockfd, key, 255);
      //printf("[Server: here is the key: %s]\n", key);

      //Prep plaintext
      int strLen = strlen(plaintext);
      char * plaintextNumbers = malloc(sizeof(char) * strLen);
      int i;

      //printf("Server plaintext: ");

      for(i = 0; i < strLen; ++i){
	plaintextNumbers[i] = convert(plaintext[i]);
      }

      strLen = strlen(key);
      char * keyNumbers = malloc(sizeof(char) * strLen);

      //printf("\nServer key: ");

      for(i = 0; i < strLen; ++i){
	keyNumbers[i] = convert(key[i]);
	//printf("%d ", (int) keyNumbers[i]);
      }

      //Process the encryption
      char * encryptedNumbers = malloc(sizeof(char) * strLen);
      //printf("\nServer encrypt: ");

      for (i = 0; i < strLen; ++i){
#ifdef ENCRYPT
	encryptedNumbers[i] = (plaintextNumbers[i] + keyNumbers[i]) % 27;
#else  // DECRYPT
	encryptedNumbers[i] = (plaintextNumbers[i] - keyNumbers[i]);
	if(encryptedNumbers[i] < 0)  //Mod on this step didn't work, so I did it manually.
	  encryptedNumbers[i] = encryptedNumbers[i] + 27;  //This 100% works though
#endif
	//printf("%d ", (int) encryptedNumbers[i]);
      }

      char * encrypted = malloc(sizeof(char) * strLen);
      //printf("Server output: ");

      for (i = 0; i < strLen; ++i){
	//printf("%c", convert(encryptedNumbers[i]));
	encrypted[i] = convert(encryptedNumbers[i]);
      }


      n = write(newsockfd, encrypted, strlen(encrypted));
      if (n < 0) error("ERROR writing to socket");
      close(newsockfd);

      exit(0); //This should only ever be a spawned process, so it must die
    }
  }while(1);

  close(sockfd);
  return 0; 
}

char convert(char input){
  if(input > 30){ //Then we are being passed a letter
    if(input == ' ')
      return 26;
    else {
      return input - 65;
    }
  }
  else{ //We are being passed a number
    if(input == 26)
      return ' ';
    else {
      return input + 65;
    }
  }

}
