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

//exit codes
#define SOMETHING_WENT_WRONG    1
#define ITS_ALL_GRAVY           0
#define NUMBER_OF_ROOMS         7

//Variables. Figure out how to use this info from files
char newestDirName[64];

//use an enum to describe the type of each room
enum type {
        START_ROOM,
        END_ROOM,
        MID_ROOM
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

//code borrowed from 2.4 Manipulating Directories in tophat
void manipDir(){
  int newestDirTime = -1; // Modified timestamp of newest subdir examined
  char targetDirPrefix[20] = "rkuenstx.rooms."; // Prefix we're looking for
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

  printf("Newest entry found is: %s\n", newestDirName);
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

	printf("\nNEWEST: %s \n", newestDirName);
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
    printf("%s \n", roomName);

	FILE *filePointer;
	//open the file!
	filePointer = fopen(roomName, "r");

	//make sure file is real
    if (filePointer == NULL){
        printf("GET OUTTA HEAH\n");
        exit(SOMETHING_WENT_WRONG);
    }
    else{
        printf("FILE OPENED!\n");
    }

    //count the lines in the file
    int numLines = countLines(filePointer);

    //skip 11 bits to get to room name string
    fseek(filePointer, 11, SEEK_SET);
    char roomNameFile[20];
    //use getline to store remainder of line
    read = getline(&line, &len, filePointer);
    strcpy(roomNameFile, line);
    printf("###ROOM NAME: %s \n", roomNameFile);

    //make array of string to hold all doors for this room
    char doorName[6][20];

    //use file contents to populate array of rooms
    int x = 0;
    fseek(filePointer, 0, SEEK_SET); //rewind back to start of file
    read = getline(&line, &len, filePointer); //Go to end of first line

    while ((read = getline(&line, &len, filePointer)) != -1) { //as long as there's something in the file to read
        //grab line from file
        char subString[16];
        char supString[40];
        char typeCheck[16];
        strcpy(supString, line);
        //printf("CHECK LINE: %s", line);

        //Make sure it's really room data
        if(line[0] == 'R'){
            //then we're looking at a room type, not a door name
            sprintf(typeCheck, "%.*s", 11, supString + 10);
            if(typeCheck[0] == 'S'){
                arrayOfRooms[i].typeOfRoom = START_ROOM;
            }
            else if(typeCheck[0] == 'E'){
                arrayOfRooms[i].typeOfRoom = END_ROOM;
            }
            else{
                arrayOfRooms[i].typeOfRoom = MID_ROOM;
            }
            printf("###ROOM TYPE: %s \n", typeCheck);
        }
        else{
        int i;
        sprintf(subString, "%.*s", 16, supString + 15);

    /*    //Make sure it's really room data
        if(subString[2] == 'D' || subString[2] == 'A'){
            //then we're looking at a room type, not a door name
            strcpy(doorName[x], subString);
            printf("###DOOR NAME: %s \n", doorName[x]);
        } */

        strcpy(doorName[x], subString);
        printf("###DOOR NAME: %s \n", doorName[x]);
        x++;
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


void hereNow(){
    if(currentRoom.typeOfRoom == END_ROOM){
        printf("YOU HAVE FOUND THE END ROOM, CONGRATULATIONS!\n");
        printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS: \n", steps);
        int y;
        for(y = 0; y < steps; y++){
            printf("%s", stepsArray[y]);
        }
        printf("\n");
        gameOn = 0;
    }
    printf("\nCURRENT LOCATION:\t%s\n", currentRoom);
    printf("POSSIBLE CONNECTIONS:\t");
        int x;
        for(x = 0; x < currentRoom.numberOfDoors; x++){
            printf("%s", currentRoom.doorNames[x]);
        }
        printf("\n");
}

void prompt(){
    printf("\nWHERE TO?\t> ");
    char move[10];
    scanf("%s", &move);

    if (strcmp(move, "time") == 0){
        //handle this later
    }

    else if (strcmp(move, "exit") == 0){
        printf("Thanks for playing!");
        exit(ITS_ALL_GRAVY);
    }

    else{
        int i;
        for(i = 0; i < currentRoom.numberOfDoors; i++){
            if(move == currentRoom.doorNames[i]){
            //go to that room
            }
            else{
                printf("HUH? SORRY, I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.");
            }
        }
    }
}


int main(){
    printf("\t* ******************** *\n");
    printf("\t*                      *\n");
    printf("\t*    ADVENTURE GAME    *\n");
    printf("\t*                      *\n");
    printf("\t* ******************** *\n");

    manipDir();
    rebuildAllRooms();

    while(gameOn == 1){
        hereNow();
        prompt();
    }

    return ITS_ALL_GRAVY;
}
