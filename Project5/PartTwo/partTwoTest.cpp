// CS3243 Operating Systems
// Fall 2013
// Project 5: Memory Swapping and Paging, Part 2
// Steven Ng and Hue Moua
// Date: 11/13/2013
// File: parttwo.cpp

#include <stdio.h>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <vector>

#define MAX_PROCESSES 52	// This will not ever change
//#define PROCESS_COUNT 23	// useful when debugging to limit # of procs
#define PROCESS_COUNT 23 // useful when debugging to limit # of procs
//#define MIN_DEATH_INTERVAL 20
#define MIN_DEATH_INTERVAL 50
//#define MAX_DEATH_INTERVAL 300
#define MAX_DEATH_INTERVAL 99
#define MAX_FRAMES 280
#define MAX_PAGES 720
//#define SHIFT_INTERVAL 10
#define SHIFT_INTERVAL 2
//#define PRINT_INTERVAL 500	// # of cpu quanta between memory map printouts
#define PRINT_INTERVAL 5	// # of cpu quanta between memory map printouts
//#define MAX_QUANTA 50000	// # quanta to run before ending simulation
#define MAX_QUANTA 100	// # quanta to run before ending simulation
#define SLEEP_LENGTH 250000	// Used with the usleep()to slow down sim between
							// cycles (makes reading screen in real-time easier!)

#define EMPTY_PROCESS_NAME ' '	// need ascii value. could use chars.
#define DEFAULT_NUM_PAGES_PER_PROCESS 10
#define MAX_NUM_PAGES_PER_PROCESS 20
#define MIN_SUBROUTINES 1
#define MAX_SUBROUTINES 5
//#define SEED 1384543729	// bugged seed time for process page creation
#define SEED time(NULL)
#define PROCS_PER_LINE 12	// # of processes to show on one line when printing per process page tables

using namespace std;

struct Page
{
	char processName;
	char suffix;
	short frameNum;
	bool valid;
	short refByte;
	bool sc;
	int startTime;

	Page();
	Page(char processName, char suffix, short frameNum, bool valid, short refByte, int startTime);
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
};

struct MainMemory
{
	int freeIndex;
	Page* memArray[MAX_FRAMES];

	MainMemory();
	void emptyMemory(int p);
	int getFreeFrame();
	void print(void);
};

struct BackingStore
{
	Page pages[MAX_PAGES];
	int freeIndex;

	BackingStore();
	int getFreePage();
	void print(void);
};

vector<Process> vectOfProcesses;
MainMemory memory;
BackingStore backingStore;
Page emptyPage;
deque<int> pageQueue;
int runTime;
int usedFrames;
//int pagesLoaded;
int loadedProc;
int refBitSet;
int refBitClear;
//int pagesUnloaded;
//int pagesFree;
int numOfPages;
//int unloadedProc;
int deadProc;

void createProcesses(void);
void createPages(Process &p);
void killProcess(void);
void touchProcess(void);
void insertIntoMemory(Page &pg);
void shiftRefByte(void);
int fifo(void);
int lru(void);
int secondChance(void);
int fifoCheck(int j);
//void printProcessPageTable(Process p);
void printPerProcessPageTables(void);

int main(void)
{
	srand(SEED);
	cout << SEED << endl;	// TODO test output
	createProcesses();
	backingStore.print();
	//backingStore.printPages();
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
			//backingStore.printPages();
			memory.print();
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
			name = 'A';
			break;
		case 'Z':
			name = 'a';
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
		numOfPages++;
	}
}

//TODO go through vectofprocess and check deathtime
//TODO when one needs to die, change its page entries process name to blank and set to invalid
//TODO go into memory and remove these invalid pages
//TODO now remove the pages from the backing store
//TODO make the process's page table have empty pages
void killProcess(void)
{
	for (uint i = 0; i < vectOfProcesses.size(); i++)
	{
		if (vectOfProcesses[i].deathTime == runTime)
		{
			cout << "killing " << vectOfProcesses[i].name << endl;	// TODO test output

			for (int j = 0; j < MAX_NUM_PAGES_PER_PROCESS; j++)	// go through dying process's pageIndex
			{
				//cout << "hello1" << endl;

				if (backingStore.pages[vectOfProcesses[i].pageIndex[j]].valid)	// if the page is in a frame
				{
					//cout << "hello2" << endl;
					memory.emptyMemory(backingStore.pages[vectOfProcesses[i].pageIndex[j]].frameNum); //from the index, access the backing store to find the frame that the page resides in
					usedFrames--;
				}
				//cout << "hello3" << endl;
				backingStore.pages[vectOfProcesses[i].pageIndex[j]].removePage(); //remove the process's page from the backing store
				numOfPages--;
				vectOfProcesses[i].pageIndex[j] = -1; //clear the process's page index at j
				//cout << "hello4" << endl;
			}
			vectOfProcesses[i].isAlive = false;
			loadedProc--;
			deadProc++;
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
		// if deathTime > 0, then we're touching a dead process
		if (pickedProcess->deathTime > 0)
		{
			deadProc--;
		}

		pickedProcess->isAlive = true;
		loadedProc++;
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
		backingStore.pages[selectedPage].sc = true; //set second chance bit to true
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
		backingStore.pages[selectedSubRoutine].refByte |= 128; //bit shift the refByte
		backingStore.pages[selectedSubRoutine].sc = true; //set second chance bit to true

		if (!(backingStore.pages[selectedSubRoutine2].valid)) //bring the second subroutine page into memory if needed
		{
			insertIntoMemory(backingStore.pages[selectedSubRoutine2]);
			//cout << backingStore.pages[selectedSubRoutine2].frameNum << endl;
		}
		backingStore.pages[selectedSubRoutine2].refByte |= 128; //bit shift the refByte
		backingStore.pages[selectedSubRoutine2].sc = true; //set second chance bit to true
	}
	else //run all of the kernel's sub routine pages
	{
		cout << backingStore.pages[0].sc << endl;
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
			backingStore.pages[selectedSubRoutine].sc = true;

			if (!(backingStore.pages[selectedSubRoutine2].valid)) //bring the second subroutine page into memory if needed
			{
				insertIntoMemory(backingStore.pages[selectedSubRoutine2]);
				//cout << backingStore.pages[selectedSubRoutine2].frameNum << endl;
			}
			backingStore.pages[selectedSubRoutine2].refByte |= 128; //bit shift the refByte
			backingStore.pages[selectedSubRoutine2].sc = true;
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
	if (pageLocation->processName != '@')
	{
		pageQueue.push_back(frame);
	}
	usedFrames++;
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
	/*int smallestStart = MAX_QUANTA;

	for (int j = 0; j < MAX_FRAMES; j++)
	{
		if (memory.memArray[j]->startTime < smallestStart
		        && memory.memArray[j]->processName != '@')
		{
			smallestStart = memory.memArray[j]->startTime;
			victimIndex = j;
		}
	}*/
	victimIndex = pageQueue[0];
	pageQueue.pop_front();
	//cout << "removing " << memory.memArray[victimIndex]->processName << memory.memArray[victimIndex]->suffix << " j: " << victimIndex << endl;
	memory.memArray[victimIndex]->valid = false;
	memory.memArray[victimIndex]->frameNum = -1;
	memory.emptyMemory(victimIndex);
	usedFrames--;
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
	usedFrames--;
	return victimIndex;
}

int secondChance(void)
{
	//TODO in a loop run fifo to get an index position of the process with the smallest time
	//TODO from the index check the second chance bit. if 1 set to 0 and run fifo again. if 0 pick that index and break
	cout << "running second chance" << endl;
	int victimIndex = -1;
	int startIndex = 20; //give a starting position and increment because the fifocheck was starting at the same spot and removing the page that was just inserted
						//cannot compare suffix and process name because the incoming pages might be just be subroutine pages.
	while (true)
	{
		victimIndex = fifoCheck(startIndex++ % MAX_FRAMES); //increment after the operation. mod by MAX_FRAMES for wrap around
		if (memory.memArray[victimIndex]->sc)
		{
			memory.memArray[victimIndex]->sc = false;
			memory.memArray[victimIndex]->startTime = runTime;
			cout << "give second chance" << endl;
			//cout << victimIndex << endl;
		}
		else
		{
			break;
		}
	}
	/*for (int j = 0; j < MAX_FRAMES; j++)
	{
		if (memory.memArray[j]->processName != '@')
		{
			if (memory.memArray[j]->sc)
			{
				memory.memArray[j]->sc = false;
				cout << "give second chance" << endl;
			}
			else
			{
				victimIndex = j;
				break;
			}
		}
	}

	if (victimIndex == -1)
	{
		victimIndex = fifo();
		cout << " j: " << victimIndex << endl;
	}*/
	cout << "removing " << memory.memArray[victimIndex]->processName << memory.memArray[victimIndex]->suffix << " j: " << victimIndex << endl;
	memory.memArray[victimIndex]->valid = false;
	memory.memArray[victimIndex]->frameNum = -1;
	memory.emptyMemory(victimIndex);
	usedFrames--;
	return victimIndex;
}

int fifoCheck(int j)
{
	cout << "running fifoCheck" << endl;
	int victimIndex = -1;
	int smallestStart = MAX_QUANTA;

	for ( ; j < MAX_FRAMES; j++)
	{
		if (memory.memArray[j]->startTime < smallestStart
		        && memory.memArray[j]->processName != '@')
		{
			smallestStart = memory.memArray[j]->startTime;
			victimIndex = j;
		}
	}
	cout << "start: " << smallestStart << endl;
	//cout << "removing " << memory.memArray[victimIndex]->processName << memory.memArray[victimIndex]->suffix << " j: " << victimIndex << endl;
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

void MainMemory::print(void)
{
	float usedFramesPercentage = 100.0 * usedFrames / MAX_FRAMES;
	int numFreeFrames = MAX_FRAMES - usedFrames;
	float freeFramesPercentage = 100.0 * numFreeFrames / MAX_FRAMES;
	float numOfPagesPercentage = 100.0 * numOfPages / MAX_PAGES;
	float pagesLoadedPercentage = 100.0 * usedFrames / MAX_PAGES;
	float pagesUnloadedPercentage = 100.0 * (numOfPages - usedFrames) / MAX_PAGES;
	float pagesFreePercentage = 100.0 * (MAX_PAGES - numOfPages) / MAX_PAGES;
	float loadedProcPercentage = 100.0 * loadedProc / PROCESS_COUNT;
	float unloadedProcPercentage = 100.0 * (PROCESS_COUNT - loadedProc) / PROCESS_COUNT;
	float deadProcPercentage = 100.0 * deadProc / PROCESS_COUNT;

	printf("QUANTA ELAPSED: %i\n", runTime);
	printf("FRAMES:%8if     USED:%5if (%5.1f%%)   FREE:%7if (%5.1f%%)\n", MAX_FRAMES, usedFrames, usedFramesPercentage, numFreeFrames, freeFramesPercentage);
	printf("SWAP SPACE:%4ip     PAGES:%4ip (%5.1f%%)   LOADED:%5ip (%5.1f%%)  UNLOADED:%4ip (%5.1f%%)   FREE:%4ip (%5.1f%%)\n", MAX_PAGES, numOfPages, numOfPagesPercentage, usedFrames, pagesLoadedPercentage, (numOfPages - usedFrames), pagesUnloadedPercentage, (MAX_PAGES - numOfPages), pagesFreePercentage);
	printf("PROCESSES:%5i      LOADED:%3i  (%5.1f%%)   UNLOADED:%3i  (%5.1f%%)  DEAD:%8i  (%5.1f%%)\n", PROCESS_COUNT, loadedProc, loadedProcPercentage, (PROCESS_COUNT - loadedProc - deadProc), unloadedProcPercentage, deadProc, deadProcPercentage);
	printf("\nPHYSICAL MEMORY (FRAMES)\n");

	for (size_t i = 0; i < MAX_FRAMES; i += 60)
	{
		for (size_t j = 4; j < 60 && i + j < MAX_FRAMES; j += 5)
		{
			printf((i < 60 ? "        %02lu" : "%10lu"), i + j);
		}

		printf("\n");

		for (size_t j = 0; j < 6 && i + 10 * j < MAX_FRAMES; ++j)
		{
			printf("--------++--------||");
		}

		printf("\n");

		for (size_t j = 0; j < 60 && i + j < MAX_FRAMES; ++j)
		{
			printf("%c%c", memory.memArray[i + j]->processName, memory.memArray[i + j]->suffix);
		}

		printf("\n");
	}
}

// TODO insert page function for BackingStore

BackingStore::BackingStore() : freeIndex(0)
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

	return fifo();	// returns an index value of the recently freed frame
	//return lru();
	//return secondChance();
}

Page::Page() : processName(EMPTY_PROCESS_NAME), suffix(' '), frameNum(-1), valid(false), refByte(0), startTime(-1) { }

Page::Page(char processName, char suffix, short frameNum, bool valid, short refByte, int startTime) : processName(processName), suffix(suffix), frameNum(frameNum), valid(valid), refByte(refByte), startTime(startTime) { }

void Page::initialize(char processName, char suffix)
{
	this->processName = processName;
	this->suffix = suffix;
	valid = false;
	refByte = 0;
	sc = false;
}

void Page::removePage()
{
	processName = EMPTY_PROCESS_NAME;
	suffix = ' ';
	frameNum = -1;
	valid = false;
	refByte = 0;
	startTime = -1;
	sc = false;
}

Process::Process(char name, int lifeTime, int subRoutines) : name(name), lifeTime(lifeTime), deathTime(0), subRoutines(subRoutines), isAlive(false)
{
	for (int i = 0; i < MAX_NUM_PAGES_PER_PROCESS; i++)
	{
		pageIndex[i] = -1;
	}
}

void BackingStore::print(void)
{
	printf("BACKING STORE (PAGES)\n");

	for (size_t i = 0; i < MAX_PAGES; i += 60)
	{
		for (size_t j = 4; j < 60 && i + j < MAX_PAGES; j += 5)
		{
			printf((i < 60 ? "        %02lu" : "%10lu"), i + j);
		}

		printf("\n");

		for (size_t j = 0; j < 6 && i + 10 * j < MAX_PAGES; ++j)
		{
			printf("--------++--------||");
		}

		printf("\n");

		for (size_t j = 0; j < 60 && i + j < MAX_PAGES; ++j)
		{
			printf("%c%c", backingStore.pages[i + j].processName, backingStore.pages[i + j].suffix);
		}

		printf("\n");
	}
}

void printPerProcessPageTables(void)
{
	printf("PAGE TABLES\n");

	for (size_t i = 0; i < PROCESS_COUNT - 1; i += PROCS_PER_LINE - 1)
	{
		for (size_t j = i; j - i < PROCS_PER_LINE - 1 && j - i < PROCESS_COUNT; ++j)
		{
			printf(((j - i) % PROCS_PER_LINE == 0 ? "%5c" : "%11c"), vectOfProcesses[j + 1].name);
		}

		printf("\n");

		for (size_t j = 0; j < MAX_NUM_PAGES_PER_PROCESS; ++j)
		{
			for (size_t k = 0; k < PROCS_PER_LINE - 1 && i + k < PROCESS_COUNT - 1; ++k)
			{
				Page p = backingStore.pages[vectOfProcesses[k + 1].pageIndex[j]];

				if (k > 0) printf("|");

				printf("%02lu %03i %c%02x", j, p.frameNum, (p.valid ? 'v' : 'i'), p.refByte);
			}

			printf("\n");
		}

		printf("\n");
	}
}
