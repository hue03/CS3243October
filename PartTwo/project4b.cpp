// CS3242 Operating Systems
// Fall 2013
// Project 4: Process Synchronization, Part 2
// Steven Ng and Hue Moua
// Date: 10/21/2013
// File: project4b.cpp

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
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

#define LOCK "lock"

// name of shared memory object of index of array of sorted numbers
#define INDEX "index"

// number of items in "numbers.txt"
#define SIZE 10000

// range of numbers to sort at a time
#define RANGE 10

using namespace std;
int child_id;
int numChild;
sem_t *lock;

pid_t performFork();
// read and store numbers from "numbers.txt"
void readNumbers(void);

// create shared memory object of sorted numbers
void createSortedArray(void);

// create shared memory semaphore and map to it for all processes
void createSemaphore(void);

// sort a subarray of numbers and appends them to the main array of sorted numbers
void childProcess(void);

// return pointer to shared memory object of unsorted numbers for use
long* mapUnsortedArray(void);

// return pointer to shared memory object of sorted numbers for use
long* mapSortedArray(void);

// return pointer to shared memory object of index of array of sorted numbers for use
uint* mapSortedArrayIndex(void);

sem_t* mapSemaphore(void);

// sortMemory specified array with specified size using selection sortMemory
void sortMemory(long*, long);

// sort the whole shared memory called sorted using quicksort
void sortAll(long*, int, int);

// sort's the children's partitions
void parentProcess(void);

int main()
{
	performFork();
}

pid_t performFork()
{
	numChild = 4;
	child_id = 0;
	readNumbers();
	createSortedArray();
	createSemaphore();
	pid_t pid;

	for (int i = 0; i < numChild; i++)
	{
		child_id++;
		pid = fork();
		if (pid == 0)
			break;
	}
	
	int id = getpid();
	if (pid < 0)
	{
		//error encoutered
		cout << "Fork failed." << endl;
		return 1;
	}
	else if (pid == 0)
	{
		//child do stuff
		childProcess();
		cout << "Child process C" << id << " terminated. " << endl;
		exit(0);
	}
	else
	{
		cout << "Parent process with PID: " << id << endl;
		//parent do stuff
		parentProcess();
		while (numChild > 0)
		{
			wait(NULL);
			numChild--;
		}

		//detach from shared memory
		shm_unlink(UNSORTED);
		shm_unlink(SORTED);
		shm_unlink(INDEX);
	}	
	return pid;
}

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

	uint *index = mapSortedArrayIndex();
	*index = 0;
}

void createSemaphore(void) {
	/* open the shared memory object of sorted numbers */
	int shm_fd = shm_open(LOCK, O_CREAT | O_RDWR, 0666);

	/* configure the size of the shared memory object of sorted numbers */
	ftruncate(shm_fd, sizeof(sem_t));

	lock = mapSemaphore();

	sem_init(lock, 1, 1);
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
			sem_wait(lock);
			for (int k = 0; k < RANGE; ++k) {
				sorted[(*index)++] = subarray[k];
				//cout << "K: " << k << endl;
			}
			sem_post(lock);
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

sem_t* mapSemaphore(void) {
	/* open shared memory object of index of array of sorted numbers */
	int shm_fd = shm_open(LOCK, O_RDWR, 0666);

	/* return pointer to memory map of shared memory object of index of array of sorted numbers */
	return (sem_t*)mmap(0, sizeof(sem_t), PROT_WRITE, MAP_SHARED, shm_fd, 0);
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
	//create a pointer to the shared sorted array
        long *sorted = mapSortedArray();

	//create a pointer to the shared memory containing the current index position of the children
	uint *index = mapSortedArrayIndex();

	//the previous index position to resume sorting from
	int prevIndex = 0;
	
	//index position where the last child was at
	int currentIndex = (int)index[0];
	while (prevIndex < SIZE - 1) {
		if (prevIndex != currentIndex)
		{
			sem_wait(lock);
		//	cout << "Got lock" << endl;
		//	cout << "counter " << index[0] << endl;
			//sort a subsection of the sorted array
			sortAll(sorted, prevIndex, currentIndex - 1);
			sem_post(lock);
		//	cout << "Let go lock" << endl;
			prevIndex = currentIndex;
		}
		currentIndex = (int)index[0];
	}
	//sort the whole sorted array
	sem_wait(lock);
	sortAll(sorted, 0, SIZE - 1);
	sem_post(lock);

	ofstream numberSorted("numbers_sorted.txt"); //opens the output file
	for (int i = 0; i < SIZE; i++)
	{
		numberSorted << sorted[i] << endl; //write to output file
	}
	numberSorted.close(); //close output file
	cout << "View sorted numbers in 'numbers_sorted.txt'" << endl;
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
