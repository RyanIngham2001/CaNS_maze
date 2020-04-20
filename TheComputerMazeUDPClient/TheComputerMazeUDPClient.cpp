// TheComputerMazeUDPClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <winsock2.h>
#include <time.h>
#include <conio.h>

#pragma comment(lib, "wsock32.lib")

#define STUDENT_NUMBER		"19003311"
#define STUDENT_FIRSTNAME	"Ryan"
#define STUDENT_FAMILYNAME	"Ingham"

#define IP_ADDRESS_SERVER	"127.0.0.1"
//#define IP_ADDRESS_SERVER "164.11.80.69"


#define PORT_SERVER 0x1984 // We define a port that we are going to use.
#define PORT_CLIENT 0x1985 // We define a port that we are going to use.

#define MAX_FILENAME_SIZE 500

#define MAX_BUFFER_SIZE   5000
#define MAX_STRING_SIZE   200
#define MAX_NO_TOKENS     50

#define MAX_ITEMS_IN_ROOM		20
#define MAX_ITEMS_IN_BACKPACK	50

#define OPTION_MOVE_NORTH 1
#define OPTION_MOVE_SOUTH 2
#define OPTION_MOVE_EAST 3
#define OPTION_MOVE_WEST 4
#define OPTION_MOVE_UP 5
#define OPTION_MOVE_DOWN 6

#define OPTION_UNLOCK_NORTH 7
#define OPTION_UNLOCK_SOUTH 8
#define OPTION_UNLOCK_EAST 9 
#define OPTION_UNLOCK_WEST 10
#define OPTION_UNLOCK_UP 11
#define OPTION_UNLOCK_DOWN 12

#define OPTION_BASE_FOR_READS 200
#define OPTION_BASE_FOR_PICKUPS 500
#define OPTION_BASE_FOR_DROPS 800
#define OPTION_BASE_FOR_DOS 1100
#define OPTION_BASE_FOR_EVENTS 1300

enum directions
{
	DIRECTION_NORTH = 0,
	DIRECTION_SOUTH = 1,
	DIRECTION_EAST = 2,
	DIRECTION_WEST = 3,
	DIRECTION_UP = 4,
	DIRECTION_DOWN = 5
};


enum direction_possible
{
	DIRECTION_NOT_PRESENT = -1,
	DIRECTION_LOCKED = 0,
	DIRECTION_OPEN = 1
};


enum item_types
{
	ITEM_NONE = 0,
	ITEM_BOOK = 1,
	ITEM_JUNK = 2,
	ITEM_EQUIPMENT = 3,
	ITEM_MANUSCRIPT = 4,
	ITEM_TEST = 5,
	ITEM_OTHER = 10
};



struct Item
{
	int  number;
	int  value;
	int  volume;
};


struct Student
{
	int level;
	int rooms_visited;
	int doors_openned;
	int number_of_moves;
	int score;
};


struct Room
{
	char name[5];
	int  type;
	int  direction[6];
	int  number_of_keys;
	int  keys[4];
	int  number_of_items;
	Item items[MAX_ITEMS_IN_ROOM];
};


struct Backpack
{
	int number_of_items;
	Item items[MAX_ITEMS_IN_BACKPACK];
};




#define MAX_OPTIONS	50

int number_of_options;
int options[MAX_OPTIONS];


Student student;
Room room;
Backpack backpack;



SOCKADDR_IN server_addr;
SOCKADDR_IN client_addr;

SOCKET sock;  // This is our socket, it is the handle to the IO address to read/write packets

WSADATA data;




char InputBuffer[MAX_BUFFER_SIZE];

char Tokens[MAX_NO_TOKENS][MAX_STRING_SIZE];

char text_student[1000];
char text_backpack[1000];
char text_room[1000];
char text_keys[1000];
char text_items[1000];
char text_options[1000];



void sentOption(int option, int key)
{
	char buffer[100];

	sprintf(buffer, "Option %d, %x", option, key);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR*)&server_addr, sizeof(SOCKADDR));
}


/*************************************************************/
/********* Your tactics code starts here *********************/
/*************************************************************/
#define KNOWN_KEYS 36
#define NO_MOVE -1
#define NUMBER_OF_WORKING_KEYS 56
#define GOOD_EVENTS 24
int numOfKeys = 36;

int goodEvents[GOOD_EVENTS] = {
	1359,1324,1302,1301,1333,1346,1308,1348,1336,1332,1322,1319,1321,1343,
	1350,1303,1303,1305,1305,1305,1306,1306,1306,1306
};

int backpackVolume = 0; //volume of our backpack

int known_keys[KNOWN_KEYS] = { 0x200F, 0xBCD5, 0xE4A8, 0x71DA, 0x29FC, 0xA100, 0xBE27, 0x9F4A, 0x8E28, 0xD75B, 0x090D, 0x0172, 0xD2F7, 0xC567, 0x8FD4, 0xFD5A,  0xD976, 0x8F32, 0x1AE1, 0x43AB, 0x3B8C, 0x1696, 0x5DC2, 0x3629, 0xD976, 0x3B8C, 0xFF24, 0x1696, 0x5DC2, 0x3629, 0x12FC, 0x7F07, 0xAB62, 0x8F32, 0x1AE1, 0x43AB };

struct WorkingKeys
{
	char room_name[5];
	int  direction;
	int key;
};


WorkingKeys working_keys[NUMBER_OF_WORKING_KEYS] =
{
	{"0Y04",1,0x200F},
	{"0Y04",2,0x0172},
	{"0Y24",3,0xC567},
	{"0Y12",0,0xFD5A},
	{"0Y02",2,0x9F4A},
	{"0Y01",3,0x8E28},
	{"0Y16",2,0x200F},
	{"0Y07",2,0xA100},
	{"0Y08",1,0x71DA},
	{"0Y28",1,0x29FC},
	{"0Y38",2,0x29FC},
	{"0Y48",1,0xC567},
	{"0Y60",2,0x9F4A},
	{"0Y50",0,0x090D},
	{"0Y29",1,0xD2F7},
	{"0Y61",1,0x200F},
	{"1Y11",1,0xC4B6},
	{"1Y21",3,0x29FC},
	{"1Y31",1,0x29FC},
	{"1Y51",1,0xE4A8},
	{"1Y71",1,0x8FD4},
	{"1Y81",1,0x200F},
	{"1Y81",3,0x8E28},
	{"2Y01",3,0xBCD5},
	{"2Y02",2,0x9F4A},
	{"2Y12",2,0xE4A8},
	{"2Y13",1,0xC4B6},
	{"2Y04",2,0xA100},
	{"2Y14",1,0x0172},
	{"2Y14",2,0xD75B},
	{"2Y35",1,0x29FC},
	{"2Y26",0,0x29FC},
	{"1Y96",0,0x200F},
	{"1Y96",2,0x29FC},
	{"1Y86",0,0xA100},
	{"1Y76",2,0x0172},
	{"1Y66",3,0xFD5A},
	{"1Y67",0,0x090D},
	{"1Y57",2,0x0172},
	{"1Y69",1,0xE4A8},
	{"1Y59",0,0x8FD4},
	{"1Y39",0,0xE4A8},
	{"0Y98",3,0xC4B6},
	{"0Y88",2,0x8E28},
	{"0Y78",2,0xBE27},
	{"2Y80",2,0x8FD4},
	{"2Y70",0,0xBE27},
	{"2Y50",2,0xBE27},
	{"2Y51",1,0x29FC},
	{"2Y31",0,0x8E28},
	{"2Y31",2,0x090D},
	{"2Y21",3,0x29FC},
	{"2Y22",2,0x8FD4},
	{"2Y19",0,0xD75B},
	{"3Y18",3,0x8FD4},
	{"3Y27",2,0x090D}
};

int option_count = 0;
int previousOption = -1; // save previous option
int myLevel = 0; // current student level
char room_name[10] = " ";

int rooms_visited[5][10][10];
int doors_tried[5][10][10][4];
int keys_tried[5][10][10][4];

int use_key;

int try_key = -1;


// TRIED TO IMPLEMENT THIS SYSTEM. DIDN'T WORK //
FILE* fp;
void saveDirection(char* direction) {
	fp = fopen("moves.txt", "a");

	fprintf(fp, "MOVE :%s\n", direction);
	fclose(fp);
}

void saveMovement(int move, int hexKey) {
	fp = fopen("movement.txt", "a");
	fprintf(fp, "move %d use %x at %s\n", move, hexKey, room.name);
	fclose(fp);
}

void saveKeys(int hexKey) {
	fp = fopen("keys.txt", "a");
	fprintf(fp, "0x%x,\n", hexKey);
	fclose(fp);
}

void saveWorkingKeys(int hexKey, int floor, int ns, int ew, int direction) {
	fp = fopen("WorkingKeys.txt", "a");
	fprintf(fp, "{\"%dY%d%d\",%d,0x%x},\n", floor, ns, ew, direction, hexKey);
	fclose(fp);
}

void saveBooks(char* word, int level) {
	fp = fopen("BOOK.txt", "a");
	fprintf(fp, "%s %d\n", word, level);
	fclose(fp);
}

void saveEvents(char* word, int level, int num) {
	fp = fopen("EVENT.txt", "a");
	fprintf(fp, "%s %d level:%d\n", word, num, level);
	fclose(fp);
}

void saveKeyDirection(int hexKey, int direction) {
	fp = fopen("Working.txt", "a");
	fprintf(fp, "Using %x to unlock %d at %s\n", hexKey, direction, room.name);
	fclose(fp);
}

void saveKeyDirection(int hexKey, int direction, char* wrong) {
	fp = fopen("Working.txt", "a");
	fprintf(fp, "Using %x to unlock %d at %s %s\n", hexKey, direction, room.name, wrong);
	fclose(fp);
}


void initRooms()
{
	int floor;
	int ns;
	int ew;
	int door;

	for (floor = 0; floor < 5; floor++)
	{
		for (ns = 0; ns < 10; ns++)
		{
			for (ew = 0; ew < 10; ew++)
			{
				rooms_visited[floor][ns][ew] = 0;

				for (door = 0; door < 4; door++)
				{
					doors_tried[floor][ns][ew][door] = -1;
					keys_tried[floor][ns][ew][door] = 0;
				}
			}
		}
	}
}


int bestDirection()
{
	int best_direction = -1;
	int room_visits = 20000;
	int floor;
	int ns;
	int ew;

	if (sscanf(room.name, "%1dY%1d%1d", &floor, &ns, &ew) == 3)
	{
		if (room.direction[DIRECTION_NORTH] == DIRECTION_OPEN)
		{
			if (rooms_visited[floor][ns - 1][ew] < room_visits)
			{
				room_visits = rooms_visited[floor][ns - 1][ew];
				best_direction = OPTION_MOVE_NORTH;
			}
		}

		if (room.direction[DIRECTION_SOUTH] == DIRECTION_OPEN)
		{
			if (rooms_visited[floor][ns + 1][ew] < room_visits)
			{
				room_visits = rooms_visited[floor][ns + 1][ew];
				best_direction = OPTION_MOVE_SOUTH;
			}
		}

		if (room.direction[DIRECTION_EAST] == DIRECTION_OPEN)
		{
			if (rooms_visited[floor][ns][ew + 1] < room_visits)
			{
				room_visits = rooms_visited[floor][ns][ew + 1];
				best_direction = OPTION_MOVE_EAST;
			}
		}

		if (room.direction[DIRECTION_WEST] == DIRECTION_OPEN)
		{
			if (rooms_visited[floor][ns][ew - 1] < room_visits)
			{
				room_visits = rooms_visited[floor][ns][ew - 1];
				best_direction = OPTION_MOVE_WEST;
			}
		}

		if ((room.direction[DIRECTION_UP] == DIRECTION_OPEN) && (floor != 3))
		{
			if (rooms_visited[floor + 1][ns][ew] < room_visits)
			{
				room_visits = rooms_visited[floor + 1][ns][ew];
				best_direction = OPTION_MOVE_UP;
			}
		}

		if (room.direction[DIRECTION_DOWN] == DIRECTION_OPEN)
		{
			if (rooms_visited[floor - 1][ns][ew] < room_visits)
			{
				room_visits = rooms_visited[floor - 1][ns][ew];
				best_direction = OPTION_MOVE_DOWN;
			}
		}
	}

	return best_direction;
}



int unlockDoor(int* use_key)
{
	int door;
	int move = -1;
	int key;
	int floor;
	int ns;
	int ew;

	if (sscanf(room.name, "%1dY%1d%1d", &floor, &ns, &ew) == 3)
	{
		for (door = 0; door < 4; door++)
		{
			if (room.direction[door] == DIRECTION_LOCKED)
			{
				doors_tried[floor][ns][ew][door]++;
				key = doors_tried[floor][ns][ew][door];
				if (key < KNOWN_KEYS)
				{
					move = OPTION_UNLOCK_NORTH + door;
					*use_key = known_keys[key];
					printf("Move = %d, key = 0x%4X\n", move, known_keys[key]);
					keys_tried[floor][ns][ew][door] = known_keys[key];
					return move;
				}
			}
		}
	}

	return move;
}


void saveKeys()
{
	int floor;
	int ns;
	int ew;
	int door;

	for (floor = 0; floor < 5; floor++)
	{
		for (ns = 0; ns < 10; ns++)
		{
			for (ew = 0; ew < 10; ew++)
			{
				for (door = 0; door < 4; door++)
				{
					//					if ((keys_tried[floor][ns][ew][door] > 0) && (room.direction[door] == DIRECTION_OPEN))
					if ((doors_tried[floor][ns][ew][door] >= 0) && (doors_tried[floor][ns][ew][door] < numOfKeys) && (room.direction[door] == DIRECTION_OPEN))
					{
						printf("Room %dY%d%d Door %d Key 0x%4X\n", floor, ns, ew, door, keys_tried[floor][ns][ew][door]);
						saveWorkingKeys(keys_tried[floor][ns][ew][door], floor, ns, ew, door);
					}
				}
			}
		}
	}
	getchar();
	getchar();
	getchar();
	getchar();
}
int getRandomMove()
{
	int move;

	if (strcmp(room_name, room.name) != 0) option_count = 0;
	move = options[option_count];
	option_count = (option_count + 1) % number_of_options;

	return move;
}


int itemValue(int item) {
	int return_value = -1;

	for (int i = 0; i < room.number_of_items; i++) {
		if (item == room.items[i].number) {
			return_value = room.items[i].value;
		}
	}

	return return_value;
}







int pickupStuff()
{
	int move = NO_MOVE;
	int i;
	int option;
	int item;

	for (i = 0; i < number_of_options; i++)
	{
		option = options[i];

		if ((option >= OPTION_BASE_FOR_PICKUPS) && (option < OPTION_BASE_FOR_DROPS))  // then this is a valid pickup
		{
			item = option - OPTION_BASE_FOR_PICKUPS;

			if (itemValue(item) > 0)  // check that it has value
			{
				move = option;
				return move;
			}
		}
	}

	return move;
}


int attendEvent()
{
	int move = -1; // default is no event found
	int i;
	int option;

	for (i = 0; i < number_of_options; i++)
	{
		option = options[i];
		if (option >= OPTION_BASE_FOR_EVENTS)  // then this is a valid event
		{
			for (int j = 0; j < GOOD_EVENTS; j++) {
				if (goodEvents[j] == option) {
					if (student.level > myLevel) {
						//saveEvents("Event I enter benefit:", student.level, lastOption);
					}
					else {
						//saveEvents("Event I enter loss:", student.level, lastOption);
					}
					previousOption = option;
					myLevel = student.level;
					move = option;
					return move;
				}
			}
		}
	}
	return move;
}

int readBook() {
	int move = NO_MOVE;
	int i;
	int option;
	int item;

	for (i = 0; i < number_of_options; i++) {
		option = options[i];
		if ((option >= OPTION_BASE_FOR_READS) && (option < OPTION_BASE_FOR_PICKUPS))  //then this is a valid event
		{
			item = option - OPTION_BASE_FOR_READS;

			if (itemValue(item) > 0) {
				move = option;
				return move;
			}
		}
	}

	return move;
}

bool inPickUpList(int item_number) {
	bool rc = false;
	int i;

	for (i = 0; i < number_of_options; i++) {
		if (options[i] == (OPTION_BASE_FOR_PICKUPS + item_number)) {
			return true;
		}
	}

	return rc;
}

int checkItem() {
	int best_found_item = 0;
	int best_found_value = -1;
	int best_found_volume = 1;
	float best_found_ratio = -1.0;

	int worst_backpack_item = 0;
	int worst_backpack_value = 1000;
	int worst_backpack_volume = 1;
	float worst_backpack_ratio = 1000.0;

	int i;
	int j;
	int move = NO_MOVE; //default no move

	//are there any items in the room?
	if (room.number_of_items > 0) {
		//loop through the items
		for (i = 0; i < room.number_of_items; i++) {
			if (best_found_ratio < (float)room.items[i].value / (float)room.items[i].volume) {
				best_found_item = room.items[i].number;
				best_found_value = room.items[i].value;
				best_found_volume = room.items[i].volume;
				best_found_ratio = (float)best_found_value / (float)best_found_volume;
			}
		}

		//did we find a positive value
		if (best_found_value > 0) {
			if (inPickUpList(best_found_item)) {
				//we can pick up an item because our worst_backpack isn't full
				return OPTION_BASE_FOR_PICKUPS + best_found_item;
			}
			else {
				if (backpack.number_of_items > 0) {
					for (j = 0; j < backpack.number_of_items; j++) {
						if (worst_backpack_ratio > (float)room.items[i].value / (float)room.items[i].volume) {
							worst_backpack_item = backpack.items[i].number;
							worst_backpack_value = backpack.items[i].value;
							worst_backpack_volume = backpack.items[i].volume;
							worst_backpack_ratio = (float)backpack.items[i].value / (float)backpack.items[i].volume;
						}
					}

					if (worst_backpack_ratio < best_found_ratio) {
						return(OPTION_BASE_FOR_DROPS + worst_backpack_item);
					}
				}
			}
		}
	}

	return move;
}

int doAction() {
	int move = NO_MOVE; // default is no dos found
	int i;
	int option;

	for (i = 0; i < number_of_options; i++) {
		option = options[i];
		if (option >= OPTION_BASE_FOR_DOS)  // then this is a dos
		{
			move = option;
			return move;
		}
	}
	return move;
}


int useWorkingKeys(int* use_key)
{
	int move = NO_MOVE;  // default is no key found
	int i;
	int door;

	for (i = 0; i < NUMBER_OF_WORKING_KEYS; i++)
	{
		door = working_keys[i].direction;

		if (DIRECTION_LOCKED == room.direction[door])
		{
			if (strcmp(room.name, working_keys[i].room_name) == 0)
			{
				*use_key = working_keys[i].key;
				move = OPTION_UNLOCK_NORTH + door;
				return move;
			}
		}
	}

	return move;
}



void yourMove()
{
	int move = -1;  // no valid move assigned yet
	int i;
	char chr;
	int floor;
	int ns;
	int ew;

	if (sscanf(room.name, "%1dY%1d%1d", &floor, &ns, &ew) == 3)
	{
		rooms_visited[floor][ns][ew]++;  // record where you have been
	}



	if (move == -1) move = doAction();
	if (move == -1) move = attendEvent();
	if (move == -1) move = readBook();
	if (move == -1) move = pickupStuff();
	if (move == NO_MOVE) move = useWorkingKeys(&use_key); // Didn't pick anything up, so try unlocking a door
	//if (move == -1) move = unlockDoor(&use_key); // Didn't pick anything up, so try unlocking a door
	if (move == -1) move = bestDirection();      // it didn't try to unlock a door, so move in best direction
	if (move == -1) move = getRandomMove();      // always returns a valid move
	if (room.name == "0Y04") move = 9;

	sentOption(move, use_key);
	printf("Move = %d\n", move);



	if (_kbhit())
	{
		chr = getchar();
		if (chr == 's')
		{
			saveKeys();
		}
	}
}



/*************************************************************/
/********* Your tactics code ends here ***********************/
/*************************************************************/



int getTokens(char* instring, char seperator)
{
	int  number_of_tokens;
	char chr;
	int  state;
	int  i;
	int  j;


	for (i = 0; i < MAX_NO_TOKENS; i++)
	{
		for (j = 0; j < MAX_STRING_SIZE; j++)
		{
			Tokens[i][j] = '\0';
		}
	}

	number_of_tokens = -1;
	chr = instring[0];
	state = 0;
	i = 0;

	while (chr != '\0')
	{
		switch (state)
		{
		case 0:  // Initial state
			if (chr == seperator)
			{
				number_of_tokens++;
				state = 1;
			}
			else if ((chr == ' ') || (chr == '\t') || (chr == '\n'))
			{
				state = 1;
			}
			else
			{
				number_of_tokens++;
				j = 0;
				Tokens[number_of_tokens][j] = chr;
				Tokens[number_of_tokens][j + 1] = '\0';
				state = 2;
			}
			break;

		case 1:  // Leading white space
			if (chr == seperator)
			{
				number_of_tokens++;
				state = 1;
			}
			else if ((chr == ' ') || (chr == '\t') || (chr == '\n'))
			{
				state = 1;
			}
			else
			{
				number_of_tokens++;
				j = 0;
				Tokens[number_of_tokens][j] = chr;
				Tokens[number_of_tokens][j + 1] = '\0';
				state = 2;
			}
			break;

		case 2:  // Collecting chars
			if (chr == seperator)
			{
				//number_of_tokens++;
				state = 1;
			}
			else
			{
				j++;
				Tokens[number_of_tokens][j] = chr;
				Tokens[number_of_tokens][j + 1] = '\0';
				state = 2;
			}
			break;

		default:
			break;
		}

		i++;
		chr = instring[i];
	}

	return (number_of_tokens + 1);
}



bool getline(FILE* fp, char* buffer)
{
	bool rc;
	bool collect;
	char c;
	int  i;

	rc = false;
	collect = true;

	i = 0;
	while (collect)
	{
		c = getc(fp);

		switch (c)
		{
		case EOF:
			if (i > 0)
			{
				rc = true;
			}
			collect = false;
			break;

		case '\n':
			if (i > 0)
			{
				rc = true;
				collect = false;
				buffer[i] = '\0';
			}
			break;

		default:
			buffer[i] = c;
			i++;
			break;
		}
	}

	return (rc);
}



void printRoom()
{
	int i;

	printf("Room\n");
	printf("Room = %s, Room type = %d\n", room.name, room.type);

	printf("Directions = ");
	for (i = 0; i < 6; i++)
	{
		printf("%d  ", room.direction[i]);
	}
	printf("\n");

	if (room.number_of_keys > 0)
	{
		printf("Keys = ");
		for (i = 0; i < room.number_of_keys; i++)
		{
			printf("0x%X  ", room.keys[i]);
		}
		printf("\n");
	}
	else
	{
		printf("No keys in this room\n");
	}

	if (room.number_of_items > 0)
	{
		for (i = 0; i < room.number_of_items; i++)
		{
			printf("Item=%d, Value=%d, Volume=%d\n", room.items[i].number, room.items[i].value, room.items[i].volume);
		}
	}
	else
	{
		printf("No items in this room\n");
	}

	printf("\n");
}


void printStudent()
{
	printf("Student\n");
	printf("Level=%d,  Number of rooms visited = %d,  Number of doors openned = %d,  Number of moves = %d,  Score = %d\n", student.level, student.rooms_visited, student.doors_openned, student.number_of_moves, student.score);
	printf("\n");
}


void printBackpack()
{
	int i;

	printf("Backpack\n");

	if (backpack.number_of_items > 0)
	{
		for (i = 0; i < backpack.number_of_items; i++)
		{
			printf("Item=%d, Value=%d, Volume=%d\n", backpack.items[i].number, backpack.items[i].value, backpack.items[i].volume);
		}
	}
	else
	{
		printf("Your backpack is empty\n");
	}
	printf("\n");
}


void printOptions()
{
	int i;

	printf("Options\n");
	printf("Options = ");
	for (i = 0; i < number_of_options; i++)
	{
		printf("%d  ", options[i]);
	}
	printf("\n");
	printf("\n");
}




void communicate_with_server()
{
	char buffer[4096];
	int  len = sizeof(SOCKADDR);
	int  i;
	char* p;
	int	 number_of_tokens;


	sprintf_s(buffer, "Register  %s %s %s", STUDENT_NUMBER, STUDENT_FIRSTNAME, STUDENT_FAMILYNAME);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR*)&server_addr, sizeof(SOCKADDR));

	while (true)
	{
		memset(buffer, '\0', sizeof(buffer));

		if (recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (SOCKADDR*)&client_addr, &len) != SOCKET_ERROR)
		{
			p = ::inet_ntoa(client_addr.sin_addr);

			if ((strcmp(IP_ADDRESS_SERVER, "127.0.0.1") == 0) || (strcmp(IP_ADDRESS_SERVER, p) == 0))
			{
				printf("%s\n\n", buffer);

				number_of_tokens = getTokens(buffer, '|');

				if (number_of_tokens == 6)
				{
					strcpy(text_student, Tokens[0]);
					strcpy(text_backpack, Tokens[1]);
					strcpy(text_room, Tokens[2]);
					strcpy(text_keys, Tokens[3]);
					strcpy(text_items, Tokens[4]);
					strcpy(text_options, Tokens[5]);

					printf("Student  = '%s'\n", text_student);
					printf("Backpack = '%s'\n", text_backpack);
					printf("Room     = '%s'\n", text_room);
					printf("Keys     = '%s'\n", text_keys);
					printf("Items    = '%s'\n", text_items);
					printf("Options  = '%s'\n", text_options);

					student.level = -1;
					student.rooms_visited = -1;
					student.doors_openned = -1;
					student.number_of_moves = -1;

					if (sscanf(text_student, "%d,%d,%d,%d,%d", &student.level, &student.rooms_visited, &student.doors_openned, &student.number_of_moves, &student.score) == 5)
					{
					}

					if (strlen(text_backpack) > 0)
					{
						backpack.number_of_items = getTokens(text_backpack, '&');

						if (backpack.number_of_items > 0)
						{
							for (i = 0; i < backpack.number_of_items; i++)
							{
								if (i < MAX_ITEMS_IN_BACKPACK)
								{
									backpack.items[i].number = -1;

									if (sscanf(Tokens[i], "%d, %d, %d", &backpack.items[i].number, &backpack.items[i].value, &backpack.items[i].volume) == 3)
									{
									}
								}
							}
						}
					}
					else
					{
						backpack.number_of_items = 0;
					}

					sscanf(text_room, "%s ,%d, %d, %d, %d, %d, %d, %d", &room.name, &room.type, &room.direction[DIRECTION_NORTH], &room.direction[DIRECTION_SOUTH], &room.direction[DIRECTION_EAST], &room.direction[DIRECTION_WEST], &room.direction[DIRECTION_UP], &room.direction[DIRECTION_DOWN]);

					if (strlen(text_keys) > 0)
					{
						room.number_of_keys = getTokens(text_keys, '&');

						if (room.number_of_keys > 0)
						{
							for (i = 0; i < room.number_of_keys; i++)
							{
								if (i < 4)
								{
									room.keys[i] = -1;

									if (sscanf(Tokens[i], "%x", &room.keys[i]) == 1)
									{
									}
								}
							}
						}
					}
					else
					{
						room.number_of_keys = 0;
					}

					if (strlen(text_items) > 0)
					{
						room.number_of_items = getTokens(text_items, '&');

						if (room.number_of_items > 0)
						{
							for (i = 0; i < room.number_of_items; i++)
							{
								if (i < MAX_ITEMS_IN_ROOM)
								{
									room.items[i].number = -1;

									if (sscanf(Tokens[i], "%d, %d, %d", &room.items[i].number, &room.items[i].value, &room.items[i].volume) == 3)
									{
									}
								}
							}
						}
					}
					else
					{
						room.number_of_items = 0;
					}

					if (strlen(text_options) > 0)
					{
						number_of_options = getTokens(text_options, ',');

						if (number_of_options > 0)
						{
							for (i = 0; i < number_of_options; i++)
							{
								if (i < MAX_OPTIONS)
								{
									options[i] = -1;

									if (sscanf(Tokens[i], "%d", &options[i]) == 1)
									{
									}
								}
							}
						}
					}
					else
					{
						number_of_options = 0;
					}
				}

				printStudent();
				printBackpack();
				printRoom();
				printOptions();

				//system("timeout /t 60");

				yourMove();
			}
		}
		else
		{
			printf_s("recvfrom error = %d\n", WSAGetLastError());
		}
	}

	printf_s("Student %s\n", STUDENT_NUMBER);
}




int main()
{
	char chr = '\0';

	printf("\n");
	printf("The Computer Maze Student Program\n");
	printf("UWE Computer and Network Systems Assignment 2 \n");
	printf("\n");

	initRooms();

	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return(0);

	//sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	//if (!sock)
	//{	
	//	printf("Socket creation failed!\n"); 
	//}

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	if (!sock)
	{
		// Creation failed! 
	}

	memset(&server_addr, 0, sizeof(SOCKADDR_IN));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS_SERVER);
	server_addr.sin_port = htons(PORT_SERVER);

	memset(&client_addr, 0, sizeof(SOCKADDR_IN));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	client_addr.sin_port = htons(PORT_CLIENT);

	//int ret = bind(sock_recv, (SOCKADDR *)&receive_addr, sizeof(SOCKADDR));
	////	int ret = bind(sock_send, (SOCKADDR *)&receive_addr, sizeof(SOCKADDR));
	//if (ret)
	//{
	//	printf("Bind failed! %d\n", WSAGetLastError());
	//}

	communicate_with_server();

	closesocket(sock);
	WSACleanup();

	while (chr != '\n')
	{
		chr = getchar();
	}

	return 0;
}
