// memory_config.h
// by Richard Quayson
// on 23/02/2024
// This file contains the configuration for the memory system
// It defines the virtual memory space size, physical memory space size, 
// page size, number of pages and number of frames.


#ifndef MEMORY_CONFIG_H
#define MEMORY_CONFIG_H

#define KB (1024)
#define MB (1024 * KB)
#define GB (1024 * MB)

// virtual memory space size: 16GB
#define VIRTUAL_MEMORY_SIZE (256ULL * MB)

// physical memory space size: 4GB
#define PHYSICAL_MEMORY_SIZE (128ULL * MB)

// page size: 4KB
#define PAGE_SIZE (4 * KB)

// frame size: 4KB
#define FRAME_SIZE PAGE_SIZE

// number of pages
#define NUM_PAGES (VIRTUAL_MEMORY_SIZE / PAGE_SIZE)

// number of frames
#define NUM_FRAMES (PHYSICAL_MEMORY_SIZE / PAGE_SIZE)

// chunk size: 1KB
#define CHUNK_SIZE (KB)

#endif // MEMORY_CONFIG_H