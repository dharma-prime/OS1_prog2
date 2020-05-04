#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <math.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MIN_CONNECTIONS 3
#define MAX_CONNECTIONS 6
#define NUM_ROOMS 7
#define MAX_ROOMS 10

struct room {
	char name[9];
	//The type of room
	int  room_ID;
	//Number of connected rooms
	int num;
	struct room *adjacent[MAX_CONNECTIONS];
};

//Function Name: make_room
//Description: Allocates memory for a new room
//Input: Name of new room as char* and ID representing room type
//Output: struct room containing initialized values
//
//Status: Working great
struct room make_room(char *NAME,int ID){

	struct room new;

	//Initalize room values
	strcpy(new.name,NAME);
	new.room_ID=ID;
	new.num=0;
	return new;

}

//Function Name: same_room
//Description: Tests two rooms to determine if they are the same
//Input: Pointers to two different rooms
//Output: Returns 1 if they are the same, 0 if they are different
//
//Status: working
int same_room(struct room *a,struct room *b){

	int result=0;

	//If the two rooms have the same name, then they are the same
	if(!strcmp(a->name,b->name)) result=1;
	return result;

}

//Funciton Name: room_for_rooms
//Description: Tests a room struct to see if there is room for more
//connections
//Input: Pointer to a room struct
//Output: Returns a 1 if there is room, 0 if there is not
//
//Status: works
int room_for_rooms(struct room *a){

	int result=0;

	//If the room has less connections than the max, there is room
	if(a->num < MAX_CONNECTIONS) result=1;
	return result;

}

//Function Name: aleady_adjacent
//Description: Tests two rooms to see if they are connected
//Input: Pointers to two different rooms
//Output: Returns 1 if they are connected, 0 if they are not
//
//Status: working
int already_adjacent(struct room *a,struct room *b) {

	struct room *temp;
	int result=0,i=0;

	//Loop through all of the connections in the room
	while( i < (a->num)	){
		temp=a->adjacent[i];

		//If the adjacent room matches b, fail out
		if(!strcmp(temp->name,b->name)){
			result=1;
			break;
		}

		//Else, keep searching
		i++;
	}
	return result;

}
	
//Function Name: map_full 
//Description: checks to see whether or not rooms need connections
//Input: Array of Struct rooms
//Output: returns a 1 is the map is full and 0 if there are some rooms
//that need more connections
//
//Status: working!
int map_full(struct room *rooms) {

	int i;

	//Look through every room in the map
	i=0;
	while(i < NUM_ROOMS){

		//If there is a room that needs more adjacent rooms, exit loop
		if(rooms[i].num < MIN_CONNECTIONS) {
			i=0;
			break;
		}	

		//Else keep searching
		i++;
	}
	return i;

}

//Function Name: add_connection
//Description: Links together two rooms without testing
//Input: Pointers to two unique room structs that arent connected
//Output: none
//
//Status: Works perfectly
void add_connection(struct room *a,struct room *b) {

	//Connect the two rooms together
	a->adjacent[a->num]=b;
	b->adjacent[b->num]=a;
	
	//Increment the counters
	a->num++;
	b->num++;
	return;

}

//Function Name: print_room
//Description: displays all of the relevant information contained in a struct.
//Mainly for testing/ debugging purposes
//Input: Pointer to room struct
//Output: prints to console
//
//Status: Working
void print_room(struct room *a){

	int i;

	//Print out the name of the room
	printf("\nROOM NAME: %s\n",a->name);	


	//List all adjacent rooms
	for( i=0; i<(a->num); i++ ){
		printf("CONNECTION %d: %s \n",(i+1),a->adjacent[i]->name);
	}

	//Test what room type the room is and print it out.
	switch(a->room_ID) {
		case 1:	printf("ROOM TYPE: START_ROOM\n"); break;
		case 7: printf("ROOM TYPE: END_ROOM\n"); break;
		default: printf("ROOM TYPE: MID_ROOM\n"); break;
	}

}

//Funcion Name: write_to
//Description: writes the contents of a struct room to a file
//Input: Pointer to room struct and pointer to folder path
//Output: none
//
//Status: Works! yay!
void write_to(struct room *a, char *folder) {

	char buf[48],filepath[32];
	int i, room_file;

	//Create a new file in the set folder
	sprintf(filepath,"%s/%s_room",folder,a->name);
	room_file=open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	
	//Check to make sure file was written properly
	if (room_file<0) {
		printf("Error writing a file to...\n%s\n",filepath);
		perror("Error with file\n");
		exit(1);
	}	
	
	//Print the room name to the file
	sprintf(buf,"ROOM NAME: %s\n",a->name);
	write(room_file,buf,strlen(buf));

	//Append the adjacent rooms
	for(i=0; i<(a->num); i++) {
		sprintf(buf,"CONNECTION %d: %s\n",i+1,a->adjacent[i]->name);
		write(room_file,buf,strlen(buf));
	}

	//Test what room type the room is and append to file
	switch(a->room_ID) {
		case 1: sprintf(buf,"ROOM TYPE: START_ROOM\n"); break;
		case 7: sprintf(buf,"ROOM TYPE: END_ROOM\n"); break;
		default: sprintf(buf,"ROOM TYPE: MID_ROOM\n"); break;
	}
	write(room_file,buf,strlen(buf));
	
	//Close the file
	close(room_file);

}

int main () {

	// Seed the random number generator once per run
	srand(time(0));

	struct room directory[NUM_ROOMS];
	struct room *a, *b;
	int i,j,pid=getpid();

	//Make directory
	char filepath[32]="./primeb.rooms.";
	sprintf(filepath,"%s%d",filepath,pid);
	mkdir(filepath,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
	
	int room_num[MAX_ROOMS]={0};
	//Hardcoded names for rooms
	char room_names[MAX_ROOMS][9]= {
		"Library",
		"Garage",
		"Attic",
		"Bedroom",
		"Study",
		"Theater",
		"Garden",
		"Bathroom",
		"Basement",
		"Kitchen"
	};


	//This function call is for testing purposes
//	testing(directory,filepath);	

//Randomly generate rooms
	for(i=0; i < NUM_ROOMS; i++) {

		//Look for unused names
		while(1) {

			//Randomly select a name
			j=rand() % MAX_ROOMS;

			//If the name is unused, name the room
			if (room_num[j]==0) {
				directory[i]=make_room(room_names[j],i+1);
				room_num[j]=1;
				break;
			}
		}

	}

//Randomly connect rooms
	//Check to make sure that there are new connections to make in the map
	while(!map_full(directory)) {

		//Randomly select a room
		i=rand() % NUM_ROOMS;
		a=&directory[i];

		//Test to see if the room has open spots
		//If there is foom to add, pick another room to connect to a
		if(room_for_rooms(a)) {

			//Randomly select another room, b, that is not connected to a
			do {
				j=rand() % NUM_ROOMS;
				b=&directory[j];
			} while( same_room(a,b) || !room_for_rooms(b) || already_adjacent(a,b) );

			//Connect room a to b 
			add_connection(a,b);
		}

	}

//Write all of the room information to files
	for(i=0; i < NUM_ROOMS; i++) {
		write_to(&directory[i],filepath);
	}

	return 0;

}
