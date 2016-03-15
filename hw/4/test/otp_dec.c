/* Client base code
 *
 * Note: This exact code is used for both of the local clients, as it's code is exactly the same. This would
 * still be true if the identification of each program was implemented, even though it was not.
 *
 * Nick McComb | www.nickmccomb.net
 * Written March 2016 for Operating Systems I
 *
 * ALGORITHM:
 *
 * 1. VERIFY INPUT
 * 2. READ IN INPUT FROM FILES SPECIFIED BY USER
 * 3. SEND INPUT TO PROCESS SCRIPT
 * 4. OUTPUT RECIEVED INFO FROM PROCESS Script
 *
 * Usage: [exename] inputfile keyfile port
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <string.h>
void error(const char *msg)
{
  perror(msg);
  exit(0);
}

int min(int a, int b){
  if(a < b)
    return a;
  else
    return b;
}

int main(int argc, char *argv[])
{
  int sockfd, portno, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;

  //Input checking
  char buffer[1005];
  if (argc != 4) {
    fprintf(stderr,"usage %s inputfile keyfile port\n", argv[0]);
    exit(0);
  }


  //Open the socket connection to the server
  portno = atoi(argv[3]);
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) 
    error("ERROR opening socket");

  server = gethostbyname("localhost");
  if (server == NULL) {
    fprintf(stderr,"ERROR, no such host\n");
    exit(0);
  }
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, 
      (char *)&serv_addr.sin_addr.s_addr,
      server->h_length);
  serv_addr.sin_port = htons(portno);

  //error("fail before connect");

  if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
    error("ERROR connecting");
  //At this point, we can assume the connection was successful

  //Load the plaintext and key from files

  FILE *plaintextFile;
  FILE *keyFile;

  plaintextFile = fopen(argv[1], "r");
  keyFile = fopen(argv[2], "r");

  if(!(plaintextFile && keyFile))
    error("[! Cannot open file !]");

  //Get the message to send
  //fgets(buffer,255,stdin);

  //At this point we can assume we have valid files and a valid socket

  char * inputBuffer = malloc(sizeof(char) * 100000);
  //char plaintext[2000];
  //char key[2000];

  fgets(inputBuffer, 100000, plaintextFile);

  char * plaintext  = malloc(sizeof(char) * (strlen(inputBuffer) + 1));
  char * plaintextHead = plaintext;

  //I want to strip the newline off if there is one
  int plaintextLen = strlen(inputBuffer);
  if(inputBuffer[plaintextLen-1] == '\n')
    inputBuffer[plaintextLen-1] = '\0';

  //printf("\n\nClient input\nPlaintext file: %s\n", inputBuffer);

  strcpy(plaintext, inputBuffer);

  //printf("%s %s\n", plaintext, inputBuffer);

  fgets(inputBuffer, 100000, keyFile);

  char * key = malloc(sizeof(char) * (strlen(inputBuffer) + 1));
  char * keyHead = key;

  strcpy(key,       inputBuffer);

  free(inputBuffer);

  if(strlen(plaintext) > strlen(key))
    error("Key is too short.");

  //strcpy(plaintext, argv[1]);
  //strcpy(key,       argv[2]);

  //Write to the server

  int plainToSend = 1;
  int keyToSend = 1;
  int cypherToRec = 1;

  //printf("%s\n", plaintext);

  if (strlen(plaintext) > 1000) {
    plainToSend += strlen(plaintext) / 1000;
  }

  //We can assume the key is longer than the plaintext, so we can make
  //the following assumption:
  cypherToRec = keyToSend = plainToSend;
  //if (strlen(key) > 1000) {
  //  keyToSend += strlen(key) / 1000;
  //}

  char temp[40];

  sprintf(temp, "%d", plainToSend);
  //itoa(plainToSend, temp, 10);

  //Write the number of thousands of plaintext (ceil) that is going to be sent,
  //so that the daemon knows what to expect
  n = write(sockfd, temp, strlen(temp));
  if(n < 0)
    error("ERROR writing to socket");
  int i; 
  for(i = 0; i < 10000000; ++i)
    continue;

  //printf("CLIENT: Length of text to send: %d\n", strlen(plaintext));

  //Send the plaintext in chunks of 1000
  bzero(buffer, 1001);
  do{
    //printf("plain to send: %d, string to send\n", plainToSend);
    n = write(sockfd, plaintext, min(strlen(plaintext), 1000));
    if (n < 0)
      error("ERROR writing to socket");

    if(strlen(plaintext) > 1000)
      plaintext = plaintext + 1000;
    --plainToSend;

    for(i = 0; i < 10000700; ++i)
      continue;

  }while(plainToSend > 0);

  for(i = 0; i < 10000000; ++i)
    continue;

  //Send the key in chunks of 1000
  bzero(buffer, 1001);
  do{
    //printf("Wrote key %d with value %d\n", keyToSend, n);
    n = write(sockfd, key, min(strlen(key), 1000));
    if (n < 0)
      error("[! ERROR writing to socket !]");

    if(strlen(key) > 1000)
      key = key + 1000;
    --keyToSend;
    for(i = 0; i < 10000000; ++i)
      continue;
  }while(keyToSend > 0);

  //printf("Waiting for response... \n");

  //Recieve the reply in chunks of 1000
  do{
    n = read(sockfd,buffer,1000);
    if (n < 0) 
      error("ERROR reading from socket");
    printf("%s", buffer);
  }while(--cypherToRec > 0);
  printf("\n");
  close(sockfd);

  free(plaintextHead);
  free(keyHead);

  return 0;
}


