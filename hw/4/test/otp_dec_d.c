/* Server Basecode for Homework #4
 *
 * Nick McComb | www.nickmccomb.net
 * Written March 2016 for Operating Systems I
 *
 * Note: This code is almost exactly the same for decrypt and encrypt, you
 * simply include the #define ENCRYPT line if you want to generate an encrypter
 * or comment it out if you want to generate a decrypter. 
 *
 * ALGORITHM:
 *
 * 1. CONFIGURE LISTEN SOCKET
 * 2. LISTEN FROM SOCKET
 * 3. WHEN RECIEVE REQUEST, FORK OFF PROCESS THEN ACCEPT
 * 3.1  INPUT ALL INCOMING DATA
 * 3.2  PROCESS INPUT DATA
 * 3.3. OUTPUT ALL INPUT DATA BACK OVER THE SAME SOCKET
 * 4. LISTEN AGAIN
 * 
 * USAGE: [exename] portnumber
 *
 * The only host supported is "localhost"
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

//#define ENCRYPT

char convert(char input);

//Outputs an error to the screen
void error(const char *msg)
{
  perror(msg);
  exit(1);
}

//Returns the minimum of two numbers
int min(int a, int b){
  if(a < b)
    return a;
  else
    return b;
}


int main(int argc, char *argv[])
{
  //Configure the listening socket
  int sockfd, newsockfd, portno;
  socklen_t clilen;
  char buffer[1005];
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

    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, 
	(struct sockaddr *) &cli_addr,
	&clilen);
    if (newsockfd < 0)
      error("ERROR on accept");


    pid_t spawnpid;

    spawnpid = fork(); //Fork off a child process to handle the computation

    if(spawnpid == 0){  //If we are the parent, we want to continue

      //We have accepted a connection
      bzero(buffer,256);

      //n = read(newsockfd,buffer,255);
      //if (n < 0) error("ERROR reading from socket");
      //printf("Here is the message: %s\n",buffer);

      int plainToRec;
      char tempBuffer[1002];
      bzero(tempBuffer, 40);
      n = read(newsockfd, tempBuffer, 40);

      //printf("Server recieved for temp send: %d\n", atoi(tempBuffer));

      //Get the number of thousand of characters to be processed (ceil)
      int cypherToSend, keyToRec;
      cypherToSend = keyToRec = plainToRec = atoi(tempBuffer);

      char * plaintext = malloc(sizeof(char) * 1000 * plainToRec);
      strcpy(plaintext, "");

      //Input the plaintext
      do{
	bzero(tempBuffer, 1000);
	n = read(newsockfd, tempBuffer, 1000);
	strcat(plaintext, tempBuffer);
        //printf("pl ");
      }while(--plainToRec > 0);
      //printf("[Server: here is the plaintext: ###### %s ######]\n", plaintext);
      //printf("Length of the plaintext: %d\n", strlen(plaintext));
     
      char * key = malloc(sizeof(char) * 1000 * keyToRec);
      strcpy(key, "");

      //Input the key
      do{
        bzero(tempBuffer, 1001);
        n = read(newsockfd, tempBuffer, 1000);
	strcat(key, tempBuffer);
	//printf("key:%s ", key);
      }while(--keyToRec);
      //printf("[Server: here is the key: %s]\n", key);

      //printf("Length of the key: %d\n", strlen(key));

      //Prep plaintext
      int strLen = strlen(plaintext);
      char * plaintextNumbers = malloc(sizeof(char) * strLen);
      int i;

      //printf("Received plaintext: ");

      //Convert the plaintext to the numbers we will be working with
      for(i = 0; i < strLen; ++i){
	plaintextNumbers[i] = convert(plaintext[i]);
      }

      strLen = strlen(plaintext);
      char * keyNumbers = malloc(sizeof(char) * strLen);

      //printf("\nServer key: ");

      //Convert the key to the numbers we will be working with
      for(i = 0; i < strLen; ++i){
	keyNumbers[i] = convert(key[i]);
	//printf("%d ", (int) keyNumbers[i]);
      }

      //Process the encryption
      char * encryptedNumbers = malloc(sizeof(char) * strLen);
      //printf("\nloop length: %d, Server encrypt: ", strLen);

      //For the following part is the only part that actually differs between encrypt/decrypt,
      //compile-time statements were used to differentiate between the two
      for (i = 0; i < strLen; ++i){
#ifdef ENCRYPT
	encryptedNumbers[i] = (plaintextNumbers[i] + keyNumbers[i]) % 27;
#endif  // DECRYPT
#ifndef ENCRYPT
	encryptedNumbers[i] = (plaintextNumbers[i] - keyNumbers[i]);
	if(encryptedNumbers[i] < 0)  //Mod on this step didn't work, so I did it manually.
	  encryptedNumbers[i] = encryptedNumbers[i] + 27;  //This 100% works though
#endif
	//printf("%c ", (int) convert(encryptedNumbers[i]));
      }

      char * encrypted = malloc(sizeof(char) * strLen);
      //printf("\nServer output: %s");

      for (i = 0; i < strLen; ++i){
	//printf("%c", convert(encryptedNumbers[i]));
	encrypted[i] = convert(encryptedNumbers[i]);
	//printf("%c", encrypted[i]);
      }

      //Write the reply to the user
      do{
        n = write(newsockfd, encrypted, min(strlen(encrypted), 1000));
        if (n < 0) error("ERROR writing to socket");
	static int j = 0;

        if(strlen(encrypted) > 1000)
	  encrypted = encrypted + 1000;
	
        for(j = 0; j < 1000000; ++j)
	  continue;
//	printf("length of enc: %d, cypherToSend: %d\n", strlen(encrypted), cypherToSend);
      }while(--cypherToSend > 0);
      close(newsockfd);

      exit(0); //This should only ever be a spawned process, so it must die
    }
  }while(1);

  close(sockfd);
  return 0; 
}

//We can actually preform the entire ASCII/Number conversion blindly, we only need the inputted number
//and we can give the correct output, so that's what this function does. 
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
