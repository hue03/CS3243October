#include <cstdlib>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <deque>  

#define MAX_PROCESSES 60
#define PROCESS_COUNT 50
#define MIN_BURST 1
#define MAX_BURST 5
#define MIN_MEMORY_PER_PROC 4
#define MAX_MEMORY_PER_PROC 160
#define MAX_MEMORY 1040
#define MAX_BLOCK_PROC_RATIO 0.85
#define PRINT_INTERVAL 5000
#define MAX_QUANTA 5
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
int usedMemory;
int loadedProc;
int largestSize;
int runTime;
vector<Process> vectOfProcesses;
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
void firstFit();
void bestFit();
void worstFit();

int main()
{
	assignName();
	assignSize();
	assignBurst();
	loadQueue();
	zeroFillMemory(0, MAX_MEMORY);
	fillMemory();
	runTime = time(NULL) + MAX_QUANTA;
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
	for (int i = 0; i < MAX_PROCESSES; i++)
	{
		cout << vectOfProcesses[i].burst << endl;
	}
	cout << "--------------------------------------------------------------------------------" << endl;

	//print the readyQueue
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
	for (int i = 0; i < MAX_MEMORY; i++)
	{
		cout << mainMemory[i]->name;
	}
	
	cout << "--------------------------------------------------------------------------------" << endl;
	cout << "Used Memory: " << usedMemory << " Free Memory: " << MAX_MEMORY - usedMemory << endl;
	cout << "Loaded: " << loadedProc << " Unloaded: " << PROCESS_COUNT - loadedProc << endl;
	cout << "Time Ended: " << time(NULL) << endl;
	
	int printCount = 0;
	while (time(NULL) <= runTime)
	{
	removeIdle();
	if (printCount % PRINT_INTERVAL == 0)
	{
	//print the readyQueue
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
	for (int i = 0; i < MAX_MEMORY; i++)
	{
		cout << mainMemory[i]->name;
	}
	
	cout << "--------------------------------------------------------------------------------" << endl;
	cout << "Used Memory: " << usedMemory << " Free Memory: " << MAX_MEMORY - usedMemory << endl;
	cout << "Loaded: " << loadedProc << " Unloaded: " << PROCESS_COUNT - loadedProc << endl;
	cout << "--------------------------------------------------------------------------------" << endl;
	}
	printCount++;
	}
	cout << "Time Ended: " << time(NULL) << endl;
}

void assignName()
{
	int val;
	myProcess.name = 64;
	vectOfProcesses.push_back(myProcess);
	for (int i = 1; i < MAX_PROCESSES; i++)
	{
		myProcess.start = MIN_MEMORY_PER_PROC * (i + 1);
		if (i < 10)
		{
			val = 48 + i;
			myProcess.name = val;
		}
		else if ((10 <= i && i < 35))
		{
			if (i == 10)
			{
				val = 65;
			}
			else if (i == 18)
			{
				val += 2;
			}
			else
			{
				val += 1;
			}
			myProcess.name = val;
		}
		else
		{
			if (i == 35)
			{
				val = 97;
			}
			else if (i == 46)
			{
				val += 2;
			}
			else
			{
				val += 1;
			}
			myProcess.name = val;
		}
		vectOfProcesses.push_back(myProcess);
	}
}

void assignSize()
{
	srand(getpid());
	int majority = MAX_PROCESSES * (LOWBYTE_PERCENT * 0.01);
	int secondMajority = (int)(MAX_PROCESSES * ((LOWBYTE_PERCENT - HIGHBYTE_PERCENT) * 0.01) + majority + 1); // upper range for the amount pf processes in the 45% range
	int thirdMajority = (int)(MAX_PROCESSES * (HIGHBYTE_PERCENT * 0.01)) + secondMajority; //subtract 1 for the kernel (120B), same comment as above
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
	srand(getpid() * time(NULL));
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
		p->start = 0;
		p->idleAt = 0;
		mainMemory[i] = p;
		//cout << "i0: " << i << endl;
	}
}

void fillMemory()
{
	int lastIndex = 0;
	for (uint i = 0; i < readyQueue.size(); i++)
	{
		short tempSize = readyQueue[i]->size;
		//cout << tempSize << endl;
		for (int j = lastIndex; j < MAX_MEMORY; j++)
		{
		//cout << tempSize << endl;
			if (tempSize < 0)
			{
				//cout << "Enough space." << endl;
				short range = lastIndex + readyQueue[i]->size;
				readyQueue[i]->start = lastIndex;
				readyQueue[i]->idleAt = time(NULL) + readyQueue[i]->burst;
				for (int k = lastIndex; k < range; k++)
				{
					mainMemory[k] = readyQueue[i];
				}
				lastIndex = --j;
				usedMemory += readyQueue[i]->size;
				loadedProc++;
				readyQueue.erase(readyQueue.begin() + i);
				i--;
				break;
			}
			else if (mainMemory[j]->size == 0)
			{
				tempSize--;
				//cout << "Empty space" << endl;
			}
			//else
			//{
				//lastIndex = ++j; //need to change this because it is inefficient
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
		if ((mainMemory[i]->idleAt <= (int)time(NULL)) && (tempSize> 0))
		{
			cout << "i: " << i << endl;
			cout << "size: " << tempSize << endl;
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
		if (tempSize > 0) //careful, may cause out of bounds/seg fault. i is incremented twice from here and the loop itself
		{				  //using a condition to try and correct this. trying to start where the process start's
			i += tempSize - 1; //could possibly shift i to the start of the process at the top instead of this if condition
		} 
	}
}
