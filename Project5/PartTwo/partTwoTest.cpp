// CS3243 Operating Systems
// Fall 2013
// Project 5: Memory Swapping and Paging, Part 2
// Steven Ng and Hue Moua
// Date: 11/13/2013
// File: parttwo.cpp

#include <cstdlib>
#include <deque>
#include <iostream>
#include <vector>

#define MAX_PROCESSES 52	// This will not ever change
#define PROCESS_COUNT 2	/*23*/	// useful when debugging to limit # of procs
#define MIN_DEATH_INTERVAL 20
#define MAX_DEATH_INTERVAL 300
#define MAX_FRAMES 280
#define MAX_PAGES 720
#define SHIFT_INTERVAL 10
#define PRINT_INTERVAL 500	// # of cpu quanta between memory map printouts
#define MAX_QUANTA 50000	// # quanta to run before ending simulation
#define SLEEP_LENGTH 2500	// Used with the usleep()to slow down sim between
							// cycles (makes reading screen in real-time easier!)

#define MAX_NUM_PAGES_PER_PROCESS 20
#define DEFAULT_NUM_PAGES_PER_PROCESS 10
#define MAX_SUBROUTINES 5
#define MIN_SUBROUTINES 1
//#define SEED 1384543729	// bugged seed time for process page creation
#define SEED time(NULL)
#define EMPTY_PROCESS_NAME 32 //need ascii value. could use chars.

using namespace std;
 
struct Page
{
	short suffix;
	short refByte;
	bool valid;
	short frameNum;
	char processName;
	int start;
	
	//Page();
	//Page(short suffix, short refByte, bool valid, short frameNum); //need help on the struct creation
	Page() : suffix(-1), refByte(-1), valid(false), frameNum(-1), processName(EMPTY_PROCESS_NAME), start(-1) {}
	Page(short suffix, short refByte, bool valid, short frameNum, char processName, int start) : suffix(suffix), refByte(refByte), valid(valid), frameNum(frameNum), processName(processName), start(start){}
};

struct Process
{
	char name;
	int lifeTime;
	int deathTime;
	int subRoutines;
	Page* pageTable[MAX_NUM_PAGES_PER_PROCESS];

	Process(char name, int lifeTime, int deathTime, int subRoutines, Page* p[MAX_NUM_PAGES_PER_PROCESS]) : name(name), lifeTime(lifeTime), deathTime(deathTime), subRoutines(subRoutines)
	{
		for (int i = 0; i < MAX_NUM_PAGES_PER_PROCESS; i++)
		{
			pageTable[i] = p[i];
			cout << p[i]->suffix << " " << endl;	// TODO test output
		}
	}
};

Page* mainMemory[MAX_FRAMES];
vector<Process> vectOfProcesses;
vector<int> freeFrames;
deque<Page> backingStore;
Page myPage;
int runTime;
int usedFrames;
int loadPages;
int loadedProc;
int refBitSet;
int refBitClear;

void zeroFillMemory(int start, int size);
void zeroFillMemory(int start);
void findFreeFrames(void);
void createProcesses(void);
void touchProcess(void);
void fifo(vector<Page*> v, int pid);
void printProcessPageTable(Process p);

int main()
{
	srand(SEED);
	cout << SEED << endl;	// TODO test output
	runTime = 0;
	zeroFillMemory(0, MAX_FRAMES);
	findFreeFrames();
	createProcesses();
	/*cout << "--------------------------------------------------------------------------------" << endl;
	cout << "List of Free Frames:" << endl;
	for (uint i = 0; i < freeFrames.size(); i++)
	{
		cout << "Frame " << freeFrames[i] << endl;
	}
	cout << "--------------------------------------------------------------------------------" << endl;
	*/
	cout << "Num of processes: " << vectOfProcesses.size() << endl;
	touchProcess();
	for (uint h = 0; h < vectOfProcesses.size(); h++)
	{
		cout << "Process: " << vectOfProcesses[h].name << " LifeTime: " << vectOfProcesses[h].lifeTime;
		cout << " Deathtime: " << vectOfProcesses[h].deathTime << " #Subroutines " << vectOfProcesses[h].subRoutines << endl;
		printProcessPageTable(vectOfProcesses[h]);
	}
	for (int i = 0; i < MAX_FRAMES; i++)
	{
		cout << mainMemory[i]->processName << mainMemory[i]->suffix;
	}
}

void zeroFillMemory(int start, int size)
{
	for (int i = 0; i < size; i++)
	{
		myPage.frameNum = i;
		mainMemory[start + i] = &myPage;
	}
}

void zeroFillMemory(int start)
{
	myPage.frameNum = start;
	mainMemory[start] = &myPage;
}

void findFreeFrames(void)
{
	for (int i = MAX_FRAMES - 1; i >= 0; i--)//first frame is last so it pops out first when inserting pages
	{
		if (mainMemory[i]->suffix == -1)
		{
			freeFrames.push_back(i);
		}
	}
}

void createProcesses(void)
{
	int lifeRange = MAX_DEATH_INTERVAL - MIN_DEATH_INTERVAL + 1;
	int subRoutineRange = MAX_SUBROUTINES - MIN_SUBROUTINES + 1;
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
		int timeOfLife = 0;
		int numSubRoutine = 0;
		if (i == 0)
		{
			timeOfLife = MAX_QUANTA;
			numSubRoutine = 5;
		}
		else 
		{
			timeOfLife = rand() % lifeRange + MIN_DEATH_INTERVAL;
			numSubRoutine = rand() % subRoutineRange + MIN_SUBROUTINES;
		}
		Page* tempTable[MAX_NUM_PAGES_PER_PROCESS];
		//int numSubRoutine = 4; 
		cout << "Num of Sub Routines " <<  numSubRoutine << endl;
		int j;
		for (j = 0; j < (DEFAULT_NUM_PAGES_PER_PROCESS + numSubRoutine * 2); j++)
		{
			//cout << "creating process loop" << endl;
			cout << "j: " << j << endl;
			if (j < 2)
			{
				Page tempPage = Page(0, 0, false, -1, name, -1);
				backingStore.push_back(tempPage);
				tempTable[j] = &backingStore[backingStore.size() - 1];//point to the last element in the backingStore
			}
			else if (j < 5)
			{
				Page tempPage = Page(1, 0, false, -1, name, -1);
				backingStore.push_back(tempPage);
				tempTable[j] = &backingStore[backingStore.size() - 1];
			}
			else if (j < 10)
			{
				Page tempPage = Page(2, 0, false, -1, name, -1);
				backingStore.push_back(tempPage);
				tempTable[j] = &backingStore[backingStore.size() - 1];
			}
			else if ((j % 10) < 2)
			{
				Page tempPage = Page(3, 0, false, -1, name, -1);
				backingStore.push_back(tempPage);
				tempTable[j] = &backingStore[backingStore.size() - 1];
			}
			else if ((j % 10) < 4)
			{
				Page tempPage = Page(4, 0, false, -1, name, -1);
				backingStore.push_back(tempPage);
				tempTable[j] = &backingStore[backingStore.size() - 1];
			}
			else if ((j % 10) < 6)
			{
				Page tempPage = Page(5, 0, false, -1, name, -1);
				backingStore.push_back(tempPage);
				tempTable[j] = &backingStore[backingStore.size() - 1];
			}
			else if ((j % 10) < 8)
			{
				Page tempPage = Page(6, 0, false, -1, name, -1);
				backingStore.push_back(tempPage);
				tempTable[j] = &backingStore[backingStore.size() - 1];
			}
			else if ((j % 10) < 10)
			{
				Page tempPage = Page(7, 0, false, -1, name, -1);
				backingStore.push_back(tempPage);
				tempTable[j] = &backingStore[backingStore.size() - 1];
			}
		}
		if (numSubRoutine != 5)//can make this better. need a way to initialize page table without this.
		{
			int numEmptySubRoutine = (MAX_SUBROUTINES - numSubRoutine) * 2;
			cout <<"Fill empty pages: " << j << " " << numEmptySubRoutine << endl; //filling the temp page table with "empty" pages because of seg fault and garbage data
			for (int k = j; k < j + numEmptySubRoutine; k++)
			{
				//myPage.frameNum = -1;
				//myPage.refByte = 0;
				tempTable[k] = &myPage;
			}
		}
		Process process = Process(name, timeOfLife, 0, numSubRoutine, tempTable);
		vectOfProcesses.push_back(process);
	}
}

void touchProcess(void)
{
	vector<Page*> pagesToLoad;
	int selectedIndex = rand() % ((vectOfProcesses.size() - 1) + 1);//choose an index from 0 - (size-1)
	cout << "Touching process at index " << selectedIndex << endl;
	Page** tempTable = vectOfProcesses[selectedIndex].pageTable;
	for (int i = 0; i < MAX_NUM_PAGES_PER_PROCESS / 2; i++) //divide by 2 to load the necessary pages first. need to randomly select a subroutine
	{
		if (!(tempTable[i]->valid))
		{
			pagesToLoad.push_back(tempTable[i]); //gets the pages from backingstore using process's page table
		}
	}

	int subRoutine = rand() % vectOfProcesses[selectedIndex].subRoutines;
	cout << "running subroutine " << subRoutine << endl;	// TODO test output

	if (!tempTable[2 * subRoutine + 10]->valid)
	{
		pagesToLoad.push_back(tempTable[2 * subRoutine + 10]);
	}

	if (!tempTable[2 * subRoutine + 10 + 1]->valid)
	{
		pagesToLoad.push_back(tempTable[2 * subRoutine + 10 + 1]);
	}

	fifo(pagesToLoad, selectedIndex);	
}

void fifo(vector<Page*> v, int pid)
{
	//cout << "process index " << pid << endl;
	vectOfProcesses[pid].deathTime = runTime + vectOfProcesses[pid].lifeTime;
	while (v.size() > 0)
	{
		if (freeFrames.size() == 0)
		{
			int victimIndex = -1;
			for (uint i = 0; i < v.size(); i++)//loop enough times to free enough frames for the pages needed to be loaded
			{
				int smallestStart = MAX_QUANTA;
				for (int j = 0; j < MAX_FRAMES; j++)
				{
					if (mainMemory[j]->start < smallestStart)
					{
						smallestStart = mainMemory[j]->start;
						victimIndex = j;
					}
				}
				mainMemory[victimIndex]->valid = false;
				mainMemory[victimIndex]->frameNum = -1;
				zeroFillMemory(victimIndex);
				freeFrames.push_back(victimIndex);
			}
		}
		else
		{
			v.back()->start = runTime;
			v.back()->frameNum = freeFrames.back();
			v.back()->valid = true;
			mainMemory[freeFrames.back()] = v.back();
			freeFrames.pop_back();
			v.pop_back();
		}
	}
}

void printProcessPageTable(Process p)
{
	Page** tempTable = p.pageTable;
	for (int i = 0; i < MAX_NUM_PAGES_PER_PROCESS; i++)
	{
		cout << "Process: " << tempTable[i]->processName << endl;
		cout << "Suffix: " << tempTable[i]->suffix << endl;
		cout << "Ref: " << tempTable[i]->refByte << endl;
		cout << "Valid: " << tempTable[i]->valid << endl;
		cout << "Frame: " << tempTable[i]->frameNum << endl;
		cout << "--------------------------------------------------------------------------------" << endl;
	}
}

