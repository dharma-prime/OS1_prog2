/* Program Name: primeb.adventure.c
 * Author: Bodhidharma Prime
 * Date Created: 10/29/2019
 * Description: Player interface that reads files from directory and utilizes file io
 * and multithreading

 * This is based off a previous submision for this class F2019
 * Original scrupt and this submission can be found on github
 * github.com/dharma-prime/OS1_prog2

 * Date Updated: 5/3/2020
 * - Added comments
 * - Properly implement pthreads and mutex locking
 */

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

#define MIN_CONNECTIONS 3
#define MAX_CONNECTIONS 6
#define NUM_ROOMS 7
#define MAX_ROOMS 10
#define MY_PATH "primeb.rooms."
#define MY_DIR "."
#define TIME_FILE "currentTime.txt"

pthread_mutex_t time_lock;

struct room {
	char name[10];
	//The type of room
	int  room_ID;
	//Number of connected rooms
	int num;
	char adjacent[MAX_CONNECTIONS][10];
};

struct room_link {
	char name[10];
	struct room_link *next;
};

//Function Name: whattimeisitrightnowdotcom
//Description: Writes the current time to a file
//Input: none
//Output:
//
void* whattimeisitrightnowdotcom(void * arg){

	char right_now[50];
	struct tm *tmp;
	time_t t;
	FILE * my_file;

	// Constantly run in background
	while(1) {

		// Read the current time and format string
		time(&t);
		tmp=localtime(&t);
		strftime(right_now,sizeof(right_now),"%I:%M%p, %A, %B %d, %Y",tmp);

		// Block main thread when updating the currentTime file
		pthread_mutex_lock(&time_lock);

		my_file=fopen(TIME_FILE,"w+");

		// Write string to output file
		fputs(strcat(right_now,"\n"),my_file);
		fclose(my_file);

		// Unblock when done with file
		pthread_mutex_unlock(&time_lock);
	}

	return NULL;

}


//Function Name: find_time
//Description: Read current time from file
//Input: none
//Output: string in stdout
void find_time() {

	FILE * my_file;
	char *line;
	size_t len=0;

	// Block pthread to update currentTime file
	pthread_mutex_lock(&time_lock);

	// Open file and print contents to std out
	my_file=fopen(TIME_FILE,"r");
	getline(&line,&len,my_file);
	printf(line);
	fclose(my_file);

	// Unblock when done with file
	pthread_mutex_unlock(&time_lock);

	return;

}

//Function Name:is_connected
//Description: Tests to see whether or not two rooms are connected
//Input: Struct room of current location, C string of next room
//Output: Returns a 1 if they are connected, 0 if not
int is_connected(struct room here, char *connected) {

	int i,result;
	char tmp[10];

	result=0;

	// Loop through connected rooms
	for(i=0;i<here.num;i++) {

		// Grab room name from adjacent room and compare to desired room name
		sprintf(tmp,"%s\n",here.adjacent[i]);
		if(strcmp(tmp,connected) == 0){

			// If the room name matches, exit out of the loop and return value of 1
			result=1; break;
		}
	}
	return result;

}

//Function Name:where_are_you
//Description:Prints out to the user where they are and all adjacent rooms
//Input:struct room of current location
//Output: output to stdout
//
//Status: works!
void where_are_you(struct room here){

	int i;

	//Display where you are
	printf("CURRENT LOCATION: %s\nPOSSIBLE CONNECTIONS:",here.name);

	//Display all possible room connections
	i=0;
	while (i < (here.num) - 1 ) {
		printf(" %s,",here.adjacent[i]);
		i++;
	}

	printf(" %s.\n",here.adjacent[i]);

}

//Function Name: find_room
//Description: Looks through the map and returns the index of the matching name
//Input: array of room structs, name of desired room
//Output: int index of desired room
//
//Status: works
int find_room(struct room *map,char *name) {

	int i,result=-1;
	char tmp[12];

	// Loop through all available rooms
	for(i=0;i<NUM_ROOMS;i++) {
		sprintf(tmp,"%s\n",map[i].name);
		// If room name matches the desired room, return the index
		if(strcmp(tmp,name)==0) {
			result=i;break;
		}
	}

	return result;

}

//Function Name: find_start
//Description: looks for the room that is the start of the map
//and returns its index
//Input: struct room array containing map
//Output: int index of start room
//
//Status: working
int find_start(struct room *map) {

	int i,result=-1;

	// Loop through all of the rooms
	for(i=0;i<NUM_ROOMS;i++) {

		// Check for room matching start room id
		if(map[i].room_ID==-1) {
			result=i;
			break;
		}

	}

	return result;
}

//Function Name: find_end
//Description: looks for the rom that is the end of the map
//and returns its index
//Index: struct room array containing map
//Output: int index of start room
//
//Status:
int find_end(struct room *map) {

	int i, result=-1;

	// Loop through all of the rooms
	for(i=0;i<NUM_ROOMS;i++) {

		// Check for room mathcing end room id
		if(map[i].room_ID==1) {
			result=i; break;
		}
	}

	return result;

}

//Function Name: get_input
//Description: reads in input from the user and returns the int value
//for the next room link
//Input: array of map, int index of current room
//Output: returns in index of next room
//
//Status: working
int get_input(struct room *map, int here){

	char *line;
	size_t line_size=0;
	int next_room;

	getline(&line,&line_size,stdin);

	// Handle user input

	// If user is asking for the time, print the time
	if(strcmp(line,"time\n")==0) {
		find_time();
		next_room=here;
	}
  // Else if moving to another room, check if they are connected
	else if(is_connected(map[here],line)) {
		next_room=find_room(map,line);
	}
	// Else ask for user to try again
	else {
		printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN...\n");
		next_room=here;
	}

	// Return pointer to players updated location
	return next_room;

}

//Function Name: new_link
//Description: creates a new link in a linked list
//Input: linked list, name of new link
//Output: none
//
//Status: unk
void new_link(struct room_link **head,char *new) {

	struct room_link *new_link=(struct room_link*)malloc(sizeof(struct room_link));
	struct room_link *end=*head;

	// Create new link
	strcpy(new_link->name,new);
	new_link->next=NULL;

	// If link is the head, add new link to head
	if (*head == NULL) {
		*head=new_link;
		return;
	}

	// Move down the linked list
	while(end->next != NULL) {
		end=end->next;
	}

	// Add the new link to the end of the list
	end->next=new_link;

}

//Function Name: rm_links
//Description: removes all of the links in the list and prints out the names
//Input: linked list
//Output: none
//
//Status: unk
void rm_links(struct room_link **head) {

	struct room_link * current= *head;
	struct room_link * next_link;

	// While there are items in the linked list
	while (current != NULL) {

		// Print the name, and remove the link
		printf("%s\n",current->name);
		next_link=current->next;
		free(current);
		current=next_link;
	}

	*head=NULL;
}

//Function Name: find_folder
//Description: Finds the newest primeb.rooms.? folder and returns the dir name
//Input: none
//Output: String containing folder
//
//Status: working
char * find_folder(){

	struct dirent *dp;
	struct stat buffer;
	char * newest;
	time_t newest_t;
	DIR *dr = opendir(MY_DIR);

	//Loop through all files in the current directory
	newest_t=0;
	while ( (dp = readdir(dr)) != NULL ) {


		//Test to see if the file is a directory beginning with the primeb.rooms
		if ( stat(dp->d_name,&buffer) == -1 ||	strstr(dp->d_name,MY_PATH) == NULL ) continue;

		//Save the directory name and the new lowest time
		if (buffer.st_mtime > newest_t) {
			newest_t=buffer.st_mtime;
			newest=dp->d_name;
		}

	}

	//Close the current directory
	closedir(dr);

	return newest;
}

//Function Name: new_rooms
//Description: reads all of the room files from the given directory and
//fills them into an array of rooms
//Input: folder location as string, struct to array of empty rooms
//Output: array of empty rooms is now initalized
//
//Status: works!
void new_rooms(char * folder_location,struct room *map) {

	char temp[strlen(folder_location)];
	strcpy(temp,folder_location);
	struct dirent *dp;
	DIR *dr = opendir(folder_location);
	char file_location[64];
	FILE * current_room;
	int i,j;

	size_t line_size=0;
	char *line=NULL;
	char buf1[10];
	char buf2[10];
	char buf3[10];

	i=0;

	// Read from every file in the directory
	while ( (dp = readdir(dr)) != NULL ) {

		// Test to only read from the room files
		if (strstr(dp->d_name,"room")==NULL) {
			continue;
		}

		// Open file
		sprintf(file_location,"./%s/%s",temp,dp->d_name);
		current_room=fopen(file_location,"r");

		j=0;
		// Ittereate through every line of the file
		while (getline(&line,&line_size,current_room) != -1) {

			sscanf(line,"%s %s %s",buf1,buf2,buf3);

			// Save the name
			if(strcmp(buf2,"NAME:")==0) {
				strcpy(map[i].name,buf3);
			}
			// Else, save the type
			else if(strcmp(buf2,"TYPE:")==0) {

				if(strcmp(buf3,"START_ROOM")==0) {
					map[i].room_ID=-1;
				}	else if(strcmp(buf3,"END_ROOM")==0) {
					map[i].room_ID=1;
				} else {
					map[i].room_ID=0;
				}

			}
			// Else, save the names of adjacent rooms
			else {
				strcpy(map[i].adjacent[j],buf3);
				j++;
			}
		}

		// Move to the next spot in the map
		map[i].num=j;
		fclose(current_room);
		i++;
	}

}

int main() {

  // Create pthread for timekeeping
	pthread_t time_thread;
	pthread_create(&time_thread,NULL,whattimeisitrightnowdotcom,NULL);

	char *my_folder=find_folder();
	int idx,end;
	struct room_link *head=NULL;
	int link_ct=0,next_idx;

	//  Create room map
	struct room map[NUM_ROOMS];
	new_rooms(my_folder,map);

	// Find the start and end of the map
	idx=find_start(map);
	end=find_end(map);

	// Test for find_start(map)
	if (idx==-1) {
		printf("Why isn't this working\n");
		return 1;
	}
	else {

		// Run until player reaches the end of the map
		while(idx != end) {

			// Print where user currently is
			where_are_you(map[idx]);
			printf("Where to? >");

			// Get user input and move rooms
			next_idx = get_input(map,idx);

			// If the player move to a different room, update the linked list
			// tracking the players movements
			if (next_idx != idx) {
				idx=next_idx;
				new_link(&head,map[idx].name);
				link_ct++;
				printf("\n");
			}
		}
	}

	// Print out how the player did
	new_link(&head,map[end].name);
	printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n",link_ct);
	rm_links(&head);

	return 0;

}
