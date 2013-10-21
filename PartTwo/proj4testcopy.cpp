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
#include <fcntl.h>
#include <fstream>
#include <sys/mman.h>

// name of file containing unsorted numbers
#define FILENAME "numbers.txt"

// name of shared memory object of unsorted numbers
#define UNSORTED "unsorted"

// name of shared memory object of sorted numbers
#define SORTED "sorted"

// name of shared memory object of index of array of sorted numbers
#define INDEX "index"

// number of items in "numbers.txt"
#define SIZE 10000

// range of numbers to sort at a time
#define RANGE 10

using namespace std;
int prevCounter, counter, child_id, range;
int arraySize, numChild;
//int arrayPartition; // size of the array that each child is responsible for
//int segment_id; //identifier for shared memory segment
//unsigned int *memory; //pointer to memory segment
sem_t lock;

pid_t performFork();
//void fillMemory();
//void sortMemory();

// read and store numbers from "numbers.txt"
void readNumbers(void);

// create shared memory object of sorted numbers
void createSortedArray(void);

// sort a subarray of numbers and appends them to the main array of sorted numbers
void childProcess(void);

// return pointer to shared memory object of unsorted numbers for use
long* mapUnsortedArray(void);

// return pointer to shared memory object of sorted numbers for use
long* mapSortedArray(void);

// return pointer to shared memory object of index of array of sorted numbers for use
uint* mapSortedArrayIndex(void);

// sortMemory specified array with specified size using selection sortMemory
void sortMemory(long*, long);

// sort the sorted shared arra
void sortAll(long*, int, int);

//
void parentProcess(void);
int main()
{	
	performFork();
}

pid_t performFork()
{
	numChild = 4;
	//Prepare shared memory
	//arraySize = 24;
	//arraySize = 16;
	//const int size = (8 * arraySize); //size in bytes of the shared memory segment
	//segment_id = shmget(IPC_PRIVATE, size, S_IRUSR | S_IWUSR); //allocate a chared memory segment
	//memory = (unsigned int *)shmat(segment_id, NULL, 0); //attach shared memory segment

	readNumbers();
	createSortedArray();
	child_id = 0;

	pid_t pid;
	//arrayPartition = arraySize / numChild;	
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
		childProcess();

		//sem_wait(&lock);
		//cout << "Child process with PID: " << child_id << endl;
		//range = child_id * arrayPartition;
		//cout << "Range = " << range << endl;
		//sem_post(&lock);
		//fillMemory();
		//sortMemory();
		cout << "Child process C" << id << " terminated." << endl;
		//return pid;
		exit(0);
	}
	else
	{
		//parent process
		//sleep(3);
		parentProcess();
		cout << "Parent process with PID: " << id << endl;
		while (numChild > 0)
		{
			wait(NULL);
			numChild--;
			//cout << "Child process C" << id << " terminated." << endl;
		}
		
		//sleep(20);
		// TODO print the first 100 numbers in array of sorted numbers
		// TODO currently, array of sorted numbers is sorted in groups of RANGE

		long *sorted = mapSortedArray();
		/*cout << "----------" << endl << "The first 100 numbers in array of sorted numbers." << endl << "Currently sorted in groups of 10." << endl;
		for (uint i = 0; i < 100; ++i) {
			if (i % RANGE == 0) {
				cout << "----------" << endl;
			}

			cout << "sorted[" << i << "] = " << sorted[i] << endl;
		}
		*/
		//sortAll(unsorted, 0, 100);
		/*cout << "----------" << endl << "The first 100 numbers in array of sorted numbers." << endl << "Completely sorted." << endl;
		for (uint i = 7000; i < 8000; ++i) {
			if (i % RANGE == 0) {
				cout << "----------" << endl;
			}

			cout << "sorted[" << i << "] = " << sorted[i] << endl;
		}
		*/
		/* remove the shared memory objects */
		shm_unlink(UNSORTED);
		shm_unlink(SORTED);
		shm_unlink(INDEX);
	}	
	return pid;
}

/*
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
*/

void readNumbers(void) {
	int shm_fd;
	long *unsorted;

	/* open the shared memory object of unsorted numbers */
	shm_fd = shm_open(UNSORTED, O_CREAT | O_RDWR, 0666);
	
	/* configure the size of shared memory object of unsorted numbers */
	ftruncate(shm_fd, SIZE * sizeof(long));
	
	/* memory map shared memory object for unsorted numbers */
	unsorted = (long*)mmap(0, SIZE * sizeof(long), PROT_WRITE, MAP_SHARED, shm_fd, 0);

	/* create file input stream */
	ifstream file(FILENAME);
	string line;

	/* read file and store numbers into shared memory object of unsorted numbers */
	while (getline(file, line))
	{
		*unsorted++ = atol(line.c_str());
	}
}

void createSortedArray(void) {
	/* open the shared memory object of sorted numbers */
	int shm_fd = shm_open(SORTED, O_CREAT | O_RDWR, 0666);

	/* configure the size of the shared memory object of sorted numbers */
	ftruncate(shm_fd, SIZE * sizeof(long));
	
	/* open the shared memory object for index in array of sorted numbers */
	shm_fd = shm_open(INDEX, O_CREAT | O_RDWR, 0666);
	
	/* configure the size of the shared memory object for the index in the array of sorted numbers */
	ftruncate(shm_fd, sizeof(int));
}

void childProcess(void) {
	// array of unsorted numbers
	long *unsorted = mapUnsortedArray();

	// array of sorted numbers
	long *sorted = mapSortedArray();

	// current index of array of sorted numbers
	uint* index = mapSortedArrayIndex();

	// size of child array
	uint childSize = SIZE / numChild;

	// starting index of child in array of unsorted numbers
	uint start = (child_id - 1) * childSize;

	// ending index of child in array of unsorted numbers
	uint end = child_id * childSize;

	// sub array of sorted numbers to be added to main array of sorted numbers
	long subarray[RANGE];

	// read assigned section of array of unsorted numbers, sortMemory subsections, and append subsections to array of sorted numbers
	for (uint i = start; i < end; ++i) {
		uint j = i % RANGE;

		subarray[j] = unsorted[i];

		// if at end of subarray ...
		if (RANGE - 1 == j) {
			// sort subarray
			sortMemory(subarray, RANGE);

			// append subarray to array of sorted numbers, requires synchronization with shared memory
			sem_wait(&lock);
			for (int k = 0; k < RANGE; ++k) {
				sorted[(*index)++] = subarray[k];
			}
			sem_post(&lock);
		}
	}
}

long* mapUnsortedArray(void) {
	/* open the shared memory object of unsorted numbers */
	int shm_fd = shm_open(UNSORTED, O_RDONLY, 0666);

	/* return pointer to memory map of shared memory object of unsorted numbers */
	return (long*)mmap(0, SIZE * sizeof(long), PROT_READ, MAP_SHARED, shm_fd, 0);
}

long* mapSortedArray(void) {
	/* open the shared memory object of sorted numbers */
	int shm_fd = shm_open(SORTED, O_RDWR, 0666);

	/* return pointer to memory map of shared memory object of sorted numbers */
	return (long*)mmap(0, SIZE * sizeof(long), PROT_WRITE, MAP_SHARED, shm_fd, 0);
}

uint* mapSortedArrayIndex(void) {
	/* open shared memory object of index of array of sorted numbers */
	int shm_fd = shm_open(INDEX, O_RDWR, 0666);

	/* return pointer to memory map of shared memory object of index of array of sorted numbers */
	return (uint*)mmap(0, sizeof(int), PROT_WRITE, MAP_SHARED, shm_fd, 0);
}

//Algorithm derived from www.algolist.net/Algorithms/Sorting/Selection_sort
void sortMemory(long array[], long size)
{
	for (uint i = 0; i < size - 1; ++i)
	{
		uint minIndex = i;

		for (uint j = i + 1; j < size; ++j)
		{
			if (array[j] < array[minIndex])
				{
					minIndex = j;
				}
		}

		if (minIndex != i)
		{
			long tmp = array[i];
			array[i] = array[minIndex];
			array[minIndex] = tmp;
		}
	}
}


void parentProcess(void) 
{
        long *sorted = mapSortedArray();
	counter = 0;
	prevCounter = 0;
	while (prevCounter < SIZE - 1) {
		//sleep so child process can fill the memory
		sleep(0.2);
		cout << "---" << endl;
		sem_wait(&lock);
		cout << "Got lock" << endl;
		cout << "counter " << counter << endl;
		for (int i = prevCounter; i < SIZE; i++)
		{
			//sem_wait(&lock);
			if (sorted[i] == 0) 
			{
				counter = i - 1;
				cout << "counter " << counter << endl;
				break;
			}
			//sem_post(&lock);
		}	
		
		if (counter == prevCounter)
		{
			counter = SIZE - 1;
			cout << "counter2 " << counter << endl;
		}	
		sortAll(sorted, prevCounter, counter);
		sem_post(&lock);
		cout << "Let go lock" << endl;
		prevCounter = counter;
	}
}


//Algorithm derived from www.algolist.net/Algorithms/Sorting/Quicksort
void sortAll(long *s, int left, int right)
{
	int i = left, j = right;
	int tmp;
	int pivot = s[(left + right) / 2];

	/* partition */
	while (i <= j) 
	{
		while (s[i] < pivot)
		{
			i++;
		}
            	while (s[j] > pivot)
		{
                	j--;
		}

            	if (i <= j)
		{
                  	tmp = s[i];
                  	s[i] = s[j];
                  	s[j] = tmp;
                  	i++;
                  	j--;
           	}
      	};

      	/* recursion */
	if (left < j)
	{
		sortAll(s, left, j);
	}

      	if (i < right)
	{
            	sortAll(s, i, right);
	}
}
