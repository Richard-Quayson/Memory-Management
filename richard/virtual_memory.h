#include "memory_config.h"

#ifndef VIRTUAL_MEMORY_H
#define VIRTUAL_MEMORY_H

// Define the Chunk structure
typedef struct Chunk {
    int offset; // Offset within the page, from 0 to PAGE_SIZE - 1
    int is_allocated; // 1 if the chunk is allocated, 0 otherwise
} Chunk;

// Define the Page structure
typedef struct Page {
    int id; // Page identifier
    Chunk chunks[PAGE_SIZE / KB]; // Array of chunks within the page
    int is_allocated; // 1 if all chunks in the page are allocated, 0 otherwise
} Page;

// Define the VirtualMemory structure
typedef struct VirtualMemory {
    Page pages[NUM_PAGES]; // Array of pages in virtual memory
    int remaining_memory;   // Remaining memory in virtual memory
} VirtualMemory;

#endif // VIRTUAL_MEMORY_H