//*******************************************************
// R O O M  A D V E N T U R E
// adventure.c
// CS 344
// ku3nsting
// May 10, 2017
//*******************************************************

//includes:
#include <stdio.h> //gives us basic file i/o and stuff
#include <stdlib.h>  //gives us all standard library functions

#include <time.h> //gives us foundation for random numbers and time output
#include <string.h> //necessary for strcpy to work
#include <dirent.h> //lets us use directory as a type
#include <sys/types.h>
#include <unistd.h> //lets us access the process ID
#include <sys/stat.h> //needed for mkdir according to http://www.gnu.org/software/libc/manual/html_node/Creating-Directories.html
#include <pthread.h> //for mutex

//exit codes
#define SOMETHING_WENT_WRONG    1
#define ITS_ALL_GRAVY           0
//8 rooms this time to sync with filenames. Room 0 does not exist in game.
#define NUMBER_OF_ROOMS         8

pthread_mutex_t annoyingMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t game;
pthread_t mtime;
int notGame;
int notTime;

//Variables. Figure out how to use this info from files
char newestDirName[64];

//use an enum to describe the type of each room
enum type {
        START_ROOM,
        MID_ROOM,
        END_ROOM
        };

// Struct to define the room object itself
struct room {
        enum type typeOfRoom;
        char roomName[16];
        int number;
        int numberOfDoors;
        char doorNames[6][16];
};

//array of all rooms in the game
struct room arrayOfRooms[NUMBER_OF_ROOMS];

//array of all room names in order
char nameArray[NUMBER_OF_ROOMS][16];

//active room for game loop
struct room currentRoom;

//index of start room (found during struct creation)
int startingRoom;

//index of current active room
int current;

void initStrArray(char array[6][16]){
    int y;
    for(y = 0; y < 6; y++){
        strcpy(array[y], NULL);
    }
}

//code borrowed from 2.4 Manipulating Directories in tophat
void manipDir(){
  int newestDirTime = -1; // Modified timestamp of newest subdir examined
  char targetDirPrefix[20] = "kuenstir.rooms."; // Prefix we're looking for
  memset(newestDirName, '\0', sizeof(newestDirName));

  DIR* dirToCheck; // Holds the directory we're starting in
  struct dirent *fileInDir; // Holds the current subdir of the starting dir
  struct stat dirAttributes; // Holds information we've gained about subdir

  dirToCheck = opendir("."); // Open up the directory this program was run in

  if (dirToCheck > 0) // Make sure the current directory could be opened
  {
    while ((fileInDir = readdir(dirToCheck)) != NULL) // Check each entry in dir
    {
      if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) // If entry has prefix
      {
       // printf("Found the prefix: %s\n", fileInDir->d_name);
        stat(fileInDir->d_name, &dirAttributes); // Get attributes of the entry

        if ((int)dirAttributes.st_mtime > newestDirTime) // If this time is bigger
        {
          newestDirTime = (int)dirAttributes.st_mtime;
          memset(newestDirName, '\0', sizeof(newestDirName));
          strcpy(newestDirName, fileInDir->d_name);
          //printf("Newer subdir: %s, new time: %d\n",
           //      fileInDir->d_name, newestDirTime);
        }
      }
    }
  }

  closedir(dirToCheck); // Close the directory we opened

  //printf("Newest entry found is: %s\n", newestDirName);
}

int countLines(FILE* filePointer){
    char placeholder;
    int lines;
    while(!feof(filePointer)){
        placeholder = fgetc(filePointer);
        if(placeholder == '\n'){
            lines++;
        }
    }
    return lines;
}

void rebuildAllRooms(){
    //find the most recently made file in the directory
	chdir(newestDirName);

	//printf("\nNEWEST: %s \n", newestDirName);
	//printf("\nENTERED DIRECTORY!");

    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    int i;
    //open all files!
    for(i = 1; i<8; i++){

    char roomName[10];
    //define file name
    sprintf(roomName, "room_%d", i);
    //printf("%s \n", roomName);

	FILE *filePointer;
	//open the file!
	filePointer = fopen(roomName, "r");

	//make sure file is real
    if (filePointer == NULL){
        printf("GET OUTTA HEAH\n");
        exit(SOMETHING_WENT_WRONG);
    }
    //else{
        //printf("FILE OPENED!\n");
    //}

    //initialize empty room
    strcpy(arrayOfRooms[0].roomName, "*");

    //count the lines in the file
    int numLines = countLines(filePointer);

    //skip 11 bits to get to room name string
    fseek(filePointer, 11, SEEK_SET);
    char roomNameFile[20];
    //use getline to store remainder of line
    read = getline(&line, &len, filePointer);

    //snap the newline character off the end of the input (thanks, stackoverflow!)
    strtok(line, "\n");
    strtok(line, " ");

    sprintf(roomNameFile, "%s", line);
    //printf("###ROOM NAME: %s \n", roomNameFile);
    strcpy(arrayOfRooms[i].roomName, roomNameFile);
    strcpy(nameArray[i-1], roomNameFile);

    //assign numberofDoors variable
    arrayOfRooms[i].numberOfDoors = 0;

    //printf("TEST THE STRUCT: %s", arrayOfRooms[i].roomName);

    //make array of string to hold all doors for this room
    char doorName[6][16];
    sprintf(doorName[0], "*");
    sprintf(doorName[1], "*");
    sprintf(doorName[2], "*");
    sprintf(doorName[3], "*");
    sprintf(doorName[4], "*");
    sprintf(doorName[5], "*");

    int y;
    //TEST THAT ARRAY WAS PROPERLY CLEANED
    //for(y = 0; y < 6; y++){
     //       printf("TEST original DOORNAME ARRAY: %s\n", doorName[y]);
     //   }

    //use file contents to populate array of rooms
    int x = 0;
    fseek(filePointer, 0, SEEK_SET); //rewind back to start of file
    read = getline(&line, &len, filePointer); //Go to end of first line

    while ((read = getline(&line, &len, filePointer)) != -1) { //as long as there's something in the file to read

        //snap the newline character off the end of the input (thanks, stackoverflow!)
        strtok(line, "\n");

        //grab line from file
        char subString[16];
        char supString[40];
        char typeCheck[16];
        strcpy(supString, line);
        //printf("CHECK LINE: %s", line);

        //Make sure it's really room data
        if(line[0] == 'R'){
            //then we're looking at a room type, not a door name

            sprintf(typeCheck, "%.*s", 12, supString + 10);
            //printf("PRE LOOP TC: %s \n", typeCheck);
            //printf("PRE LOOP TC0: %c \n", typeCheck[0]);
            //printf("PRE LOOP TC1: %c \n", typeCheck[1]);

            if(typeCheck[1] == 'S'){
                //printf("CONDITIONAL CHECK: %c \n", typeCheck[0]);
                arrayOfRooms[i].typeOfRoom = START_ROOM;
                startingRoom = i;
            }
            else if(typeCheck[1] == 'E'){
                //printf("CONDITIONAL CHECK: %c \n", typeCheck[0]);
                arrayOfRooms[i].typeOfRoom = END_ROOM;
            }
            else{
                //printf("CONDITIONAL CHECK: %c \n", typeCheck[0]);
                arrayOfRooms[i].typeOfRoom = MID_ROOM;
            }
            //printf("###ROOM TYPE: %s \n", typeCheck);
            //printf("CHECK STRUCT: %d \n", arrayOfRooms[i].typeOfRoom);
        }
        else{
        int i;
        sprintf(subString, "%.*s", 12, supString + 15);

    /*    //Make sure it's really room data
        if(subString[2] == 'D' || subString[2] == 'A'){
            //then we're looking at a room type, not a door name
            strcpy(doorName[x], subString);
            printf("###DOOR NAME: %s \n", doorName[x]);
        } */
        if(subString[0] != ' ' && subString[0] != 'r'){
            strcpy(doorName[x], subString);

            //printf("###DOOR NAME: %s \n", doorName[x]);
        }

        //strcpy(arrayOfRooms[i].doorNames[x], doorName[x]);

        //build an array of all room names in index order
        //give each room an array of room numbers it can access
        //int y;
        //for(y = 0; y < NUMBER_OF_ROOMS; y++){
        //    if(strcmp(doorName[x], nameArray[y]) == 0){
        //        arrayOfRooms[i].doors[y] = y;
        //puts(arrayOfRooms[i].doorNames[x]);
        x++;

        }
    }

    //TEST THAT ASSIGNMENT WORKED
    //for(y = 0; y < 6; y++){
    //    if(strcmp(doorName[y], "*") != 0) {
     //       printf("$$TEST DOORNAME AFTER ASSIGNMENT: %s\n", doorName[y]);
     //   }
    //}
    //int y;
    sprintf(arrayOfRooms[i].doorNames[0], "*");
    sprintf(arrayOfRooms[i].doorNames[1], "*");
    sprintf(arrayOfRooms[i].doorNames[2], "*");
    sprintf(arrayOfRooms[i].doorNames[3], "*");
    sprintf(arrayOfRooms[i].doorNames[4], "*");
    sprintf(arrayOfRooms[i].doorNames[5], "*");

    //printf("\n ITERATION: %d\n", i);
    for(y = 0; y < 6; y++){

         if(strcmp(doorName[y], "*") != 0) {
            strcpy(arrayOfRooms[i].doorNames[y], doorName[y]);
            arrayOfRooms[i].numberOfDoors++;
            //printf("TEST NAMES in ROOM ARRAY: %s\n", arrayOfRooms[i].doorNames[y]);
        }
    }
    }
}

//initially set this to whatever room has type START_ROOM
struct room currentRoom;

//increment for every step taken
int steps = 0;
//controller for game loop
int gameOn = 1;

//array to hold steps taken
char stepsArray[20][16]; //use strcpy(stepsTaken[index], "string") to write to this array



int hereNow(){

    //printf("TEST CURRENT ROOM: %s", currentRoom.roomName);

    if(currentRoom.typeOfRoom == END_ROOM){

        printf("\nYOU HAVE FOUND THE END ROOM, CONGRATULATIONS!\n");
        printf("\nYOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS: \n", steps);
            //print stepsarray
            int i;
            for(i = 1; i <= steps; i++){
                printf("%s",stepsArray[i]);
                if(i != (steps)){
                printf(", ");
            }
    }
        printf("\n");
        exit(ITS_ALL_GRAVY);
    }
    printf("\nCURRENT LOCATION: %s\n", currentRoom.roomName);
    printf("POSSIBLE CONNECTIONS: ");
        int x;
        for(x = 0; x < currentRoom.numberOfDoors; x++){
            printf("%s", currentRoom.doorNames[x]);
            if(x != (currentRoom.numberOfDoors-1)){
                printf(", ");
            }
            else{
                printf(".");
            }
        }
        printf("\n");
        return 1;
}

void* getTheTime(){

    pthread_mutex_lock(&annoyingMutex);

    time_t systemTime;
    systemTime = time(NULL);
    printf("\n");
    printf(ctime(&systemTime));

    pthread_mutex_unlock(&annoyingMutex);
    return NULL;
}

int prompt(int current){
    printf("WHERE TO? >");
    char move[16];
    scanf("%s", &move);

    if (strcmp(move, "time") == 0){
        //print the time for the user
        pthread_mutex_unlock(&annoyingMutex);
        getTheTime();
        pthread_mutex_lock(&annoyingMutex);
        return current;
    }

    else if (strcmp(move, "exit") == 0){
        printf("Thanks for playing!");
        exit(ITS_ALL_GRAVY);
    }

    else if(strcmp(move, "") != 0){
        int i;
        for(i = 0; i < currentRoom.numberOfDoors; i++){
            if(strcmp(move, currentRoom.doorNames[i]) != 0){

                //printf("\nCHECK MOVE: %s", move);
                //find the room number with that name
                for(i = 0; i < NUMBER_OF_ROOMS; i++){

                    char checker[14];
                    sprintf(checker, "%s%c", arrayOfRooms[i].roomName, '\0');
                    //printf("\nCHECK CHECKER %d: %s", i, checker);
                    //printf("\nCHECK MOVE %d: %s", i, move);

                    if(strcmp(move, checker) == 0){
                        //printf("\nIT's A MATCH!\n");
                        //when found, make that the new currentRoom
                        //printf("\nTESTING C ROOM BEFORE: %s\n", currentRoom.roomName);
                        //currentRoom = arrayOfRooms[i];
                        //printf("TESTING move matched: %s\n", move);
                        steps++;
                        //add current room to stepsarray
                        strcpy(stepsArray[steps], move);
                        //strcpy(stepsArray[steps], move);

                        return i;
                    }


            }
        printf("HUH? SORRY, I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
        return current;
        }
    }
    }
}


void* doTheGame(){
    pthread_mutex_lock(&annoyingMutex);
    notTime = pthread_create(&mtime, NULL, getTheTime, NULL);
    printf("\t* ******************** *\n");
    printf("\t*                      *\n");
    printf("\t*    ADVENTURE GAME    *\n");
    printf("\t*                      *\n");
    printf("\t* ******************** *\n");

    //Get into the most recently created director
    manipDir();

    //form an array of room objects with data from the files in the directory
    rebuildAllRooms();

    //clean out stepsarrau
    sprintf(stepsArray[0], "*");
    sprintf(stepsArray[1], "*");
    sprintf(stepsArray[2], "*");
    sprintf(stepsArray[3], "*");
    sprintf(stepsArray[4], "*");
    sprintf(stepsArray[5], "*");

    //assign starting room
    currentRoom = arrayOfRooms[startingRoom];
    current = startingRoom;
    //strcpy(stepsArray[0], arrayOfRooms[startingRoom].roomName);

    //make sure all rooms exist
    //int i;
    //for(i = 0; i < NUMBER_OF_ROOMS; i++){
    //    printf("%s, ",arrayOfRooms[i].roomName);
    //}

    while(gameOn == 1){
        gameOn = hereNow();
        if(gameOn == 1){
            int newCur = prompt(current);
            //if(current != newCur){
            //    strcpy(stepsArray[steps], arrayOfRooms[current + 1].roomName);
            //}
            currentRoom = arrayOfRooms[newCur];
            current = newCur;
        }
    }
    pthread_mutex_unlock(&annoyingMutex);
    return;
}

int main(){
    notGame = pthread_create(&game, NULL, doTheGame, NULL);
    doTheGame();
    pthread_join(game,NULL);

    return ITS_ALL_GRAVY;
}
