// CS3242 Operating Systems
// Fall 2013
// Project 4: Process Synchronization, Part 1
// Steven Ng and Hue Moua
// Date: 10/14/2013
// File: project4a.cpp

#include <cstdlib>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include "buffer.h"

/* the buffer */
buffer_item buffer[BUFFER_SIZE];

pthread_mutex_t mutex;
sem_t empty;
sem_t full;

int main(int argc, char *argv[]) {
	int sleep, numProducer, numConsumer;

	if (argc != 4) {
		fprintf(stderr, "usage: a.out <sleep> <numProducer> <numConsumer>");
		return -1;
	} else {
		sleep = atoi(argv[1]);
		numProducer = atoi(argv[2]);
		numConsumer = atoi(argv[3]);
	}

	pthread_mutex_init(&mutex,NULL);
	sem_init(&empty, 0, BUFFER_SIZE);
	sem_init(&full, 0, 0);

	/* 2. Initialize buffer */
	/* 3. Create producer thread(s) */
	/* 4. Create consumer thread(s) */
	/* 5. Sleep */
	/* 6. Exit */
}

int insert_item(buffer_item item) {
	/* insert item into buffer
	return 0 if successful, otherwise
	return -1 indicating an error condition */

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
}

int remove_item(buffer_item item) {
	/* remove an object from buffer
	placing it in item
	return 0 if successful, otherwise
	return -1 indicating an error condition */

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
}

/*
 * Code from Section 4.4.1 - Pthreads
 */

//int sum; /* this data is shared by the thread(s) */
//void *runner(void *param); /* threads call this function */
//
//int main(int argc, char *argv[])
//{
//	pthread_t tid; /* the thread identifier */
//	pthread_attr_t attr; /* set of thread attributes */
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
//
///* The thread will begin control in this function */
//void *runner(void *param)
//{
//	int i, upper = atoi((char*)param);
//	sum = 0;
//
//	for (i = 1; i <= upper; i++)
//		sum += i;
//
//	pthread_exit(0);
//}

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
 * Protecting a critical section using a semaphore.
 */

//* acquire the semaphore */
//sem_wait(&sem);
//
///* critical section */
//
///* release the semaphore */
//sem_post(&sem);
