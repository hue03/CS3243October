// CS3243 Operating Systems
// Fall 2013
// Project 4: Process Synchronization, Part 2
// Steven Ng and Hue Moua
// Date: 10/28/2013
// File: parttwo.cpp

#define MAX_PROCESSES 60	// This will not ever change
#define PROCESS_COUNT 60	// useful when debugging to limit # of procs
#define MIN_BURST 100
#define MAX_BURST 5000
#define MIN_MEMORY_PER_PROC 4
#define MAX_MEMORY_PER_PROC 160
#define MAX_MEMORY 1040
#define MAX_BLOCK_PROC_RATIO 0.85
#define PRINT_INTERVAL 5000	// # of cpu quanta between memory map printouts
#define MAX_QUANTA 50000	// # quanta to run before ending simulation
#define ENABLE_COMPACTION 0
