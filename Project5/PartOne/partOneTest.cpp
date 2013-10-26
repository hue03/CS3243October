#include <cstdlib>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <vector>

#define MAX_PROCESSES 60
#define PROCESS_COUNT 5
#define MIN_BURST 100
#define MAX_BURST 5000
#define MIN_MEMORY_PER_PROC 4
#define MAX_MEMORY_PER_PROC 160
#define MAX_MEMORY 1040
#define MAX_BLOCK_PROC_RATIO 0.85
#define PRINT_INTERVAL 5000
#define MAX_QUANTA 50000
#define ENABLE_COMPACTION 0

using namespace std;

struct Process {
	char name;
	ushort burst;
	ushort size;
	int start;
	int idleAt;
};

vector<Process> vectOfProcesses;
Process myProcess;

void assignName();
void assignSize();
int main()
{
	assignName();
	assignSize();
	for (int i = 0; i < MAX_PROCESSES; i++)
	{
		cout << vectOfProcesses[i].name << endl;
	}
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
	int majority = MAX_PROCESSES / 2;
	int secondMajority = MAX_PROCESSES - (int)(MAX_PROCESSES * 0.45);
	int thirdMajority = MAX_PROCESSES - (int)(MAX_PROCESSES * 0.05);
	cout << majority << endl;
	cout << secondMajority << endl;
	cout << thirdMajority << endl;
}
