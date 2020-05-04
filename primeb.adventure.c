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
//Description: Displays the time to stdout
//Input: none
//Output: String in stdout
//
//Status: works great, needs to write to output file
void whattimeisitrightnowdotcom() {

	char right_now[50];
	struct tm *tmp;
	time_t t;
	FILE * my_file;

	//Find and format the current time	
	time(&t);
	tmp=localtime(&t);
	strftime(right_now,sizeof(right_now),"%I:%M%p, %A, %B %d, %Y",tmp);

	//Print to stdout
	printf("\n\t%s\n\n",right_now);

	//Write time to file (do not append)
	my_file=fopen(TIME_FILE,"w+");
	fputs(strcat(right_now,"\n"),my_file);
	fclose(my_file);

}

//Function Name:is_connected
//Description: Tests to see whether or not two rooms are connected
//Input: Struct room of current location, C string of next room
//Output: Returns a 1 if they are connected, 0 if not
//
//Status: working
int is_connected(struct room here, char *connected) {

	int i,result;
	char tmp[10];

	//Itterate through the adjacent rooms
	result=0;
	for(i=0;i<here.num;i++) {

		//Test if the adjacent room matches the desired room
		sprintf(tmp,"%s\n",here.adjacent[i]);
		if(strcmp(tmp,connected) == 0){
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
	//Propper punctuation for the last room
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

	//Itterate through the map and test to see if the room
	//matches the desired name
	for(i=0;i<NUM_ROOMS;i++) {

		//Add a newline to the end to match getline input
		sprintf(tmp,"%s\n",map[i].name);

		//If the names match, change the output		
		if(strcmp(tmp,name)==0) {
			result=i;
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

	//Itterate through the map and look for the starting room
	for(i=0;i<NUM_ROOMS;i++) {

		//Set the index to the correct spot
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
//Status: working!
int find_end(struct room *map) {

	int i, result=-1;
	
	//Itterate through the map and look to the final room
	for(i=0;i<NUM_ROOMS;i++) {
		
		//Set the index to the end room
		if(map[i].room_ID==1) {
			result=i;
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

	//Grab user input
	getline(&line,&line_size,stdin);

	//If the user asks for the time, display the time
	if(strcmp(line,"time\n")==0) {
		//Run multithreading stuff here
		whattimeisitrightnowdotcom();
		next_room=here;
	} 
	//If the user asks for an adjacent room, change the return index
	else if(is_connected(map[here],line)) {
		next_room=find_room(map,line);
	} 
	//If the input is unrecognized, tell the user to try again
	else {
		printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN...\n");
		next_room=here;
	}

	return next_room;

}

//Function Name: new_link
//Description: creates a new link in a linked list
//Input: linked list, name of new link
//Output: none
//
//Status: works
void new_link(struct room_link **head,char *new) {

	//Pseudo code for linked lists from GeeksforGeeks.org
	struct room_link *new_link=(struct room_link*)malloc(sizeof(struct room_link));
	struct room_link *end=*head;

	//Add data to the new link
	strcpy(new_link->name,new);
	new_link->next=NULL;

	//Base care for if the linked list is empty
	if (*head == NULL) {
		*head=new_link;
		return;
	}

	//Get the last link that exists
	while(end->next != NULL) {
			end=end->next;
	}	

	//Add the new link to the end of the list
	end->next=new_link;

}

//Function Name: rm_links
//Description: removes all of the links in the list and prints out the names
//Input: linked list
//Output: none
//
//Status: unk
void rm_links(struct room_link **head) {

	//Pseudocode for linked list from GeeksforGeeks.org
	struct room_link * current= *head;
	struct room_link * next_link;

	//Itterate through the list and deallocate memory
	while (current != NULL) {
		printf("%s\n",current->name);
		next_link=current->next;		
		free(current);
		current=next_link;
	}

	//Clear the linked list pointer
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

	//Itterate through the given directory and look at each file
	i=0;
	while ( (dp = readdir(dr)) != NULL ) {

		//Test to make sure the file is a room file 
		if (strstr(dp->d_name,"room")==NULL) {
			continue;
		}

		//If room file, create string for file location
		sprintf(file_location,"./%s/%s",temp,dp->d_name);	
		current_room=fopen(file_location,"r");	
		
		j=0;
		//Read the file line by line
		while (getline(&line,&line_size,current_room) != -1) {

			//Seperate the data from the newest line
			sscanf(line,"%s %s %s",buf1,buf2,buf3);

			//If the line contains the name, write to the map name
			if(strcmp(buf2,"NAME:")==0) {	
				strcpy(map[i].name,buf3);					
			} 
			//If the line contains the room type, write to room type as int
			else if(strcmp(buf2,"TYPE:")==0) {

				// -1 = Start Room
				//  0 = Mid Room
				//  1 = End Room
				if(strcmp(buf3,"START_ROOM")==0) {
					map[i].room_ID=-1;
				}	else if(strcmp(buf3,"END_ROOM")==0) {
					map[i].room_ID=1;
				} else {
					map[i].room_ID=0;
				}

			} 
			//Else, write the line to an adjacent room
			else {
				strcpy(map[i].adjacent[j],buf3);
				//Itterate to the next spot in the adjacent array
				j++;
			}
		}

		//Close the file, finish writing to the current room
		//itterate to the next room number
		map[i].num=j;
		fclose(current_room);
		i++;
	}
	
}

int main() {

	char *my_folder=find_folder();
	int idx,end;
	struct room_link *head=NULL;
	int link_ct=0,next_idx;

	struct room map[NUM_ROOMS];
	new_rooms(my_folder,map);

	//Set the starting and ending index
	idx=find_start(map);
	end=find_end(map);

	//This is for testing 
	if (idx==-1) {
		printf("Why isn't this working\n");
		return 1;
	} else {

		//Test case for the end of the game
		while(idx != end) {

			//Print out where the player is
			//and ask there where they want to go next
			where_are_you(map[idx]);
			printf("Where to? >");
			next_idx = get_input(map,idx);	

			//If the player is moving to a new room,
			//update the linked list and move forward
			if (next_idx != idx) {
				idx=next_idx;
				new_link(&head,map[idx].name);
				link_ct++;
				printf("\n");
			}
		}
	}

	//Print the steps taken and deallocate the memory
	printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n",link_ct);
	rm_links(&head);

	return 0;

}
