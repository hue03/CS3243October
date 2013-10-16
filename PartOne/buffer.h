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
	pthread_mutex_t bufferMutex;
	sem_t empty;
	sem_t full;

	buffer_item& operator[](int);

	Buffer();

	/* insert item into buffer
	return 0 if successful, otherwise
	return -1 indicating an error condition */
	int insert_item(buffer_item);

	/* remove an object from buffer
	placing it in item
	return 0 if successful, otherwise
	return -1 indicating an error condition */
	int remove_item(buffer_item*);

	int numEmpty();

	int numFull();

	void printCount();
};

#endif /* BUFFER_H_ */
