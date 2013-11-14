// CS3243 Operating Systems
// Fall 2013
// Project 5: Memory Swapping and Paging, Part 2
// Steven Ng and Hue Moua
// Date: 11/13/2013
// File: parttwo.cpp

#include <cstdlib>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <deque>  

#define MAX_PROCESSES 52	// This will not ever change
#define PROCESS_COUNT 23	// useful when debugging to limit # of procs
#define MIN_DEATH_INTERVAL 20
#define MAX_DEATH_INTERVAL 300
#define MAX_FRAMES 280
#define MAX_PAGES 720
#define SHIFT_INTERVAL 10
#define PRINT_INTERVAL 500	// # of cpu quanta between memory map printouts
#define MAX_QUANTA 50000	// # quanta to run before ending simulation
#define SLEEP_LENGTH
using namespace std;
 
struct Page
{
	short suffix;
	short refByte;
	bool valid;
	short frameNum;	
};

struct Process
{
	char name;
	int lifeTime;
	Page* pageTable[20];
};

Page mainMemory[MAX_FRAMES];
vector<Process> vectOfProcesses;
vector<int> freeFrames;
deque<Page> backingStore;
int usedFrames;
int loadPages;
int loadedProc;
int refBitSet;
int refBitClear;

int main()
{
	return 0;
}
