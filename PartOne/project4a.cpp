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

int n;
sem_t mutex /*= 1*/;
sem_t empty /*= n*/;
sem_t full /*= 0*/;

int main(int argc, char *argv[]) {
	int sleep, numProducer, numConsumer;

	//	if (argc != 2) {
	//		fprintf(stderr,"usage: a.out <integer value>\n");
	//		return -1;
	//	}
	if (argc != 4) {
		fprintf(stderr, "usage: a.out <sleep> <numProducer> <numConsumer>");
	} else {
		sleep = atoi(argv[1]);
		numProducer = atoi(argv[2]);
		numConsumer = atoi(argv[3]);
	}
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

<<<<<<< HEAD
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

=======
//int sum; /* this data is shared by the thread(s) */
//void *runner(void *param); /* threads call this function */
//
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
>>>>>>> fa68a275fac2633a10d3681de017eac686a27b14
//	printf("sum = %d\n",sum);
}
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
<<<<<<< HEAD
//sem post(&sem);

=======
//sem_post(&sem);
>>>>>>> fa68a275fac2633a10d3681de017eac686a27b14
