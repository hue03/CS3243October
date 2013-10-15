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

using namespace std;

/* the buffer */
buffer_item buffer[BUFFER_SIZE];

pthread_mutex_t bufferMutex = PTHREAD_MUTEX_INITIALIZER; // mutex lock
sem_t empty; // semaphore consumer increments producer decrements
sem_t full; // semaphore producer increments consumer decrements

void *createProducer(void *param); /* threads call this function */
void *createConsumer(void *param); /* threads call this function */
void printStuff(int);

int main(int argc, char *argv[]) {
	if (argc != 4) {
		fprintf(stderr, "usage: a.out <sleep> <numProducer> <numConsumer>");
		return -1;
	}

	int sleep = atoi(argv[1]);
	int numProducer = atoi(argv[2]);
	int numConsumer = atoi(argv[3]);

	pthread_t consumerThread[numConsumer]; /*create array of threads using the sum of the arguments*/
	pthread_t producerThread[ numProducer]; /*create array of threads using the sum of the arguments*/

	for (int i = 0; i < numConsumer; i++)
	{
		pthread_create(&consumerThread[i], NULL, createConsumer, 0);
	}
	for (int j = 0; j < numProducer; j++)
	{
		pthread_create(&producerThread[j], NULL, createProducer, 0);
	}

	/* wait for the thread to exit */
	for (int i = 0; i < numConsumer; i++)
	{
		pthread_join(consumerThread[i], NULL);
	}
	for (int j = 0; j < numProducer; j++)
	{
		pthread_join(producerThread[j], NULL);
	}

	// TODO These aren't in the right place.
	pthread_mutex_init(&bufferMutex, NULL);
	sem_init(&empty, 0, BUFFER_SIZE);
	sem_init(&full, 0, 0);

	/* 2. Initialize buffer */
	/* 5. Sleep */
	/* 6. Exit */
}

void *createProducer(void *param) {
//	do {
//		//. . .
//		/* produce an item in next produced */
//		//. . .
//		sem_wait(&empty);
//		pthread_mutex_lock(&bufferMutex);
//		//. . .
//		/* add next produced to the buffer */
//		//. . .
//		pthread_mutex_unlock(&bufferMutex);
//		sem_post(&full);
//	} while (true);

	/*
	 * Windows API Implementation
	 */

//	buffer_item item;
//
//	while (true) {
//		/* sleep for a random period of time */
//		sleep(...);
//		/* generate a random number */
//		item = rand();
//		if (insert_item(item))
//			printf("report error condition");
//		else
//			printf("producer produced %d\n", item);
//	}

	cout << pthread_self() << endl;

	pthread_exit(NULL);
}

void *createConsumer(void *param) {
//	do {
//		sem_wait(&full);
//		pthread_mutex_lock(&bufferMutex);
//		//. . .
//		/* remove an item from buffer to next consumed */
//		//. . .
//		pthread_mutex_unlock(&bufferMutex);
//		sem_post(&empty);
//		//. . .
//		/* consume the item in next consumed */
//		. . .
//	} while (true);

	/*
	 * Windows API Implementation
	 */

//	buffer_item item;
//
//	while (true) {
//		/* sleep for a random period of time */
//		sleep(...);
//		if (remove_item(item))
//			printf("report error condition");
//		else
//			printf("consumer consumed %d\n", item);
//	}

	cout << pthread_self() << endl;

	pthread_exit(NULL);
}

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

void printStuff(int tid)
{
	pthread_mutex_unlock(&bufferMutex);
	cout <<"Thread: " << tid << endl;
	pthread_mutex_unlock(&bufferMutex);
}
/*
 * Code from Section 4.4.1 - Pthreads
 */

//int main(int argc, char *argv[])
//{
//	pthread_attr_t attr; /* set of thread attributes */
//
//	/* get the default attributes */
//	pthread_attr_init(&attr);
//	/* create the thread */
//	pthread_create(&tid,&attr,runner,argv[1]);
//}
