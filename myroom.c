// Room adventure - CS 344
// ku3nsting
// May 10, 2017

//includes:
#include <stdio.h> //gives us basic file i/o and stuff
#include <stdlib.h>  //gives us all standard library functions

#include <time.h> //gives us foundation for random numbers and time output
#include <string.h> //necessary for strcpy to work
#include <unistd.h> //lets us access the process ID
#include <sys/stat.h> //needed for mkdir according to http://www.gnu.org/software/libc/manual/html_node/Creating-Directories.html

//preprocessor variables:
#define NUMBER_OF_ROOMS         7
#define SOMETHING_WENT_WRONG    1
#define MAX_DOORS               6

//use an enum to describe the type of each room
enum type {
        BEGINNING,
        END,
        HALLWAY
};

// Struct to define the room object itself
struct room {
        enum type typeOfRoom;
        char roomName[16];
        int doors[7];  //we'll push values into this array to define which active doors this room actually has.
        //though each room can only have a max of 6 connections, this array holds 7 so that index of door in door array can match index of room in room array
        //if the door represents an active connection, the value at that index will be > 0
};

//array of all rooms in the game
struct room arrayOfRooms[NUMBER_OF_ROOMS];

//declare placeholders to identify indexes of target room and current room
int currentRoomIndex;
int endRoomIndex;

//create directory function
char* newGameDirectory(int suffix){
    char *name = (char *) malloc(sizeof(char) * 32);
	//sprintf works like printf, but stores string in first parameter instead of printing:
	//use it to make name of new directory
	sprintf(name, "rkuenstx_adventure.%d", suffix);
	int result = mkdir(name, 0777);
	if (result != 0){
	    //if for some reason the directory can't be made, return error
	    printf("Something went wrong with directory creation. Exiting program.");
		exit(SOMETHING_WENT_WRONG);
	}
	return name;
}

void newFileForRoom(char* directory, struct room currentRoom, int numOfRoom){

    //navigate into appropriate directory
	chdir(directory);
	//sprintf works like printf, but stores string in first parameter instead of printing:
	//use it to make name of new file
	char fileName[8];
	sprintf(fileName, "room_%d", numOfRoom);
	//make a new file named room_number
	FILE *pointerToNewFile;
	pointerToNewFile = fopen(fileName, "w");

	//make sure the new file really exists
	if (pointerToNewFile == NULL){
		printf("New file couldn't be opened. Exiting program.");
		exit(SOMETHING_WENT_WRONG);
	}

	//Put the name of the current room in that room's file
	fprintf(pointerToNewFile, "ROOM NAME: %s\n", currentRoom.roomName);

	//print all doors from that room in the file
	//Right now, I'm only printing connections if they exist
	int i;
	//loop through all possible doors from current room
	for (i = 0; i < MAX_DOORS; i++){
        if (currentRoom.doors[i] == 0){
            //don't do anything, this door does not open
		}
		else{
            //there is a room beyond this door!
            fprintf(pointerToNewFile, "\nCONNECTION %d: ", (i + 1));
            fprintf(pointerToNewFile, "\t%s\n", arrayOfRooms[i].roomName);
		}
	}

	//print the type of room in the file
	fprintf(pointerToNewFile, "\nROOM TYPE: %s\n", currentRoom.typeOfRoom);

	//close the file
	fclose(pointerToNewFile);
	return;
}

int main(){

    //Get and store the processID, then use it to build a new directory
    int processID = getpid();
    char* targetDirectory = newGameDirectory(processID);
    printf("All done building directory!\n");

    newFileForRoom(targetDirectory, arrayOfRooms[1], 1);
    printf("All done making files!\n");

    return 0;
}