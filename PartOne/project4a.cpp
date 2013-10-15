// CS3242 Operating Systems
// Fall 2013
// Project 4: Process Synchronization, Part 1
// Steven Ng and Hue Moua
// Date: 10/14/2013
// File: project4a.cpp

/*
 * Code from Section 5.7.1 - The Bounded Buffer Problem
 */

/*
 * Data structures shared by the producer and consumer processes.
 */

//int n;
//semaphore mutex = 1;
//semaphore empty = n;
//semaphore full = 0

/*
 * The structure of the producer process.
 */

//do {
//	. . .
//	/* produce an item in next produced */
//	. . .
//	wait(empty);
//	wait(mutex);
//	. . .
//	/* add next produced to the buffer */
//	. . .
//	signal(mutex);
//	signal(full);
//} while (true);

/*
 * The structure of the consumer process.
 */

//do {
//	wait(full);
//	wait(mutex);
//	. . .
//	/* remove an item from buffer to next consumed */
//	. . .
//	signal(mutex);
//	signal(empty);
//	. . .
//	/* consume the item in next consumed */
//	. . .
//} while (true);

/*
 * Code from Programming Project description
 */

/*
 * Outline of buffer operations.
 */

//#include "buffer.h"
//
///* the buffer */
//buffer item buffer[BUFFER SIZE];
//
//int insert item(buffer item item) {
//	/* insert item into buffer
//	return 0 if successful, otherwise
//	return -1 indicating an error condition */
//}
//
//int remove item(buffer item) {
//	/* remove an object from buffer
//	placing it in item
//	return 0 if successful, otherwise
//	return -1 indicating an error condition */
//}

/*
 * The buffer header file
 */

///* buffer.h */
//typedef int buffer item;
//#define BUFFER SIZE 5

/*
 * The main() function
 */

//#include "buffer.h"
//int main(int argc, char *argv[]) {
//	/* 1. Get command line arguments argv[1],argv[2],argv[3] */
//	/* 2. Initialize buffer */
//	/* 3. Create producer thread(s) */
//	/* 4. Create consumer thread(s) */
//	/* 5. Sleep */
//	/* 6. Exit */
//}

/*
 * Code from Section 4.4.1 - Pthreads
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
//
//int sum; /* this data is shared by the thread(s) */
pthread_mutex_t bufferMutex = PTHREAD_MUTEX_INITIALIZER; //mutex lock for the shared buffer
//semaphore empty = n; //semaphore consumer increments producer decrements
//semaphore full = 0 //semaphore producer increments consumer decrements

void *createProducer(void *param); /* threads call this function */
void *createConsumer(void *param); /* threads call this function */

int main(int argc, char *argv[])
{
	pthread_t tid[numConsumers + numProducers]; /*create array of threads using the sum of the arguments*/
	//pthread attr t attr; /* set of thread attributes */

	if (argc != 2) {
		fprintf(stderr,"usage: a.out <integer value>\n");
		return -1;
	}
	if (atoi(argv[1]) < 0) {
		fprintf(stderr,"%d must be >= 0\n",atoi(argv[1]));
		return -1;
	}

	/* get the default attributes */
	//pthread attr init(&attr);
	/* create the thread */
	for (int i = 0; i < numConsumers; i++)
	{
		pthread_create(&tid[i], NULL, createConsumer, void*);
	}
	for (int j = 0; j < numProducers; j++)
	{
		pthread_create(&tid[j], NULL, createConsumer, void*);
	}	

	/* wait for the thread to exit */
	for (int i = 0; i < numConsumers; i++)
	{
		pthread_join(tid[i], NULL);
	}
	for (int j = 0; j < numConsumers; j++)
	{
		pthread_join(tid[j], NULL);
	}

//	printf("sum = %d\n",sum);
}
//
///* The thread will begin control in this function */
//void *runner(void *param)
//{
//	int i, upper = atoi(param);
//	sum = 0;
//
//	for (i = 1; i <= upper; i++)
//		sum += i;
//
//	pthread exit(0);
//}

/*
 * pthread.h header file
 */

//#define NUM THREADS 10
//
///* an array of threads to be joined upon */
//pthread t workers[NUM THREADS];
//
//for (int i = 0; i < NUM THREADS; i++)
//	pthread join(workers[i], NULL);

/*
 * Code from Section 5.9.4 - Pthreads Synchronization
 */

/*
 * Creating a mutex.
 */

//#include <pthread.h>
//
//pthread mutex t mutex;
//
///* create the mutex lock */
//pthread mutex init(&mutex,NULL);

/*
 * Protecting a critical section with mutex locks.
 */

///* acquire the mutex lock */
//pthread mutex lock(&mutex);
//
///* critical section */
//
///* release the mutex lock */
//pthread mutex unlock(&mutex);

/*
 * Creating and initializing an unnamed semaphore.
 */

//#include <semaphore.h>
//sem t sem;
//
///* Create the semaphore and initialize it to 1 */
//sem init(&sem, 0, 1);

/*
 * Protecting a critical section using a semaphore.
 */

//* acquire the semaphore */
//sem wait(&sem);
//
///* critical section */
//
///* release the semaphore */
//sem post(&sem);

