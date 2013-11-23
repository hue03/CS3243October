// CS3243 Operating Systems
// Fall 2013
// Project 5: Memory Swapping and Paging, Part 2
// Steven Ng and Hue Moua
// Date: 11/22/2013
// File: parttwo.cpp

#include <stdio.h>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <vector>

#define MAX_PROCESSES 52	// This will not ever change
#define PROCESS_COUNT 45	// useful when debugging to limit # of procs
#define MIN_DEATH_INTERVAL 40
#define MAX_DEATH_INTERVAL 150
#define MAX_FRAMES 280
#define MAX_PAGES 720
#define SHIFT_INTERVAL 10
#define PRINT_INTERVAL 500	// # of cpu quanta between memory map printouts
#define MAX_QUANTA 50000	// # quanta to run before ending simulation
#define SLEEP_LENGTH 250000

#define EMPTY_PROCESS_NAME ' '
#define DEFAULT_NUM_PAGES_PER_PROCESS 10 //The number of code, stack and heap pages combined
#define MAX_NUM_PAGES_PER_PROCESS 20 //The number of code, stack, heap and subroutine pages combined
#define MIN_SUBROUTINES 1 //min number of subroutines a process has
#define MAX_SUBROUTINES 5 //max number of subroutines a process has
#define SEED time(NULL)
#define PROCS_PER_LINE 12	//# of processes to show on one line when printing per process page tables
using namespace std;

typedef unsigned char byte;

struct Page
{
	char processName; //the name of the process that the page belongs to
	char suffix;
	short frameNum;
	bool valid;
	byte refByte;

	Page();
	void initialize(char processName, char suffix);
	void removePage();
};

struct Process
{
	char name;
	int lifeTime; //random time of how much time the process has to live
	int deathTime; //time the process dies
	int subRoutines; //num of sub routines a process has
	bool isAlive;
	int pageIndex[MAX_NUM_PAGES_PER_PROCESS]; //array of int that tells the process where its pages are at in the backing store

	Process(char name, int lifeTime, int subRoutines);
	void initialize();
	void createPages();
	void die(void);
};

struct MainMemory
{
	Page* memArray[MAX_FRAMES]; //array of pointers to pages in the backing store. this has the frames
	int usedFrames;

	MainMemory();
	void emptyMemory(int p);
	int getFreeFrame();
	void print(void);
};

struct BackingStore
{
	Page pages[MAX_PAGES]; //array of physical pages belonging to a process
	int freeIndex;
	int numOfPages;
	int numOfPageFaults;

	BackingStore();
	int getFreePage();
	void removePage(int index);
	void print(void);
};

vector<Process> vectOfProcesses;
MainMemory memory;
BackingStore backingStore;
Page emptyPage; //an empty page object that gets filled into the empty elements in the backing store. frames point to this object initially to indicate that they are empty
deque<int> pageQueue; //a queue of ints that are frame numbers that will be used to determine the order in which a page entered a frame
int runTime;
int loadedProc;
int refBitSet;
int refBitClear;
int deadProc;
int (*pageReplacement)(void);

void createProcesses(void);
void killProcess(void);
void touchProcess(void);
void insertIntoMemory(Page &pg);
void shiftRefByte(void);
int fifo(void);
int lru(void);
int secondChance(void);
void printPerProcessPageTables(void);

int main(void)
{
	srand(SEED);
	cout << SEED << endl;	// TODO test output
	createProcesses();
	int input = 0;
	do
	{
		printf("Choose a page replacement algorithm:\n");
		printf("1. FIFO\n2. LRU\n3. Second Chance\n");
		cin >> input;
	} while (input < 1 || input > 3);

	pageReplacement = &(input == 1 ? fifo :
					   (input == 2 ? lru  : secondChance));

	for (runTime = 0; runTime < MAX_QUANTA; runTime++)
	{
		if (runTime != 0) killProcess();

		if (lru == pageReplacement && runTime % SHIFT_INTERVAL == 0 && runTime != 0)
		{
			shiftRefByte();
			cout << "Bit shifted right" << endl;
		}

		if (runTime % PRINT_INTERVAL == 0)
		{
			memory.print();
			puts("\n");
			printPerProcessPageTables();
			puts("\n");
			backingStore.print();
		}

		touchProcess();
		cout << "Running Time: " << runTime << endl;
		cout << "--------------------------------------------" << endl;
		memory.print();
		backingStore.print();
		printPerProcessPageTables();
		//usleep(SLEEP_LENGTH);

		cout << "Press anything to continue" << endl; //uncomment these to step through the outputs
		cin.ignore();
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

void killProcess(void)
{
	for (uint i = 0; i < vectOfProcesses.size(); i++)
	{
		if (vectOfProcesses[i].deathTime == runTime)
		{
			cout << "killing " << vectOfProcesses[i].name << endl;	// TODO test output
			vectOfProcesses[i].die();
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

	if (!vectOfProcesses[processIndex].isAlive) vectOfProcesses[processIndex].initialize();

	int selectedPage = -1;

	for (int i = 0; i < MAX_NUM_PAGES_PER_PROCESS / 2; i++)
	{
		selectedPage = pickedProcess->pageIndex[i];

		if (!(backingStore.pages[selectedPage].valid))//bring the selected page into memory if necessary
		{
			insertIntoMemory(backingStore.pages[selectedPage]);
		}

		if (lru == pageReplacement)
		{
			backingStore.pages[selectedPage].refByte |= 128; //set the refByte for lru
		}
		else if (secondChance == pageReplacement)
		{
			backingStore.pages[selectedPage].refByte |= 1; //set the refByte for second chance
		}
	}

	int subRoutine = -1;

	if (runTime != 0)
	{
		subRoutine = rand() % pickedProcess->subRoutines; //randomly pick a sub routine of the process from 0 - 4 (sub routine 1 - 5)
		cout << "running subroutine " << subRoutine << endl;// TODO test output

		int selectedSubRoutine = pickedProcess->pageIndex[2 //first page of the sub routine
		        * subRoutine + 10];
		int selectedSubRoutine2 = pickedProcess->pageIndex[2 //second page of the same same routine
		        * subRoutine + 10 + 1];

		if (!(backingStore.pages[selectedSubRoutine].valid)) //bring the first subroutine page into memory if needed
		{
			insertIntoMemory(backingStore.pages[selectedSubRoutine]);
			//cout << backingStore.pages[selectedSubRoutine].frameNum << endl;
		}

		if (lru == pageReplacement)
		{
			backingStore.pages[selectedSubRoutine].refByte |= 128; //set the refByte for lru
		}
		else if (secondChance == pageReplacement)
		{
			backingStore.pages[selectedSubRoutine].refByte |= 1; //set the refByte for second chance
		}

		if (!(backingStore.pages[selectedSubRoutine2].valid)) //bring the second subroutine page into memory if needed
		{
			insertIntoMemory(backingStore.pages[selectedSubRoutine2]);
			//cout << backingStore.pages[selectedSubRoutine2].frameNum << endl;
		}
		if (lru == pageReplacement)
		{
			backingStore.pages[selectedSubRoutine2].refByte |= 128; //set the refByte for lru
		}
		else if (secondChance == pageReplacement)
		{
			backingStore.pages[selectedSubRoutine2].refByte |= 1; //set the refByte for second chance
		}
	}
	else //run all of the kernel's sub routine pages
	{
		//cout << backingStore.pages[0].sc << endl;
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

			if (lru == pageReplacement)
			{
				backingStore.pages[selectedSubRoutine].refByte |= 128; //set the refByte for lru
			}
			else if (secondChance == pageReplacement)
			{
				backingStore.pages[selectedSubRoutine].refByte |= 1; //set the refByte for second chance
			}

			if (!(backingStore.pages[selectedSubRoutine2].valid)) //bring the second subroutine page into memory if needed
			{
				insertIntoMemory(backingStore.pages[selectedSubRoutine2]);
				//cout << backingStore.pages[selectedSubRoutine2].frameNum << endl;
			}

			if (lru == pageReplacement)
			{
				backingStore.pages[selectedSubRoutine2].refByte |= 128; //set the refByte for lru
			}
			else if (secondChance == pageReplacement)
			{
				backingStore.pages[selectedSubRoutine2].refByte |= 1; //set the refByte for second chance
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
	memory.memArray[frame] = pageLocation; //pointer to the inserted page in the backing store
	if (pageLocation->processName != '@') pageQueue.push_back(frame);

	memory.usedFrames++;
}

void shiftRefByte(void)
{
	for (int i = 0; i < MAX_FRAMES; i++)
	{
		// bit shift right
		if (memory.memArray[i]->processName != '@') memory.memArray[i]->refByte >>= 1;
	}
}

int fifo(void)
{
	cout << "running fifo" << endl;
	int victimIndex = -1;
	victimIndex = pageQueue.front();
	pageQueue.pop_front();
	cout << "removing " << memory.memArray[victimIndex]->processName << memory.memArray[victimIndex]->suffix << " j: " << victimIndex << endl;
	memory.memArray[victimIndex]->valid = false;
	memory.memArray[victimIndex]->refByte = 0;
	memory.memArray[victimIndex]->frameNum = -1;
	memory.emptyMemory(victimIndex);
	memory.usedFrames--;
	return victimIndex;
}

int lru(void)
{
	cout << "running lru" << endl;
	int victimIndex = 20; //start at frame 20 to skip the kernel
	byte smallestRef = memory.memArray[victimIndex]->refByte; //biggest number that the refByte can be is 255 (1111 1111). set to the page in the 20th frame

	for (int j = victimIndex + 1; j < MAX_FRAMES; j++)
	{
		if (memory.memArray[j]->refByte < smallestRef
		        && memory.memArray[j]->processName != '@') //find the page with the smallest refByte value since that is the least recently used
		{
			smallestRef = memory.memArray[j]->refByte;
			victimIndex = j;
		}
	}
	cout << "removing " << memory.memArray[victimIndex]->processName << memory.memArray[victimIndex]->suffix << " j: " << victimIndex << endl;
	memory.memArray[victimIndex]->valid = false;
	memory.memArray[victimIndex]->refByte = 0;
	memory.memArray[victimIndex]->frameNum = -1;
	memory.emptyMemory(victimIndex);
	memory.usedFrames--;
	return victimIndex;
}

int secondChance(void)
{
	cout << "running second chance" << endl;
	int victimIndex = -1;
	while (true)
	{
		victimIndex = pageQueue.front(); //get the first page that entered into a frame
		if (memory.memArray[victimIndex]->refByte) //if not 0 or > 0
		{
			memory.memArray[victimIndex]->refByte &= 0; //reset the refByte to 0
			pageQueue.push_back(victimIndex); //insert the frame index to the back of the queue
			pageQueue.pop_front(); //remove the checked index from the front of the queue
			cout << "give second chance" << endl;
		}
		else
		{
			pageQueue.pop_front();
			break;
		}
	}
	cout << "removing " << memory.memArray[victimIndex]->processName << memory.memArray[victimIndex]->suffix << " j: " << victimIndex << endl;
	memory.memArray[victimIndex]->valid = false;
	memory.memArray[victimIndex]->refByte = 0;
	memory.memArray[victimIndex]->frameNum = -1;
	memory.emptyMemory(victimIndex);
	memory.usedFrames--;
	return victimIndex;
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
				Page p = backingStore.pages[vectOfProcesses[i + k + 1].pageIndex[j]];

				if (k > 0) printf("|");

				printf("%02lu %03i %c%02x", j, p.frameNum, (p.valid ? 'v' : 'i'), p.refByte);
			}

			printf("\n");
		}

		printf("\n");
	}
}

Process::Process(char name, int lifeTime, int subRoutines) : name(name), lifeTime(lifeTime), deathTime(0), subRoutines(subRoutines), isAlive(false)
{
	for (int i = 0; i < MAX_NUM_PAGES_PER_PROCESS; i++)
	{
		pageIndex[i] = -1; //initialize the process's page index values to -1 as a way for saying its pages are not in the backing store
	}
}

void Process::initialize(void)
{
	// if deathTime > 0, then touching a dead process
	if (deathTime > 0)
	{
		deadProc--;
	}

	isAlive = true;
	loadedProc++;
	createPages();
	deathTime = runTime + lifeTime;
}

void Process::createPages()
{
	for (int i = 0; i < DEFAULT_NUM_PAGES_PER_PROCESS + 2 * subRoutines; i++)
	{
		// Set suffix of page based on the page index i
		char suffix = ('@' == name ? '@' :
						   (i <  2 ? '0' :
						   (i <  5 ? '1' :
						   (i < 10 ? '2' :
						   (i < 12 ? '3' :
						   (i < 14 ? '4' :
						   (i < 16 ? '5' :
						   (i < 18 ? '6' : '7'))))))));

		int freeIndex = backingStore.getFreePage();

		pageIndex[i] = freeIndex; //update the process's page index with the index position of where the page is at in the backing store
		backingStore.pages[freeIndex].initialize(name, suffix); //the backing store has an "empty" page in it so change that empty page's attributes to the selected process's name and suffix
		backingStore.numOfPages++;
	}
}

void Process::die(void)
{
	for (int i = 0; i < MAX_NUM_PAGES_PER_PROCESS; i++)	// go through dying process's pageIndex
	{
		if (pageIndex[i] != -1) //skip the the pages not in the backing store (-1)
		{
			backingStore.removePage(pageIndex[i]);	// remove the process's page from the backing store
			pageIndex[i] = -1; //clear the process's page index at i
		}
	}

	isAlive = false;
	loadedProc--;
	deadProc++;
}

MainMemory::MainMemory() : usedFrames(0)
{
	for (int i = 0; i < MAX_FRAMES; i++)
	{
		memArray[i] = &emptyPage; //have each frame in memory point to the empty page object to indicate that the frame is empty
	}
}

void MainMemory::emptyMemory(int p)
{
	memArray[p] = &emptyPage; //the selected frame index is emptied by pointing to the empty page object
	usedFrames--;
}

int MainMemory::getFreeFrame()
{
	for (int i = 0; i < MAX_FRAMES; i++)
	{
		if (memArray[i]->suffix == ' ')
		{
			return i; //if a free frame is already available return the index position
		}
	}

	backingStore.numOfPageFaults++;

	return pageReplacement(); //if reached here there are no free frames available so a page replacement algorithm is executed
}

void MainMemory::print(void)
{
	float usedFramesPercentage = 100.0 * usedFrames / MAX_FRAMES;
	int numFreeFrames = MAX_FRAMES - usedFrames;
	float freeFramesPercentage = 100.0 * numFreeFrames / MAX_FRAMES;
	float numOfPagesPercentage = 100.0 * backingStore.numOfPages / MAX_PAGES;
	float pagesLoadedPercentage = 100.0 * usedFrames / MAX_PAGES;
	float pagesUnloadedPercentage = 100.0 * (backingStore.numOfPages - usedFrames) / MAX_PAGES;
	float pagesFreePercentage = 100.0 * (MAX_PAGES - backingStore.numOfPages) / MAX_PAGES;
	float loadedProcPercentage = 100.0 * loadedProc / PROCESS_COUNT;
	float unloadedProcPercentage = 100.0 * (PROCESS_COUNT - loadedProc) / PROCESS_COUNT;
	float deadProcPercentage = 100.0 * deadProc / PROCESS_COUNT;

	printf("QUANTA ELAPSED: %i\n", runTime);
	printf("FRAMES:%8if     USED:%5if (%5.1f%%)   FREE:%7if (%5.1f%%)\n", MAX_FRAMES, usedFrames, usedFramesPercentage, numFreeFrames, freeFramesPercentage);
	printf("SWAP SPACE:%4ip     PAGES:%4ip (%5.1f%%)   LOADED:%5ip (%5.1f%%)  UNLOADED:%4ip (%5.1f%%)   FREE:%4ip (%5.1f%%)\n", MAX_PAGES, backingStore.numOfPages, numOfPagesPercentage, usedFrames, pagesLoadedPercentage, (backingStore.numOfPages - usedFrames), pagesUnloadedPercentage, (MAX_PAGES - backingStore.numOfPages), pagesFreePercentage);
	printf("PROCESSES:%5i      LOADED:%3i  (%5.1f%%)   UNLOADED:%3i  (%5.1f%%)  DEAD:%8i  (%5.1f%%)\n", PROCESS_COUNT, loadedProc, loadedProcPercentage, (PROCESS_COUNT - loadedProc - deadProc), unloadedProcPercentage, deadProc, deadProcPercentage);
	printf("PAGE FAULTS:%4i", backingStore.numOfPageFaults);
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

Page::Page() : processName(EMPTY_PROCESS_NAME), suffix(' '), frameNum(-1), valid(false), refByte(0){ }

void Page::initialize(char processName, char suffix)
{
	this->processName = processName;
	this->suffix = suffix;
	valid = false;
	refByte = 0;
}

BackingStore::BackingStore() : freeIndex(0), numOfPages(0), numOfPageFaults(0)
{
	for (size_t i = 0; i < MAX_PAGES; ++i)
	{
		pages[i] = Page(); //fill the backing store with default/empty pages
	}
}

int BackingStore::getFreePage()
{
	for (size_t i = 0; i < MAX_PAGES; ++i)
	{
		//printf("%5lu | %10c | %6c | %6i | %5c | %9x | %5x\n", i, pages[i].processName, pages[i].suffix, pages[i].frameNum, (pages[i].valid ? 'v' : 'i'), pages[i].refByte/*, pages[i].startTime*/);
		if (' ' == pages[i].suffix)
		{
			return i;
		}
	}

	fprintf(stderr, "The Backing Store is Full!");
	cout << "Press any key to proceed with terminating the program to prevent overflow." << endl;
	cin.ignore();
	exit(0);
}

void BackingStore::removePage(int index)
{
	Page *page = &pages[index];

	if (page->valid)	// if the page is in a frame
	{
		memory.emptyMemory(page->frameNum); //from the index, remove the page from the frame
	}

	page->processName = EMPTY_PROCESS_NAME;
	page->suffix = ' ';
	page->frameNum = -1;
	page->valid = false;
	page->refByte = 0;

	numOfPages--;
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
