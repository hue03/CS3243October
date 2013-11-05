#include <cstdlib>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <iostream>
#include <vector>
#include <deque>  

#define MAX_PROCESSES 60
#define PROCESS_COUNT 50
#define MIN_BURST 100
#define MAX_BURST 5000
#define MIN_MEMORY_PER_PROC 4
#define MAX_MEMORY_PER_PROC 160
#define MAX_MEMORY 1040
#define MAX_BLOCK_PROC_RATIO 0.85
#define PRINT_INTERVAL 5000
#define MAX_QUANTA 50000
#define ENABLE_COMPACTION 0

#define LOWBYTE_PERCENT 50
#define HIGHBYTE_PERCENT 5
#define LOWBYTE_SIZE_INTERVAL_PERCENT 4
#define MEDBYTE_SIZE_INTERVAL_PERCENT 56

using namespace std;

struct Process {
	char name;
	ushort burst;
	ushort size;
	int start;
	int idleAt;
};

struct freeBlock {
	ushort size;
	int start;
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
vector<freeBlock> vectOfFreeSpace;
deque<Process*> readyQueue;
Process* mainMemory [MAX_MEMORY];
Process myProcess;

void assignName();
void assignSize();
void assignBurst();
void loadQueue();
void zeroFillMemory(int start, int end);
void fillMemory();
void removeIdle();
void findFreeBlocks();
void sortByIdle(int left, int right);
void firstFit();
void bestFit();
void worstFit();
void printMemoryMap(void);

int main()
{
	assignName();
	srand(time(NULL));
	assignSize();
	assignBurst();
	loadQueue();
	zeroFillMemory(0, MAX_MEMORY);
	findFreeBlocks();
	//fillMemory();
	//gettimeofday(&start, NULL);
	runTime = 0;
	//cout << "Choose a swapping method: " << endl;
	//cout << "1. First Fit\n2. Best Fit\n3. Worst Fit" << endl;
	//int input;
	//cin >> input;
	//switch (input)
	//{
	//	case 1: firstFit();
	//		break;
	//	default: cout << "Invalid input. Try again." << endl;
	//		break;
	//}
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
	int printCount = 0;
	while (runTime < MAX_QUANTA)
	{
		//firstFit();
		if (printCount % PRINT_INTERVAL == 0)
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
		//firstFit();
		//worstFit();
		bestFit();
		removeIdle();
		if (printCount % PRINT_INTERVAL == 0)
		{
			//print mainMemory
			printMemoryMap();
		}
		findFreeBlocks();
		if (printCount % PRINT_INTERVAL == 0)
		{
			cout << "List of free blocks" << endl;
			for (uint i = 0; i < vectOfFreeSpace.size(); i++)
			{
				cout << "Block start: " << vectOfFreeSpace[i].start << " Block size: " << vectOfFreeSpace[i].size << endl;
			}
			cout << "--------------------------------------------------------------------------------" << endl;
		}
		printCount++;
		runTime++;
		//cout << "Runtime: " << runTime << endl;
	}
	cout << "Runtime: " << runTime << endl;
	//cout << "Time Ended: " << stop.tv_usec << endl;
	cout << "--------------------------------------------------------------------------------" << endl;
}


void assignName()
{
	char name = '?';

	for (int i = 0; i < PROCESS_COUNT; i++)
	{
		myProcess.start = MIN_MEMORY_PER_PROC * (i + 1);

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

		myProcess.name = name;
		vectOfProcesses.push_back(myProcess);
	}
}

void assignSize()
{
	//srand(getpid());
	int majority = MAX_PROCESSES * (LOWBYTE_PERCENT * 0.01);
	int secondMajority = (int)(MAX_PROCESSES * ((LOWBYTE_PERCENT - HIGHBYTE_PERCENT) * 0.01) + majority + 1); // upper range for the amount of processes in the 45% range
	int thirdMajority = (int)(MAX_PROCESSES * (HIGHBYTE_PERCENT * 0.01)) + secondMajority; //not adding 1 for the kernel (120B), same comment as above
	//cout << majority << endl;
	//cout << secondMajority << endl;
	//cout << thirdMajority << endl;
	int minMax, midMin, midMax;
	minMax = MIN_MEMORY_PER_PROC + (int)((MAX_MEMORY_PER_PROC - MIN_MEMORY_PER_PROC) * (LOWBYTE_SIZE_INTERVAL_PERCENT * 0.01));
	midMin = minMax + 1;
	midMax = midMin + (int)((MAX_MEMORY_PER_PROC - MIN_MEMORY_PER_PROC) * (MEDBYTE_SIZE_INTERVAL_PERCENT * 0.01));
	
	vectOfProcesses[0].size = 120;
	for (int i = 1; i <= majority; i++)
	{
		ushort tempSize = rand() % (minMax - MIN_MEMORY_PER_PROC + 1) + MIN_MEMORY_PER_PROC;
		vectOfProcesses[i].size = tempSize;
	}

	for (int j = majority + 1; j < secondMajority; j++)
	{
		ushort tempSize = rand() % (midMax - midMin + 1) + midMin;
		vectOfProcesses[j].size = tempSize;
	}
	
	for (int k = secondMajority; k < thirdMajority; k++)
	{
		ushort tempSize = rand() % (MAX_MEMORY_PER_PROC - (midMax + 1) + 1) + (midMax + 1);
		vectOfProcesses[k].size = tempSize;
	}
}

void assignBurst()
{
	vectOfProcesses[0].burst = MAX_QUANTA;
	//srand(getpid() * time(NULL));
	for (uint i = 1; i < vectOfProcesses.size(); i++)
	{
		ushort tempBurst = rand() % (MAX_BURST - MIN_BURST + 1) + MIN_BURST;
		vectOfProcesses[i].burst = tempBurst;
	}
}

void loadQueue()
{
	Process *proc_ptr; 
	for (uint i = 0; i < PROCESS_COUNT; i++)
	{
		vectOfProcesses[i].start = 0;
		vectOfProcesses[i].idleAt = 0;
		proc_ptr = &vectOfProcesses[i];
		readyQueue.push_back(proc_ptr);		
	}
}

void zeroFillMemory(int start, int end)
{
	Process *p;
	for (int i = start; i < end; i++)
	{
		p = &myProcess;
		p->name = 248;
		p->size = 0;
		p->burst = 0;
		p->start = start;
		p->idleAt = 0;
		mainMemory[i] = p;
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
			zeroFillMemory(mainMemory[i]->start, mainMemory[i]->start + tempSize);
			//Process *p;
			/*for (int j = mainMemory[i]->start; j < mainMemory[i]->start + tempSize; j++)
			{
				p = &myProcess;
				p->name = 248;
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
		if (tempSize > 0) //careful, may cause out of bounds/seg fault. i is incremented twice from here and the loop itself
		{				  //using a condition to try and correct this. trying to start where the process starts
			i += tempSize - 1; //could possibly shift i to the start of the process at the beginning of the for loop instead of this if condition
		} 
	}
}

void findFreeBlocks()
{
	vectOfFreeSpace.erase(vectOfFreeSpace.begin(), vectOfFreeSpace.begin() + vectOfFreeSpace.size());
	largestFreeBlock = 0;
	smallestFreeBlock = 0;
	int count = 0;
	int i;
	for (i = 0; i < MAX_MEMORY; i++)
	{
		if (mainMemory[i]->size == 0)
		{
			count++;
		}
		else
		{
			if (count != 0)
			{
				freeBlock block;
				block.size = count;
				block.start = i - count;
				vectOfFreeSpace.push_back(block);
				if (count > largestFreeBlock)
				{
					largestFreeBlock = count;
					if (smallestFreeBlock == 0)
					{
						smallestFreeBlock = count;
					}
				}
				else if (count < smallestFreeBlock)
				{
					smallestFreeBlock = count;
				}
				freeBlocks = vectOfFreeSpace.size();
			}
			i += mainMemory[i]->size - 1; //offset by 1 because loop will increment i
			count = 0;
		}
	}
	
	if (count != 0) //handles the case where you go past the end of memory or else the data won't be saved
	{
		freeBlock block;
		block.size = count;
		block.start = i - count;
		vectOfFreeSpace.push_back(block);
		if (count > largestFreeBlock)
		{
			largestFreeBlock = count;
			if (smallestFreeBlock == 0)
			{
				smallestFreeBlock = count;
			}
		}
		else if (count < smallestFreeBlock)
		{
			smallestFreeBlock = count;
		}
		freeBlocks = vectOfFreeSpace.size();
	}
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
	bool flag = true;
	uint queueSize = readyQueue.size();
	while (flag && queueSize > 0)
	{
		for (uint i = 0; i < vectOfFreeSpace.size(); i++)
		{
			if (vectOfFreeSpace[i].size >= readyQueue[0]->size)
			{
				//cout << "free space: " << vectOfFreeSpace[i].size << endl;
				//cout << "process in Q size: " << readyQueue[0]->size << endl;
				for (int j = vectOfFreeSpace[i].start; j < (vectOfFreeSpace[i].start + readyQueue[0]->size); j++)
				{
					//cout << "j: " << j << endl;
					readyQueue[0]->start = vectOfFreeSpace[i].start;
					readyQueue[0]->idleAt = runTime + readyQueue[0]->burst;
					mainMemory[j] = readyQueue[0];
				}
				usedMemory += readyQueue[0]->size;
				loadedProc++;
				readyQueue.erase(readyQueue.begin());
				findFreeBlocks();
				//tempSize = readyQueue[0];
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
	bool flag = true;
	int tempSize = 0;
	int tempStart = 0;
	uint queueSize = readyQueue.size();
	while (flag && queueSize > 0)
	{
		for (uint i = 0; i < vectOfFreeSpace.size(); i++)
		{
			if (vectOfFreeSpace[i].size > tempSize)
			{
				tempSize = vectOfFreeSpace[i].size;
				tempStart = vectOfFreeSpace[i].start;
				//cout << "start: " << tempStart << " size: " << tempSize << endl;
			}
		}
		if (tempSize >= readyQueue[0]->size)
		{
			//cout << "beginfor loop" << endl;
			for (int j = tempStart; j < (tempStart + readyQueue[0]->size); j++)
			{
				//cout << "inside for loop" << endl;
				readyQueue[0]->start = tempStart;
				//cout << "1" << endl;
				readyQueue[0]->idleAt = runTime + readyQueue[0]->burst;
				//cout << "j: " << j << endl;
				mainMemory[j] = readyQueue[0];
			}
			//cout << readyQueue[0]->name << endl;
			usedMemory += readyQueue[0]->size;
			loadedProc++;
			readyQueue.erase(readyQueue.begin());
			findFreeBlocks();
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
			tempSize = 0;
			tempStart = 0;
		}
	}
}

void bestFit()
{
	bool flag = true;
	int tempSize = 0;
	int tempStart = 0;
	uint queueSize = readyQueue.size();
	while (flag && queueSize > 0)
	{
		//cout << "Free space vect size" << vectOfFreeSpace.size() << endl;
		for (uint i = 0; i < vectOfFreeSpace.size(); i++)
		{
			if (vectOfFreeSpace[i].size >= readyQueue[0]->size)
			{
				if (tempSize == 0)
				{
					tempSize = vectOfFreeSpace[i].size;
					tempStart = vectOfFreeSpace[i].start;
				}
				else if (vectOfFreeSpace[i].size < tempSize)
				{
					tempSize = vectOfFreeSpace[i].size;
					tempStart = vectOfFreeSpace[i].start;
					//cout << "start: " << tempStart << " size: " << tempSize << endl;
				}
			}
		}
		if (tempSize >= readyQueue[0]->size)
		{
			//cout << "beginfor loop" << endl;
			for (int j = tempStart; j < (tempStart + readyQueue[0]->size); j++)
			{
				//cout << "inside for loop" << endl;
				readyQueue[0]->start = tempStart;
				//cout << "1" << endl;
				readyQueue[0]->idleAt = runTime + readyQueue[0]->burst;
				//cout << "j: " << j << endl;
				mainMemory[j] = readyQueue[0];
			}
			//cout << readyQueue[0]->name << endl;
			usedMemory += readyQueue[0]->size;
			loadedProc++;
			readyQueue.erase(readyQueue.begin());
			findFreeBlocks();
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
			tempSize = 0;
			tempStart = 0;
		}
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
	cout << "         10        20        30        40        50        60        70        80" << endl;
	cout << "|----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----|" << endl;
	for (size_t i = 0; i < 80; ++i) cout << mainMemory[i]->name;	cout << endl;
	cout << "         90       100       110       120       130       140       150       160" << endl;
	cout << "|----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----|" << endl;
	for (size_t i = 80; i < 160; ++i) cout << mainMemory[i]->name;	cout << endl;
	cout << "        170       180       190       200       210       220       230       240" << endl;
	cout << "|----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----|" << endl;
	for (size_t i = 160; i < 240; ++i) cout << mainMemory[i]->name;	cout << endl;
	cout << "        250       260       270       280       290       300       310       320" << endl;
	cout << "|----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----|" << endl;
	for (size_t i = 240; i < 320; ++i) cout << mainMemory[i]->name;	cout << endl;
	cout << "        330       340       350       360       370       380       390       400" << endl;
	cout << "|----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----|" << endl;
	for (size_t i = 320; i < 400; ++i) cout << mainMemory[i]->name;	cout << endl;
	cout << "        410       420       430       440       450       460       470       480" << endl;
	cout << "|----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----|" << endl;
	for (size_t i = 400; i < 480; ++i) cout << mainMemory[i]->name;	cout << endl;
	cout << "        490       500       510       520       530       540       550       560" << endl;
	cout << "|----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----|" << endl;
	for (size_t i = 480; i < 560; ++i) cout << mainMemory[i]->name;	cout << endl;
	cout << "        570       580       590       600       610       620       630       640" << endl;
	cout << "|----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----|" << endl;
	for (size_t i = 560; i < 640; ++i) cout << mainMemory[i]->name;	cout << endl;
	cout << "        650       660       670       680       690       700       710       720" << endl;
	cout << "|----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----|" << endl;
	for (size_t i = 640; i < 720; ++i) cout << mainMemory[i]->name;	cout << endl;
	cout << "        730       740       750       760       770       780       790       800" << endl;
	cout << "|----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----|" << endl;
	for (size_t i = 720; i < 800; ++i) cout << mainMemory[i]->name;	cout << endl;
	cout << "        810       820       830       840       850       860       870       880" << endl;
	cout << "|----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----|" << endl;
	for (size_t i = 800; i < 880; ++i) cout << mainMemory[i]->name;	cout << endl;
	cout << "        890       900       910       920       930       940       950       960" << endl;
	cout << "|----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----|" << endl;
	for (size_t i = 880; i < 960; ++i) cout << mainMemory[i]->name;	cout << endl;
	cout << "        970       980       990      1000      1010      1020      1030      1040" << endl;
	cout << "|----+----|----+----|----+----|----+----|----+----|----+----|----+----|----+----|" << endl;
	for (size_t i = 960; i < 1040; ++i) cout << mainMemory[i]->name;	cout << endl;
}
