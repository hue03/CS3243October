// CS3242 Operating Systems
// Fall 2013
// Project 4: Process Synchronization, Part 1
// Steven Ng and Hue Moua
// Date: 10/14/2013
// File: project4a.cpp

#include <cstdlib>
#include <iostream>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include "buffer.h"

#define SEED time(NULL)
#define P_RAND_SLEEP 10
#define C_RAND_SLEEP 10

using namespace std;

/* the buffer */
Buffer buffer;

bool running;

void *producer(void*); /* threads call this function */
void *consumer(void*); /* threads call this function */

string Color(size_t);

int main(int argc, char *argv[]) {
	/* 1. Get command line arguments argv[1],argv[2],argv[3] */

	if (argc != 4) {
		cout << "!!!Invalid Arguments!!!\n_______________________" << endl;
		fprintf(stderr,
				"Format your arguments as follow:  [output file] <integer # for sleep> <integer # for amount of producer threads> <integer # for amount of consumer threads>");
		return -1;
	}

	uint sleepTime = atoi(argv[1]);
	uint numProducer = atoi(argv[2]);
	uint numConsumer = atoi(argv[3]);

	/* 2. Initialize buffer */
	buffer = Buffer();

	running = true;

	srand(SEED);

	vector<pthread_t> threads;

	/* 3. Create producer thread(s) */
	for (uint i = 0; i < numProducer; ++i) {
		pthread_t thread;
		int n = i;

		pthread_create(&thread, NULL, producer, &n);

		threads.push_back(thread);
	}

	/* 4. Create consumer thread(s) */
	for (uint i = 0; i < numConsumer; ++i) {
		pthread_t thread;
		int n = i;

		pthread_create(&thread, NULL, consumer, &n);

		threads.push_back(thread);
	}

	/* 5. Sleep */
	sleep(sleepTime);

	/* 6. Exit */

	running = false;

	cout << "main() is exiting.";

	if (buffer.numEmpty() > 0 || buffer.numFull() < BUFFER_SIZE) {
		cout << " There are still threads actively running. FINISH THEM!";
	}

	cout << endl;

	for (uint i = 0; i < threads.size(); ++i) {
		pthread_join(threads.at(i), NULL);

		cout << "Joining thread " << (i + 1) << endl;
	}

	return 0;
}

void *producer(void *param) {
	int i = *(int*) param + 1;

	bool moreConsumers;

	while (running || moreConsumers) {
		/* sleep for a random period of time */
		sleep(rand() % P_RAND_SLEEP + 1);

		/* produce an item in next produced */
		/* generate a random number */
		buffer_item item = rand();

		sem_wait(&buffer.empty); /* acquire the semaphore */
		pthread_mutex_lock(&buffer.mutex); /* acquire the mutex lock */

		/* add next produced to the buffer */
		if (buffer.insert_item(item)) {
			printf("report error condition");
		} else {
			cout << Color(-1) << "Buffer size = " << buffer.count
					<< " | Producer " << i << " (ID: " << pthread_self()
					<< ") produced random number " << Color(buffer.end) << item
					<< Color(-1) << endl;
		}

		pthread_mutex_unlock(&buffer.mutex); /* release the mutex lock */
		sem_post(&buffer.full); /* release the semaphore */

		moreConsumers = (BUFFER_SIZE - buffer.numFull() > buffer.numEmpty());
	}

	pthread_exit(NULL);
}

void *consumer(void *param) {
	int i = *(int*) param + 1;

	bool moreProducers;

	while (running || moreProducers) {
		/* sleep for a random period of time */
		sleep(rand() % C_RAND_SLEEP + 1);

		buffer_item item;

		sem_wait(&buffer.full); /* acquire the semaphore */
		pthread_mutex_lock(&buffer.mutex); /* acquire the mutex lock */

		/* critical section */
		/* remove an item from buffer to next consumed */
		if (buffer.remove_item(&item)) {
			printf("report error condition");
		} else {
			/* consume the item in next consumed */
			cout << Color(-1) << "Buffer size = " << buffer.count
					<< " | Consumer " << i << " (ID: " << pthread_self()
					<< ") consumed random number " << Color(buffer.start)
					<< item << Color(-1) << endl;
		}

		pthread_mutex_unlock(&buffer.mutex); /* release the mutex lock */
		sem_post(&buffer.empty); /* release the semaphore */

		moreProducers = (buffer.numEmpty( ) > BUFFER_SIZE - buffer.numFull());
	}

	pthread_exit(NULL);
}

string Color(size_t n) {
	string result = "";

	switch (n) {
	case 0:
		return result + "\033[31m";
	case 1:
		return result + "\033[32m";
	case 2:
		return result + "\033[33m";
	case 3:
		return result + "\033[34m";
	case 4:
		return result + "\033[35m";
	case 5:
		return result + "\033[36m";
	}

	return "\033[0m";
}
