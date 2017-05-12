//*******************************************************
// R O O M  A D V E N T U R E
// Buildrooms.c
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
        int doorNumber; //number of doors
        int doors[MAX_DOORS];  //we'll push values into this array to define which active doors this room actually has.
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
int testArray[NUMBER_OF_ROOMS];
int randNumArray[NUMBER_OF_ROOMS];
int numberOfDoors[NUMBER_OF_ROOMS];
int writtenDoors[NUMBER_OF_ROOMS];
int doorsPerRoom[NUMBER_OF_ROOMS * MAX_DOORS];

//*******************************************************
//  A R R A Y  I N I T I A L I Z E R  function
//
//  fills array with 0
//*******************************************************
void initArray(int array[], int size){
    //overwrite whole array
    int k;
    for(k = 0; k <= size; k++){
        array[k] = -1;
    }
}

//*******************************************************
//  D I R E C T O R Y  C R E A T I O N  function
//
//  makes a new directory named with a given suffix
//*******************************************************
//create directory function
char* newGameDirectory(int suffix){
    char *name = (char *) malloc(sizeof(char) * 32);
	//sprintf works like printf, but stores string in first parameter instead of printing:
	//use it to make name of new directory
	sprintf(name, "kuenstir.rooms.%d", suffix);
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

    //printf("Got into the function!");

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
	int x = 0;

	//init array of doors
	//initArray(currentRoom.doors);

	//TEST
	//print all possible doors for current room
    //int i;
    //printf("TEST THIS THANG!!!");
    //printf("Doors for %d: ", currentRoom.number);
    //for (i = ((currentRoom.number - 1) * MAX_DOORS); i < (((currentRoom.number - 1) * MAX_DOORS) + MAX_DOORS); i++){
    //  printf("%d ", doorsPerRoom[i]);
    //}
    //printf("\n");

    int curIdx = 0;
    //printf("ROOM %s:\n ", currentRoom.roomName);
	//loop through all possible doors from current room
	for (r = ((currentRoom.number - 1) * MAX_DOORS); r < (((currentRoom.number - 1) * MAX_DOORS) + MAX_DOORS); r++){
        if (curIdx == (currentRoom.number - 1)){
            curIdx++;
        }
        if (doorsPerRoom[r] != 1){
            //don't do anything, this door does not open
		}
		else{
            //there is a room beyond this door!
            fprintf(pointerToNewFile, "CONNECTION %d: ", (x + 1));
            //printf("CONNECTION %d: ", (x + 1));
            x++;
            fprintf(pointerToNewFile, "\t%s\n", arrayOfRooms[curIdx].roomName);
            //printf("\t%s\n", arrayOfRooms[curIdx].roomName);
		}
		//printf("\n");
    curIdx++;
	}

	//print the type of room in the file
	if(currentRoom.typeOfRoom == START_ROOM){
        fprintf(pointerToNewFile, "ROOM TYPE: START_ROOM\n");
	}
    else if(currentRoom.typeOfRoom == END_ROOM){
        fprintf(pointerToNewFile, "ROOM TYPE: END_ROOM\n");
	}
    else {
        fprintf(pointerToNewFile, "ROOM TYPE: MID_ROOM\n");
	}


	//close the file
	//fclose(pointerToNewFile);
	return;
}

//*******************************************************
//  R A N D O M  N U M B E R function
//
//  generates an array of random numbers
//  Takes howMany numbers to generate and range -
//  (0 .. (range)) will be numbers generated with offset 1
//  increasing offset adds offset to all results, so
//  range == 3 offset == 3 would generate numbers 3 through 5
//  new array overwrites array given as 4th parameter
//*******************************************************
void randArray(int howMany, int range, int offset, int overwriteArray[], int unique, int size){
//array of random numbers to use for room selection

    //if(unique == 1){
    //    printf("CHOSE UNIQUE\n");
    //}
    //else{
    //    printf("NOT UNIQUE\n");
    //}

    //overwrite whole array
    initArray(overwriteArray, size);

    //int h;
    //for(int h = 0; h < 7; h++){
    //printf("CHECK THE ARRAY BEFORE ANYTHING ELSE: %d\n", overwriteArray[h], 7);
    //}
    //printf("\n");

    int i;
    for(i = 0; i < howMany; i++){
        //printf("\nFOR position %d\n", i);
        int randomNum;
        randomNum = (rand() % range) + offset;
        //printf("INITIAL RANDNUM, got %d\n", randomNum);

        //keep offset at 1 for 0-range

        //if requested, search array to ensure no duplicates
        if(unique == 1){
            int x;
            for(x = 0; x < i; x++){
                while(overwriteArray[x] == randomNum){
                    //if number has been used already, generate new numbers until you get one that doesn't match
                    //printf("HAD TO REROLL, had %d\n", randomNum);
                    randomNum = (rand() % range) + offset;
                    //printf("HAD TO REROLL, got %d\n", randomNum);
                }
            }
        }

        overwriteArray[i] = randomNum;
    }

    //print the array to test
    //int j;
   // for(j = 0; j < NUMBER_OF_ROOMS; j++){
    //    printf("%d, ", overwriteArray[j]);
   // }
}

//*******************************************************
//  R O O M  I N I T I A L I Z E R
//
//  initializes room objects and
//*******************************************************
void roomInitializer(){
    //generate an array of random numbers to determine room names
    initArray(randomNameArray, 7);
    randArray(NUMBER_OF_ROOMS, 9, 0, randomNameArray, 1, 7);

    //int y;
    //for(y = 0; y < NUMBER_OF_ROOMS; y++){
    //    printf("\nRANDOM NAME ARRAY CHECK FOR DUPES: %d, ", randomNameArray[y]);
    //}
    //printf("\n");

            //find largest and smallest integers in randomNameArray
            int largest = 6;
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
    randArray(NUMBER_OF_ROOMS, 4, 3, numberOfDoors, 0, 7);

    //this loop initializes all room data members except connections
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
        tempRoom.doorNumber = numberOfDoors[i];

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

        //TEST DOORS ARRAY FOR EACH ROOM
        //int g;
        //printf("\nTESTING THIS TEMPROOM MOFO:\n");
        //for(g = (MAX_DOORS * (i-1)); g < (MAX_DOORS * i); g++){
        //    printf("%d ", doorsPerRoom[g]);
        //}
        //printf("\n");

        //printf("\ntesting temproom name = %s", tempRoom.roomName);
        arrayOfRooms[i] = tempRoom;
        //printf("\ntesting arrayOfRooms name = %s", arrayOfRooms[i].roomName);
    }


    //set the doors array to 0
    initArray(doorsPerRoom, 6);

    //now, total = total number of incoming and outgoing doors.
    //since all doors are shared between rooms, divide by 2
    //now, distribute doors between all rooms:

    //figure out where doors connect
    int b = 0;
    int x = 0;

    int d;
    for(d = 0; d<6; d++){
        writtenDoors[d] = -1;
        }

    //keep going until b == 7 to hit every room
    while(b < NUMBER_OF_ROOMS){
        int doorsMade[6] = {-1, -1, -1, -1, -1, -1};

        //printf("\n\nITERATION: %d", b);

        //keep going until we've written the right number of doors for this room
        while(writtenDoors[b] < arrayOfRooms[b].doorNumber){

            //random door between 0 and 6, not including current room #
            //b == current room number
            int randomDoor = b;

            //if random door points to current room, make a new one
            while(randomDoor == b){
                randomDoor = (rand() % 7);

                //check against all established doors for this room
                //if random door is duplicate, make a new one
                int i;
                for(i=0; i<6; i++){
                    if(randomDoor == doorsMade[i]){
                        randomDoor = b;
                    }
                }
                //printf("\nRandom door: %d", randomDoor);
            }

            //store created door in doorsmade array
            doorsMade[x] = randomDoor;
            x++;

            //open door from b to random, increment writtendoor for index b
            doorsPerRoom[randomDoor + (b * MAX_DOORS)] = 1;
            writtenDoors[b] = (writtenDoors[b] + 1);

            //open door in the reverse direction too
            doorsPerRoom[b + (randomDoor * MAX_DOORS)] = 1;
            writtenDoors[randomDoor] = (writtenDoors[randomDoor] + 1);

            //printf("\nwritten doors: %d", writtenDoors[b]);
            //printf("\nneeded doors: %d", arrayOfRooms[b].doorNumber);

            if( writtenDoors[b]== arrayOfRooms[b].doorNumber){
                //printf("\nBOY HOWDY, THAT'S ENOUGH DOORS! for %d\n", b);
            }
            }

            b++;
        }
    }




int main(){

    srand(time(NULL)); //gives us pseudo-random numbers

    //Get and store the processID, then use it to build a new directory
    int processID = getpid();

    roomInitializer();
    //randArray(NUMBER_OF_ROOMS, 4, 3, numberOfDoors);

    //print the array to test
    //int j;
    //for(j = 0; j < NUMBER_OF_ROOMS; j++){
     //   printf("num of doors: %d, ", numberOfDoors[j]);
    //    printf("random name %d \n", randomNameArray[j]);
    //}

    //test it out
    //int i;
    //printf("\nTESTING ARRAY OF ROOMS: \n");
    //for(i=0; i<NUMBER_OF_ROOMS; i++){
    //printf("%s ", arrayOfRooms[i].roomName);
    //printf("%d \n", randomNameArray[i]);
    //}
    //printf("\n");

    //printf("All done initializing rooms!\n");

    newGameDirectory(processID);
    //printf("All done building directory!\n");

    //generate files for all rooms:
    int a;
    for(a = 0; a < NUMBER_OF_ROOMS; a++){
        newFileForRoom(arrayOfRooms[a]);
        //printf("\nRoom %s built!\n", arrayOfRooms[a].roomName);
    }
    //printf("All done making files!\n");

    //randArray(NUMBER_OF_ROOMS, 9, 1, randNumArray);
    //printf("\nAll done generating random array!\n");

    return 0;
}
