/*
 *
 * This program will output a number of random chars between (char) 65-90, and also including 32 
 *
 *
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]){
  if(argc != 2){
    printf("usage: %s keylength \n", argv[0]);
    return 1;
  }

  int keyno = atoi(argv[1]);
  char outputChar;
  srand(time(NULL));

  for(int i = 0; i < keyno; ++i){
    outputChar = (char) (rand()%27) + 65;
    if((char) outputChar == 91){
       ///printf("excep");
       outputChar = 32;
    }
    printf("%c", outputChar);

  }

  printf("\n"); //Puts a newline on the end of the file, because _apparently_ that's necessary (lol)

  return 0;
}
