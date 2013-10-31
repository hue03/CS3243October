#include <cstdlib>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <vector>

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

struct Queue {
	Process *data_ptr;
	Queue *next_ptr;
	Queue(Process *d, Queue *n):data_ptr(d),next_ptr(n){}
};
int usedMemory;
int loadedProc;
int largestSize;
Queue *head_ptr = 0;
vector<Process> vectOfProcesses;
Process* mainMemory [MAX_MEMORY];
Process myProcess;

void assignName();
void assignSize();
void assignBurst();
void loadQueue();
unsigned lengthOfQueue();
void initializeMemory();
void fillMemory();
void firstFit();
void bestFit();
void worstFit();

int main()
{
	assignName();
	assignSize();
	assignBurst();
	loadQueue();
	initializeMemory();
	fillMemory();
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
	for (int i = 0; i < PROCESS_COUNT; i++)
	{
		cout << vectOfProcesses[i].name << ":";
		cout << "Size: " << vectOfProcesses[i].size << " ";
		cout << "Start: " << vectOfProcesses[i].start << " ";
		cout << "Burst time: " << vectOfProcesses[i].burst << " ";
		cout << "Idle at: " <<  vectOfProcesses[i].idleAt;
		cout << endl;
	}
	cout << "--------------------------------------------------------------------------------" << endl;
	
	//print the readyQueue
	for (Queue *current_ptr = head_ptr; current_ptr != 0; current_ptr = current_ptr->next_ptr)
	{
		cout << current_ptr->data_ptr->name << ":";
		cout << "Size: " << current_ptr->data_ptr->size  << " ";
		cout << "Start: " << current_ptr->data_ptr->start  << " ";
		cout << "Burst time: " << current_ptr->data_ptr->burst << " ";
		cout << "Idle at: " <<  current_ptr->data_ptr->idleAt;
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
	//Queue *temp_ptr = head_ptr;
	for (uint i = 0; i < PROCESS_COUNT; i++)
	{
		vectOfProcesses[i].start = 0;
		vectOfProcesses[i].idleAt = 0;
		Process *proc_ptr = &vectOfProcesses[i];
		if (head_ptr == 0)
		{
			head_ptr = new Queue(proc_ptr, 0);
		}
		else
		{
			Queue *temp_ptr = head_ptr; //this for some reason cannot be decalred outside, wastes memory.
			while (temp_ptr->next_ptr != 0)
			{
				temp_ptr = temp_ptr->next_ptr;
			}
			temp_ptr->next_ptr = new Queue(proc_ptr, 0);
		}
	}
}

unsigned lengthOfQueue(Queue *q)
{
	if (q == 0)
	{
		return 0;
	}
	return 1 + lengthOfQueue(q->next_ptr);
}

void initializeMemory()
{
	Process *p;
	for (int i = 0; i < MAX_MEMORY; i++)
	{
		p = &myProcess;
		p->name = 108;
		p->size = 0;
		p->burst = 0;
		p->start = 0;
		p->idleAt = 0;
		mainMemory[i] = p;
	}
}

void fillMemory()
{
	int lastIndex = 0;
	Queue *prev_ptr = head_ptr;
	cout << prev_ptr << " " <<head_ptr << endl;
	for (Queue *current_ptr = head_ptr; current_ptr != 0; current_ptr = current_ptr->next_ptr)
	{
		short tempSize = current_ptr->data_ptr->size;
		//cout << tempSize << endl;
		for (int j = lastIndex; j < MAX_MEMORY; j++)
		{
		//cout << tempSize << endl;
			if (tempSize < 0)
			{
				//cout << "Enough space." << endl;
				short range = lastIndex + current_ptr->data_ptr->size;
				for (int k = lastIndex; k < range; k++)
				{
					current_ptr->data_ptr->start = lastIndex;
					current_ptr->data_ptr->idleAt = time(NULL) + current_ptr->data_ptr->burst;
					mainMemory[k] = current_ptr->data_ptr;
				}
				lastIndex = --j;
				usedMemory += current_ptr->data_ptr->size;
				loadedProc++;
				if (current_ptr == head_ptr)
				{
					//cout << "Head" << endl;
					prev_ptr = head_ptr->next_ptr;
					free(head_ptr);
					head_ptr = prev_ptr;
				}
				else
				{
					prev_ptr->next_ptr = current_ptr->next_ptr;
					free(current_ptr);
				}
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
		prev_ptr = current_ptr;
		if ((MAX_MEMORY - usedMemory) < 11)
		{
			cout << "Stop filling in memory. Free space is <11. CAUTION: THIS IS ASSUMING THAT THE QUEUE HAS PROCESSES OF SIZE >=11!!!" << endl;
		break;
		}
	}
}
