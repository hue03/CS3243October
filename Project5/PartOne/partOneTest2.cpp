#include <cstdlib>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <iostream>
#include <vector>
#include <deque>  

#define MAX_PROCESSES 60
#define PROCESS_COUNT 50
#define MIN_BURST 5
#define MAX_BURST 15
#define MIN_MEMORY_PER_PROC 4
#define MAX_MEMORY_PER_PROC 160
#define MAX_MEMORY 1040
#define MAX_BLOCK_PROC_RATIO 0.30
#define PRINT_INTERVAL 5
#define MAX_QUANTA 50
#define ENABLE_COMPACTION true //flags whether the program will run the compaction algorithm
#define SEED 2

#define LOWBYTE_PERCENT 50
#define MEDBYTE_PERCENT 45
//#define HIGHBYTE_PERCENT 5
#define LOWBYTE_SIZE_INTERVAL_PERCENT 5
#define MEDBYTE_SIZE_INTERVAL_PERCENT 57

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

struct FreeBlock {
	ushort size;
	int start;

	FreeBlock(int start, ushort size);
};
//timeval stop, start;
int usedMemory;
int loadedProc;
int largestSize;
int runTime;
int freeBlocks;
int largestFreeBlock;
int smallestFreeBlock;
vector<Process> vectOfProcesses;
vector<FreeBlock> vectOfFreeSpace;
deque<Process*> readyQueue;
Process* mainMemory[MAX_MEMORY];
Process myProcess;

//void assignName();
//void assignSize();
//void assignBurst();
void loadQueue();
void createProcesses(void);
void zeroFillMemory(int start, int end);
void fillMemory();
void removeIdle();
void findFreeBlocks();
void sortByIdle(int left, int right);
void firstFit();
void bestFit();
void worstFit();
void compaction();
void recoverToMemory(int);
void printMemoryMap(void);

int main()
{
//	assignName();
	srand(SEED);
//	assignSize();
//	assignBurst();
	createProcesses();
	loadQueue();
	zeroFillMemory(0, MAX_MEMORY);
//	findFreeBlocks();
	//fillMemory();
	//gettimeofday(&start, NULL);
//	runTime = 0;
	void (*fit)(void);
	int input;
	do
	{
		cout << "Choose a swapping method: " << endl;
		cout << "1. First Fit\n2. Best Fit\n3. Worst Fit" << endl;
//		int input;
		cin >> input;
		switch (input)
		{
//			case 1: firstFit();
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
	//print vectOfProcesses
	/*cout << "List of every processes" << endl;
	for (int i = 0; i < MAX_PROCESSES; i++)
	{
		cout << "Process size: " << vectOfProcesses[i].idleAt << endl;
	}
	cout << "--------------------------------------------------------------------------------" << endl;
	*/
	//print the readyQueue
	/*cout << "List of processes in the readyQueue" << endl;
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
	*/
//	int printCount = 0;
//	while (runTime < MAX_QUANTA)
	for (runTime = 0; runTime < MAX_QUANTA; ++runTime)
	{
		findFreeBlocks();
		//print list of free blocks
//		if (printCount % PRINT_INTERVAL == 0)
		if (runTime % PRINT_INTERVAL == 0)
		{
			//print list of free blocks
			cout << "List of free blocks" << endl;
			for (uint i = 0; i < vectOfFreeSpace.size(); i++)
			{
				cout << "Block start: " << vectOfFreeSpace[i].start << " Block size: " << vectOfFreeSpace[i].size << endl;
			}
			cout << "--------------------------------------------------------------------------------" << endl;

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
//		if (printCount % PRINT_INTERVAL == 0)
//		{
//			//print the readyQueue
//			cout << "List of processes in the readyQueue:" << endl;
//			for (uint i = 0; i < readyQueue.size(); i++)
//			{
//				cout << readyQueue[i]->name << ":";
//				cout << "Size: " << readyQueue[i]->size << " ";
//				cout << "Start: " << readyQueue[i]->start << " ";
//				cout << "Burst time: " << readyQueue[i]->burst << " ";
//				cout << "Idle at: " <<  readyQueue[i]->idleAt;
//				cout << endl;
//			}
//			cout << "--------------------------------------------------------------------------------" << endl;
//			//print mainMemory
//			printMemoryMap();
//		}
		removeIdle();
//		findFreeBlocks(); //need this to update the block status
//		if (printCount % PRINT_INTERVAL == 0)
		if (runTime % PRINT_INTERVAL == 0)
		{
			cout << "AFTER REMOVAL" << endl;
			//print mainMemory
			printMemoryMap();
		}
		fit();
//		findFreeBlocks();
//		firstFit();
		//worstFit();
//		bestFit();
//		if (printCount % PRINT_INTERVAL == 0)
		if (runTime % PRINT_INTERVAL == 0)
		{
			cout << "AFTER INSERTING" << endl;
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
//		printCount++;
//		runTime++;
		//cout << "Runtime: " << runTime << endl;
	}
	cout << "Runtime: " << runTime << endl;
	//cout << "Time Ended: " << stop.tv_usec << endl;
	cout << "--------------------------------------------------------------------------------" << endl;
}


//void assignName()
//{
//	char name = '?';
//
//	for (int i = 0; i < PROCESS_COUNT; i++)
//	{
//		myProcess.start = MIN_MEMORY_PER_PROC * (i + 1);
//
//		switch(name)
//		{
//		case '@':
//			name = '1';
//			break;
//		case '9':
//			name = 'A';
//			break;
//		case 'Z':
//			name = 'a';
//			break;
//		case 'H':
//		case 'k':
//			name += 2;
//			break;
//		default:
//			name +=1;
//			break;
//		}
//
//		myProcess.name = name;
//		vectOfProcesses.push_back(myProcess);
//	}
//}
//
//void assignSize()
//{
//	//srand(getpid());
//	int memoryPerProcSizeRange = MAX_MEMORY_PER_PROC - MIN_MEMORY_PER_PROC;
//	int highByteSizeIntervalPercent = MAX_MEMORY_PER_PROC - MEDBYTE_SIZE_INTERVAL_PERCENT - LOWBYTE_SIZE_INTERVAL_PERCENT;
//	int majority = MAX_PROCESSES * (LOWBYTE_PERCENT * 0.01);
//	int numLowByte = MAX_PROCESSES * (LOWBYTE_PERCENT * 0.01);
//	int secondMajority = (int)(MAX_PROCESSES * ((LOWBYTE_PERCENT - HIGHBYTE_PERCENT) * 0.01) + majority + 1); // upper range for the amount of processes in the 45% range
//	int numMedByte = (int)(MAX_PROCESSES * (MEDBYTE_PERCENT * 0.01)); // upper range for the amount of processes in the 45% range
//	int thirdMajority = (int)(MAX_PROCESSES * (HIGHBYTE_PERCENT * 0.01)) + secondMajority; //not adding 1 for the kernel (120B), same comment as above
	//cout << majority << endl;
	//cout << secondMajority << endl;
	//cout << thirdMajority << endl;
//	int minMax, midMin, midMax;
//	minMax = MIN_MEMORY_PER_PROC + (int)((MAX_MEMORY_PER_PROC - MIN_MEMORY_PER_PROC) * (LOWBYTE_SIZE_INTERVAL_PERCENT * 0.01));
//	minMax = MIN_MEMORY_PER_PROC + (int)((MAX_MEMORY_PER_PROC - MIN_MEMORY_PER_PROC) * (LOWBYTE_SIZE_INTERVAL_PERCENT * 0.01));
//	int lowByteSizeRange = (int)(memoryPerProcSizeRange * (LOWBYTE_SIZE_INTERVAL_PERCENT * 0.01));
//	midMin = minMax + 1;
//	midMax = midMin + (int)((MAX_MEMORY_PER_PROC - MIN_MEMORY_PER_PROC) * (MEDBYTE_SIZE_INTERVAL_PERCENT * 0.01));
//	int medByteSizeRange = (int)(memoryPerProcSizeRange * (MEDBYTE_SIZE_INTERVAL_PERCENT * 0.01));
//	int highByteSizeRange = (int)(memoryPerProcSizeRange * (highByteSizeIntervalPercent * 0.01));
//
//	vectOfProcesses[0].size = 120;
//	for (int i = 1; i <= majority; i++)
//	for (int i = 0; i < PROCESS_COUNT; i++)
//	{
//		ushort tempSize = rand() % (minMax - MIN_MEMORY_PER_PROC + 1) + MIN_MEMORY_PER_PROC;
//		ushort tempSize;
//
//		if (0 == i)
//		{
//			tempSize = 120;
//		}
//		else if (i <= numLowByte)
//		{
//			tempSize= rand() % lowByteSizeRange + MIN_MEMORY_PER_PROC;
//		}
//		else if (i <= numLowByte + numMedByte)
//		{
//			tempSize= rand() % medByteSizeRange + MIN_MEMORY_PER_PROC + lowByteSizeRange;
//		}
//		else
//		{
//			tempSize= rand() % highByteSizeIntervalPercent + MIN_MEMORY_PER_PROC + lowByteSizeRange + medByteSizeRange;
//		}
//
//		vectOfProcesses[i].size = tempSize;
//	}
//
//	for (int j = majority + 1; j < secondMajority; j++)
//	{
//		ushort tempSize = rand() % (midMax - midMin + 1) + midMin;
//		vectOfProcesses[j].size = tempSize;
//	}
//
//	for (int k = secondMajority; k < thirdMajority; k++)
//	{
//		ushort tempSize = rand() % (MAX_MEMORY_PER_PROC - (midMax + 1) + 1) + (midMax + 1);
//		vectOfProcesses[k].size = tempSize;
//	}
//}
//
//void assignBurst()
//{
//	vectOfProcesses[0].burst = MAX_QUANTA;
//	//srand(getpid() * time(NULL));
//	for (uint i = 1; i < vectOfProcesses.size(); i++)
//	{
//		ushort tempBurst = rand() % (MAX_BURST - MIN_BURST + 1) + MIN_BURST;
//		vectOfProcesses[i].burst = tempBurst;
//	}
//}

void createProcesses(void)
{
//	int highByteSizeIntervalPercent = 100 - MEDBYTE_SIZE_INTERVAL_PERCENT - LOWBYTE_SIZE_INTERVAL_PERCENT;
	int numLowByte = (int)(PROCESS_COUNT * (LOWBYTE_PERCENT * 0.01));
	int numMedByte = (int)(PROCESS_COUNT * (MEDBYTE_PERCENT * 0.01)); // upper range for the amount of processes in the 45% range
	int memoryPerProcSizeRange = MAX_MEMORY_PER_PROC - MIN_MEMORY_PER_PROC + 1;
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

//		Process process(name, burst, size, 0, 0);

		vectOfProcesses.push_back(Process(name, burst, size, 0, 0));
//		vectOfProcesses.push_back(process);

//		readyQueue.push_back(&vectOfProcesses.back());
//		readyQueue.push_back(&process);
	}
}

void loadQueue()
{
//	Process *proc_ptr;
	for (uint i = 0; i < PROCESS_COUNT; i++)
	{
//		vectOfProcesses[i].start = 0;
//		vectOfProcesses[i].idleAt = 0;
//		proc_ptr = &vectOfProcesses[i];
//		readyQueue.push_back(proc_ptr);
		readyQueue.push_back(&vectOfProcesses.at(i));
	}
}

void zeroFillMemory(int start, int size)
{
//	Process process = new Process(' ', 0, size, start, -1);

//	Process *p;
	Process *p = new Process(' ', 0, size, start, -1);
	for (int i = 0; i < size; i++)
	{
//		p = &myProcess;
//		p->name = ' ';
//		p->size = 0;
//		p->burst = 0;
//		p->start = start;
//		p->idleAt = 0;
		mainMemory[start + i] = p;
		//cout << "i0: " << i << endl;
	}
}

void fillMemory()
{
	int startIndex = 0;
	for (uint i = 0; i < readyQueue.size(); i++)
	{
		short tempSize = readyQueue[i]->size;
		//cout << tempSize << endl;
		for (int j = startIndex; j < MAX_MEMORY; j++)
		{
		//cout << tempSize << endl;
			if (tempSize < 0)
			{
				//cout << "Enough space." << endl;
				short range = startIndex + readyQueue[i]->size;
				readyQueue[i]->start = startIndex;
				//gettimeofday(&start, NULL);
				readyQueue[i]->idleAt = runTime + readyQueue[i]->burst;
				for (int k = startIndex; k < range; k++)
				{
					mainMemory[k] = readyQueue[i];
				}
				startIndex = --j; //offset by 1 because j is past the beginning of the first spot of free space
				usedMemory += readyQueue[i]->size;
				loadedProc++;
				readyQueue.erase(readyQueue.begin() + i);
				i--; //makes the first process in queue next because the queue shrunk and would get skipped
				break;
			}
			else if (mainMemory[j]->size == 0)
			{
				tempSize--;
				//cout << "Empty space" << endl;
			}
			//else
			//{
				//startIndex = ++j; //need to change this because it is inefficient
			//	cout << "No free space" << endl;
			//	break;
			//}
		}
	if ((MAX_MEMORY - usedMemory) < 11)
	{
		cout << "Stop filling in memory. Free space is <11. CAUTION: THIS IS ASSUMING THAT THE PROCESSES NEAR THE END OF THE QUEUE HAS A SIZE OF >=11!!!" << endl;
		break;
	}
	}
}

void removeIdle()
{
//	for (int i = 120; i < MAX_MEMORY; i++)
	for (int i = 0; i < MAX_MEMORY; i++)
	{
		int tempSize = mainMemory[i]->size;
		//cout << "i: " << i << endl;
//		if ((mainMemory[i]->idleAt <= runTime) && (tempSize> 0))
		if (mainMemory[i]->idleAt == runTime)
		{
			//cout << "Current time " << runTime << endl;
			//cout << "i: " << i << endl;
			//cout << "size: " << tempSize << endl;
			readyQueue.push_back(mainMemory[i]);
//			zeroFillMemory(mainMemory[i]->start, tempSize);

			int nextIndex = mainMemory[i]->start + tempSize;

			if (nextIndex < MAX_MEMORY && ' ' == mainMemory[nextIndex]->name)
			{
				Process *p = mainMemory[nextIndex];
				zeroFillMemory(mainMemory[i]->start, tempSize + p->size);
				delete p;
			}
			else
			{
				zeroFillMemory(mainMemory[i]->start, tempSize);
			}
			//Process *p;
			/*for (int j = mainMemory[i]->start; j < mainMemory[i]->start + tempSize; j++)
			{
				p = &myProcess;
				p->name = ' ';
				p->size = 0;
				p->burst = 0;
				p->start = 0;
				p->idleAt = 0;
				mainMemory[j] = p;
				cout << "j: " << j << endl;
			}*/
			usedMemory -= tempSize;
			loadedProc--;
			//cout << "sizej: " << mainMemory[i + 1]->size << endl;
		}
		/*advancing the iterator by the size of the process to reduce the amount of checks it need to do if the whole process is removed or the process is not ready to be removed.
		advances normally if the space is empty*/
//		if (tempSize > 0) //careful, may cause out of bounds/seg fault. i is incremented twice from here and the loop itself
//		{				  //using a condition to try and correct this. trying to start where the process starts
		i += tempSize - 1; //could possibly shift i to the start of the process at the beginning of the for loop instead of this if condition
//		}
	}

	findFreeBlocks();
}

void findFreeBlocks()
{
//	vectOfFreeSpace.erase(vectOfFreeSpace.begin(), vectOfFreeSpace.begin() + vectOfFreeSpace.size());
	vectOfFreeSpace.clear();
//	largestFreeBlock = 0;
//	smallestFreeBlock = 0;
//	int count = 0;
//	bool found = false;
//	int start;
//	int size;
//	int i;
//	for (i = 0; i < MAX_MEMORY; i++)
	for (int i = 0; i < MAX_MEMORY; i++)
	{
//		if (mainMemory[i]->size == 0)
		if (mainMemory[i]->name == ' ')
		{
			int size = mainMemory[i]->size;

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

			vectOfFreeSpace.push_back(FreeBlock(mainMemory[i]->start, size));
		}
//		else

		i += mainMemory[i]->size - 1;

//		if (found && (mainMemory[i]->name != ' ' || MAX_MEMORY - 1 == i))
//		{
//			int size = i - start;
//
//			if (MAX_MEMORY - 1 == i) {
//				++size;
//			}
//
//			if (vectOfFreeSpace.size() == 0)
//			{
//				largestFreeBlock = size;
//				smallestFreeBlock = size;
//			}
//			else if (size > largestFreeBlock)
//			{
//				largestFreeBlock = size;
//			}
//			else if (size < smallestFreeBlock)
//			{
//				smallestFreeBlock = size;
//			}
//
//			vectOfFreeSpace.push_back(freeBlock(start, size));
//
//			found = false;
//
//			if (count != 0)
//			{
//				freeBlock block;
//				block.size = count;
//				block.start = i - count;
//				vectOfFreeSpace.push_back(block);
//				if (count > largestFreeBlock)
//				{
//					largestFreeBlock = count;
//					if (smallestFreeBlock == 0)
//					{
//						smallestFreeBlock = count;
//					}
//				}
//				else if (count < smallestFreeBlock)
//				{
//					smallestFreeBlock = count;
//				}
//				freeBlocks = vectOfFreeSpace.size();
//			}
//			i += mainMemory[i]->size - 1; //offset by 1 because loop will increment i
//			count = 0;
//		}
//
//		if (mainMemory[i]->name != ' ')
//		{
//			i += mainMemory[i]->size - 1;
//		}
	}

	freeBlocks = vectOfFreeSpace.size();

//	if (count != 0) //handles the case where you go past the end of memory or else the data won't be saved
//	{
//		freeBlock block;
//		block.size = count;
//		block.start = i - count;
//		vectOfFreeSpace.push_back(block);
//		if (count > largestFreeBlock)
//		{
//			largestFreeBlock = count;
//			if (smallestFreeBlock == 0)
//			{
//				smallestFreeBlock = count;
//			}
//		}
//		else if (count < smallestFreeBlock)
//		{
//			smallestFreeBlock = count;
//		}
//		freeBlocks = vectOfFreeSpace.size();
//	}
}

//Algorithm derived from www.algolist.net/Algorithms/Sorting/Quicksort
void sortByIdle(int left, int right)
{
	int i = left, j = right;
	Process* tmp;
	int pivot = readyQueue[(left + right) / 2]->idleAt;

	/* partition */
	while (i <= j) 
	{
		while (readyQueue[i]->idleAt < pivot)
		{
			i++;
		}
            	while (readyQueue[j]->idleAt > pivot)
		{
                	j--;
		}

            	if (i <= j)
		{
                  	tmp = readyQueue[i];
                  	readyQueue[i] = readyQueue[j];
                  	readyQueue[j] = tmp;
                  	i++;
                  	j--;
           	}
      	};

    /* recursion */
	if (left < j)
	{
		sortByIdle(left, j);
	}

      	if (i < right)
	{
        sortByIdle(i, right);
	}
}

void firstFit()
{
//	bool flag = true;
//	uint queueSize = readyQueue.size();
//	while (flag && queueSize > 0)
	while (readyQueue.size() > 0)
	{
		Process *process = readyQueue.front();
		int processSize = process->size;

		if (processSize > largestFreeBlock)
		{
			break;
		}

		FreeBlock best(-1, 0);

		for (uint i = 0; i < vectOfFreeSpace.size(); i++)
		{
//			if (vectOfFreeSpace[i].size >= readyQueue[0]->size)
			if (vectOfFreeSpace[i].size >= process->size)
			{
				best = vectOfFreeSpace[i];
				int start = best.start;
				process->start = start;
				process->idleAt = runTime + process->burst;

				if (process->size == best.size)
				{
					delete mainMemory[start];
				}
				else
				{
					mainMemory[start]->start += process->size;
					mainMemory[start]->size -= process->size;
				}

				//cout << "free space: " << vectOfFreeSpace[i].size << endl;
				//cout << "process in Q size: " << readyQueue[0]->size << endl;
//				for (int j = vectOfFreeSpace[i].start; j < (vectOfFreeSpace[i].start + readyQueue[0]->size); j++)
				for (int j = 0; j < process->size; j++)
				{
					//cout << "j: " << j << endl;
//					readyQueue[0]->start = vectOfFreeSpace[i].start;
//					readyQueue[0]->idleAt = runTime + readyQueue[0]->burst;
//					mainMemory[j] = readyQueue[0];
					mainMemory[start + j] = process;
				}
//				usedMemory += readyQueue[0]->size;
				usedMemory += process->size;
				loadedProc++;
//				readyQueue.erase(readyQueue.begin());
				readyQueue.pop_front();
				findFreeBlocks();
				//tempSize = readyQueue[0];

				break;
			}
		}
		//cout << "outside for loop" << endl;
//		if (queueSize == readyQueue.size())
//		{
			//cout << "Q " << queueSize << endl;
//			flag = false;
//		}
//		else
//		{
//			queueSize = readyQueue.size();
//		}

		if (-1 == best.start) {
			break;
		}
	}
	
	/*int startIndex = 0;
	bool flag = true;
	uint queueSize = readyQueue.size();
	while (flag && queueSize > 0)
	{
		short tempSize = readyQueue[0]->size;
		//cout << "Q " << queueSize << endl;
		for (int j = startIndex; j < MAX_MEMORY; j++)
		{
		//cout << tempSize << endl;
			if (tempSize < 0)
			{
				//cout << "Enough space." << endl;
				short range = startIndex + readyQueue[0]->size;
				readyQueue[0]->start = startIndex;
				readyQueue[0]->idleAt = runTime + readyQueue[0]->burst;
				for (int k = startIndex; k < range; k++)
				{
					mainMemory[k] = readyQueue[0];
				}
				startIndex = --j; //offset by 1 because j is past the beginning of the first spot of free space
				usedMemory += readyQueue[0]->size;
				loadedProc++;
				readyQueue.erase(readyQueue.begin());
				//i--; //makes the first process in queue next because the queue shrunk and would get skipped
				break;
			}
			else if (mainMemory[j]->size == 0)
			{
				tempSize--;
				//cout << "Empty space" << endl;
			}
			else
			{
				tempSize = readyQueue[0]->size; //reset the tempSize counter because there is not enough contiguous memory
                j += mainMemory[j]->size - 1; //offset by 1 because loop will increment after this
                startIndex = j + 1; //set the start index to where j will be after it gets changed
			}
		}
		//cout << "outside for loop" << endl;
		if (queueSize == readyQueue.size())
		{
			//cout << "Q " << queueSize << endl;
			flag = false;
		}
		else
		{
			queueSize = readyQueue.size();
		}
	}*/
}

void worstFit()
{
//	bool flag = true;
//	int tempSize = 0;
//	int tempStart = 0;
//	uint queueSize = readyQueue.size();
//	while (flag && queueSize > 0)
	while (readyQueue.size() > 0)
	{
		Process *process = readyQueue.front();
		int processSize = process->size;

		if (processSize > largestFreeBlock)
		{
			break;
		}

		FreeBlock largest(-1, 0);

		for (uint i = 0; i < vectOfFreeSpace.size(); i++)
		{
//			if (vectOfFreeSpace[i].size > tempSize)
			if (vectOfFreeSpace[i].size == largestFreeBlock)
			{
//				tempSize = vectOfFreeSpace[i].size;
//				tempStart = vectOfFreeSpace[i].start;
				//cout << "start: " << tempStart << " size: " << tempSize << endl;

				largest = vectOfFreeSpace[i];
				int start = largest.start;
				process->start = start;
				process->idleAt = runTime + process->burst;

				if (process->size == largest.size)
				{
					delete mainMemory[start];
				}
				else
				{
					mainMemory[start]->start += process->size;
					mainMemory[start]->size -= process->size;
				}

				for (int j = 0; j < process->size; j++)
				{
					mainMemory[start + j] = process;
				}

				usedMemory += process->size;
				loadedProc++;
				readyQueue.pop_front();
				findFreeBlocks();

				break;
			}
		}

//		if (tempSize >= readyQueue[0]->size)
//		{
			//cout << "beginfor loop" << endl;
//			for (int j = tempStart; j < (tempStart + readyQueue[0]->size); j++)
//			for (int j = 0; j < process->size; j++)
//			{
				//cout << "inside for loop" << endl;
//				readyQueue[0]->start = tempStart;
				//cout << "1" << endl;
//				readyQueue[0]->idleAt = runTime + readyQueue[0]->burst;
				//cout << "j: " << j << endl;
//				mainMemory[j] = readyQueue[0];
//			}
			//cout << readyQueue[0]->name << endl;
//			usedMemory += readyQueue[0]->size;
//			usedMemory += process->size;
//			loadedProc++;
//			readyQueue.erase(readyQueue.begin());
//			readyQueue.pop_front();
//			findFreeBlocks();
//		}

		//cout << "outside for loop" << endl;
//		if (queueSize == readyQueue.size())
//		{
			//cout << "Q " << queueSize << endl;
//			flag = false;
//		}
//		else
//		{
//			queueSize = readyQueue.size();
//			tempSize = 0;
//			tempStart = 0;
//		}

		if (-1 == largest.start) {
			break;
		}
	}
}

void bestFit()
{
//	bool flag = true;
//	int tempSize = 0;
//	int tempStart = 0;
//	uint queueSize = readyQueue.size();
//	while (flag && queueSize > 0)
	while (readyQueue.size() > 0)
	{
		//cout << "Free space vect size" << vectOfFreeSpace.size() << endl;

		Process *process = readyQueue.front();
		int processSize = process->size;

		if (processSize > largestFreeBlock)
		{
			break;
		}

		FreeBlock smallest(-1, MAX_MEMORY + 1);

		for (uint i = 0; i < vectOfFreeSpace.size(); i++)
		{
			int freeSize = vectOfFreeSpace[i].size;

//			if (vectOfFreeSpace[i].size >= readyQueue[0]->size)
			if (processSize <= freeSize && freeSize < smallest.size)
			{
//				if (tempSize == 0)
//				{
//					tempSize = vectOfFreeSpace[i].size;
//					tempStart = vectOfFreeSpace[i].start;
//				}
//				else if (vectOfFreeSpace[i].size < tempSize)
//				{
//					tempSize = vectOfFreeSpace[i].size;
//					tempStart = vectOfFreeSpace[i].start;
					//cout << "start: " << tempStart << " size: " << tempSize << endl;
//				}

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
//		if (tempSize >= readyQueue[0]->size)
		else
		{
			int start = smallest.start;
			process->start = start;
			process->idleAt = runTime + process->burst;

			if (process->size == smallest.size)
			{
				delete mainMemory[start];
			}
			else
			{
				mainMemory[start]->start += process->size;
				mainMemory[start]->size -= process->size;
			}

			//cout << "beginfor loop" << endl;
//			for (int j = tempStart; j < (tempStart + readyQueue[0]->size); j++)
			for (int j = 0; j < process->size; j++)
			{
				//cout << "inside for loop" << endl;
//				readyQueue[0]->start = tempStart;
				//cout << "1" << endl;
//				readyQueue[0]->idleAt = runTime + readyQueue[0]->burst;
				//cout << "j: " << j << endl;
//				mainMemory[j] = readyQueue[0];
				mainMemory[start + j] = process;
			}

			//cout << readyQueue[0]->name << endl;
//			usedMemory += readyQueue[0]->size;
			usedMemory += process->size;
			loadedProc++;
//			readyQueue.erase(readyQueue.begin());
			readyQueue.pop_front();
			findFreeBlocks();
		}
		//cout << "outside for loop" << endl;

//		if (queueSize == readyQueue.size())
//		{
			//cout << "Q " << queueSize << endl;
//			flag = false;
//		}
//		else
//		{
//			queueSize = readyQueue.size();
//			tempSize = 0;
//			tempStart = 0;
//		}
	}
}

void compaction()
{
	int startBlock = vectOfFreeSpace.size() - 1;
	int lastIndex = vectOfFreeSpace[startBlock].start - 1;
	while (startBlock > -1)
	{
		//cout << "Inside while" << endl;
		FreeBlock targetBlock = vectOfFreeSpace[startBlock];
		//cout << "1st target " << targetBlock.start << " " << targetBlock.size << endl;
		//freeBlock lastTargetBlock = vectOfFreeSpace[0];
		//cout << "2nd target" << endl;
		Process *targetProcess, *secondTargetProcess;
		//cout << "Declare" << endl;
		Process tempProcess;
		targetProcess = secondTargetProcess = &tempProcess;
		targetProcess->size = MAX_MEMORY;
		//cout << "a1" << endl;
		secondTargetProcess->size = MAX_MEMORY;
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
				targetProcess = mainMemory[i]; //can go to a hole towards the front
				//cout << "2nd if " << targetProcess->size << endl;
				i -= mainMemory[i]->size + 1; //offset by 1 because of double increment
			}
		}
		cout << "Finished for" << endl;
		if (targetProcess->size != MAX_MEMORY)
		{
			cout << "moving 1" << endl;
			zeroFillMemory(targetProcess->start, targetProcess->size);
			//targetProcess->start = targetBlock.start;
			cout <<"Target " << targetBlock.size - 1 << " " << targetBlock.start << endl;
			int fillStart = targetBlock.start + targetBlock.size - 1;
			targetProcess->start = fillStart - targetProcess->size + 1;
			cout << targetProcess->start << endl;
			for (int j = fillStart; j > (fillStart - targetProcess->size); j--)
			{
				cout << "j: " << j << endl;
				mainMemory[j] = targetProcess;
			}
			findFreeBlocks();
			startBlock = vectOfFreeSpace.size() - 1; //restart back at the end free block in case the vect of free blocks changes. not efficient
			lastIndex = vectOfFreeSpace[startBlock].start - 1;
		}
		else
		{
			startBlock--;
			//if (vectOfFreeSpace[startBlock].start - 1 < lastIndex)
			//{
				lastIndex = vectOfFreeSpace[startBlock].start - 1;
				cout << "start from " << lastIndex << endl;
			//}
		}
		cout << "Num of blocks: " << freeBlocks << endl;
		printMemoryMap();
	}
	printMemoryMap();
	findFreeBlocks();
	FreeBlock targetBlock = vectOfFreeSpace[0];
	int unload = 0;
	while ((targetBlock.start + targetBlock.size) < MAX_MEMORY)
	{
		//freeBlock targetBlock = vectOfFreeSpace[0];
		for (int m = (targetBlock.start + targetBlock.size); m < MAX_MEMORY; m++)
		{
			cout << m << endl;
			if (mainMemory[m]->size > 0)
			{
				cout << "found " << mainMemory[m]->name << endl;
				Process* targetProcess = mainMemory[m];
				int start = mainMemory[m]->start;
				if (targetProcess->size > targetBlock.size)
				{
					unload++;
					cout << "unload here" << endl;
					readyQueue.push_front(mainMemory[m]);
					zeroFillMemory(start, mainMemory[m]->size);
					findFreeBlocks();
				}
				else
				{
					for (int n = targetBlock.start; n < (targetBlock.start + targetProcess->size); n++)
					{
						cout << "insert " << n << endl;
						mainMemory[n] = targetProcess;
						cout << mainMemory[n]->name << endl;
						mainMemory[n]->start = targetBlock.start;
					}
					cout << "main " << start << " " << mainMemory[m]->size << endl;
					zeroFillMemory(start, mainMemory[m]->size);
					findFreeBlocks();
				}
				printMemoryMap();
				for (uint i = 0; i < vectOfFreeSpace.size(); i++)
				{
					cout << "Block: " << vectOfFreeSpace[i].start << " " << vectOfFreeSpace[i].size << endl;
				}
				targetBlock = vectOfFreeSpace[0];
				cout << "start " << targetBlock.start << endl;
				cout << "Num of blocks: " << freeBlocks << endl;
				printMemoryMap();
				break;
			}
		}
	}
	if (unload > 0)
	{
		cout << "special case" << endl;
		recoverToMemory(unload);
		printMemoryMap();
		findFreeBlocks();
	}
	cout << "End of code " << targetBlock.start + targetBlock.size << endl;
}

void recoverToMemory(int end)
{
	for (int i = 0; i < end; i++)
	{
		readyQueue[i]->start = vectOfFreeSpace[0].start; //careful here vectOfFreeSpace[0] might not exist
		for (int j = 0; j < readyQueue[i]->size; j++)
		{
			mainMemory[vectOfFreeSpace[0].start + j] = readyQueue[i]; //careful here vectOfFreeSpace[0] might not exist
		}
		readyQueue.pop_front();
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

Process::Process() : name(' '), burst(0), size(0), start(-1), idleAt(-1)
{

}

Process::Process(char name, ushort burst, ushort size, int start, int idleAt) : name(name), burst(burst), size(size), start(start), idleAt(idleAt)
{

}

FreeBlock::FreeBlock(int start, ushort size) : size(size), start(start)
{

}
