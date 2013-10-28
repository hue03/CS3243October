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

typedef int buffer_item;
#define BUFFER_SIZE 6
#define SEED time(NULL)
#define P_RAND_SLEEP 6
#define C_RAND_SLEEP 6

// buffer implemented as a circular queue
struct Buffer {
	size_t count;	// distinguishes a full buffer from an empty one
	size_t start, end;
	buffer_item buffer[BUFFER_SIZE];
	pthread_mutex_t bufferMutex;	// mutex for writing to this buffer
	sem_t empty;	// semaphore for number of empty elements
	sem_t full;	// semaphore for number of full elements

	Buffer();	// construct new Buffer
	buffer_item& operator[](size_t);	// return reference to nth element
	int numEmpty();	// return value of "empty" semaphore
	int numFull();	// return value of "full" semaphore
	void outputCount();	// output buffer count

	/* insert item into buffer
	 return 0 if successful, otherwise
	 return -1 indicating an error condition */
	int insert_item(buffer_item);

	/* remove an object from buffer
	 placing it in item
	 return 0 if successful, otherwise
	 return -1 indicating an error condition */
	int remove_item(buffer_item*);
};

using namespace std;

/* the buffer */
Buffer buffer;

bool running;// true while threads work while main() sleeps; false when main() wakes and threads join main()
size_t numProducers;	// number of producer threads; specified by user
size_t numConsumers;	// number of consumer threads; specified by user
size_t producersLeft;// number of producers still running while threads join main()
size_t consumersLeft;// number of consumers still running while threads join main()
pthread_mutex_t output;	// mutex for cout and cerr
pthread_mutex_t mProducersLeft;	// mutex for producersLeft
pthread_mutex_t mConsumersLeft;	// mutex for consumersLeft

void *producer(void*); /* producer threads call this function */
void *consumer(void*); /* consumer threads call this function */

string color(size_t);// change color of output for random numbers; increases readability and traceability when random numbers are produced and consumed

int main(int argc, char *argv[]) {
	cout << color(-1);	// change color to white.

	/* 1. Get command line arguments argv[1],argv[2],argv[3] */

	// validate command line arguments
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

	// time, in seconds, that main() sleeps; specified by user
	uint sleepTime = atoi(argv[1]);

	numProducers = atoi(argv[2]);
	numConsumers = atoi(argv[3]);

	/* 2. Initialize buffer */
	buffer = Buffer();

	vector<pthread_t> threads;// vector of producer and consumer threads; used for joining threads

	producersLeft = 0;
	consumersLeft = 0;
	running = true;
	srand(SEED);
	pthread_mutex_init(&output, NULL);
	pthread_mutex_init(&mProducersLeft, NULL);
	pthread_mutex_init(&mConsumersLeft, NULL);

	/* 3. Create producer thread(s) */
	for (size_t i = 0; i < numProducers; ++i) {
		pthread_t thread;
		size_t *index = new size_t(i + 1);

		pthread_create(&thread, NULL, producer, index);
		threads.push_back(thread);
	}

	/* 4. Create consumer thread(s) */
	for (size_t i = 0; i < numConsumers; ++i) {
		pthread_t thread;
		size_t *index = new size_t(i + 1);

		pthread_create(&thread, NULL, consumer, index);
		threads.push_back(thread);
	}

	/* 5. Sleep */

	pthread_mutex_lock(&output);
	cout << "                         main() is sleeping." << endl;
	pthread_mutex_unlock(&output);

	sleep(sleepTime);

	/* 6. Exit */

	// indicate that threads should finish their work
	running = false;

	pthread_mutex_lock(&output);
	cout << "                         main() is exiting.  Joining threads."
			<< endl;
	pthread_mutex_unlock(&output);

	// join producer threads
	for (size_t i = 0; i < numProducers; ++i) {
		pthread_join(threads.at(i), NULL);

		pthread_mutex_lock(&output);
		cout << "                   P" << (i + 1) << "    joined with main()."
				<< endl;
		pthread_mutex_unlock(&output);
	}

	// join consumer threads
	for (size_t i = 0; i < numConsumers; ++i) {
		pthread_join(threads.at(i + numProducers), NULL);

		pthread_mutex_lock(&output);
		cout << "                      C" << (i + 1) << " joined with main()."
				<< endl;
		pthread_mutex_unlock(&output);
	}

	cout << "                         All threads joined with main()." << endl;

	return 0;
}

void *producer(void *param) {
	size_t *index = (size_t*) param;
	bool moreConsumers;	// true if there are more consumers left than there are buffer items.

	pthread_mutex_lock(&mProducersLeft);
	++producersLeft;

	pthread_mutex_lock(&output);
	cout << "                   P" << *index << "    created." << endl;
	pthread_mutex_unlock(&output);

	pthread_mutex_unlock(&mProducersLeft);

	do {
		if (running) {
			/* sleep for a random period of time */
			sleep(rand() % P_RAND_SLEEP + 1);
		}

		/* produce an item in next produced */
		// generate a random number
		buffer_item item = rand();

		// if cannot acquire "empty" semaphore ...
		if (buffer.numEmpty() == 0) {
			// outputs that it is waiting
			pthread_mutex_lock(&output);
			buffer.outputCount();
			cout << "P" << *index << "    waiting for Consumers." << endl;
			pthread_mutex_unlock(&output);
		}

		sem_wait(&buffer.empty); /* acquire the semaphore */

		// tries to lock the buffer mutex without blocking (i.e., trylock); if unavailable ...
		if (pthread_mutex_trylock(&buffer.bufferMutex) != 0) {
			// outputs that it is waiting
			pthread_mutex_lock(&output);
			cout << "                   P" << *index
					<< "    waiting for buffer." << endl;
			pthread_mutex_unlock(&output);

			// tries to lock the buffer with blocking (i.e., lock)
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
			buffer.outputCount();
			cout << "P" << *index << "    produced random number "
					<< color(buffer.end) << item << color(-1) << endl;
			pthread_mutex_unlock(&output);
		}

		pthread_mutex_unlock(&buffer.bufferMutex); /* release the mutex lock */
		sem_post(&buffer.full); /* release the semaphore */

		// if main() signals threads to stop running ...
		if (!running) {
			pthread_mutex_lock(&mConsumersLeft);
			pthread_mutex_lock(&mProducersLeft);
			moreConsumers = (consumersLeft > producersLeft);
			pthread_mutex_unlock(&mProducersLeft);
			pthread_mutex_unlock(&mConsumersLeft);

			if (moreConsumers) {
				pthread_mutex_lock(&output);
				buffer.outputCount();
				cout << "P" << *index
						<< "    won't join main() yet. There are more Consumers left than Producers left."
						<< endl;
				pthread_mutex_unlock(&output);
			}
		}
	} while (running || moreConsumers);
	// loop conditions
	// (1) main() is sleeping (running == true), or
	// (2) there are more consumers left than there are producers left.

	pthread_mutex_lock(&mProducersLeft);
	--producersLeft;
	pthread_mutex_unlock(&mProducersLeft);

	// note:  index was created with new in main()
	delete index;

	pthread_exit(NULL);
}

void *consumer(void *param) {
	size_t *index = (size_t*) param;
	bool moreProducers;	// true if there is at least one producer left

	pthread_mutex_lock(&mConsumersLeft);
	++consumersLeft;

	pthread_mutex_lock(&output);
	cout << "                      C" << *index << " created." << endl;
	pthread_mutex_unlock(&output);

	pthread_mutex_unlock(&mConsumersLeft);

	do {
		if (running) {
			/* sleep for a random period of time */
			sleep(rand() % C_RAND_SLEEP + 1);
		}

		buffer_item item;

		// if cannot acquire "full" semaphore ...
		if (0 == buffer.numFull()) {
			// outputs that it is waiting
			pthread_mutex_lock(&output);
			buffer.outputCount();
			cout << "   C" << *index << " waiting for Producers." << endl;
			pthread_mutex_unlock(&output);
		}

		sem_wait(&buffer.full); /* acquire the semaphore */

		// tries to lock the buffer mutex without blocking (i.e., trylock)
		// if unavailable ...
		if (pthread_mutex_trylock(&buffer.bufferMutex) != 0) {
			// outputs that it is waiting
			pthread_mutex_lock(&output);
			cout << "                      C" << *index
					<< " waiting for buffer." << endl;
			pthread_mutex_unlock(&output);

			// tries to lock the buffer with blocking (i.e., lock)
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
			buffer.outputCount();
			cout << "   C" << *index << " consumed random number "
					<< color(buffer.start) << item << color(-1) << endl;
			pthread_mutex_unlock(&output);
		}

		pthread_mutex_unlock(&buffer.bufferMutex); /* release the mutex lock */
		sem_post(&buffer.empty); /* release the semaphore */

		// if main() signals threads to stop running ...
		if (!running) {
			pthread_mutex_lock(&mProducersLeft);
			pthread_mutex_lock(&mConsumersLeft);
			moreProducers = (producersLeft > consumersLeft);
			pthread_mutex_unlock(&mConsumersLeft);
			pthread_mutex_unlock(&mProducersLeft);

			if (moreProducers) {
				pthread_mutex_lock(&output);
				cout << "                      C" << *index
						<< " won't join main() yet. There are more Producers left than Consumers left."
						<< endl;
				pthread_mutex_unlock(&output);
			}
		}
	} while (running || moreProducers);
	// loop conditions
	// (1) main() is sleeping, or
	// (2) there are more producers left than consumers left.

	pthread_mutex_lock(&mConsumersLeft);
	--consumersLeft;
	pthread_mutex_unlock(&mConsumersLeft);

	// note:  index was created with new in main()
	delete index;

	pthread_exit(NULL);
}

string color(size_t n) {
	string result = "";

	switch (n) {
	case 0:
		return result + "\033[31m"; // red
	case 1:
		return result + "\033[32m"; // green
	case 2:
		return result + "\033[33m"; // brown
	case 3:
		return result + "\033[34m"; // blue
	case 4:
		return result + "\033[35m"; // purple
	case 5:
		return result + "\033[36m"; // cyan
	}

	return "\033[0m"; // white
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

int Buffer::insert_item(buffer_item it) {
	if (BUFFER_SIZE == count) {
		return -1;
	}

	buffer[end] = it;
	end = (end + 1) % BUFFER_SIZE;
	++count;
	return 0;
}

int Buffer::remove_item(buffer_item *it) {
	if (0 == count) {
		return -1;
	}

	*it = buffer[start];
	start = (start + 1) % BUFFER_SIZE;
	--count;
	return 0;
}

buffer_item& Buffer::operator[](size_t i) {
	buffer_item &value = buffer[i];
	return value;
}

int Buffer::numEmpty() {
	int temp;
	sem_getvalue(&empty, &temp);
	return temp;
}

int Buffer::numFull() {
	int temp;
	sem_getvalue(&full, &temp);
	return temp;
}

void Buffer::outputCount() {
	cout << "Buffer count = " << count << " | ";
}
