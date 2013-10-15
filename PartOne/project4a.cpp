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
#include <math.h>
#include "buffer.h"

using namespace std;

/* the buffer */
buffer_item buffer[BUFFER_SIZE];
buffer_item item;
pthread_mutex_t bufferMutex; // mutex lock
sem_t empty; // semaphore consumer increments producer decrements
sem_t full; // semaphore producer increments consumer decrements
uint timer; //sleep timer
int seed; //use to help seed random number
buffer_item *element; //pointer to the location of an element
int index; //use for selecting which element to remove

void *createProducer(void *param); /* threads call this function */
void *createConsumer(void *param); /* threads call this function */
int insert_item(buffer_item);
int remove_item(buffer_item *);
void printStuff(int);

int main(int argc, char *argv[]) {
	if (argc != 4) {
		cout << "!!!Invalid Arguments!!!\n_________________________" << endl;
		fprintf(stderr, "Format your arguments as follow: [output file] <integer # for sleep> <integer # for amount of producer threads> <integer # for amount of consumer threads>");
		return -1;
	}
	//initialize index to 0 and have element point to the memory address of the element at that index
	index = 0;
	element = &(buffer[index]);
	
	seed = 2;
	timer = atoi(argv[1]);
	int numProducer = atoi(argv[2]);
	int numConsumer = atoi(argv[3]);

	pthread_mutex_init(&bufferMutex, NULL);
	sem_init(&empty, 0, BUFFER_SIZE);
	sem_init(&full, 0, 0);

	pthread_t consumerThread[numConsumer]; /*create array of consumer threads*/
	pthread_t producerThread[numProducer]; /*create array of producer threads*/
	
	//create threads
	for (int i = 0; i < numConsumer; i++)
	{
		pthread_create(&consumerThread[i], NULL, createConsumer, (void*)((intptr_t) i));
	}
	
	for (int j = 0; j < numProducer; j++)
	{
		pthread_create(&producerThread[j], NULL, createProducer, (void*)((intptr_t) j));
	}

	/* wait for the thread to exit */
	for (int i = 0; i < numProducer; i++)
	{
		pthread_join(producerThread[i], NULL);
	}
	
	for (int j = 0; j < numConsumer; j++)
	{
		pthread_join(consumerThread[j], NULL);
	}

	// TODO These aren't in the right place.
	//pthread_mutex_init(&bufferMutex, NULL);
	//sem_init(&empty, 0, BUFFER_SIZE);
	//sem_init(&full, 0, 0);

	/* 2. Initialize buffer */
	/* 5. Sleep */
	/* 6. Exit */
}

void *createProducer(void *param) {
	do 
	{
		//printStuff(pthread_self());
		sleep(timer);
		sem_wait(&empty);
		pthread_mutex_lock(&bufferMutex);
		
		srand(pthread_self() * log(seed)); //log op on seed will generate new random numbers without having it become too large over time
		seed++;
		item = rand();
		//int id = *((int*)(&param));
		//cout << "Thread: " << id <<" Is Inserting " << item << endl;
		cout << "Inserting " << item << endl;
		if (insert_item(item) == 0)
		{
			cout << "Insertion Successful!" << endl;
			cout << "_____________________" << endl;
		}
		else
		{
			cout << "Insertion Unsuccessful." << endl;
		}
		
		pthread_mutex_unlock(&bufferMutex);
		sem_post(&full);
	} while(true);

	/*for (int i = 0; i < BUFFER_SIZE; i++)
	{
		cout << buffer[i] << endl;
	}*/	
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

	//cout << pthread_self() << endl;

	pthread_exit(NULL);
}

void *createConsumer(void *param) {
	do 
	{
		//printStuff(pthread_self());
		sem_wait(&full);
		pthread_mutex_lock(&bufferMutex);
		
		//srand(pthread_self() * log(seed));
		//seed++;
		//item = rand();
		//int id = *((int*)(&param));
		//cout << "Thread: " << id <<" Is Inserting " << item << endl;
		if (buffer[index] != 0)
		{
			cout << "Removing an item from buffer: " << buffer[index] << endl;
			if (remove_item(element) == 0)
			{
				cout << "Removal Successful!" << endl;
				cout << "_____________________" << endl;
			}
			else
			{
				cout << "Removal Unsuccessful." << endl;
			}
		}
		else
		{
			cout << "Element at " << index << " is empty." << endl;
			cout << "_____________________" << endl;
		}
		index = (index + 1) % BUFFER_SIZE;
		element = &(buffer[index]);
		
		pthread_mutex_unlock(&bufferMutex);
		sem_post(&empty);
	} while(true);

	/*for (int i = 0; i < BUFFER_SIZE; i++)
	{
		cout << buffer[i] << endl;
	}*/
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

	//cout << pthread_self() << endl;

	pthread_exit(NULL);
}

/* insert item into buffer.
return 0 if successful, otherwise
 return -1 indicating an error condition.*/
int insert_item(buffer_item it) {
	for (int i = 0; i < BUFFER_SIZE; i++)
	{
		if (buffer[i] == 0)
		{
			buffer[i] = it;
			return 0;
		}
	}
	return -1;
}

/*Remove an object from buffer
placing it in item.
return 0 if successful, otherwise
return -1 indicating an error condition.*/
int remove_item(buffer_item *it) {
	int temp = *it;
	for (int i = 0; i < BUFFER_SIZE; i++)
	{
		if (buffer[i] == temp)
		{
			//buffer[i] = 0;
			//it = 0;
			*it = 0;
			return 0;
		}	
	}
	return -1;
}

void printStuff(int tid)
{
	pthread_mutex_lock(&bufferMutex);
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
