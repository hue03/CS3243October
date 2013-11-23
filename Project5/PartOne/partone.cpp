// CS3243 Operating Systems
// Fall 2013
// Project 5: Memory Swapping and Paging, Part 1
// Steven Ng and Hue Moua
// Date: 11/6/2013
// File: partone.cpp

#include <cstdlib>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <deque>

#define MAX_PROCESSES 60
#define PROCESS_COUNT 60
#define MIN_BURST 10
#define MAX_BURST 200
#define MIN_MEMORY_PER_PROC 10
#define MAX_MEMORY_PER_PROC 250
#define MAX_MEMORY 1040
#define MAX_BLOCK_PROC_RATIO 0.50
#define PRINT_INTERVAL 500
#define MAX_QUANTA 50000
#define ENABLE_COMPACTION true //flags whether the program will run the compaction algorithm
#define SEED time(NULL)
#define EMPTY_PROCESS_NAME ' '
#define LOWBYTE_PERCENT 50 //percent of how many processes' size are low
#define MEDBYTE_PERCENT 45 //percent of how many processes' size are med
#define LOWBYTE_SIZE_INTERVAL_PERCENT 5 //computes the upper range of the size for the low byte interval
#define MEDBYTE_SIZE_INTERVAL_PERCENT 57 //computes the upper range of the size for the med byte interval based on the upper range of the low byte size
#define SLEEPTIME 500000
using namespace std;

struct Process {
	char name;
	ushort burst;
	ushort size;
	int start;
	int idleAt;

	Process();
	Process(char name, ushort burst, ushort size, int start, int idleAt);
};

struct freeBlock {
	ushort size;
	int start;

	freeBlock(int start, ushort size);
};
int usedMemory;
int loadedProc;
int largestSize;
int runTime;
int freeBlocks;
int largestFreeBlock;
int smallestFreeBlock;
vector<Process> vectOfProcesses;
vector<freeBlock> vectOfFreeSpace;
deque<Process*> readyQueue; //a queue of pointers to the elements in the vectOfProcesses
Process* mainMemory[MAX_MEMORY]; //gets filled with pointers to the elements in the vectOfProcesses
Process myProcess; //the empty process object

void createProcesses(void);
void zeroFillMemory(int start, int end);
void removeIdle();
void findFreeBlocks();
void firstFit();
void bestFit();
void worstFit();
void compaction();
void recoverToMemory(int);
void printMemoryMap(void);

int main()
{
	srand(SEED);
	cout << "Seed " << SEED << endl;
	createProcesses();
	zeroFillMemory(0, MAX_MEMORY);
	void (*fit)(void);
	int input;
	//double totalratio = 0;
	do
	{
		cout << "Choose a swapping method: " << endl;
		cout << "1. First Fit\n2. Best Fit\n3. Worst Fit" << endl;
		cin >> input;
		switch (input)
		{
			case 1: fit = &firstFit;
				break;
			case 2: fit = &bestFit;
				break;
			case 3: fit = &worstFit;
				break;
			default: cout << "Invalid input. Try again." << endl;
				break;
		}
	} while (input < 1 || input > 3);

	for (runTime = 0; runTime <= MAX_QUANTA; ++runTime)
	{
		findFreeBlocks();
		if (runTime % PRINT_INTERVAL == 0)
		{
			//print the readyQueue
			cout << "List of processes in the readyQueue:" << endl;
			for (uint i = 0; i < readyQueue.size(); i++)
			{
				cout << readyQueue[i]->name << ":";
				cout << "Size: " << readyQueue[i]->size << " ";
				cout << "Start: " << readyQueue[i]->start << " ";
				cout << "Burst time: " << readyQueue[i]->burst << " ";
				cout << "Idle at: " <<  readyQueue[i]->idleAt;
				cout << endl;
			}
			cout << "--------------------------------------------------------------------------------" << endl;
			//print mainMemory
			printMemoryMap();
		}
		fit();
		if (runTime % PRINT_INTERVAL == 0)
		{
			cout << "AFTER INSERTING" << endl;
			//print mainMemory
			printMemoryMap();
		}
		removeIdle();
		findFreeBlocks(); //update the amount of free blocks
		if (runTime % PRINT_INTERVAL == 0)
		{
			cout << "AFTER REMOVAL" << endl;
			//print mainMemory
			printMemoryMap();
		}
		if ((1.0 * freeBlocks / loadedProc) >= MAX_BLOCK_PROC_RATIO && ENABLE_COMPACTION)
		{
			cout << "BEFORE COMPACTION" << endl;
			//print mainMemory
			printMemoryMap();
			compaction();
			cout << "AFTER COMPACTION" << endl;
			//print mainMemory
			printMemoryMap();
		}
		//cout << "Runtime: " << runTime << endl;
		//cout << "--------------------------------------------------------------------------------" << endl;
		usleep(SLEEPTIME);
		//cout << "ratio: " << 1.0 * freeBlocks / loadedProc << endl; for answering questions
		//totalratio += 1.0 * freeBlocks / loadedProc;
	}
	//cout << totalratio << endl;
}

void createProcesses(void)
{
	int memoryPerProcSizeRange = MAX_MEMORY_PER_PROC - MIN_MEMORY_PER_PROC;
	int numLowByte = (int)(PROCESS_COUNT * (LOWBYTE_PERCENT * 0.01));
	int numMedByte = (int)(PROCESS_COUNT * (MEDBYTE_PERCENT * 0.01)); // upper range for the amount of processes in the 45% range
	int lowByteSizeRange = (int)(memoryPerProcSizeRange * (LOWBYTE_SIZE_INTERVAL_PERCENT * 0.01));
	int medByteSizeRange = (int)(memoryPerProcSizeRange * (MEDBYTE_SIZE_INTERVAL_PERCENT * 0.01));
	int highByteSizeRange = (int)(memoryPerProcSizeRange - medByteSizeRange - lowByteSizeRange);

	int burstRange = MAX_BURST - MIN_BURST + 1;

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

		ushort burst;
		ushort size;

		if (0 == i)
		{
			burst = MAX_QUANTA;
			size = 120;
		}
		else if (i <= numLowByte)
		{
			burst = rand() % burstRange + MIN_BURST;
			size= rand() % lowByteSizeRange + MIN_MEMORY_PER_PROC;
		}
		else if (i <= numLowByte + numMedByte)
		{
			burst = rand() % burstRange + MIN_BURST;
			size= rand() % medByteSizeRange + MIN_MEMORY_PER_PROC + lowByteSizeRange;
		}
		else
		{
			burst = rand() % burstRange + MIN_BURST;
			size= rand() % highByteSizeRange + MIN_MEMORY_PER_PROC + lowByteSizeRange + medByteSizeRange;
		}

		Process *process = new Process(name, burst, size, 0, 0);

		vectOfProcesses.push_back(*process);
		readyQueue.push_back(process);
	}
}

void zeroFillMemory(int start, int size)
{
	myProcess = Process(EMPTY_PROCESS_NAME, 0, 0, start, 0); //create "empty" processes to be inserted into free spaces in memory

	Process *p = &myProcess;
	for (int i = 0; i < size; i++)
	{
		mainMemory[start + i] = p; //empty memory points to the empty process object
	}
}

void removeIdle()
{
	for (int i = 120; i < MAX_MEMORY; i++)
	{
		int tempSize = mainMemory[i]->size;
		//cout << "i: " << i << endl;
		if ((mainMemory[i]->idleAt <= runTime) && (tempSize> 0))
		{
			//cout << "Current time " << runTime << endl;
			//cout << "i: " << i << endl;
			//cout << "size: " << tempSize << endl;
			readyQueue.push_back(mainMemory[i]);
			zeroFillMemory(mainMemory[i]->start, tempSize);
			usedMemory -= tempSize;
			loadedProc--;
			//cout << "sizej: " << mainMemory[i + 1]->size << endl;
		}
		/*advancing the iterator by the size of the process to reduce the amount of checks it need to do if the whole process is removed or the process is not ready to be removed.
		advances normally if the space is empty*/
		if (tempSize > 0) //careful, may cause out of bounds/seg fault. i is incremented twice from here and the loop itself
		{				  //using a condition to try and correct this. trying to start where the process starts
			i += tempSize - 1; //could possibly shift i to the start of the process at the beginning of the for loop instead of this if condition
		}
	}
}

void findFreeBlocks()
{
	vectOfFreeSpace.clear(); //wipes the vector clean. not efficient but easy to just find the new holes instead of modifying what is in the vector
	bool found = false;
	int start;
	for (int i = 0; i < MAX_MEMORY; i++)
	{
		if (!found && mainMemory[i]->name == (char)EMPTY_PROCESS_NAME) //finding empty blocks
		{
			start = i;
			found = true;
		}

		if (found && (mainMemory[i]->name != (char)EMPTY_PROCESS_NAME || MAX_MEMORY - 1 == i))
		{
			int size = i - start;

			if (MAX_MEMORY - 1 == i) {
				++size; //at the end of memory. increment by 1 before the loop terminates
			}

			if (vectOfFreeSpace.size() == 0)
			{
				largestFreeBlock = size;
				smallestFreeBlock = size;
			}
			else if (size > largestFreeBlock)
			{
				largestFreeBlock = size;
			}
			else if (size < smallestFreeBlock)
			{
				smallestFreeBlock = size;
			}

			vectOfFreeSpace.push_back(freeBlock(start, size));

			found = false;
		}
	}

	freeBlocks = vectOfFreeSpace.size();
}

void firstFit()
{

	while (readyQueue.size() > 0)
	{
		Process *process = readyQueue.front();
		int processSize = process->size;

		if (processSize > largestFreeBlock)
		{
			break; //no free blocks with enough space
		}

		freeBlock best(-1, 0);
		//cout << "First Fit Start" << endl;
		//printMemoryMap();
		for (uint i = 0; i < vectOfFreeSpace.size(); i++)
		{
			if (vectOfFreeSpace[i].size >= process->size)
			{
				best = vectOfFreeSpace[i];
				int start = best.start;
				process->start = start;
				process->idleAt = runTime + process->burst;

				//cout << "free space: " << vectOfFreeSpace[i].size << endl;
				//cout << "process in Q size: " << readyQueue[0]->size << endl;
				for (int j = 0; j < process->size; j++)
				{
					mainMemory[start + j] = process;
				}
				usedMemory += process->size;
				loadedProc++;
				readyQueue.pop_front();
				findFreeBlocks();
				//cout << "Name: " << process->name << endl;
				//printMemoryMap();
				break;
			}
		}
		if (-1 == best.start) {
			break;
		}
	}
}

void worstFit()
{
	while (readyQueue.size() > 0)
	{
		Process *process = readyQueue.front();
		int processSize = process->size;

		if (processSize > largestFreeBlock) //biggest block can't fit the process
		{
			break;
		}

		freeBlock largest(-1, 0);
		//cout << "Worst Fit Start" << endl;
		//printMemoryMap();
		for (uint i = 0; i < vectOfFreeSpace.size(); i++)
		{
			if (vectOfFreeSpace[i].size == largestFreeBlock)
			{
				//cout << "start: " << tempStart << " size: " << tempSize << endl;

				largest = vectOfFreeSpace[i];
				int start = largest.start;
				process->start = start;
				process->idleAt = runTime + process->burst;

				for (int j = 0; j < process->size; j++)
				{
					mainMemory[start + j] = process;
				}

				usedMemory += process->size;
				loadedProc++;
				readyQueue.pop_front();
				findFreeBlocks();
				//cout << "Name: " << process->name << endl;
				//printMemoryMap();
				break;
			}
		}
		if (-1 == largest.start) {
			break;
		}
	}
}

void bestFit()
{
	while (readyQueue.size() > 0)
	{
		//cout << "Free space vect size" << vectOfFreeSpace.size() << endl;

		Process *process = readyQueue.front();
		int processSize = process->size;

		if (processSize > largestFreeBlock) //no free blocks with enough space
		{
			break;
		}

		freeBlock smallest(-1, MAX_MEMORY + 1);
		//cout << "Best Fit Start" << endl;
		//printMemoryMap();
		for (uint i = 0; i < vectOfFreeSpace.size(); i++)
		{
			int freeSize = vectOfFreeSpace[i].size;
			if (processSize <= freeSize && freeSize < smallest.size)
			{
				smallest = vectOfFreeSpace[i];

				if (freeSize == smallestFreeBlock || freeSize == processSize)
				{
					break;
				}
			}
		}

		if (-1 == smallest.start)
		{
			break;
		}
		else
		{
			int start = smallest.start;
			process->start = start;
			process->idleAt = runTime + process->burst;

			//cout << "beginfor loop" << endl;
			for (int j = 0; j < process->size; j++)
			{
				//cout << "inside for loop" << endl;
				//cout << "1" << endl;
				//cout << "j: " << j << endl;
				mainMemory[start + j] = process;
			}
			//cout << readyQueue[0]->name << endl;
			usedMemory += process->size;
			loadedProc++;
			readyQueue.pop_front();
			findFreeBlocks();
			//cout << "Name: " << process->name << endl;
			//printMemoryMap();
		}
	}
}

void compaction()
{
	int oldVectFreeSize = vectOfFreeSpace.size();
	int startBlock = vectOfFreeSpace.size() - 1; //element/position number to indicate which free block to fill
	int lastIndex = vectOfFreeSpace[startBlock].start - 1; //the starting index position where the loops will start from. subtract by 1 to move off of the free process.
	while (startBlock > -1) //begin first part of compaction. move smallest things to the end of the free block.
	{
		//cout << "Inside while" << endl;
		freeBlock targetBlock = vectOfFreeSpace[startBlock];
		//cout << "1st target " << targetBlock.start << " " << targetBlock.size << endl;
		//cout << "2nd target" << endl;
		Process *targetProcess;
		//cout << "Declare" << endl;
		Process tempProcess;
		targetProcess = &tempProcess;
		targetProcess->size = MAX_MEMORY;
		//cout << "a1" << endl;
		//cout << "a2" << endl;
		for (int i = lastIndex; i > mainMemory[0]->size; i--)
		{
			//cout << "Inside 1st for " << i << endl;
			if (mainMemory[i]->size == 0)
			{
				//cout << "nothing" << endl;
				; //do nothing
			}
			else if (mainMemory[i]->size <= targetBlock.size && mainMemory[i]->size < targetProcess->size)
			{
				targetProcess = mainMemory[i]; //can go to a hole towards the back
				//cout << "2nd if " << targetProcess->size << endl;
				i -= mainMemory[i]->size + 1; //offset by 1 because of double increment
			}
		}
		//cout << "Finished for" << endl;
		if (targetProcess->size != MAX_MEMORY)
		{
			//cout << "moving " << targetProcess->name << endl;
			zeroFillMemory(targetProcess->start, targetProcess->size);
			//targetProcess->start = targetBlock.start;
			//cout <<"Target " << targetBlock.size - 1 << " " << targetBlock.start << endl;
			int fillStart = targetBlock.start + targetBlock.size - 1;
			targetProcess->start = fillStart - targetProcess->size + 1;
			//cout << targetProcess->start << endl;
			for (int j = fillStart; j > (fillStart - targetProcess->size); j--)
			{
				//cout << "j: " << j << endl;
				mainMemory[j] = targetProcess;
			}
			findFreeBlocks();
			//cout << "start from " << lastIndex << endl;
			/*if vector shrinks in size the target block is now the one to the right. could be ok that is why less than instead of != */
			if ((uint)oldVectFreeSize != vectOfFreeSpace.size()) //reduce the times going to the last free block. if the vector grew start over again in case.
			{
				oldVectFreeSize = vectOfFreeSpace.size();
				startBlock = vectOfFreeSpace.size() - 1; //restart back at the end free block in case the vect of free blocks changes size. not efficient
				lastIndex = vectOfFreeSpace[startBlock].start - 1;
			}
			else
			{
				lastIndex = vectOfFreeSpace[startBlock].start - 1;
				//cout << "start from " << lastIndex << endl;
			}
		}
		else
		{
			startBlock--; //if nothing can be moved go down to the next free block to start from
			lastIndex = vectOfFreeSpace[startBlock].start - 1; //subtracted to move off of the empty block
			//cout << "start from " << lastIndex << endl;
		}
		//cout << "Num of blocks: " << freeBlocks << endl;
		//printMemoryMap();
	}
	cout << "End of first phase of compaction." << endl;
	printMemoryMap();
	findFreeBlocks();
	freeBlock targetBlock = vectOfFreeSpace[0];
	int unload = 0;
	while ((targetBlock.start + targetBlock.size) < MAX_MEMORY) //begin second part of compaction. move the first process found from the first block and move that into the first free block
	{
		//freeBlock targetBlock = vectOfFreeSpace[0];
		for (int m = (targetBlock.start + targetBlock.size); m < MAX_MEMORY; m++)
		{
			//cout << m << endl;
			if (mainMemory[m]->size > 0)
			{
				//cout << "found " << mainMemory[m]->name << endl;
				//Process* targetProcess = mainMemory[m];
				int start = mainMemory[m]->start; //saving the start location to use for deletion. the start is going to change in the else clause
				//if (targetProcess->size > targetBlock.size) //if a process cannot move it is removed from memory because it will not be able to move anywhere and full compaction is not possible
				if (mainMemory[m]->size > targetBlock.size) //if a process cannot move it is removed from memory because it will not be able to move anywhere and full compaction is not possible
				{
					unload++;
					cout << "Unload here: Process " << mainMemory[m]->name << " was forcefully removed to finish compaction." << endl;
					readyQueue.push_front(mainMemory[m]);
					zeroFillMemory(start, mainMemory[m]->size);
					findFreeBlocks();
				}
				else
				{
					//for (int n = targetBlock.start; n < (targetBlock.start + targetProcess->size); n++) //begin moving into the frontmost empty block
					for (int n = targetBlock.start; n < (targetBlock.start + mainMemory[m]->size); n++) //begin moving into the frontmost empty block
					{
						//cout << "insert " << n << endl;
						//mainMemory[n] = targetProcess;
						mainMemory[n] = mainMemory[m];
						//cout << mainMemory[n]->name << endl;
						mainMemory[n]->start = targetBlock.start;
					}
					//cout << "main " << start << " " << mainMemory[m]->size << endl;
					zeroFillMemory(start, mainMemory[m]->size);
					findFreeBlocks();
				}
				//printMemoryMap();
				/*for (uint i = 0; i < vectOfFreeSpace.size(); i++)
				{
					cout << "Block: " << vectOfFreeSpace[i].start << " " << vectOfFreeSpace[i].size << endl;
				}*/
				targetBlock = vectOfFreeSpace[0];
				//cout << "start " << targetBlock.start << endl;
				//cout << "Num of blocks: " << freeBlocks << endl;
				//printMemoryMap();
				break;
			}
		}
	}
	if (unload > 0) //begin third optional part. bring the removed process back into memory at the end of all the processes
	{
		cout << "Special case: removed " << unload << " process/es from memory." << endl;
		/*cout << "List of processes in the readyQueue:" << endl;
		for (uint i = 0; i < readyQueue.size(); i++)
		{
			cout << readyQueue[i]->name << ":";
			cout << "Size: " << readyQueue[i]->size << " ";
			cout << "Start: " << readyQueue[i]->start << " ";
			cout << "Burst time: " << readyQueue[i]->burst << " ";
			cout << "Idle at: " <<  readyQueue[i]->idleAt;
			cout << endl;
		}
		cout << "--------------------------------------------------------------------------------" << endl;*/
		recoverToMemory(unload);
		/*cout << "List of processes in the readyQueue:" << endl;
		for (uint i = 0; i < readyQueue.size(); i++)
		{
			cout << readyQueue[i]->name << ":";
			cout << "Size: " << readyQueue[i]->size << " ";
			cout << "Start: " << readyQueue[i]->start << " ";
			cout << "Burst time: " << readyQueue[i]->burst << " ";
			cout << "Idle at: " <<  readyQueue[i]->idleAt;
			cout << endl;
		}
		cout << "--------------------------------------------------------------------------------" << endl;*/
		//printMemoryMap();
		findFreeBlocks();
	}
	cout << "End of code " << targetBlock.start + targetBlock.size << endl;
}

void recoverToMemory(int end)
{
	while (end > 0)
	{
		readyQueue[0]->start = vectOfFreeSpace[0].start; //careful here vectOfFreeSpace[0] might not exist
		for (int j = 0; j < readyQueue[0]->size; j++)
		{
			mainMemory[vectOfFreeSpace[0].start + j] = readyQueue[0]; //careful here vectOfFreeSpace[0] might not exist
		}
		//printMemoryMap();
		findFreeBlocks();
		readyQueue.pop_front();
		end--;
	}
}

void printMemoryMap(void) {
	float usedMemoryPercentage = 100.0 * usedMemory / MAX_MEMORY;
	int freeMemory = MAX_MEMORY - usedMemory;
	float freeMemoryPercentage = 100.0 * freeMemory / MAX_MEMORY;
	float loadedProcPercentage = 100.0 * loadedProc / PROCESS_COUNT;
	int unloadedProc = PROCESS_COUNT - loadedProc;
	float unloadedProcPercentage = 100.0 * unloadedProc / PROCESS_COUNT;
	float blocksProcsRatio = 1.0 * freeBlocks / loadedProc;

		cout << "QUANTA ELAPSED: " << runTime << endl;
	cout << "MEMORY: " << MAX_MEMORY << "b\t" << "USED: " << usedMemory << " (" << usedMemoryPercentage << "%)\tFREE:" << freeMemory << " (" << freeMemoryPercentage << "%)" << endl;
	cout << "PROCESSES: " << PROCESS_COUNT << "\tLOADED: " << loadedProc << " (" << loadedProcPercentage << "%)\tUNLOADED: " << unloadedProc << " (" << unloadedProcPercentage << "%)" << endl;
	cout << "FREE BLOCKS: " << freeBlocks << "\tLARGEST: " << largestFreeBlock << "\tSMALLEST: " << smallestFreeBlock << "\tBLOCKS/PROCS RATIO: " << blocksProcsRatio << endl;
	cout << "         10        20        30        40        50        60        70       79" << endl;
	cout << "|----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----" << endl;
	for (size_t i = 0; i < 80; ++i) cout << mainMemory[i]->name;	cout << endl;
	cout << "         90       100       110       120       130       140       150      159" << endl;
	cout << "|----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----" << endl;
	for (size_t i = 80; i < 160; ++i) cout << mainMemory[i]->name;	cout << endl;
	cout << "        170       180       190       200       210       220       230      239" << endl;
	cout << "|----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----" << endl;
	for (size_t i = 160; i < 240; ++i) cout << mainMemory[i]->name;	cout << endl;
	cout << "        250       260       270       280       290       300       310      319" << endl;
	cout << "|----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----" << endl;
	for (size_t i = 240; i < 320; ++i) cout << mainMemory[i]->name;	cout << endl;
	cout << "        330       340       350       360       370       380       390      399" << endl;
	cout << "|----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----" << endl;
	for (size_t i = 320; i < 400; ++i) cout << mainMemory[i]->name;	cout << endl;
	cout << "        410       420       430       440       450       460       470      479" << endl;
	cout << "|----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----" << endl;
	for (size_t i = 400; i < 480; ++i) cout << mainMemory[i]->name;	cout << endl;
	cout << "        490       500       510       520       530       540       550      559" << endl;
	cout << "|----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----" << endl;
	for (size_t i = 480; i < 560; ++i) cout << mainMemory[i]->name;	cout << endl;
	cout << "        570       580       590       600       610       620       630      639" << endl;
	cout << "|----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----" << endl;
	for (size_t i = 560; i < 640; ++i) cout << mainMemory[i]->name;	cout << endl;
	cout << "        650       660       670       680       690       700       710      719" << endl;
	cout << "|----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----" << endl;
	for (size_t i = 640; i < 720; ++i) cout << mainMemory[i]->name;	cout << endl;
	cout << "        730       740       750       760       770       780       790      799" << endl;
	cout << "|----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----" << endl;
	for (size_t i = 720; i < 800; ++i) cout << mainMemory[i]->name;	cout << endl;
	cout << "        810       820       830       840       850       860       870      879" << endl;
	cout << "|----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----" << endl;
	for (size_t i = 800; i < 880; ++i) cout << mainMemory[i]->name;	cout << endl;
	cout << "        890       900       910       920       930       940       950      959" << endl;
	cout << "|----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----" << endl;
	for (size_t i = 880; i < 960; ++i) cout << mainMemory[i]->name;	cout << endl;
	cout << "        970       980       990      1000      1010      1020      1030     1039" << endl;
	cout << "|----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----" << endl;
	for (size_t i = 960; i < 1040; ++i) cout << mainMemory[i]->name;	cout << endl;
}

Process::Process() : name(' '), burst(0), size(0), start(0), idleAt(0)
{

}

Process::Process(char name, ushort burst, ushort size, int start, int idleAt) : name(name), burst(burst), size(size), start(start), idleAt(idleAt)
{

}

freeBlock::freeBlock(int start, ushort size) : size(size), start(start)
{

}
