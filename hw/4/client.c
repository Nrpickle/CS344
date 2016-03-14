#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg)
{
  perror(msg);
  exit(0);
}

int main(int argc, char *argv[])
{
  int sockfd, portno, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;

  //Input checking
  char buffer[256];
  if (argc < 2) {
    fprintf(stderr,"usage %s hostname port\n", argv[0]);
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

  char inputBuffer[2000];
  char plaintext[2000];
  char key[2000];

  fgets(inputBuffer, 1000, plaintextFile);

  //I want to strip the newline off if there is one
  int plaintextLen = strlen(inputBuffer);
  if(inputBuffer[plaintextLen-1] == '\n')
    inputBuffer[plaintextLen-1] = '\0';

  //printf("\n\nClient input\nPlaintext file: %s\n", inputBuffer);

  strcpy(plaintext, inputBuffer);

  fgets(inputBuffer, 1000, keyFile);
  //printf("Keyfile       : %s\n\n", inputBuffer);

  strcpy(key,       inputBuffer);

  //strcpy(plaintext, argv[1]);
  //strcpy(key,       argv[2]);

  //Write to the server
  //TODO: Implement sending in chunks of 1000
  bzero(buffer, 256);
  n = write(sockfd, plaintext, strlen(plaintext));
  if (n < 0)
    error("ERROR writing to socket");

  n = write(sockfd, key, strlen(key));
  if (n < 0)
    error("[! ERROR writing to socket !]");

  n = read(sockfd,buffer,255);
  if (n < 0) 
    error("ERROR reading from socket");
  printf("%s\n",buffer);
  close(sockfd);
  return 0;
}
