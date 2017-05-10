//*******************************************************
// R O O M  A D V E N T U R E
// CS 344
// ku3nsting
// May 10, 2017
//*******************************************************

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
        START_ROOM,
        END_ROOM,
        MID_ROOM
        };

// Struct to define the room object itself
struct room {
        enum type typeOfRoom;
        char roomName[16];
        int number;
        int doors[7];  //we'll push values into this array to define which active doors this room actually has.
        //though each room can only have a max of 6 connections, this array holds 7 so that index of door in door array can match index of room in room array
        //if the door represents an active connection, the value at that index will be > 0
};

//array of all rooms in the game
struct room arrayOfRooms[NUMBER_OF_ROOMS];

//declare placeholders to identify indexes of target room and current room
int currentRoomIndex;
int endRoomIndex;

//array of all possible room names
char *nameArray[] = {"Asparagus", "Broccoli", "Carrot", "Daikon", "Eggplant", "Fennel", "Grapefruit", "Huckleberry", "Jicama", "Kale"};
int randomNameArray[NUMBER_OF_ROOMS];
int randNumArray[NUMBER_OF_ROOMS];
int numberOfDoors[NUMBER_OF_ROOMS];



//*******************************************************
//  D I R E C T O R Y  CSTART_ROOM, END_ROOM, and MID_ROOM R E A T I O N  function
//
//  makes a new directory named with a given suffix
//*******************************************************
//create directory function
char* newGameDirectory(int suffix){
    char *name = (char *) malloc(sizeof(char) * 32);
	//sprintf works like printf, but stores string in first parameter instead of printing:
	//use it to make name of new directory
	sprintf(name, "rkuenstx.rooms.%d", suffix);
	int result = mkdir(name, 0777);
	if (result != 0){
	    //if for some reason the directory can't be made, return error
	    printf("Something went wrong with directory creation. Exiting program.");
		exit(SOMETHING_WENT_WRONG);
	}

    //navigate into appropriate directory
	chdir(name);

	return name;
}

//*******************************************************
//  F I L E  C R E A T I O N  function
//
//  Gets the directory where the files should go, and
//  the room object that contains the data.
//  Creates a file within the directory and writes
//  some data inside of it.
//*******************************************************
void newFileForRoom(struct room currentRoom){

    printf("Got into the function!");

	//sprintf works like printf, but stores string in first parameter instead of printing:
	//use it to make name of new file
	char fileName[8];
	sprintf(fileName, "room_%d", currentRoom.number);

	//make a new file named room_number
	FILE *pointerToNewFile;
	pointerToNewFile = fopen(fileName, "w");

	//make sure the new file really exists
	if (pointerToNewFile == NULL){
		printf("New file couldn't be opened. Exiting program.");
		exit(SOMETHING_WENT_WRONG);
	}

	//Put the name of the current room in that room's file
	fprintf(pointerToNewFile, "ROOM NAME: %s \n", currentRoom.roomName);

	//print all doors from that room in the file
	//Right now, I'm only printing connections if they exist
	int r;
	//loop through all possible doors from current room
	for (r = 0; r < MAX_DOORS; r++){
        if (currentRoom.doors[r] == 0){
            //don't do anything, this door does not open
		}
		else{
            //there is a room beyond this door!
            fprintf(pointerToNewFile, "CONNECTION %d: ", (r + 1));
            fprintf(pointerToNewFile, "\t%s\n", arrayOfRooms[r].roomName);
		}
	}

	//print the type of room in the file
	if(currentRoom.typeOfRoom == START_ROOM){
        fprintf(pointerToNewFile, "\nROOM TYPE: START_ROOM\n");
	}
    else if(currentRoom.typeOfRoom == END_ROOM){
        fprintf(pointerToNewFile, "\nROOM TYPE: END_ROOM\n");
	}
    else {
        fprintf(pointerToNewFile, "\nROOM TYPE: MID_ROOM\n");
	}


	//close the file
	//fclose(pointerToNewFile);
	return;
}

//*******************************************************
//  R A N D O M  N U M B E R function
//
//  generates an array of random numbers for room
//  selection
//  Takes howMany numbers to generate and range -
//  (0 .. (range - 1)) will be numbers generated with offset 1
//  increasing offset adds offset to all results, so
//  range == 3 offset == 3 would generate numbers 3 through 5
//  new array overwrites array given as 4th parameter
//*******************************************************
void randArray(int howMany, int range, int offset, int overwriteArray[NUMBER_OF_ROOMS]){
//array of random numbers to use for room selection
    int newArray[howMany];
    int i;
    for(i = 0; i < howMany; i++){
        int randomNum;
        randomNum = (rand() % range) + offset;
        //keep offset at 1 for 0-range
        //search array to ensure no duplicates
        int x;
        for(x = 0; x <= i; x++){
            while(randNumArray[x] == randomNum){
                //if number has been used already, generate new numbers until you get one that doesn't match
                randomNum = (rand() % range) + offset;
            }
            overwriteArray[i] = randomNum;
            }
        }

    //print the array to test
    //int j;
    //for(j = 0; j < NUMBER_OF_ROOMS; j++){
    //    printf("%d, ", randNumArray[j]);
    //}
}

//*******************************************************
//  R O O M  I N I T I A L I Z E R
//
//  initializes room objects and
//*******************************************************
void roomInitializer(){
    //generate an array of random numbers to determine room names
    randArray(NUMBER_OF_ROOMS, 9, 1, randomNameArray);

            //find largest and smallest integers in randomNameArray
            int largest = 7;
            int smallest = 3;
            int idx;
            for(idx = 0; idx < NUMBER_OF_ROOMS; idx++){
                if(randomNameArray[idx] < smallest){
                    smallest = randomNameArray[idx];
                }
                if(randomNameArray[idx] > largest){
                    largest = randomNameArray[idx];
                }
            }

    //generate an array of random numbers to determine room doors
    randArray(NUMBER_OF_ROOMS, 3, 3, numberOfDoors);

    int i;
    for(i = 0; i < NUMBER_OF_ROOMS; i++){
        struct room tempRoom;
        //get random number from random name array
        //assign that index's name to room
        int x = randomNameArray[i];
        strcpy(tempRoom.roomName, nameArray[x]);
        //printf("assigning name: %s", nameArray[x]);

        //number the room according to where it appears in arrayOfRooms
        tempRoom.number = (i + 1);

        //decide how many doors the room has
        //the room has numberOfDoors[i] doors.
        int doorNumber = numberOfDoors[i];
        int p;
        for(p = 0; p < doorNumber; p++){
            tempRoom.doors[p] = randomNameArray[i + p];
        }

        //Make it so that largest name index room is start
        //and smallest name index room is end
        if(x == largest){
            tempRoom.typeOfRoom = START_ROOM;
        }
        else if(x == smallest){
            tempRoom.typeOfRoom = END_ROOM;
        }
        else{
            tempRoom.typeOfRoom = MID_ROOM;
        }
        //printf("\ntesting temproom name = %s", tempRoom.roomName);
        arrayOfRooms[i] = tempRoom;
        //printf("\ntesting arrayOfRooms name = %s", arrayOfRooms[i].roomName);
    }

}


int main(){

    //Get and store the processID, then use it to build a new directory
    int processID = getpid();

    roomInitializer();
    //randArray(NUMBER_OF_ROOMS, 4, 3, numberOfDoors);

    //print the array to test
    //int j;
    //for(j = 0; j < NUMBER_OF_ROOMS; j++){
    //    printf("%d, ", numberOfDoors[j]);
    //}

    printf("All done initializing rooms!\n");

    newGameDirectory(processID);
    printf("All done building directory!\n");

    //generate files for all rooms:
    int a;
    for(a = 0; a < NUMBER_OF_ROOMS; a++){
        newFileForRoom(arrayOfRooms[a]);
        //printf("\nRoom %s built!\n", arrayOfRooms[a].roomName);
    }
    printf("All done making files!\n");

    //randArray(NUMBER_OF_ROOMS, 9, 1, randNumArray);
    //printf("\nAll done generating random array!\n");

    return 0;
}
