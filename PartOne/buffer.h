// CS3242 Operating Systems
// Fall 2013
// Project 4: Process Synchronization, Part 1
// Steven Ng and Hue Moua
// Date: 10/14/2013
// File: buffer.h

#ifndef BUFFER_H_
#define BUFFER_H_

typedef int buffer_item;
#define BUFFER_SIZE 5

struct Buffer {
	size_t start;
	size_t end;
	buffer_item buffer[BUFFER_SIZE];

	buffer_item& operator[](int i) {
		buffer_item& value = buffer[i];
		return value;
	}

	Buffer() {
		start = 0;
		end = 0;
	}

	int insert_item(buffer_item it) {
		buffer[end] = it;
		end = (end + 1) % BUFFER_SIZE;
		return 0;
	}

	int remove_item(buffer_item *it) {
		*it = buffer[start];
		start = (start + 1) % BUFFER_SIZE;
		return 0;
	}
};

#endif /* BUFFER_H_ */
