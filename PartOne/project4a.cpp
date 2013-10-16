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
#define P_RAND_SLEEP 9
#define C_RAND_SLEEP 9

using namespace std;

/* the buffer */
Buffer buffer;

bool running;

pthread_mutex_t output;

void *producer(void*); /* threads call this function */
void *consumer(void*); /* threads call this function */

string color(size_t);

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

	vector<pthread_t> threads;

	srand(SEED);

	running = true;

	pthread_mutex_init(&output, NULL);

	cout << color(-1);

	/* 3. Create producer thread(s) */
	for (uint i = 0; i < numProducer; ++i) {
		pthread_t thread;
		int n = i;

		pthread_create(&thread, NULL, producer, &n);

		pthread_mutex_lock(&output);
		cout << "Producer " << (i + 1) << " thread created." << endl;
		pthread_mutex_unlock(&output);

		threads.push_back(thread);
	}

	/* 4. Create consumer thread(s) */
	for (uint i = 0; i < numConsumer; ++i) {
		pthread_t thread;
		int n = i;

		pthread_mutex_lock(&output);
		cout << "Consumer " << (i + 1) << " thread created." << endl;
		pthread_mutex_unlock(&output);

		pthread_create(&thread, NULL, consumer, &n);

		threads.push_back(thread);
	}

	/* 5. Sleep */
	pthread_mutex_lock(&output);
	cout << "main() sleeping." << endl;
	pthread_mutex_unlock(&output);

	sleep(sleepTime);

	/* 6. Exit */

	running = false;

	pthread_mutex_lock(&output);
	cout << "main() is exiting.";
	pthread_mutex_unlock(&output);

	if (buffer.numEmpty() > 0 || buffer.numFull() < BUFFER_SIZE) {
		pthread_mutex_lock(&output);
		cout << " There are still threads actively running. FINISH THEM!";
		pthread_mutex_unlock(&output);
	}

	cout << endl;

	for (uint i = 0; i < threads.size(); ++i) {
		pthread_join(threads.at(i), NULL);

		pthread_mutex_lock(&output);
		cout << "Joining thread " << (i + 1) << endl;
		pthread_mutex_unlock(&output);
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

		if (0 == buffer.numEmpty()) {
			pthread_mutex_lock(&output);
			buffer.printCount();
			cout << "P" << i << "    waiting for Consumers." << endl;
			pthread_mutex_unlock(&output);
		}

		sem_wait(&buffer.empty); /* acquire the semaphore */

		if (pthread_mutex_trylock(&buffer.mutex) != 0) {
			pthread_mutex_lock(&output);
			buffer.printCount();
			cout << "P" << i << "    waiting for buffer." << endl;
			pthread_mutex_unlock(&output);

			pthread_mutex_lock(&buffer.mutex); /* acquire the mutex lock */
		}

		/* critical section */
		/* add next produced to the buffer */
		if (buffer.insert_item(item)) {
			pthread_mutex_lock(&output);
			printf("report error condition");
			pthread_mutex_unlock(&output);
		} else {
			pthread_mutex_lock(&output);
			buffer.printCount();
			cout << "P" << i << "    produced random number "
					<< color(buffer.end) << item << color(-1) << endl;
			pthread_mutex_unlock(&output);
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

		if (0 == buffer.numFull()) {
			pthread_mutex_lock(&output);
			buffer.printCount();
			cout << "   C" << i << " waiting for Producers." << endl;
			pthread_mutex_unlock(&output);
		}

		sem_wait(&buffer.full); /* acquire the semaphore */

		if (pthread_mutex_trylock(&buffer.mutex) != 0) {
			pthread_mutex_lock(&output);
			buffer.printCount();
			cout << "   C" << i << " waiting for buffer." << endl;
			pthread_mutex_unlock(&output);

			pthread_mutex_lock(&buffer.mutex); /* acquire the mutex lock */
		}

		/* critical section */
		/* remove an item from buffer to next consumed */
		if (buffer.remove_item(&item)) {
			pthread_mutex_lock(&output);
			printf("report error condition");
			pthread_mutex_unlock(&output);
		} else {
			/* consume the item in next consumed */
			pthread_mutex_lock(&output);
			buffer.printCount();
			cout << "   C" << i << " consumed random number "
					<< color(buffer.start) << item << color(-1) << endl;
			pthread_mutex_unlock(&output);
		}

		pthread_mutex_unlock(&buffer.mutex); /* release the mutex lock */
		sem_post(&buffer.empty); /* release the semaphore */

		moreProducers = (buffer.numEmpty() > BUFFER_SIZE - buffer.numFull());
	}

	pthread_exit(NULL);
}

string color(size_t n) {
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
