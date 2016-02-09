/* Nick McComb
 * CS344 - February 2016
 * 
 * Written for Operating Systems I
 *
 * This program follows a specific set of UI for an "Adventure Cave" type game
 * that demonstrates an ability to use file I/O. Most of the code in this file
 * has to do with generating rooms for the game, and reading them from a file/
 * writing them to files.
 *
 * The program intentionally leaves the room files that were generated so that 
 * the grading TA can easily observe their contents
 *
 * ALGORITHM:
 *
 * 1 GENERATE ROOMS
 * 2 WRITE ROOMS TO FILES
 * 3 READ ROOMS FROM FILES
 * 4 PLAY GAME
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_ROOM_NAME_LENGTH 20
#define MAX_ROOM_NO     10
#define DESIRED_ROOM_NO 7
#define TRUE            1
#define FALSE           0
#define MIN_CON		3
#define MAX_CON		6

const char  roomNameBank[MAX_ROOM_NO][MAX_ROOM_NAME_LENGTH + 1] = 
                 { "space\0",   "adventure\0", "fact\0",      "anger\0", 
		   "cake\0",    "wheatley\0",  "curiosity\0", "morality\0", 
		   "central\0", "paranoia\0" };
      short roomNameFlags[MAX_ROOM_NO] = { 0 };  //Variable to hold the flags that signify 
						 //if a room name was used (so sorry for the global)
      short roomCounter = 0; //Counts the number of rooms generated (so sorry for the global)
      char  folderName[40];  //contains the foldername

//Custom Data Types

//Enum holds the room types
enum ROOM_TYPE_ENUM { START_ROOM, END_ROOM, MID_ROOM };

//Holds all the information for one room
typedef struct
{
  char  roomName[MAX_ROOM_NAME_LENGTH + 1]; 
  short roomType;
  char  connections[MAX_CON][MAX_ROOM_NAME_LENGTH];
  short connectionCount;
} room_typ;


//  Function Prototypes
//All prototypes were written to be easy to read and intrepret (for the grading TA's pleasure)

room_typ* createRoom();
void writeRoomsToFile( room_typ** rooms, char* folderName);
room_typ* readRoomFromFile ( short roomID, char* folderName );
void connectRooms( room_typ** rooms );
short checkRoomConnectionCount( room_typ** rooms );
short checkAndMakeConnections( room_typ** rooms, short targetA, short targetB );
short roomSearch( room_typ** rooms, char* roomName );

main()
{
  // ** Program Initialization **
  srand (time(NULL));
  int i, j;  //Variables used for loop iteration

  //Setup filename
  int processId = getpid();
  sprintf(folderName, "mccombn.rooms.%d\0", processId); 

  // ** Main Program Execution **

  //Holds all of the rooms
  room_typ *rooms[DESIRED_ROOM_NO];

  //Loops through and creates all of the rooms (allocating mem too)
  for(i = 0; i < DESIRED_ROOM_NO; i = i + 1){
    rooms[i] = createRoom();
  }
  
  //Connects all of the rooms
  connectRooms(rooms);

  //Creates the folder with the appropriate permissions
  if(!mkdir(folderName, 0770))
    i; //exit(2);
  
  //At this point, we can assume that the correct folder is created,
  //and that it's name is stored in folderName
  
  writeRoomsToFile( rooms, folderName );

  //At this point, all of the information is written to the file, so we 
  //need to erase all of our current information :( and read it back in
  //from the files.
  
  for(i = 0; i < DESIRED_ROOM_NO; i = i + 1){
    free(rooms[i]);
  }

  //For clarity to the grading TA, the name of the retrieved rooms was changed
  //(no copying internal to the program was done, as should be obvious)
  room_typ *gameRooms[DESIRED_ROOM_NO];

  for(i = 0; i < DESIRED_ROOM_NO; i = i + 1){
    gameRooms[i] = readRoomFromFile( i, folderName );
  }

  //TODO: Add code to delete the folder and recreate if it fails to make a folder
  //so that we can make the stuff that is okie dokie
  
  // ** MAIN GAME CODE **
  
  //Game-specific variables
  char  currentRoom = START_ROOM;
  char  userInput[150];  //Holds user input
  char  match;           //Used later to determine if a match was found
  char  userPath[500];   //Massive variable incase the user is having a bad day
        strcpy(userPath, ""); //Init it with an empty string, because of how the
                              //code is written later
  short userPathCount = 0;
  
  //Start the actual game loop

  do{

  printf("CURRENT LOCATION: %s\n", gameRooms[currentRoom]->roomName);

  printf("POSSIBLE CONNECTIONS: ");
  
  match = 0; //Default that we haven't found a match
  
  for(i = 0; i < gameRooms[currentRoom]->connectionCount; i = i + 1){
    printf("%s", gameRooms[currentRoom]->connections[i]);
    if( i == gameRooms[currentRoom]->connectionCount - 1 )
      printf(".\n");
    else
      printf(", ");
  }

  printf("WHERE TO? >");
  
  scanf("%s", userInput);

  //Search for connections:
  for(i = 0; i < gameRooms[currentRoom]->connectionCount; i = i + 1){
    if(!strcmp(userInput, gameRooms[currentRoom]->connections[i])){ //If the user entered a valid match
      //We need to go there, bitch
      match = 1;

      //First record our path
      sprintf(userPath, "%s%s\n", userPath, userInput);

      //Increment path counter
      userPathCount = userPathCount + 1;

      //Move the "current room" pointer
      currentRoom = roomSearch( gameRooms, userInput );

      //Print a newline for spacing
      printf("\n");
    }
    //Else, do nothing!
  }

  if(!match)
    printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");

  }while(currentRoom != END_ROOM);

  //Yay! The user won.
  //Now we need to output the ending messages.

  printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
  printf("YOU TOOK %i STEPS. YOUR PATH TO VICTORY WAS: \n%s", userPathCount, userPath);
  
  // ** Program Cleanup **
  for(i = 0; i < DESIRED_ROOM_NO; i = i + 1){
    free(gameRooms[i]);
  }
  
  exit(0);
}

void connectRooms( room_typ** rooms ){
  do {

    int targetA = rand() % DESIRED_ROOM_NO;
    int targetB = rand() % DESIRED_ROOM_NO;
    
    //DEBUGSTR
    //printf("TargetA: %i, TargetB: %i \n", targetA, targetB);
    
    if ( targetA == targetB ) //If they are the same number, then we want new numbers
      continue;  

    //At this point we can assume we have valid, unique numbers to attempt to connect rooms
    //printf("Connection result: %i \n", checkAndMakeConnections( rooms, targetA, targetB ));

    checkAndMakeConnections( rooms, targetA, targetB );

    //sleep(1);
    

  }while(checkRoomConnectionCount(rooms));
}

//checks if connections can be made between the two passed in indexes,
//and makes the connection if it can be made. 
//
//Returns: non-zero if a case cannot be made. 0 if a connection was made
short checkAndMakeConnections( room_typ** rooms, short targetA, short targetB ){
  //Check if either have the max connections
  if( rooms[targetA]->connectionCount >= MAX_CON )
    return 2; //No connection possible
  if( rooms[targetB]->connectionCount >= MAX_CON )
    return 3; //No connection possible

  //Check for a pre-existing connection between the two
  int i;
  for( i = 0; i < rooms[targetA]->connectionCount; i = i + 1){ //Loop through all the connections in targetA
    if( ! strcmp( rooms[targetA]->connections[i], rooms[targetB]->roomName ) ) //If there is a connection
      return 1; //Then we cannot make another connection
  }
  
  int targetAIndex = rooms[targetA]->connectionCount;
  int targetBIndex = rooms[targetB]->connectionCount;

  //Make the connection
  strncpy(rooms[targetA]->connections[targetAIndex], rooms[targetB]->roomName, MAX_ROOM_NAME_LENGTH);
  strncpy(rooms[targetB]->connections[targetBIndex], rooms[targetA]->roomName, MAX_ROOM_NAME_LENGTH);

  rooms[targetA]->connectionCount = rooms[targetA]->connectionCount + 1;
  rooms[targetB]->connectionCount = rooms[targetB]->connectionCount + 1;
  //Connection done.

  return 0;
}


//Returns 1 if not all rooms have enough connections
short checkRoomConnectionCount( room_typ** rooms ){
  int i;
  short check = 0;  //Assume all are valid
  for ( i = 0; i < DESIRED_ROOM_NO; i = i + 1 )
    if( rooms[i]->connectionCount < 3 )
      return 1;

  return 0;
}

//Writes all rooms to the particular file, using the foldername that was determiend
//in main. The rooms are in files called, for example, "room1", with an increasing
//index based on their index in the game.
void writeRoomsToFile( room_typ** rooms, char* folderName ){
  FILE *file;
  
  char targetFilePath[50]; //Holds the current file name
  char stringToWrite[500]; //Holds what we are going to write to the file
  char tempString[100];    //Temporary string
  int i; //Loop iteration variable
  int j;

  //We want to write all DESIRED_ROOM_NO rooms to files, named room0 though room{DESIRED_ROOM_NO}

  for(i = 0; i < DESIRED_ROOM_NO; i = i + 1){
    sprintf(targetFilePath, "%s/room%i", folderName, i);

    file = fopen( targetFilePath, "w" ); //Open our file

    if (!file){ //Then the file failed to open
      printf("File failed to open!\n");
      exit(1);
    }

    strcpy(stringToWrite, "");  //Zero out string to send
    sprintf(stringToWrite, "ROOM NAME: %s\n", rooms[i]->roomName);

    for(j = 0; j < rooms[i]->connectionCount; j = j + 1){
      sprintf(stringToWrite, "%sCONNECTION %i: %s\n", stringToWrite, j + 1, rooms[i]->connections[j]);
    }

    //TODO: Add room type output
    if (rooms[i]->roomType == START_ROOM)
      strcpy(tempString, "START_ROOM");
    else if (rooms[i]->roomType == MID_ROOM)
      strcpy(tempString, "MID_ROOM");
    else if (rooms[i]->roomType == END_ROOM)
      strcpy(tempString, "END_ROOM");
    
    //Write to file  
    sprintf(stringToWrite, "%sROOM TYPE: %s\n", stringToWrite, tempString); 
   
    fputs( stringToWrite, file);

    fclose(file);  //Close the open file
  }
  
}

//roomID is our target room (index 0) and is associated with the according file room0 = id 0
room_typ* readRoomFromFile ( short roomID, char* folderName ){
  FILE *file;

  char targetFilePath[50];  //Holds the current file name
  char tempString[50];      //Temporary string for holding input
  char *tokResult;

  room_typ *temp = (room_typ*) malloc(sizeof(room_typ));
  temp->connectionCount = 0;

  sprintf(targetFilePath, "%s/room%i", folderName,  roomID);

  file = fopen(targetFilePath, "r");

  if( !file ){
    printf("[#! Couldn't open the file! !#]");
    exit(1);
  }

  //Get the room name
  fgets( tempString, 50, file );
  tokResult = strtok(tempString, " ");
  tokResult = strtok(NULL, " ");
  tokResult = strtok(NULL, " ");
  tokResult[strcspn(tokResult, "\n")] = '\0';
  strcpy(temp->roomName, tokResult);

  //Get the connections
  fgets( tempString, 50, file );
  while( tempString[0] == 'C' ){
    //printf("Connection: %s", tempString);
    tokResult = strtok(tempString, " ");
    tokResult = strtok(NULL, " ");
    tokResult = strtok(NULL, " ");
    tokResult[strcspn(tokResult, "\n")] = '\0';
   
    strcpy(temp->connections[temp->connectionCount], tokResult);
    temp->connectionCount = temp->connectionCount + 1;

    //Read the next line
    fgets( tempString, 50, file );
  }

  //This code is based on the fact that the start room and the end room
  //are always the first and second rooms, respectively
  if( roomID == START_ROOM )
    temp->roomType = START_ROOM;
  else if ( roomID == END_ROOM )
    temp->roomType = END_ROOM;
  else
    temp->roomType = MID_ROOM;

  //Close the file
  fclose(file);

  //TODO remove free
  return temp;
}


//Creates a blank room with all required info except for connections
room_typ* createRoom(){
  room_typ *temp = (room_typ*) malloc(sizeof(room_typ));

  //Check if the OS actually returned an address
  if( temp ){
    //Init connections
    temp->connectionCount = 0;  //Init connection count to zero
    int j;
    for(j = 0; j < MAX_CON; j = j + 1){
      strcpy(temp->connections[j], "|");
    }

    //Set a room type for this room
    if( roomCounter == 0 )
      temp->roomType = START_ROOM;
    else if( roomCounter == 1)
      temp->roomType = END_ROOM;
    else
      temp->roomType = MID_ROOM;
    
    //Choose a room name
    short validRoomName = 0;
    short nameIndex;
    do {
      nameIndex = rand() % MAX_ROOM_NO;
      if(!roomNameFlags[nameIndex]){ //Then we've found an unused name
	validRoomName = 1;           //We want to end the loop
	strcpy(temp->roomName, roomNameBank[nameIndex]);  //Copy the current room's name into it's storage
	roomNameFlags[nameIndex] = 1;                     //Mark the name as "taken"
      }
      //Otherwise, do nothing, we want to try again with a new random number
    } while (!validRoomName);
    roomCounter = roomCounter + 1;
  }
  else{
    //In this case, the OS wasn't able to return any memory, 
    //so something seriously wrong has happened. Exit with
    //error state.
    exit(1);
  }

  return temp;
}

//Note: coming into this function we assume that there is a valid room number.
//The function returns a -1 if there is no room found.
short roomSearch( room_typ** rooms, char* roomName ){
  short roomID = -1;
  int i;

  for(i = 0; i < DESIRED_ROOM_NO; i = i + 1){
    if(!strcmp(roomName, rooms[i]->roomName)) //Check if the current room is the one
      roomID = i; //This is the index we want
  }

  return roomID;
}
