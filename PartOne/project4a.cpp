// CS3242 Operating Systems
// Fall 2013
// Project 4: Process Synchronization, Part 1
// Steven Ng and Hue Moua
// Date: 10/14/2013
// File: project4a.cpp

#include <cstdlib>
#include <iostream>
#include <list>
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

// true while the threads work while main() sleeps
// false when main() wakes and joins the threads
bool running;

// number of producer threads specified by the user
uint numProducers;

// number of consumer threads specified by the user
uint numConsumers;

// keeps track of the producers that are still running while main() is joining threads
uint producersLeft;

// keeps track of the consumers that are still running while main() is joining threads
uint consumersLeft;

// a mutex for cout and cerr
pthread_mutex_t output;

// a mutex for producersLeft
pthread_mutex_t mProducersLeft;

// a mutex for consumersLeft
pthread_mutex_t mConsumersLeft;

void *producer(void*); /* producer threads call this function */

void *consumer(void*); /* consumer threads call this function */

// changes the color of the output for random numbers
// increases readability and tracing of when random numbers are produced and consumed
string color(size_t);

int main(int argc, char *argv[]) {
	/* 1. Get command line arguments argv[1],argv[2],argv[3] */

	if (argc != 4) {
		cerr << "!!!Invalid Arguments!!!" << endl << "_______________________"
				<< endl << "Format your arguments as follow:" << endl
				<< "[output file] <positive integer # for sleep time> <positive integer # for amount of producer threads> <positive integer # for amount of consumer threads>"
				<< endl;
		return -1;
	} else if (atoi(argv[1]) <= 0 || atoi(argv[2]) <= 0 || atoi(argv[3]) <= 0) {
		cerr << "One or more of the arguments need to be greater than 0."
				<< endl;
		return -1;
	}

	// the length of time that main() sleeps; supplied by the user
	uint sleepTime = atoi(argv[1]);

	numProducers = atoi(argv[2]);
	numConsumers = atoi(argv[3]);

	/* 2. Initialize buffer */
	buffer = Buffer();

	// the list of producer and consumer threads
	// used for joining threads
	vector<pthread_t> threads;

	producersLeft = 0;
	consumersLeft = 0;
	running = true;

	srand(SEED);
	pthread_mutex_init(&output, NULL);

	cout << color(-1);

	/* 3. Create producer thread(s) */
	for (uint i = 0; i < numProducers; ++i) {
		// The thread to be created
		pthread_t thread;

		// The number of the producer
		int pNum = i + 1;

		pthread_create(&thread, NULL, producer, &pNum);

		pthread_mutex_lock(&mProducersLeft);
		++producersLeft;
		pthread_mutex_unlock(&mProducersLeft);

		pthread_mutex_lock(&output);
		cout << "Producer " << pNum << " thread created." << endl;
		pthread_mutex_unlock(&output);

		threads.push_back(thread);
	}

	/* 4. Create consumer thread(s) */
	for (uint i = 0; i < numConsumers; ++i) {
		// The thread to be created
		pthread_t thread;

		// The number of the consumer
		int cNum = i + 1;

		pthread_mutex_lock(&mConsumersLeft);
		++consumersLeft;
		pthread_mutex_unlock(&mConsumersLeft);

		pthread_mutex_lock(&output);
		cout << "Consumer " << cNum << " thread created." << endl;
		pthread_mutex_unlock(&output);

		pthread_create(&thread, NULL, consumer, &cNum);

		threads.push_back(thread);
	}

	/* 5. Sleep */

	pthread_mutex_lock(&output);
	cout << "main() sleeping." << endl;
	pthread_mutex_unlock(&output);

	sleep(sleepTime);

	/* 6. Exit */

	pthread_mutex_lock(&output);
	cout << "main() is exiting.  Joining threads." << endl;
	pthread_mutex_unlock(&output);

	// indicates that threads should finish their work
	running = false;

	for (uint i = 0; i < numProducers; ++i) {
		pthread_mutex_lock(&output);
		cout << "Joining P" << (i + 1) << endl;
		pthread_mutex_unlock(&output);

		pthread_join(threads.at(i), NULL);
	}

	for (uint i = numProducers; i < numConsumers; ++i) {
		pthread_mutex_lock(&output);
		cout << "Joining    C" << (i + 1) << endl;
		pthread_mutex_unlock(&output);

		pthread_join(threads.at(i), NULL);
	}

	cout << "All threads joined with main()." << endl;

	return 0;
}

void *producer(void *param) {
	// The number of the producer
	uint i = *(uint*) param;

	// true if there are more consumers than producers while main() is joining threads
	bool moreConsumers;

	// true if the number of this producer is the lowest of the threads that haven't joined main() yet
	bool lowestProducer;

	// true if this thread should proceed with the do-while loop
	// consists of moreConsumers && lowestProducer
	bool proceed;

	do {
		/* sleep for a random period of time */
		sleep(rand() % P_RAND_SLEEP + 1);

		/* produce an item in next produced */
		// generate a random number
		buffer_item item = rand();

		// checks empty semaphore
		// if unavailable, then outputs that it is waiting
		if (0 == buffer.numEmpty()) {
			pthread_mutex_lock(&output);
			buffer.printCount();
			cout << "P" << i << "    waiting for Consumers." << endl;
			pthread_mutex_unlock(&output);
		}

		sem_wait(&buffer.empty); /* acquire the semaphore */

		// tries to lock the buffer mutex without blocking
		// if unavailable, then outputs that it is waiting
		if (pthread_mutex_trylock(&buffer.bufferMutex) != 0) {
			pthread_mutex_lock(&output);
			buffer.printCount();
			cout << "P" << i << "    waiting for buffer." << endl;
			pthread_mutex_unlock(&output);

			// blocks while waiting for mutex lock
			pthread_mutex_lock(&buffer.bufferMutex); /* acquire the mutex lock */
		}

		/* critical section */
		// add next produced to the buffer
		if (buffer.insert_item(item)) {
			pthread_mutex_lock(&output);
			cerr << "report error condition" << endl;
			pthread_mutex_unlock(&output);
		} else {
			pthread_mutex_lock(&output);
			buffer.printCount();
			cout << "P" << i << "    produced random number "
					<< color(buffer.end) << item << color(-1) << endl;
			pthread_mutex_unlock(&output);
		}

		pthread_mutex_unlock(&buffer.bufferMutex); /* release the mutex lock */
		sem_post(&buffer.full); /* release the semaphore */

		// if main() signaled threads to stop,
		// then calculates moreConsumers, lowestProducer, and proceed
		if (!running) {
			pthread_mutex_lock(&mProducersLeft);
			moreConsumers = consumersLeft > producersLeft;
			lowestProducer = i > numProducers - producersLeft;
			pthread_mutex_unlock(&mProducersLeft);

			proceed = moreConsumers && lowestProducer;
		}
	} while (running || proceed);
	// loops while
	// (1) main() is still sleeping (running == true) or
	// (2) moreConsumers == true && lowestProducer == true

	pthread_mutex_lock(&output);
	buffer.printCount();
	cout << "P" << i << "    ready to join main()" << endl;
	pthread_mutex_unlock(&output);

	pthread_mutex_lock(&mProducersLeft);
	--producersLeft;
	pthread_mutex_unlock(&mProducersLeft);

	pthread_exit(NULL);
}

void *consumer(void *param) {
	// The number of the producer
	uint i = *(uint*) param;

	// true if there are more producers than consumers while main() is joining threads
	bool moreProducers;

	// true if the number of this consumer is the lowest of the threads that haven't joined main() yet
	bool lowestConsumer;

	// true if this thread should proceed with the do-while loop
	// consists of moreProducers && lowestConsumer
	bool proceed;

	do {
		/* sleep for a random period of time */
		sleep(rand() % C_RAND_SLEEP + 1);

		buffer_item item;

		// checks full semaphore
		// if unavailable, then outputs that it is waiting
		if (0 == buffer.numFull()) {
			pthread_mutex_lock(&output);
			buffer.printCount();
			cout << "   C" << i << " waiting for Producers." << endl;
			pthread_mutex_unlock(&output);
		}

		sem_wait(&buffer.full); /* acquire the semaphore */

		// tries to lock the buffer mutex without blocking
		// if unavailable, then outputs that it is waiting
		if (pthread_mutex_trylock(&buffer.bufferMutex) != 0) {
			pthread_mutex_lock(&output);
			buffer.printCount();
			cout << "   C" << i << " waiting for buffer." << endl;
			pthread_mutex_unlock(&output);

			pthread_mutex_lock(&buffer.bufferMutex); /* acquire the mutex lock */
		}

		/* critical section */
		/* remove an item from buffer to next consumed */
		if (buffer.remove_item(&item)) {
			pthread_mutex_lock(&output);
			cerr << "report error condition" << endl;
			pthread_mutex_unlock(&output);
		} else {
			/* consume the item in next consumed */
			pthread_mutex_lock(&output);
			buffer.printCount();
			cout << "   C" << i << " consumed random number "
					<< color(buffer.start) << item << color(-1) << endl;
			pthread_mutex_unlock(&output);
		}

		pthread_mutex_unlock(&buffer.bufferMutex); /* release the mutex lock */
		sem_post(&buffer.empty); /* release the semaphore */

		// if main() signalled threads to stop,
		// then calculates moreProducers, lowestConsumer, and proceed
		if (!running) {
			pthread_mutex_lock(&mConsumersLeft);
			moreProducers = producersLeft > consumersLeft;
			lowestConsumer = i > numConsumers - consumersLeft;
			pthread_mutex_unlock(&mConsumersLeft);

			proceed = moreProducers && lowestConsumer;
		}
	} while (running || (proceed && buffer.count > 0));
	// loops while
	// (1) main() is still sleeping (running == true) or
	// (2) moreConsumers == true && lowestProducer == true

	pthread_mutex_lock(&output);
	buffer.printCount();
	cout << "   C" << i << " ready to join main()" << endl;
	pthread_mutex_unlock(&output);

	pthread_mutex_lock(&mConsumersLeft);
	--consumersLeft;
	pthread_mutex_unlock(&mConsumersLeft);

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

buffer_item& Buffer::operator[](int i) {
	buffer_item& value = buffer[i];
	return value;
}

Buffer::Buffer() {
	start = 0;
	end = 0;
	count = 0;

	/* create the mutex lock */
	pthread_mutex_init(&bufferMutex, NULL);

	/* Create the semaphore and initialize it to BUFFER_SIZE */
	sem_init(&empty, 0, BUFFER_SIZE);

	/* Create the semaphore and initialize it to 0 */
	sem_init(&full, 0, 0);
}

/* insert item into buffer
 return 0 if successful, otherwise
 return -1 indicating an error condition */
int Buffer::insert_item(buffer_item it) {
	if (BUFFER_SIZE == count) {
		return -1;
	}

	buffer[end] = it;
	end = (end + 1) % BUFFER_SIZE;
	++count;
	return 0;
}

/* remove an object from buffer
 placing it in item
 return 0 if successful, otherwise
 return -1 indicating an error condition */
int Buffer::remove_item(buffer_item *it) {
	if (0 == count) {
		return -1;
	}

	*it = buffer[start];
	start = (start + 1) % BUFFER_SIZE;
	--count;
	return 0;
}

// returns the value of the empty semaphore
int Buffer::numEmpty() {
	int temp;

	sem_getvalue(&empty, &temp);

	return temp;
}

// returns the value of the full semaphore
int Buffer::numFull() {
	int temp;

	sem_getvalue(&full, &temp);

	return temp;
}

// prints the current buffer count
void Buffer::printCount() {
	std::cout << "Buffer count = " << count << " | ";
}
