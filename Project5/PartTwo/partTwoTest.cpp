// CS3243 Operating Systems
// Fall 2013
// Project 5: Memory Swapping and Paging, Part 2
// Steven Ng and Hue Moua
// Date: 11/13/2013
// File: parttwo.cpp

#include <cstdlib>
//#include <deque>
#include <iostream>
#include <stdio.h>
#include <string.h>
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
#define MIN_SUBROUTINES 1
#define MAX_SUBROUTINES 5
//#define SEED 1384543729	// bugged seed time for process page creation
#define SEED time(NULL)
#define EMPTY_PROCESS_NAME ' ' //need ascii value. could use chars.

using namespace std;

struct Page
{
	char processName;
	char suffix;
	short frameNum;
	bool valid;
	short refByte;
	int startTime;

	Page();
	Page(char processName, char suffix, short frameNum, bool valid,
	        short refByte, int startTime);
	void initialize(char processName, char suffix);
};

struct Process
{
	char name;
	int lifeTime;
	int deathTime;
	int subRoutines;
	bool isAlive;
	int pageIndex[MAX_NUM_PAGES_PER_PROCESS];

	Process(char name, int lifeTime, int subRoutines);
	void print(void);	// TODO test output
};

struct MainMemory
{
	int freeIndex;
	Page* memArray[MAX_FRAMES];

	MainMemory();
	void emptyMemory(int p);
	int getFreeFrame();
};

struct BackingStore
{
	Page pages[MAX_PAGES];
	int freeIndex;

	BackingStore();
	int getFreePage();
	void printPages(void);	// TODO test output
};

vector<Process> vectOfProcesses;
MainMemory memory;
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
int fifo(void);
void printProcessPageTable(Process p);
void printMemoryMap(void);
void printProcesses(void);	// TODO test output

int main(void)
{
	srand(SEED);
	cout << SEED << endl;	// TODO test output
	runTime = 0;
	createProcesses();
	printProcesses();	// TODO test output
	backingStore.printPages();	// TODO test output

	for (runTime = 0; runTime < MAX_QUANTA; runTime++)
	{
		touchProcess();
if (0 == runTime) backingStore.printPages();
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
	int subRoutinesRange = MAX_SUBROUTINES - MIN_SUBROUTINES + 1;
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

		int timeOfLife;
		int numSubRoutines;

		if (i == 0)
		{
			timeOfLife = MAX_QUANTA;
			numSubRoutines = 5;
		}
		else
		{
			timeOfLife = rand() % lifeRange + MIN_DEATH_INTERVAL;
			numSubRoutines = rand() % subRoutinesRange + MIN_SUBROUTINES;
		}

		vectOfProcesses.push_back(Process(name, timeOfLife, numSubRoutines));
	}
}

void createPages(Process &p)
{
	for (int i = 0; i < DEFAULT_NUM_PAGES_PER_PROCESS + 2 * p.subRoutines; i++)
	{
		// Set suffix of page based on the page index i
		char suffix = ('@' == p.name ? '@' :
					   (i <  2 ? '0' :
					   (i <  5 ? '1' :
					   (i < 10 ? '2' :
					   (i < 12 ? '3' :
					   (i < 14 ? '4' :
					   (i < 16 ? '5' :
					   (i < 18 ? '6' : '7'))))))));

		int freeIndex = backingStore.getFreePage();

		p.pageIndex[i] = freeIndex;
		backingStore.pages[freeIndex].initialize(p.name, suffix);
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
	// Choose the kernel at the start of the program
	// Otherwise, choose an index from 0 to (size - 1)
	int processIndex = (runTime == 0 ? 0 : rand() % vectOfProcesses.size());
	Process *pickedProcess = &vectOfProcesses[processIndex];

	cout << "Touching process at index " << processIndex << endl;	// TODO test output

	if (!pickedProcess->isAlive)
	{
		pickedProcess->isAlive = true;
		createPages(*pickedProcess);
	}

	pickedProcess->deathTime = runTime + pickedProcess->lifeTime;

	int selectedPage = -1;

	for (int i = 0; i < MAX_NUM_PAGES_PER_PROCESS / 2; i++)
	{
		selectedPage = pickedProcess->pageIndex[i];

		if (!(backingStore.pages[selectedPage].valid))//bring the selected page into memory if necessary
		{
			insertIntoMemory(backingStore.pages[selectedPage]);
		}
	}

	int subRoutine = -1;

	if (runTime != 0)
	{
		subRoutine = rand() % pickedProcess->subRoutines;
		cout << "running subroutine " << subRoutine << endl;// TODO test output

		int selectedSubRoutine = pickedProcess->pageIndex[2
		        * subRoutine + 10];
		int selectedSubRoutine2 = pickedProcess->pageIndex[2
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
			selectedSubRoutine = pickedProcess->pageIndex[2
			        * subRoutine + 10];
			selectedSubRoutine2 = pickedProcess->pageIndex[2
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
	float usedFramesPercentage = 100.0 * usedFrames / MAX_FRAMES;
	int numFreeFrames = MAX_FRAMES - usedFrames;
	float freeFramesPercentage = 100.0 * numFreeFrames / MAX_FRAMES;
	float loadedProcPercentage = 100.0 * loadedProc / PROCESS_COUNT;
	int unloadedProc = PROCESS_COUNT - loadedProc;
	float unloadedProcPercentage = 100.0 * unloadedProc / PROCESS_COUNT;

	printf("QUANTA ELAPSED: %i", runTime);
	printf("FRAMES:%-4if         USED:%-4if (%4.1f%%    FREE:%-4if (%4.1f%%)\n", MAX_FRAMES, usedFrames, usedFramesPercentage, numFreeFrames, freeFramesPercentage);
	cout << "SWAP SPACE: " << MAX_PAGES << "\tPAGES: " << usedPages
	        << "\tLOADED: " << loadedPages << "\tFREE: " << numFreeFrames
	        << endl;
	cout << "PROCESSES: " << PROCESS_COUNT << "\tLOADED: " << loadedProc << " ("
	        << loadedProcPercentage << "%)\tUNLOADED: " << unloadedProc << " ("
	        << unloadedProcPercentage << "%)" << endl;
	cout
	        << "        04        09        14        19        24        29        34"
	        << endl;
	cout
	        << "--------++--------||--------++--------||--------++--------||--------++"
	        << endl;
	for (size_t i = 0; i < 35; ++i)
		cout << memory.memArray[i]->processName << memory.memArray[i]->suffix;
	cout << endl;
	cout
	        << "        39        44        49        54        59        64        69"
	        << endl;
	cout
	        << "--------++--------||--------++--------||--------++--------||--------++"
	        << endl;
	for (size_t i = 35; i < 70; ++i)
		cout << memory.memArray[i]->processName << memory.memArray[i]->suffix;
	cout << endl;
	cout
	        << "        74        79        84        89        94        99       104"
	        << endl;
	cout
	        << "--------++--------||--------++--------||--------++--------||--------++"
	        << endl;
	for (size_t i = 70; i < 105; ++i)
		cout << memory.memArray[i]->processName << memory.memArray[i]->suffix;
	cout << endl;
	cout
	        << "       109       114       119       124       129       134       139"
	        << endl;
	cout
	        << "--------++--------||--------++--------||--------++--------||--------++"
	        << endl;
	for (size_t i = 105; i < 140; ++i)
		cout << memory.memArray[i]->processName << memory.memArray[i]->suffix;
	cout << endl;
	cout
	        << "       144       149       154       159       164       169       174"
	        << endl;
	cout
	        << "--------++--------||--------++--------||--------++--------||--------++"
	        << endl;
	for (size_t i = 140; i < 175; ++i)
		cout << memory.memArray[i]->processName << memory.memArray[i]->suffix;
	cout << endl;
	cout
	        << "       179       184       189       194       199       204       209"
	        << endl;
	cout
	        << "--------++--------||--------++--------||--------++--------||--------++"
	        << endl;
	for (size_t i = 175; i < 210; ++i)
		cout << memory.memArray[i]->processName << memory.memArray[i]->suffix;
	cout << endl;
	cout
	        << "       214       219       224       229       234       239       244"
	        << endl;
	cout
	        << "--------++--------||--------++--------||--------++--------||--------++"
	        << endl;
	for (size_t i = 210; i < 245; ++i)
		cout << memory.memArray[i]->processName << memory.memArray[i]->suffix;
	cout << endl;
	cout
	        << "       249       254       259       264       269       274       279"
	        << endl;
	cout
	        << "--------++--------||--------++--------||--------++--------||--------++"
	        << endl;
	for (size_t i = 245; i < 280; ++i)
		cout << memory.memArray[i]->processName << memory.memArray[i]->suffix;
	cout << endl;
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
	} while (pages[freeIndex].suffix != ' ' && freeIndex != freePage);

	// If the do-while completely loops around the BackingStore, then there are no more free pages
	if (freeIndex == freePage)
	{
		cerr << "The Backing Store is full" << endl;
		return -1;
	}

	return freePage;
}

MainMemory::MainMemory() : freeIndex(0)
{
	for (int i = 0; i < MAX_FRAMES; i++)
	{
		memArray[i] = &emptyPage;
	}
}

void MainMemory::emptyMemory(int p)
{
	memArray[p] = &emptyPage;
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

	return fifo();	// returns an index value of the recently freed frame
}

Page::Page() :
		processName(EMPTY_PROCESS_NAME), suffix(' '), frameNum(-1), valid(false), refByte(
		        -1), startTime(-1)
{
}

Page::Page(char processName, char suffix, short frameNum, bool valid,
        short refByte, int startTime) :
		processName(processName), suffix(suffix), frameNum(frameNum), valid(
		        valid), refByte(refByte), startTime(startTime)
{
}

void Page::initialize(char processName, char suffix)
{
	this->processName = processName;
	this->suffix = suffix;
	this->valid = false;
}

Process::Process(char name, int lifeTime, int subRoutines) : name(name), lifeTime(lifeTime), deathTime(0), subRoutines(subRoutines), isAlive(false)
{
	for (int i = 0; i < MAX_NUM_PAGES_PER_PROCESS; i++)
	{
		pageIndex[i] = -1;
	}
}

// TODO test output
void Process::print(void)
{
	string index = "";

	printf("%4c | %4i | %5i | %11i | %5i | {", name, lifeTime, deathTime, subRoutines, isAlive);

	for (size_t i = 0; i < MAX_NUM_PAGES_PER_PROCESS; ++i)
	{
		printf("%s%i", (i > 0 ? ", " : ""), pageIndex[i]);
	}

	printf("}\n");
}

// TODO test output
void printProcesses(void)
{
	printf("%4s | %4s | %5s | %11s | %5s | %5s\n", "", "", "", "Number", "", "");
	printf("%4s | %4s | %5s | %11s | %5s | %5s\n", "", "Life", "Death", "of", "Is", "Page");
	printf("%4s | %4s | %5s | %11s | %5s | %5s\n", "Name", "Time", "Time", "Subroutines", "Alive", "Index");
	printf("-----+------+-------+-------------+-------+-------\n");

	for (size_t i = 0; i < vectOfProcesses.size(); ++i)
	{
		vectOfProcesses[i].print();
	}
}

// TODO test output
void BackingStore::printPages(void)
{
	printf("%5s | %7s | %6s | %7s | %6s\n", "", "Process", "", "Frame", "Valid");
	printf("%5s | %7s | %6s | %7s | %6s\n", "Index", "Name", "Suffix", "Number", "Bit");
	printf("------+---------+--------\n");

	for (size_t i = 0; i < MAX_PAGES; ++i)
	{
		printf("%5lu | %7c | %6c | %7i | %6d\n", i, pages[i].processName, pages[i].suffix, pages[i].frameNum, pages[i].valid);
	}
}
