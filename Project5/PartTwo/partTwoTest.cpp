// CS3243 Operating Systems
// Fall 2013
// Project 5: Memory Swapping and Paging, Part 2
// Steven Ng and Hue Moua
// Date: 11/13/2013
// File: parttwo.cpp

#include <cstdlib>
//#include <deque>
#include <iostream>
#include <vector>

#define MAX_PROCESSES 52	// This will not ever change
//#define PROCESS_COUNT 23	// useful when debugging to limit # of procs
#define PROCESS_COUNT 5	// useful when debugging to limit # of procs
//#define MIN_DEATH_INTERVAL 20
#define MIN_DEATH_INTERVAL 5
//#define MAX_DEATH_INTERVAL 300
#define MAX_DEATH_INTERVAL 15
#define MAX_FRAMES 280
#define MAX_PAGES 720
#define SHIFT_INTERVAL 10
//#define PRINT_INTERVAL 500	// # of cpu quanta between memory map printouts
#define PRINT_INTERVAL 5	// # of cpu quanta between memory map printouts
#define MAX_QUANTA 50 /* 50000 */	// # quanta to run before ending simulation
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
	char processName;
	short suffix;
	short frameNum;
	bool valid;
	short refByte;
	int startTime;

	Page();
	Page(char processName, short suffix, short frameNum, bool valid,
	        short refByte, int start);
	void initialize(short suffix, char processName);
};

struct Process
{
	char name;
	int lifeTime;
	int deathTime;
	int subRoutines;
	bool isAlive;
	int pageIndex[MAX_NUM_PAGES_PER_PROCESS];

	Process(char name, int lifeTime, int deathTime, int subRoutines,
	        bool isAlive);
};

vector<Process> vectOfProcesses;

struct MainMemory
{
	int freeIndex;
	Page* memArray[MAX_FRAMES];

	MainMemory();
	void emptyMemory(int p);
	int getFreeFrame();
};

MainMemory memory;

struct BackingStore
{
	Page pages[MAX_PAGES];
	int freeIndex;

	BackingStore();
	int getFreePage();
};

BackingStore backingStore;
Page emptyPage;
int runTime;
int usedFrames;
int usedPages;
int loadedPages;
int loadedProc;
int refBitSet;
int refBitClear;

void createProcesses(void);
void createPages(Process &p);
void killProcess(void);
void touchProcess(void);
void insertIntoMemory(Page &pg);
int fifo();
void printProcessPageTable(Process p);
void printMemoryMap(void);

int main()
{
	srand(SEED);
	cout << SEED << endl;	// TODO test output
	runTime = 0;
	createProcesses();
	cout << "Num of processes: " << vectOfProcesses.size() << endl;

	for (runTime = 0; runTime <= MAX_QUANTA; ++runTime)
	{
		touchProcess();
		for (int i = 0; i < MAX_FRAMES; i++)
		{
			cout << memory.memArray[i]->processName
			        << memory.memArray[i]->suffix;
		}
		cout << endl;
	}

}

void createProcesses(void)
{
	int lifeRange = MAX_DEATH_INTERVAL - MIN_DEATH_INTERVAL + 1;
	char name = '?';

	for (int i = 0; i < PROCESS_COUNT; i++)
	{
		switch (name)
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
			name += 1;
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

		Process process = Process(name, timeOfLife, 0, 0, false);
		vectOfProcesses.push_back(process);
	}
}

void createPages(Process &p)
{
	int subRoutineRange = MAX_SUBROUTINES - MIN_SUBROUTINES + 1;
	int numSubRoutine = 0;
	if (p.name != '@')
	{
		numSubRoutine = rand() % subRoutineRange + MIN_SUBROUTINES;
		p.subRoutines = numSubRoutine;
	}
	else
	{
		numSubRoutine = 5;
		p.subRoutines = numSubRoutine;
	}
	cout << "Num of Sub Routines " << numSubRoutine << endl;
	int j;
	for (j = 0; j < (DEFAULT_NUM_PAGES_PER_PROCESS + numSubRoutine * 2); j++)
	{
		//cout << "creating process loop" << endl;
		//cout << "j: " << j << endl;
		if (j < 2)
		{
			int freeIndex = backingStore.getFreePage();
			p.pageIndex[j] = freeIndex;
			backingStore.pages[freeIndex].initialize(0, p.name);
		}
		else if (j < 5)
		{
			int freeIndex = backingStore.getFreePage();
			p.pageIndex[j] = freeIndex;
			backingStore.pages[freeIndex].initialize(1, p.name);
		}
		else if (j < 10)
		{
			int freeIndex = backingStore.getFreePage();
			p.pageIndex[j] = freeIndex;
			backingStore.pages[freeIndex].initialize(2, p.name);
		}
		else if ((j % 10) < 2)
		{
			int freeIndex = backingStore.getFreePage();
			p.pageIndex[j] = freeIndex;
			backingStore.pages[freeIndex].initialize(3, p.name);
		}
		else if ((j % 10) < 4)
		{
			int freeIndex = backingStore.getFreePage();
			p.pageIndex[j] = freeIndex;
			backingStore.pages[freeIndex].initialize(4, p.name);
		}
		else if ((j % 10) < 6)
		{
			int freeIndex = backingStore.getFreePage();
			p.pageIndex[j] = freeIndex;
			backingStore.pages[freeIndex].initialize(5, p.name);
		}
		else if ((j % 10) < 8)
		{
			int freeIndex = backingStore.getFreePage();
			p.pageIndex[j] = freeIndex;
			backingStore.pages[freeIndex].initialize(6, p.name);
		}
		else if ((j % 10) < 10)
		{
			int freeIndex = backingStore.getFreePage();
			p.pageIndex[j] = freeIndex;
			backingStore.pages[freeIndex].initialize(7, p.name);
		}
	}
}

void killProcess(void)
{
	//TODO go through vectofprocess and check deathtime
	//TODO when one needs to die, change its page entries process name to blank and set to invalid
	//TODO go into memory and remove these invalid pages
	//TODO now remove the pages from the backing store
	//TODO make the process's page table have empty pages
}

void touchProcess(void)
{
	int selectedProcess = 0;	//choose the kernel at the start of the program
	if (runTime != 0)
	{
		selectedProcess = rand() % ((vectOfProcesses.size() - 1) + 1);//choose an index from 0 - (size-1)
	}
	cout << "Touching process at index " << selectedProcess << endl;
	if (!(vectOfProcesses[selectedProcess].isAlive))
	{
		vectOfProcesses[selectedProcess].isAlive = true;
		createPages(vectOfProcesses[selectedProcess]);
	}
	vectOfProcesses[selectedProcess].deathTime = runTime
	        + vectOfProcesses[selectedProcess].lifeTime;
	int selectedPage = -1;
	for (int i = 0; i < MAX_NUM_PAGES_PER_PROCESS / 2; i++)
	{
		selectedPage = vectOfProcesses[selectedProcess].pageIndex[i];
		if (!(backingStore.pages[selectedPage].valid))//bring the selected page into memory if necessary
		{
			insertIntoMemory(backingStore.pages[selectedPage]);
		}
	}
	int subRoutine = -1;
	if (runTime != 0)
	{
		subRoutine = rand() % vectOfProcesses[selectedProcess].subRoutines;
		cout << "running subroutine " << subRoutine << endl;// TODO test output

		int selectedSubRoutine = vectOfProcesses[selectedProcess].pageIndex[2
		        * subRoutine + 10];
		int selectedSubRoutine2 = vectOfProcesses[selectedProcess].pageIndex[2
		        * subRoutine + 10 + 1];

		if (!(backingStore.pages[selectedSubRoutine].valid)) //bring the first subroutine page into memory if needed
		{
			insertIntoMemory(backingStore.pages[selectedSubRoutine]);
			//cout << backingStore.pages[selectedSubRoutine].frameNum << endl;
		}

		if (!(backingStore.pages[selectedSubRoutine2].valid)) //bring the second subroutine page into memory if needed
		{
			insertIntoMemory(backingStore.pages[selectedSubRoutine2]);
			//cout << backingStore.pages[selectedSubRoutine2].frameNum << endl;
		}
	}
	else //run all of the kernel's sub routine pages
	{
		int selectedSubRoutine;
		int selectedSubRoutine2;
		for (int j = 0; j < 5; j++)
		{
			subRoutine = j;
			selectedSubRoutine = vectOfProcesses[selectedProcess].pageIndex[2
			        * subRoutine + 10];
			selectedSubRoutine2 = vectOfProcesses[selectedProcess].pageIndex[2
			        * subRoutine + 10 + 1];

			if (!(backingStore.pages[selectedSubRoutine].valid)) //bring the first subroutine page into memory if needed
			{
				insertIntoMemory(backingStore.pages[selectedSubRoutine]);
				//cout << backingStore.pages[selectedSubRoutine].frameNum << endl;
			}

			if (!(backingStore.pages[selectedSubRoutine2].valid)) //bring the second subroutine page into memory if needed
			{
				insertIntoMemory(backingStore.pages[selectedSubRoutine2]);
				//cout << backingStore.pages[selectedSubRoutine2].frameNum << endl;
			}
		}
	}

}

void insertIntoMemory(Page &pg)
{
	Page *pageLocation = &pg;
	int frame = memory.getFreeFrame();
	pageLocation->valid = true;
	pageLocation->frameNum = frame;
	pageLocation->startTime = runTime;
	memory.memArray[frame] = pageLocation;
}

int fifo()
{
	int victimIndex = -1;
	int smallestStart = MAX_QUANTA;

	for (int j = 0; j < MAX_FRAMES; j++)
	{
		if (memory.memArray[j]->startTime < smallestStart
		        && memory.memArray[j]->processName != '@')
		{
			smallestStart = memory.memArray[j]->startTime;
			victimIndex = j;
		}
	}

	memory.memArray[victimIndex]->valid = false;
	memory.memArray[victimIndex]->frameNum = -1;
	memory.emptyMemory(victimIndex);
	return victimIndex;
}

void printProcessPageTable(Process p)
{
	//Page** tempTable = p.pageTable;
	for (int i = 0; i < MAX_NUM_PAGES_PER_PROCESS; i++)
	{
		cout << p.name << "temp2 " << p.pageIndex[i] << endl;
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
	/*float usedFramesPercentage = 100.0 * usedFrames / MAX_FRAMES;
	 int numFreeFrames = MAX_FRAMES - usedFrames;
	 float freeFramesPercentage = 100.0 * freeFrames.size() / MAX_FRAMES;
	 float loadedProcPercentage = 100.0 * loadedProc / PROCESS_COUNT;
	 int unloadedProc = PROCESS_COUNT - loadedProc;
	 float unloadedProcPercentage = 100.0 * unloadedProc / PROCESS_COUNT;

	 cout << "QUANTA ELAPSED: " << runTime << endl;
	 cout << "FRAMES: " << MAX_FRAMES << "f\t" << "USED: " << usedFrames << "f (" << usedFramesPercentage << "%)\tFREE:" << numFreeFrames << "f (" << freeFramesPercentage << "%)" << endl;
	 cout << "SWAP SPACE: " << MAX_FRAMES << "\tPAGES: " << usedPages << "\tLoaded: " << loadedPages << "\tFREE: " << freeFrames.size() << endl;
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
	 */
}

// TODO insert page function for BackingStore

BackingStore::BackingStore() :
		freeIndex(0)
{
	for (size_t i = 0; i < MAX_PAGES; ++i)
	{
		pages[i] = Page();
	}
}

int BackingStore::getFreePage()
{
	int freePage = freeIndex;

	// Find the next free page in the backing store.
	do
	{
		freeIndex = (freeIndex + 1) % MAX_PAGES;
	} while (pages[freeIndex].suffix > 0 && freeIndex != freePage);

	// If the do-while completely loops around the BackingStore, then there are no more free pages
	if (freeIndex == freePage)
	{
		cerr << "The Backing Store is full" << endl;
		return -1;
	}

	return freePage;
}

MainMemory::MainMemory()
{
	freeIndex = 0;
	for (int i = 0; i < MAX_FRAMES; i++)
	{
		memArray[i] = &emptyPage;
	}
}

void MainMemory::emptyMemory(int p)
{
	this->memArray[p] = &emptyPage;
}

int MainMemory::getFreeFrame()
{
	for (int i = 0; i < MAX_FRAMES; i++)
	{
		if (memArray[i]->suffix == -1)
		{
			return i;
		}
	}
	return fifo(); //returns an index value of the recently freed frame
}

Page::Page()
{
	suffix = -1;
	refByte = -1;
	valid = false;
	frameNum = -1;
	processName = EMPTY_PROCESS_NAME;
	startTime = -1;
}

Page::Page(char processName, short suffix, short frameNum, bool valid,
        short refByte, int start)
{
	this->suffix = suffix;
	this->refByte = refByte;
	this->valid = valid;
	this->frameNum = frameNum;
	this->processName = processName;
	this->startTime = start;
}

void Page::initialize(short suffix, char processName)
{
	this->suffix = suffix;
	this->valid = false;
	this->processName = processName;
}

Process::Process(char name, int lifeTime, int deathTime, int subRoutines,
        bool isAlive)
{
	this->name = name;
	cout << name << endl;
	this->lifeTime = lifeTime;
	this->deathTime = deathTime;
	this->subRoutines = subRoutines;
	this->isAlive = isAlive;
	for (int i = 0; i < MAX_NUM_PAGES_PER_PROCESS; i++)
	{
		pageIndex[i] = -1;
	}
}
