#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <semaphore.h>

using namespace std;
int counter, child_id, range;
int arraySize, numChild;
int arrayPartition; // size of the array that each child is responsible for
int segment_id; //identifier for shared memory segment
unsigned int *memory; //pointer to memory segment
sem_t lock;

pid_t performFork();
void fillMemory();
void sortMemory();
int main()
{	
	performFork();
}

pid_t performFork()
{
	numChild = 4;
	//Prepare shared memory
	arraySize = 24;
	//arraySize = 16;
	const int size = (8 * arraySize); //size in bytes of the shared memory segment
	segment_id = shmget(IPC_PRIVATE, size, S_IRUSR | S_IWUSR); //allocate a chared memory segment
	memory = (unsigned int *)shmat(segment_id, NULL, 0); //attach shared memory segment

	pid_t pid;
	arrayPartition = arraySize / numChild;	
	//initialize semaphore
	sem_init(&lock, 0, 1);
	/*
	memory[0] = 6;
	memory[1] = 2;
	memory[2] = 1;
	memory[3] = 8;
	memory[4] = 3;
	memory[5] = 5;
	memory[6] = 9;
	memory[7] = 7;
	memory[8] = 10;
	memory[9] = 4;
	memory[10] = 11;
	memory[11] = 1;
	memory[12] = 1;
	memory[13] = 15;
	memory[14] = 9;
	memory[15] = 12;
	memory[16] = 6;
	memory[17] = 4;
	memory[18] = 3;
	memory[19] = 3;
	memory[20] = 16;
	memory[21] = 1;
	memory[22] = 9;
	memory[23] = 3;
	*/
	cout << "Before forking." << endl;
	for (int i = 0; i < numChild; i++)
	{
		child_id++;
		pid = fork();
		if (pid == 0)
			break;
	}
	int id = getpid();	
	//determine if parent or child
	if (pid < 0)
	{
		//error encoutered
		cout << "Fork failed." << endl;
		return 1;
	}
	else if (pid == 0)
	{
		//child process
		//sem_wait(&lock);
		cout << "Child process with PID: " << child_id << endl;
		range = child_id * arrayPartition;
		cout << "Range = " << range << endl;
		//sem_post(&lock);
		fillMemory();
		sortMemory();
		cout << "Child process C" << id << " terminated." << endl;
		exit(0);
	}
	else
	{
		//parent process
		cout << "Parent process with PID: " << id << endl;
		//simulateBusyWork('P');
		while (numChild > 0)
		{
			wait(NULL);
			numChild--;
			//cout << "Child process C" << id << " terminated." << endl;
		}
		for (int i = 0; i < arraySize; i++)
		{
			cout << memory[i] << " " << endl;
		}
	}	
	return pid;
}

void fillMemory()
{
	srand(getpid());
	for (int i = range; i >= (range - arrayPartition); i--)
	{
		//sem_wait(&lock);
		memory[i] = 1 + (rand() % (int)(24));
		//sem_post(&lock);
	}
	
}

//Algorithm derived from www.algolist.net/Algorithms/Sorting/Selection_sort
void sortMemory()
{
	int i, j, minIndex, tmp; 
	for (i = range - arrayPartition; i < range - 1; i++)
	{
		minIndex = i;
		for (j = i + 1; j < range; j++)
		{
			if (memory[j] < memory[minIndex])
				{
					minIndex = j;
				}
		}

		if (minIndex != i)
		{
			tmp = memory[i];
			//sem_wait(&lock);
			memory[i] = memory[minIndex];
			memory[minIndex] = tmp;
			//sem_post(&lock);
		}
	}	
}
