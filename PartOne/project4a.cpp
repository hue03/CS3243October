// CS3242 Operating Systems
// Fall 2013
// Project 4: Process Synchronization, Part 1
// Steven Ng and Hue Moua
// Date: 10/14/2013
// File: project4a.cpp

#include "buffer.h"
#include <semaphore.h>
#include <stdio.h>
#include <iostream>
#include <pthread.h>
#include <cstdlib>

/* the buffer */
buffer_item buffer[BUFFER_SIZE];

int n;
sem_t mutex /*= 1*/;
sem_t empty /*= n*/;
sem_t full /*= 0*/;

int insert_item(buffer_item item) {
	/* insert item into buffer
	return 0 if successful, otherwise
	return -1 indicating an error condition */
}

int remove_item(buffer_item item) {
	/* remove an object from buffer
	placing it in item
	return 0 if successful, otherwise
	return -1 indicating an error condition */
}

//1. How long to sleep before terminating
//2. The number of producer threads
//3. The number of consumer threads

int main(int argc, char *argv[]) {
	int sleep, numProducer, numConsumer;

	if (argc != 4) {
		std::cout << "You entered the wrong number of command-line arguments."
				<< "How long should main sleep before terminating?\n";
		std::cin >> sleep;
		std::cout << "How many producer threads are there?\n";
		std::cin >> numProducer;
		std::cout << "How many consumer threads are there?\n";
		std::cin >> numConsumer;
	} else {
		sleep = atoi(argv[1]);
		numProducer = atoi(argv[2]);
		numConsumer = atoi(argv[3]);
	}
	/* 1. Get command line arguments argv[1],argv[2],argv[3] */
	/* 2. Initialize buffer */
	/* 3. Create producer thread(s) */
	/* 4. Create consumer thread(s) */
	/* 5. Sleep */
	/* 6. Exit */
}

/*
 * Code from Section 5.7.1 - The Bounded Buffer Problem
 */

/*
 * Data structures shared by the producer and consumer processes.
 */

/*
 * The structure of the producer process.
 */

//do {
//	//. . .
//	/* produce an item in next produced */
//	//. . .
//	sem_wait(&empty);
//	sem_wait(&mutex);
//	//. . .
//	/* add next produced to the buffer */
//	//. . .
//	sem_post(&mutex);
//	sem_post(&full);
//} while (true);

/*
 * The structure of the consumer process.
 */

//do {
//	sem_wait(&full);
//	sem_wait(&mutex);
//	//. . .
//	/* remove an item from buffer to next consumed */
//	//. . .
//	sem_post(&mutex);
//	sem_post(&empty);
//	//. . .
//	/* consume the item in next consumed */
//	. . .
//} while (true);

/*
 * Code from Section 4.4.1 - Pthreads
 */

int sum; /* this data is shared by the thread(s) */
void *runner(void *param); /* threads call this function */

//int main(int argc, char *argv[])
//{
//	pthread_t tid; /* the thread identifier */
//	pthread_attr_t attr; /* set of thread attributes */
//
//	if (argc != 2) {
//		fprintf(stderr,"usage: a.out <integer value>\n");
//		return -1;
//	}
//	if (atoi(argv[1]) < 0) {
//		fprintf(stderr,"%d must be >= 0\n",atoi(argv[1]));
//		return -1;
//	}
//
//	/* get the default attributes */
//	pthread_attr_init(&attr);
//	/* create the thread */
//	pthread_create(&tid,&attr,runner,argv[1]);
//	/* wait for the thread to exit */
//	pthread_join(tid,NULL);
//
//	printf("sum = %d\n",sum);
//}

/* The thread will begin control in this function */
void *runner(void *param)
{
	int i, upper = atoi((char*)param);
	sum = 0;

	for (i = 1; i <= upper; i++)
		sum += i;

	pthread_exit(0);
}

/*
 * Joining multiple threads.
 */

//#define NUM_THREADS 10
//
///* an array of threads to be joined upon */
//pthread_t workers[NUM THREADS];
//
//for (int i = 0; i < NUM_THREADS; i++)
//	pthread_join(workers[i], NULL);

/*
 * Code from Section 5.9.4 - Pthreads Synchronization
 */

/*
 * Creating a mutex.
 */

//#include <pthread.h>
//
//pthread_mutex_t mutex;
//
///* create the mutex lock */
//pthread_mutex_init(&mutex,NULL);

/*
 * Protecting a critical section with mutex locks.
 */

///* acquire the mutex lock */
//pthread_mutex_lock(&mutex);
//
///* critical section */
//
///* release the mutex lock */
//pthread_mutex_unlock(&mutex);

/*
 * Creating and initializing an unnamed semaphore.
 */

//#include <semaphore.h>
//sem_t sem;
//
///* Create the semaphore and initialize it to 1 */
//sem_init(&sem, 0, 1);

/*
 * Protecting a critical section using a semaphore.
 */

//* acquire the semaphore */
//sem_wait(&sem);
//
///* critical section */
//
///* release the semaphore */
//sem_post(&sem);
