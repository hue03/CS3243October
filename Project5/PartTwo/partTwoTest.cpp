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
#define PROCESS_COUNT 2	// useful when debugging to limit # of procs
#define MIN_DEATH_INTERVAL 20
#define MAX_DEATH_INTERVAL 300
#define MAX_FRAMES 280
#define MAX_PAGES 720
#define MAX_NUM_PAGES_PER_PROCESS 20
#define DEFAULT_NUM_PAGES_PER_PROCESS 10
#define MAX_SUBROUTINES 5
#define MIN_SUBROUTINES 1
#define SHIFT_INTERVAL 10
#define PRINT_INTERVAL 500	// # of cpu quanta between memory map printouts
#define MAX_QUANTA 50000	// # quanta to run before ending simulation
#define SLEEP_LENGTH 250000
#define SEED time(NULL)
using namespace std;
 
struct Page
{
	short suffix;
	short refByte;
	bool valid;
	short frameNum;
	
	//Page();
	//Page(short suffix, short refByte, bool valid, short frameNum); //need help on the struct creation
	Page():suffix(-1), refByte(-1), valid(false), frameNum(-1){}
	Page(short s, short r, bool v, short f):suffix(s), refByte(r), valid(v), frameNum(f){}	
};

struct Process
{
	char name;
	int lifeTime;
	Page* pageTable[MAX_NUM_PAGES_PER_PROCESS]; //this is giving a problem. i forget if you can assign an array to array. gives error saying Page* cannot go into Page* [20] don't know why the [20] is there
	Process(char n, int l, Page* p[MAX_NUM_PAGES_PER_PROCESS]):name(n), lifeTime(l){ //need help on the struct creation
		for (int i = 0; i < MAX_NUM_PAGES_PER_PROCESS; i++)
		{
			pageTable[i] = p[i];
		}
		
	};
};

Page mainMemory[MAX_FRAMES];
vector<Process> vectOfProcesses;
vector<int> freeFrames;
deque<Page> backingStore;
Page myPage;
int usedFrames;
int loadPages;
int loadedProc;
int refBitSet;
int refBitClear;

void zeroFillMemory(int, int);
void findFreeFrames(void);
void createProcesses(void);
int main()
{
	srand(SEED);
	zeroFillMemory(0, MAX_FRAMES);
	findFreeFrames();
	createProcesses();
	cout << "--------------------------------------------------------------------------------" << endl;
	cout << "List of Free Frames:" << endl;
	for (uint i = 0; i < freeFrames.size(); i++)
	{
		cout << "Frame " << freeFrames[i] << endl;
	}
	cout << "--------------------------------------------------------------------------------" << endl;
	
	//createProcesses();
	cout << vectOfProcesses.size() << endl;
	for (uint h = 0; h < vectOfProcesses.size(); h++)
	{
		cout << "Process: " << vectOfProcesses[h].name << " LifeTime: " << vectOfProcesses[0].lifeTime << endl;
		for (int i = 0; i < MAX_NUM_PAGES_PER_PROCESS; i++)
		{
			cout << "Suffix: " << vectOfProcesses[h].pageTable[i]->suffix << endl;
			cout << "Ref: " << vectOfProcesses[h].pageTable[i]->refByte << endl;
			cout << "Valid: " << vectOfProcesses[h].pageTable[i]->valid << endl;
			cout << "Frame: " << vectOfProcesses[h].pageTable[i]->frameNum << endl;
			cout << "--------------------------------------------------------------------------------" << endl;
		}
	}
}

void zeroFillMemory(int start, int size)
{
	for (int i = 0; i < size; i++)
	{
		myPage.frameNum = i;
		mainMemory[start + i] = myPage;
	}
}

void findFreeFrames(void)
{
	for (int i = 0; i < MAX_FRAMES; i++)
	{
		if (mainMemory[i].suffix == -1)
		{
			freeFrames.push_back(i);
		}
	}
}

void createProcesses(void)
{
	int lifeRange = MAX_DEATH_INTERVAL - MIN_DEATH_INTERVAL + 1;
	char name = '?';
	for (int i = 0; i < PROCESS_COUNT; i++)
	{
		switch(name)
		{
		case '@':
			name = '1';
			break;
		case '9':
			name = 'A';
			break;
		case 'Z':
			name = 'a';
			break;
		case 'H':
		case 'k':
			name += 2;
			break;
		default:
			name +=1;
			break;
		}
		//need to make a case for kernel initialization
		int timeOfLife = rand() % lifeRange + MIN_DEATH_INTERVAL; //not randomizing time between process creation
		Page* tempTable[MAX_NUM_PAGES_PER_PROCESS];
		int numSubRoutine = rand() % (MAX_SUBROUTINES - MIN_SUBROUTINES + 1) + MIN_SUBROUTINES;
		cout << "Num of Sub Routines " <<  numSubRoutine << endl;
		int j;
		for (j = 0; j < (DEFAULT_NUM_PAGES_PER_PROCESS + numSubRoutine * 2); j++)
		{
			//cout << "creating process loop" << endl;
			if (j < 2)
			{
				Page tempPage = Page(0, 0, false, -1);
				backingStore.push_back(tempPage);
				tempTable[j] = &backingStore[j];
			}
			else if (j < 5)
			{
				Page tempPage = Page(1, 0, false, -1);
				backingStore.push_back(tempPage);
				tempTable[j] = &backingStore[j];
			}
			else if (j < 10)
			{
				Page tempPage = Page(2, 0, false, -1);
				backingStore.push_back(tempPage);
				tempTable[j] = &backingStore[j];
			}
			else if ((j % 10) < 2)
			{
				Page tempPage = Page(3, 0, false, -1);
				backingStore.push_back(tempPage);
				tempTable[j] = &backingStore[j];
			}
			else if ((j % 10) < 4)
			{
				Page tempPage = Page(4, 0, false, -1);
				backingStore.push_back(tempPage);
				tempTable[j] = &backingStore[j];
			}
			else if ((j % 10) < 6)
			{
				Page tempPage = Page(5, 0, false, -1);
				backingStore.push_back(tempPage);
				tempTable[j] = &backingStore[j];
			}
			else if ((j % 10) < 8)
			{
				Page tempPage = Page(6, 0, false, -1);
				backingStore.push_back(tempPage);
				tempTable[j] = &backingStore[j];
			}
			else if ((j % 10) < 10)
			{
				Page tempPage = Page(7, 0, false, -1);
				backingStore.push_back(tempPage);
				tempTable[j] = &backingStore[j];
			}
		}
		if (numSubRoutine != 5)//can make this better. need a way to initialize page table without this.
		{
			numSubRoutine = (MAX_SUBROUTINES - numSubRoutine) * 2;
			cout <<"Fill empty pages: " << j << " " << numSubRoutine << endl; //filling the temp page table with "empty" pages because of seg fault and garbage data
			for (int k = j; k < j + numSubRoutine; k++)
			{
				myPage.frameNum = -1;
				myPage.refByte = 0;
				tempTable[k] = &myPage;
			}
		}
		Process process = Process(name, timeOfLife, tempTable);
		vectOfProcesses.push_back(process);
	}
}
