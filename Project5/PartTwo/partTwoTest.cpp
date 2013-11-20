// CS3243 Operating Systems
// Fall 2013
// Project 5: Memory Swapping and Paging, Part 2
// Steven Ng and Hue Moua
// Date: 11/13/2013
// File: parttwo.cpp

#include <stdio.h>
#include <cstdlib>
//#include <deque>
#include <iostream>
#include <vector>

#define MAX_PROCESSES 52	// This will not ever change
#define PROCESS_COUNT 50	// useful when debugging to limit # of procs
//#define PROCESS_COUNT 5	// useful when debugging to limit # of procs
//#define MIN_DEATH_INTERVAL 20
#define MIN_DEATH_INTERVAL 45
//#define MAX_DEATH_INTERVAL 300
#define MAX_DEATH_INTERVAL 49
#define MAX_FRAMES 280
#define MAX_PAGES 720
#define SHIFT_INTERVAL 10
//#define PRINT_INTERVAL 500	// # of cpu quanta between memory map printouts
#define PRINT_INTERVAL 5	// # of cpu quanta between memory map printouts
//#define MAX_QUANTA 50000	// # quanta to run before ending simulation
#define MAX_QUANTA 50	// # quanta to run before ending simulation
#define SLEEP_LENGTH 250000	// Used with the usleep()to slow down sim between
							// cycles (makes reading screen in real-time easier!)

#define EMPTY_PROCESS_NAME ' '	// need ascii value. could use chars.
#define DEFAULT_NUM_PAGES_PER_PROCESS 10
#define MAX_NUM_PAGES_PER_PROCESS 20
#define MIN_SUBROUTINES 1
#define MAX_SUBROUTINES 5
//#define SEED 1384543729	// bugged seed time for process page creation
#define SEED time(NULL)

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
	void removePage();
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
int pagesLoaded;
int loadedPages;
int loadedProc;
int refBitSet;
int refBitClear;
int pagesUnloaded;
int pagesFree;
int numOfPages;
int unloadedProc;
int deadProc;

void createProcesses(void);
void createPages(Process &p);
void killProcess(void);
void touchProcess(void);
void insertIntoMemory(Page &pg);
void shiftRefByte(void);
int fifo(void);
int lru(void);
//void printProcessPageTable(Process p);
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
	char a;
	for (runTime = 0; runTime < MAX_QUANTA; runTime++)
	{
		if (runTime != 0)
		{
			killProcess();
		}
		if (runTime % SHIFT_INTERVAL == 0 && runTime != 0)
		{
			shiftRefByte();
		}
		
		touchProcess();
		cout << "Running Time: " << runTime << endl;
			cout << "--------------------------------------------" << endl;
			backingStore.printPages();
			printMemoryMap();
		/*if (0 == runTime || runTime % PRINT_INTERVAL == 0)
		{
			cout << "Running Time: " << runTime << endl;
			cout << "--------------------------------------------" << endl;
			backingStore.printPages();
			printMemoryMap();
			usleep(SLEEP_LENGTH);
		}*/
		cout << "Press anything to continue" << endl;
		cin >> a;
	}
	
	/*Test refbyte stuff
	createPages(vectOfProcesses[1]);
	insertIntoMemory(backingStore.pages[1]);
	cout << "ref: " << dec << backingStore.pages[1].refByte << endl;
	cout << "ref: " << hex << backingStore.pages[1].refByte << endl;
	backingStore.pages[1].refByte |= 128;
	cout << "ref: " << dec << backingStore.pages[1].refByte << endl;
	cout << "ref: " << hex << backingStore.pages[1].refByte << endl;
	backingStore.pages[1].refByte >>= 1;
	cout << "ref: " << dec << backingStore.pages[1].refByte << endl;
	cout << "ref: " << hex << backingStore.pages[1].refByte << endl;
	printMemoryMap();
	cout << "valid " << backingStore.pages[1].valid << endl;
	lru();
	cout << "valid " << backingStore.pages[1].valid << endl;
	printMemoryMap();
	*/
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

		int timeOfLife = (0 == i ? MAX_QUANTA : rand() % lifeRange + MIN_DEATH_INTERVAL);
		int numSubRoutines = (0 == i ? 5 : rand() % subRoutinesRange + MIN_SUBROUTINES);

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
	for (uint i = 0; i < vectOfProcesses.size(); i++)
	{
		if (vectOfProcesses[i].deathTime == runTime)
		{
			cout << "killing " << vectOfProcesses[i].name << endl;
			for (int j = 0; j < MAX_NUM_PAGES_PER_PROCESS; j++)//go through dying process's pageIndex
			{
				//cout << "hello1" << endl;
				if (backingStore.pages[vectOfProcesses[i].pageIndex[j]].valid) //if the page is in a frame
				{
					//cout << "hello2" << endl;
					memory.emptyMemory(backingStore.pages[vectOfProcesses[i].pageIndex[j]].frameNum); //from the index, access the backing store to find the frame that the page resides in
				}
				//cout << "hello3" << endl;
				backingStore.pages[vectOfProcesses[i].pageIndex[j]].removePage(); //remove the process's page from the backing store
				vectOfProcesses[i].pageIndex[j] = -1; //clear the process's page index at j
				//cout << "hello4" << endl;
			}
		}
	}
}

void touchProcess(void)
{
	// Choose the kernel at the start of the program
	// Otherwise, choose an index from 0 to (size - 1)
	int processIndex = (runTime == 0 ? 0 : rand() % vectOfProcesses.size());
	Process *pickedProcess = &vectOfProcesses[processIndex];

	cout << "Touching process at index " << processIndex << endl;	// TODO test output
	cout << "Touching process " << pickedProcess->name << endl;	// TODO test output

	if (!pickedProcess->isAlive)
	{
		pickedProcess->isAlive = true;
		createPages(*pickedProcess);
		pickedProcess->deathTime = runTime + pickedProcess->lifeTime;//should death time keep changing everytime it is touched?
	}

	int selectedPage = -1;

	for (int i = 0; i < MAX_NUM_PAGES_PER_PROCESS / 2; i++)
	{
		selectedPage = pickedProcess->pageIndex[i];

		if (!(backingStore.pages[selectedPage].valid))//bring the selected page into memory if necessary
		{
			insertIntoMemory(backingStore.pages[selectedPage]);
		}
		backingStore.pages[selectedPage].refByte |= 128; //bit shift the refByte
		//cout << "ref shift " << backingStore.pages[selectedPage].refByte << endl;
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
		backingStore.pages[selectedSubRoutine].refByte |= 128; //bit shift the refByte //bit shift the refByte


		if (!(backingStore.pages[selectedSubRoutine2].valid)) //bring the second subroutine page into memory if needed
		{
			insertIntoMemory(backingStore.pages[selectedSubRoutine2]);
			//cout << backingStore.pages[selectedSubRoutine2].frameNum << endl;
		}
		backingStore.pages[selectedSubRoutine2].refByte |= 128; //bit shift the refByte //bit shift the refByte
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
			backingStore.pages[selectedSubRoutine].refByte |= 128; //bit shift the refByte
			
			if (!(backingStore.pages[selectedSubRoutine2].valid)) //bring the second subroutine page into memory if needed
			{
				insertIntoMemory(backingStore.pages[selectedSubRoutine2]);
				//cout << backingStore.pages[selectedSubRoutine2].frameNum << endl;
			}
			backingStore.pages[selectedSubRoutine2].refByte |= 128; //bit shift the refByte
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
	//pageLocation->refByte = 128;
	memory.memArray[frame] = pageLocation;
}

void shiftRefByte(void)
{
	for (int i = 0; i < MAX_FRAMES; i++)
	{
		if (memory.memArray[i]->processName != '@')
		{
			memory.memArray[i]->refByte >>= 1; //bit shift right
		}
	}
}

int fifo(void)
{
	cout << "running fifo" << endl;
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
	//cout << "removing " << memory.memArray[victimIndex]->processName << memory.memArray[victimIndex]->suffix << " j: " << victimIndex << endl;
	memory.memArray[victimIndex]->valid = false;
	memory.memArray[victimIndex]->frameNum = -1;
	memory.emptyMemory(victimIndex);
	return victimIndex;
}

int lru(void)
{
	cout << "running lru" << endl;
	int victimIndex = -1;
	int smallestRef = 256; //biggest number that the refByte can be is 255 (1111 1111)

	for (int j = 0; j < MAX_FRAMES; j++)
	{
		if (memory.memArray[j]->refByte < smallestRef
		        && memory.memArray[j]->processName != '@')
		{
			smallestRef = memory.memArray[j]->refByte;
			victimIndex = j;
		}
	}
	cout << "removing " << memory.memArray[victimIndex]->processName << memory.memArray[victimIndex]->suffix << " j: " << victimIndex << endl;
	memory.memArray[victimIndex]->valid = false;
	memory.memArray[victimIndex]->frameNum = -1;
	memory.emptyMemory(victimIndex);
	return victimIndex;
}

/*void printProcessPageTable(Process p)
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
}*/

void printMemoryMap(void)
{
	float usedFramesPercentage = 100.0 * usedFrames / MAX_FRAMES;
	int numFreeFrames = MAX_FRAMES - usedFrames;
	float freeFramesPercentage = 100.0 * numFreeFrames / MAX_FRAMES;
	float numOfPagesPercentage = 100.0 * numOfPages / MAX_PAGES;
	float pagesLoadedPercentage = 100.0 * pagesLoaded / MAX_PAGES;
	float pagesUnloadedPercentage = 100.0 * pagesUnloaded / MAX_PAGES;
	float pagesFreePercentage = 100.0 * pagesFree / MAX_PAGES;
	float loadedProcPercentage = 100.0 * loadedProc / PROCESS_COUNT;
	float unloadedProcPercentage = 100.0 * unloadedProc / PROCESS_COUNT;
	float deadProcPercentage = 100.0 * deadProc / PROCESS_COUNT;

	printf("QUANTA ELAPSED: %i\n", runTime);
	printf("FRAMES:%8if     USED:%5if (%5.1f%%)   FREE:%7if (%5.1f%%)\n", MAX_FRAMES, usedFrames, usedFramesPercentage, numFreeFrames, freeFramesPercentage);
	printf("SWAP SPACE:%4ip     PAGES:%4ip (%5.1f%%)   LOADED:%5ip (%5.1f%%)  UNLOADED:%4ip (%5.1f%%)   FREE:%4ip (%5.1f%%)\n", MAX_PAGES, numOfPages, numOfPagesPercentage, pagesLoaded, pagesLoadedPercentage, pagesUnloaded, pagesUnloadedPercentage, pagesFree, pagesFreePercentage);
	printf("PROCESSES:%5i      LOADED:%3i  (%5.1f%%)   UNLODAED:%3i  (%5.1f%%)  DEAD:%8i  (%5.1f%%)\n", PROCESS_COUNT, loadedProc, loadedProcPercentage, unloadedProc, unloadedProcPercentage, deadProc, deadProcPercentage);
	printf("\nPHYSICAL MEMORY (FRAMES)\n");
	printf("        %02i        %02i        %02i        %02i        %02i        %02i        %02i        %02i        %02i        %02i        %02i        %02i\n", 4, 9, 14, 19, 24, 29, 34, 39, 44, 49, 54, 59);
	printf("--------++--------||--------++--------||--------++--------||--------++--------||--------++--------||--------++--------||\n");
	for (size_t i = 0; i < 60; ++i) printf("%c%c", memory.memArray[i]->processName, memory.memArray[i]->suffix);	printf("\n");
	printf("        %02i        %02i        %02i        %02i        %02i        %02i        %02i        %02i       %3i       %3i       %3i       %3i\n", 64, 69, 74, 79, 84, 89, 94, 99, 104, 109, 114, 119);
	printf("--------++--------||--------++--------||--------++--------||--------++--------||--------++--------||--------++--------||\n");
	for (size_t i = 60; i < 120; ++i) printf("%c%c", memory.memArray[i]->processName, memory.memArray[i]->suffix);	printf("\n");
	printf("       %3i       %3i       %3i       %3i       %3i       %3i       %3i       %3i       %3i       %3i       %3i       %3i\n", 124, 129, 134, 139, 144, 149, 154, 159, 164, 169, 174, 179);
	printf("--------++--------||--------++--------||--------++--------||--------++--------||--------++--------||--------++--------||\n");
	for (size_t i = 120; i < 180; ++i) printf("%c%c", memory.memArray[i]->processName, memory.memArray[i]->suffix);	printf("\n");
	printf("       %3i       %3i       %3i       %3i       %3i       %3i       %3i       %3i       %3i       %3i       %3i       %3i\n", 184, 189, 194, 199, 204, 209, 214, 219, 224, 229, 234, 239);
	printf("--------++--------||--------++--------||--------++--------||--------++--------||--------++--------||--------++--------||\n");
	for (size_t i = 180; i < 240; ++i) printf("%c%c", memory.memArray[i]->processName, memory.memArray[i]->suffix);	printf("\n");
	printf("       %3i       %3i       %3i       %3i       %3i       %3i       %3i       %3i\n", 244, 249, 254, 259, 264, 269, 274, 279);
	printf("--------++--------||--------++--------||--------++--------||--------++--------||\n");
	for (size_t i = 240; i < 280; ++i) printf("%c%c", memory.memArray[i]->processName, memory.memArray[i]->suffix);	printf("\n");
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
	for (size_t i = 0; i < MAX_PAGES; ++i)
	{
		if (' ' == pages[i].suffix)
		{
			return i;
		}
	}

	fprintf(stderr, "The Backing Store is Full!");
	exit(0);
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
		if (memArray[i]->suffix == ' ')
		{
			return i;
		}
	}

	//return fifo();	// returns an index value of the recently freed frame
	return lru();
	//return secondChance();
}

Page::Page() : processName(EMPTY_PROCESS_NAME), suffix(' '), frameNum(-1), valid(false), refByte(0), startTime(-1) { }

Page::Page(char processName, char suffix, short frameNum, bool valid, short refByte, int startTime) : processName(processName), suffix(suffix), frameNum(frameNum), valid(valid), refByte(refByte), startTime(startTime) { }

void Page::initialize(char processName, char suffix)
{
	this->processName = processName;
	this->suffix = suffix;
	this->valid = false;
	this->refByte = 0;
}

void Page::removePage()
{
	processName = EMPTY_PROCESS_NAME;
	suffix = ' ';
	frameNum = -1;
	valid = false;
	refByte = 0;
	startTime = -1;
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
	printf("%5s | %7s | %6s | %6s | %5s | %9s | %5s\n", "", "Assoc.", "", "Frame", "Valid", "Reference", "Start");
	printf("%5s | %7s | %6s | %6s | %5s | %9s | %5s\n", "Index", "Process", "Suffix", "Number", "Bit", "Byte", "Time");
	printf("------+---------+--------+--------+-------+-----------+-------\n");

	for (size_t i = 0; i < MAX_PAGES; ++i)
	{
		printf("%5lu | %7c | %6c | %6i | %5d | %9x | %5d\n", i, pages[i].processName, pages[i].suffix, pages[i].frameNum, pages[i].valid, pages[i].refByte, pages[i].startTime);
	}
}
