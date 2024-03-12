#include <stdio.h>  // For printf
#include "memory_config.h"
#include "virtual_memory.h"

#ifndef PHYSICAL_MEMORY_H
#define PHYSICAL_MEMORY_H

// Define the Frame structure
typedef struct Frame {
    int id; // Frame identifier
    Chunk chunks[FRAME_SIZE / KB]; // Array of chunks within the frame
    int is_allocated; // 1 if all chunks in the frame are allocated, 0 otherwise
} Frame;

/**
 * Define the PhysicalMemory structure
 * This function initializes physical memory by allocating memory for a structure called PhysicalMemory using malloc.
 * It iterates through each frame in the physical memory, setting its ID. 
 * The remaining memory in the physical memory structure is initialized, and a pointer to the allocated memory is returned.
 * It returns a pointer to the allocated PhysicalMemory structure if successful, 
 * otherwise NULL in case of memory allocation failure.
**/
typedef struct PhysicalMemory {
    Frame frames[NUM_FRAMES]; // Array of frames in physical memory
    int remaining_memory;     // Remaining memory in physical memory
} PhysicalMemory;

// Function prototypes

/**
 * initializeVirtualMemory function initializes virtual memory by allocating memory for a structure called VirtualMemory using malloc.
 * It iterates through each page in the virtual memory, setting its ID. 
 * The remaining memory in the virtual memorystructure is initialized, and a pointer to the allocated memory is returned.
 * It returns a pointer to the allocated VirtualMemory structure if successful, otherwise NULL in case of memory allocation failure.
**/
VirtualMemory* initializeVirtualMemory(void); // Initialize virtual memory

/**
 * initializePhysicalMemory a function initializes physical memory by allocating memory for a structure called PhysicalMemory using malloc.
 * It iterates through each frame in the physical memory, setting its ID. The remaining memory in the physical memory
 * structure is initialized, and a pointer to the allocated memory is returned.
 * It returns a pointer to the allocated PhysicalMemory structure if successful, otherwise NULL in case of memory allocation failure.
**/
PhysicalMemory* initializePhysicalMemory(void); // Initialize physical memory

/**
 * freeMemory function frees the allocated memory for both virtual and physical memory. 
 * It checks if the pointers to virtual and physical memory are not NULL, 
 * and if so, it frees the memory using the free function.
 
   Parameters:
  - vm: A pointer to the VirtualMemory structure.
  - pm: A pointer to the PhysicalMemory structure.
 **/
void freeMemory(VirtualMemory* vm, PhysicalMemory* pm); 

/**
 * printVirtualMemory function prints the values stored in virtual memory and returns none. 
 * It checks if the virtual memory is initialized (vm != NULL), and if not, 
 * it prints a message indicating that virtual memory is not initialized.
 * For each page in the virtual memory, it prints whether the page is allocated or not, 
 * along with information about each chunk within the page, including its offset and whether it's allocated.
   
   Parameters:
   - vm: A pointer to the VirtualMemory structure.
 * 
**/
void printVirtualMemory(const VirtualMemory* vm); 

/**
 * printPhysicalMemory function prints the values stored in physical memory and returns none. 
 * It checks if the physical memory is initialized (pm != NULL), and if not, 
 * it prints a message indicating that physical memory is not initialized.
 * For each frame in the physical memory, it prints whether the frame is allocated or not, 
 * along with information about each chunk within the frame, including its offset and whether it's allocated.

   Parameters:
   - pm: A pointer to the PhysicalMemory structure.
**/
void printPhysicalMemory(const PhysicalMemory* pm); 

/**
 * printAllocatedVirtualMemory function prints the allocated pages and their chunks in virtual memory. 
 * It iterates through each page in the virtual memory, checks if the page is allocated, and if so, 
 * prints information about the allocated page and its chunks. For each allocated chunk within the allocated page, 
 * it prints the chunk ID and offset. If no allocated pages are found, it prints a message indicating so.

   Parameters:
   - vm: A pointer to the VirtualMemory structure.
*/
void printAllocatedVirtualMemory(const VirtualMemory* vm);

/**
 * printAllocatedFrameMemory function prints the allocated frames and their chunks in physical memory. 
 * It iterates through each frame in the physical memory, checks if the frame is allocated, and if so, 
 * prints information about the allocated frame and its chunks. For each allocated chunk within the allocated frame, 
 * it prints the chunk ID and offset.If no allocated frames are found, it prints a message indicating so.

   Parameters:
   - pm: A pointer to the PhysicalMemory structure.
**/
void printAllocatedFrameMemory(const PhysicalMemory* pm);

#endif // PHYSICAL_MEMORY_H