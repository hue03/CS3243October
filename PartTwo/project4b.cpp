// CS3242 Operating Systems
// Fall 2013
// Project 4: Process Synchronization, Part 2
// Steven Ng and Hue Moua
// Date: 10/21/2013
// File: project4b.cpp

#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <semaphore.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#define FILENAME "numbers.txt"	// name of file containing unsorted numbers
#define UNSORTED "unsorted"	// name of shared memory object of unsorted numbers
#define SORTED "sorted"	// name of shared memory object of sorted numbers
#define INDEX "index"	// name of shared memory object of index of array of sorted numbers
#define LOCK "lock"	// name of shared memory object of semaphore
#define SIZE 10000	// number of items in "numbers.txt"
#define RANGE 10	// range of numbers to insert at a time
#define NUM_OF_CHILDREN 4

using namespace std;

int child_id;
long *unsorted, *sorted;
size_t *index;
sem_t *lock;

pid_t performFork();
void createSharedMemory(void);	// create shared memory objects
void readNumbers(void);	// read and store numbers from "numbers.txt"
void childProcess(void);	// sort a subarray of numbers and append them to main array of sorted numbers
void sortMemory(long*, size_t);	// sort specified array with specified size using selection sortMemory
void sortAll(long*, int, int);	// sort the whole shared memory called sorted using quicksort
void parentProcess(void);	// sort the children's partitions

int main()
{
	performFork();
}

pid_t performFork()
{
	child_id = 0;
	createSharedMemory();
	readNumbers();
	pid_t pid;

	for (int i = 0; i < NUM_OF_CHILDREN; i++)
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
		for (size_t i = 0; i < NUM_OF_CHILDREN; ++i) {
			wait(NULL);
		}

		// destory semaphore
		sem_destroy(lock);

		//detach from shared memory
		shm_unlink(UNSORTED);
		shm_unlink(SORTED);
		shm_unlink(INDEX);
		shm_unlink(LOCK);
	}	
	return pid;
}

void createSharedMemory(void) {
	/* open the shared memory object of unsorted numbers */
	int shm_fd = shm_open(UNSORTED, O_CREAT | O_RDWR, 0666);

	/* configure the size of shared memory object of unsorted numbers */
	ftruncate(shm_fd, SIZE * sizeof(long));
	
	/* memory map shared memory object for unsorted numbers */
	unsorted = (long*)mmap(0, SIZE * sizeof(long), PROT_WRITE, MAP_SHARED, shm_fd, 0);

	/* open the shared memory object of sorted numbers */
	shm_fd = shm_open(SORTED, O_CREAT | O_RDWR, 0666);

	/* configure the size of the shared memory object of sorted numbers */
	ftruncate(shm_fd, SIZE * sizeof(long));
	
	/* memory map shared memory object of sorted numbers */
	sorted = (long*)mmap(0, SIZE * sizeof(long), PROT_WRITE, MAP_SHARED, shm_fd, 0);

	/* open the shared memory object for index in array of sorted numbers */
	shm_fd = shm_open(INDEX, O_CREAT | O_RDWR, 0666);
	
	/* configure the size of the shared memory object for the index in the array of sorted numbers */
	ftruncate(shm_fd, sizeof(int));

	/* memory map shared memory object of index of array of sorted numbers */
	index = (size_t*)mmap(0, sizeof(int), PROT_WRITE, MAP_SHARED, shm_fd, 0);
	*index = 0;

	/* open the shared memory object of sorted numbers */
	shm_fd = shm_open(LOCK, O_CREAT | O_RDWR, 0666);

	/* configure the size of the shared memory object of sorted numbers */
	ftruncate(shm_fd, sizeof(sem_t));

	/* memory map shared memory object of semaphore lock */
	lock = (sem_t*)mmap(0, sizeof(sem_t), PROT_WRITE, MAP_SHARED, shm_fd, 0);

	sem_init(lock, 1, 1);
}

void readNumbers(void) {
	// create file input stream
	ifstream file(FILENAME);
	string line;
	int i = 0;

	// read file and store numbers into shared memory object of unsorted numbers
	while (getline(file, line))
	{
		*(unsorted + i++) = atol(line.c_str());
	}
}

void childProcess(void) {
	size_t childSize = SIZE / NUM_OF_CHILDREN;	// size of child array
	size_t offset = (child_id - 1) * childSize;
	long subarray[childSize];	// sub array of sorted numbers to be added to main array of sorted numbers

	for (size_t i = 0; i < childSize; ++i) {
		subarray[i] = unsorted[i + offset]; // fill child's subarray with the 2500 numbers in its partition
	}

	sortMemory(subarray, childSize); // child uses selection sort to sort its 2500 numbers

	for (size_t i = 0; i < childSize; ++i) {
		if (i % RANGE == 0) {
			sem_wait(lock);
		}

		sorted[(*index)++] = subarray[i]; // insert into the sorted array 10 numbers at a time

		if (i % RANGE == RANGE - 1) {
			sem_post(lock);
		}
	}
}

//Algorithm derived from www.algolist.net/Algorithms/Sorting/Selection_sort
void sortMemory(long array[], size_t size) {
	for (size_t i = 0; i < size - 1; ++i) {
		size_t minIndex = i;

		for (size_t j = i + 1; j < size; ++j) {
			if (array[j] < array[minIndex]) {
				minIndex = j;
			}
		}

		if (minIndex != i) {
			long tmp = array[i];
			array[i] = array[minIndex];
			array[minIndex] = tmp;
		}
	}
}

void parentProcess(void) 
{
	//the previous index position to resume sorting from
	int prevIndex = 0;
	
	//index position where the last child was at
	int currentIndex = (int)index[0];
	while (prevIndex < SIZE - 1) {
		sem_wait(lock);
		currentIndex = (int)index[0];
	//	cout << "Got lock" << endl;
	//	cout << "counter " << index[0] << endl;
		if (currentIndex != prevIndex)
		{
			//sort a subsection of the sorted array
			sortAll(sorted, 0, currentIndex - 1);
			cout << "P" << getpid() << " finished sorting from 0 to " << currentIndex - 1 << endl; 
			prevIndex = currentIndex;
		}
		sem_post(lock);
		//cout << "Let go lock" << endl;
	}

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
