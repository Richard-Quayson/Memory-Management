#include <stdio.h>  // For printf
#include "memory_config.h"
#include "virtual_memory.h"

#ifndef PHYSICAL_MEMORY_H
#define PHYSICAL_MEMORY_H

// Define the Frame structure
typedef struct Frame {
    int id;                             // Frame identifier
    Chunk chunks[FRAME_SIZE / KB];      // Array of chunks within the frame
    int is_allocated;                   // 1 if all chunks in the frame are allocated, 0 otherwise
} Frame;

// Define the PhysicalMemory structure
typedef struct PhysicalMemory {
    Frame frames[NUM_FRAMES];           // Array of frames in physical memory
    int remaining_memory;               // Remaining memory in physical memory
} PhysicalMemory;

// Function prototypes
VirtualMemory* initializeVirtualMemory(void); // Initialize virtual memory
PhysicalMemory* initializePhysicalMemory(void); // Initialize physical memory
void freeMemory(VirtualMemory* vm, PhysicalMemory* pm); // Free allocated memory for virtual and physical memory
void printVirtualMemory(const VirtualMemory* vm); // Print the contents of virtual memory
void printPhysicalMemory(const PhysicalMemory* pm); // Print the contents of physical memory
void printAllocatedVirtualMemory(const VirtualMemory* vm);
void printAllocatedFrameMemory(const PhysicalMemory* pm);

#endif // PHYSICAL_MEMORY_H