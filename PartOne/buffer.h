// CS3242 Operating Systems
// Fall 2013
// Project 4: Process Synchronization, Part 1
// Steven Ng and Hue Moua
// Date: 10/14/2013
// File: buffer.h

#ifndef BUFFER_H_
#define BUFFER_H_

typedef int buffer_item;
#define BUFFER_SIZE 6

struct Buffer {
	size_t start;
	size_t end;
	size_t count;
	buffer_item buffer[BUFFER_SIZE];
	pthread_mutex_t mutex;
	sem_t empty;
	sem_t full;

	buffer_item& operator[](int i) {
		buffer_item& value = buffer[i];
		return value;
	}

	Buffer() {
		start = 0;
		end = 0;
		count = 0;

		/* create the mutex lock */
		pthread_mutex_init(&mutex, NULL);

		/* Create the semaphore and initialize it to BUFFER_SIZE */
		sem_init(&empty, 0, BUFFER_SIZE);

		/* Create the semaphore and initialize it to 0 */
		sem_init(&full, 0, 0);
	}

	/* insert item into buffer
	return 0 if successful, otherwise
	return -1 indicating an error condition */
	int insert_item(buffer_item it) {
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
	int remove_item(buffer_item *it) {
		if (0 == count) {
			return -1;
		}

		*it = buffer[start];
		start = (start + 1) % BUFFER_SIZE;
		--count;
		return 0;
	}
};

#endif /* BUFFER_H_ */
