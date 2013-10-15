// CS3242 Operating Systems
// Fall 2013
// Project 4: Process Synchronization, Part 1
// Steven Ng and Hue Moua
// Date: 10/14/2013
// File: project4a.cpp

#include <cstdlib>
#include <iostream>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include "buffer.h"

using namespace std;

/* the buffer */
Buffer buffer;
//buffer_item buffer[BUFFER_SIZE];
size_t a;
buffer_item item;// TODO This scope is too large.  It should probably be declared inside the producers and consumers.

pthread_mutex_t bufferMutex; // mutex lock
sem_t empty; // semaphore consumer increments producer decrements
sem_t full; // semaphore producer increments consumer decrements

int seed; //use to help seed random number

void *createProducer(void *param); /* threads call this function */
void *createConsumer(void *param); /* threads call this function */
int insert_item(buffer_item);
int remove_item(buffer_item *);
void printStuff(int);

void insert();

int main(int argc, char *argv[]) {
	if (argc != 4) {
		fprintf(stderr, "usage: a.out <sleepTime> <numProducer> <numConsumer>");
		return -1;
	}

	seed = 2;

	int sleepTime = atoi(argv[1]);
	int numProducer = atoi(argv[2]);
	int numConsumer = atoi(argv[3]);

	buffer = Buffer();

	pthread_mutex_init(&bufferMutex, NULL);
	sem_init(&empty, 0, BUFFER_SIZE);
	sem_init(&full, 0, 0);

	pthread_t producerThread[numProducer]; /*create array of threads using the sum of the arguments*/
	pthread_t consumerThread[numConsumer]; /*create array of threads using the sum of the arguments*/

//	for (int j = 0; j < numProducer; j++) {
//		pthread_create(&producerThread[j], NULL, createProducer, 0);
//	}
	for (int i = 0; i < numConsumer; i++) {
		pthread_create(&consumerThread[i], NULL, createConsumer, 0);
	}

	int n;

	do
	{
		n = -1;

		cout << "Enter how many random numbers into the buffer?\n";
		cin >> n;

		while (n < 0) {
			cin >> n;
		}

		n = min(n, BUFFER_SIZE);

		for (int i = 0; i < n; ++i) {
			insert();
		}
	} while (n > 0);

	sleep(sleepTime);

	/* wait for the thread to exit */
	for (int j = 0; j < numProducer; j++) {
		pthread_join(producerThread[j], NULL);
	}
	for (int i = 0; i < numConsumer; i++) {
		pthread_join(consumerThread[i], NULL);
	}

	/* 2. Initialize buffer */
}

void insert() {
	sleep(1);

	buffer_item item = rand();

	sem_wait(&empty);

	pthread_mutex_lock(&bufferMutex);

	if (buffer.insert_item(item)) {
		printf("report error condition");
	} else {
		switch(buffer.end) {
		case 0:
//			"\033[1;31mbold red text\033[0m\n"
//			\[\033[0;32m\]
			cout << "\033[32m";
			cout << "blah";
			break;
		case 1:
			cout << "\033[37m";
			break;
		case 2:
			cout << "\033[36m";
			system("Color 3C");
			break;
		case 3:
			cout << "\033[33m";
			system("Color 4D");
			break;
		case 4:
			cout << "\033[35m";
			system("Color 5E");
			break;
		}
		printf("Inserted %d\n", item);
	}

	pthread_mutex_unlock(&bufferMutex);
	sem_post(&full);
}

void *createProducer(void *param) {
	do {
		//printStuff(pthread_self());
		sem_wait(&empty);
		pthread_mutex_lock(&bufferMutex);

		srand(pthread_self() * log(seed));
		seed++;
		item = rand();
		cout << "Inserting " << item << endl;
		insert_item(item);

		pthread_mutex_unlock(&bufferMutex);
		sem_post(&full);
	} while (seed < BUFFER_SIZE);

	for (int i = 0; i < BUFFER_SIZE; i++) {
		buffer[i];
		cout << buffer[i] << endl;
	}
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
	for (;;) {
		sleep(1);

		buffer_item item;
		int size;

		do {
			sem_getvalue(&full, &size);
		} while (BUFFER_SIZE == size);

		sem_wait(&full);

		pthread_mutex_lock(&bufferMutex);

		if (buffer.remove_item(&item)) {
			printf("report error condition");
		} else {
			printf("Consumer %u consumed %d\n", pthread_self(), item);
		}

		pthread_mutex_unlock(&bufferMutex);
		sem_post(&empty);
	}

	pthread_exit(NULL);
}

int insert_item(buffer_item it) {
	/* insert item into buffer
	 return 0 if successful, otherwise
	 return -1 indicating an error condition */
	for (int i = 0; i < BUFFER_SIZE; i++) {
		if (buffer[i] == 0) {
			buffer[i] = it;
			return 0;
		}
	}
	return -1;
}

int remove_item(buffer_item *it) {
	/* remove an object from buffer
	 placing it in item
	 return 0 if successful, otherwise
	 return -1 indicating an error condition */
	return 0;
}

void printStuff(int tid) {
	pthread_mutex_lock(&bufferMutex);
	cout << "Thread: " << tid << endl;
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
