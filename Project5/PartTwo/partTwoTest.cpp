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
#define MIN_DEATH_INTERVAL 5
#define MAX_DEATH_INTERVAL 15
#define MAX_FRAMES 280
#define MAX_PAGES 720
#define SHIFT_INTERVAL 10
#define PRINT_INTERVAL 5	// # of cpu quanta between memory map printouts
#define MAX_QUANTA 50	// # quanta to run before ending simulation
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
	int pageIndex[MAX_NUM_PAGES_PER_PROCESS];

	Process(char name, int lifeTime, int deathTime, int subRoutines) : name(name), lifeTime(lifeTime), deathTime(deathTime), subRoutines(subRoutines){}
};

Page* mainMemory[MAX_FRAMES];
vector<Process> vectOfProcesses;
vector<int> freeFrames;
deque<Page> backingStore;
Page myPage;
int runTime;
int usedFrames;
int loadedPages;
int loadedProc;
int refBitSet;
int refBitClear;

void zeroFillMemory(int start, int size);
void zeroFillMemory(int start);
void findFreeFrames(void);
void createProcesses(void);
void createPages(Process p);
void killProcess(void);
void touchProcess(void);
void fifo(vector<Page*> v, int pid);
void printProcessPageTable(Process p);
void printMemoryMap(void);

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
	createPages(vectOfProcesses[0]);
	printProcessPageTable(vectOfProcesses[0]);
	/*for (runTime = 0; runTime <= MAX_QUANTA; ++runTime)
	{
		touchProcess();
		if (runTime % PRINT_INTERVAL == 0)
		{
			for (uint h = 0; h < vectOfProcesses.size(); h++)
			{
				cout << "Process: " << vectOfProcesses[h].name << " LifeTime: " << vectOfProcesses[h].lifeTime;
				cout << " Deathtime: " << vectOfProcesses[h].deathTime << " #Subroutines " << vectOfProcesses[h].subRoutines << endl;
				printProcessPageTable(vectOfProcesses[h]);
			}
			cout << "Memory Map " << runTime << endl;
			for (int i = 0; i < MAX_FRAMES; i++)
			{
				cout << mainMemory[i]->processName << mainMemory[i]->frameNum;
			}
		}
	}*/
}

void zeroFillMemory(int start, int size)
{
	for (int i = 0; i < size; i++)
	{
		//myPage.frameNum = i;
		mainMemory[start + i] = &myPage;
	}
}

void zeroFillMemory(int start)
{
	//myPage.frameNum = start;
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
		int timeOfLife = 0;
		if (i == 0)
		{
			timeOfLife = MAX_QUANTA;
		}
		else 
		{
			timeOfLife = rand() % lifeRange + MIN_DEATH_INTERVAL;
		}
		Process process = Process(name, timeOfLife, 0, 0);
		vectOfProcesses.push_back(process);
	}
}

void createPages(Process p)
{
	int subRoutineRange = MAX_SUBROUTINES - MIN_SUBROUTINES + 1;
	int numSubRoutine = 0;
	if (p.name != '@')
	{
		numSubRoutine = rand() % subRoutineRange + MIN_SUBROUTINES;
	}
	else
	{
		numSubRoutine = 5;
	}
	cout << "Num of Sub Routines " <<  numSubRoutine << endl;
	int j;
	int tempIndex[MAX_NUM_PAGES_PER_PROCESS];
	for (j = 0; j < (DEFAULT_NUM_PAGES_PER_PROCESS + numSubRoutine * 2); j++)
	{
		//cout << "creating process loop" << endl;
		cout << "j: " << j << endl;
		if (j < 2)
		{
			Page tempPage = Page(0, 0, false, -1, p.name, -1);
			backingStore.push_back(tempPage);
			tempIndex[j] = backingStore.size() - 1;//index to the last element in the backingStore
		}
		else if (j < 5)
		{
			Page tempPage = Page(1, 0, false, -1, p.name, -1);
			backingStore.push_back(tempPage);
			tempIndex[j] = backingStore.size() - 1;//index to the last element in the backingStore
		}
		else if (j < 10)
		{
			Page tempPage = Page(2, 0, false, -1, p.name, -1);
			backingStore.push_back(tempPage);
			tempIndex[j] = backingStore.size() - 1;//index to the last element in the backingStore
		}
		else if ((j % 10) < 2)
		{
			Page tempPage = Page(3, 0, false, -1, p.name, -1);
			backingStore.push_back(tempPage);
			tempIndex[j] = backingStore.size() - 1;//index to the last element in the backingStore
		}
		else if ((j % 10) < 4)
		{
			Page tempPage = Page(4, 0, false, -1, p.name, -1);
			backingStore.push_back(tempPage);
			tempIndex[j] = backingStore.size() - 1;//index to the last element in the backingStore
		}
		else if ((j % 10) < 6)
		{
			Page tempPage = Page(5, 0, false, -1, p.name, -1);
			backingStore.push_back(tempPage);
			tempIndex[j] = backingStore.size() - 1;//index to the last element in the backingStore
		}
		else if ((j % 10) < 8)
		{
			Page tempPage = Page(6, 0, false, -1, p.name, -1);
			backingStore.push_back(tempPage);
			tempIndex[j] = backingStore.size() - 1;//index to the last element in the backingStore
		}
		else if ((j % 10) < 10)
		{
			Page tempPage = Page(7, 0, false, -1, p.name, -1);
			backingStore.push_back(tempPage);
			tempIndex[j] = backingStore.size() - 1;//index to the last element in the backingStore
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
			tempIndex[k] = -1;
		}
	}
	//p.pageIndex[MAX_NUM_PAGES_PER_PROCESS] = tempIndex[MAX_NUM_PAGES_PER_PROCESS];
	for (int i = 0; i < MAX_NUM_PAGES_PER_PROCESS; i++)
	{
		p.pageIndex[i] = tempIndex[i];
	}
	
}

void killProcess(void)
{
	//TODO go through vectofprocess and check deathtime
	//TODO when one needs to die, change its page entries process name to blank and set to invalid
	//TODO go into memory and remove these invalid pages
	//TODO now remove the pages from the backing store
	//TODO make the process's page table have empty pages
	for (uint i = 0; i < vectOfProcesses.size(); i++)
	{
		if (vectOfProcesses[i].deathTime == runTime)
		{
			//vectOfProcesses[i].isAlive = false;
			for (int j = 0; j < MAX_FRAMES; j++)
			{
				if (mainMemory[j]->processName == vectOfProcesses[i].name)//removing the process's pages from memory
				{
					mainMemory[j]->valid = false;
					zeroFillMemory(j);
				}
			}
			
			for (int k = 0; k < MAX_NUM_PAGES_PER_PROCESS; k++)//fill with "empty" pages in the process's page table 
			{
				//vectOfProcesses[i].pageTable[k] = &myPage; 
			}
			
			for (uint l = 0; l < backingStore.size(); l++)//removing the process's pages from the backing store
			{
				if (backingStore[l].processName == vectOfProcesses[i].name)
				{
					backingStore.erase(backingStore.begin() + l); //verify this
				}
			}
		}
	}
	
}

void touchProcess(void)
{
	vector<Page*> pagesToLoad;
	int selectedIndex = rand() % ((vectOfProcesses.size() - 1) + 1);//choose an index from 0 - (size-1)
																	//need to change this to choose a process name.
	cout << "Touching process at index " << selectedIndex << endl;
	/*Page** tempTable = vectOfProcesses[selectedIndex].pageTable;
	for (int i = 0; i < MAX_NUM_PAGES_PER_PROCESS / 2; i++) //divide by 2 to load the necessary pages first
	{
		if (!(tempTable[i]->valid))
		{
			pagesToLoad.push_back(tempTable[i]); //gets a pointer to the pages int the backingstore using process's page table
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
	}*/

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
	//Page** tempTable = p.pageTable;
	for (int i = 0; i < MAX_NUM_PAGES_PER_PROCESS; i++)
	{
		cout << "temp " << p.pageIndex[i] << endl;
	}
		//cout << "Process: " << tempTable[i]->processName << endl;
		//cout << "Suffix: " << tempTable[i]->suffix << endl;
		//cout << "Ref: " << tempTable[i]->refByte << endl;
		//cout << "Valid: " << tempTable[i]->valid << endl;
		//cout << "Frame: " << tempTable[i]->frameNum << endl;
		//cout << "--------------------------------------------------------------------------------" << endl;
}

void printMemoryMap(void)
{
	float usedFramesPercentage = 100.0 * usedFrames / MAX_FRAMES;
	int numFreeFrames = MAX_FRAMES - usedFrames;
	float freeFramesPercentage = 100.0 * freeFrames.size() / MAX_FRAMES;
	float loadedProcPercentage = 100.0 * loadedProc / PROCESS_COUNT;
	int unloadedProc = PROCESS_COUNT - loadedProc;
	float unloadedProcPercentage = 100.0 * unloadedProc / PROCESS_COUNT;

	cout << "QUANTA ELAPSED: " << runTime << endl;
	cout << "FRAMES: " << MAX_FRAMES << "f\t" << "USED: " << usedFrames << "f (" << usedFramesPercentage << "%)\tFREE:" << numFreeFrames << "f (" << freeFramesPercentage << "%)" << endl;
	cout << "SWAP SPACE: " << MAX_FRAMES << "\tPAGES: " <<  backingStore.size() << "\tLoaded: " << loadedPages << "\tFREE: " << freeFrames.size() << endl;
	cout << "PROCESSES: " << PROCESS_COUNT << "\tLOADED: " << loadedProc << " (" << loadedProcPercentage << "%)\tUNLOADED: " << unloadedProc << " (" << unloadedProcPercentage << "%)" << endl;
	cout << "        04        09        14        19        24        29        34" << endl;
	cout << "--------++--------||--------++--------||--------++--------||--------++" << endl;
	for (size_t i = 0; i < 35; ++i) cout << mainMemory[i]->processName << mainMemory[i]->suffix;	cout << endl;
	cout << "        39        44        49        54        59        64        69" << endl;
	cout << "--------++--------||--------++--------||--------++--------||--------++" << endl;
	for (size_t i = 35; i < 70; ++i) cout << mainMemory[i]->processName << mainMemory[i]->suffix;	cout << endl;
	cout << "        74        79        84        89        94        99       104" << endl;
	cout << "--------++--------||--------++--------||--------++--------||--------++" << endl;
	for (size_t i = 70; i < 105; ++i) cout << mainMemory[i]->processName << mainMemory[i]->suffix;	cout << endl;
	cout << "       109       114       119       124       129       134       139" << endl;
	cout << "--------++--------||--------++--------||--------++--------||--------++" << endl;
	for (size_t i = 105; i < 140; ++i) cout << mainMemory[i]->processName << mainMemory[i]->suffix;	cout << endl;
	cout << "       144       149       154       159       164       169       174" << endl;
	cout << "--------++--------||--------++--------||--------++--------||--------++" << endl;
	for (size_t i = 140; i < 175; ++i) cout << mainMemory[i]->processName << mainMemory[i]->suffix;	cout << endl;
	cout << "       179       184       189       194       199       204       209" << endl;
	cout << "--------++--------||--------++--------||--------++--------||--------++" << endl;
	for (size_t i = 175; i < 210; ++i) cout << mainMemory[i]->processName << mainMemory[i]->suffix;	cout << endl;
	cout << "       214       219       224       229       234       239       244" << endl;
	cout << "--------++--------||--------++--------||--------++--------||--------++" << endl;
	for (size_t i = 210; i < 245; ++i) cout << mainMemory[i]->processName << mainMemory[i]->suffix;	cout << endl;
	cout << "       249       254       259       264       269       274       279" << endl;
	cout << "--------++--------||--------++--------||--------++--------||--------++" << endl;
	for (size_t i = 245; i < 280; ++i) cout << mainMemory[i]->processName << mainMemory[i]->suffix;	cout << endl;
}
