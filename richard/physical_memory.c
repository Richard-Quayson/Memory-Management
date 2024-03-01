#include <stdlib.h> // For dynamic memory allocation
#include "physical_memory.h"


// Function to initialize virtual memory
VirtualMemory* initializeVirtualMemory() {
    VirtualMemory* vm = malloc(sizeof(VirtualMemory)); // Allocate memory for the virtual memory structure
    if (vm == NULL) {
        // Handle memory allocation failure
        return NULL;
    }

    for (unsigned long long i = 0; i < NUM_PAGES; i++) {
        vm->pages[i].id = i; // Set page ID
    }

    // remaining_memory = VIRTUAL_MEMORY_SIZE; // Initialize remaining memory in virtual memory
    vm->remaining_memory = VIRTUAL_MEMORY_SIZE;
    return vm;
}

// Function to initialize physical memory
PhysicalMemory* initializePhysicalMemory() {
    PhysicalMemory* pm = malloc(sizeof(PhysicalMemory)); // Allocate memory for the physical memory structure
    if (pm == NULL) {
        // Handle memory allocation failure
        return NULL;
    }

    for (unsigned long long i = 0; i < NUM_FRAMES; i++) {
        pm->frames[i].id = i; // Set frame ID
    }

    // remaining_memory = PHYSICAL_MEMORY_SIZE; // Initialize remaining memory in physical memory
    pm->remaining_memory = PHYSICAL_MEMORY_SIZE;
    return pm;
}

// Function to free the allocated memory for virtual and physical memory
void freeMemory(VirtualMemory* vm, PhysicalMemory* pm) {
    if (vm != NULL) {
        free(vm); // Free virtual memory
    }
    if (pm != NULL) {
        free(pm); // Free physical memory
    }
}

// Function to print the values in virtual memory
void printVirtualMemory(const VirtualMemory* vm) {
    if (vm == NULL) {
        printf("\nVirtual Memory is not initialized.\n");
        return;
    }

    printf("Virtual Memory Contents:\n");
    for (unsigned long long i = 0; i < NUM_PAGES; i++) {
        printf("Page %llu (Allocated: %s): ", i, vm->pages[i].is_allocated ? "Yes" : "No");
        for (int j = 0; j < PAGE_SIZE / KB; j++) {
            printf("Chunk %d (Offset: %d, Allocated: %s), ", j, vm->pages[i].chunks[j].offset, vm->pages[i].chunks[j].is_allocated ? "Yes" : "No");
        }
        printf("\n");
    }
}

// Function to print the values in physical memory
void printPhysicalMemory(const PhysicalMemory* pm) {
    if (pm == NULL) {
        printf("\nPhysical Memory is not initialized.\n");
        return;
    }

    printf("Physical Memory Contents:\n");
    for (unsigned long long i = 0; i < NUM_FRAMES; i++) {
        printf("Frame %llu (Allocated: %s): ", i, pm->frames[i].is_allocated ? "Yes" : "No");
        for (int j = 0; j < FRAME_SIZE / KB; j++) {
            printf("Chunk %d (Offset: %d, Allocated: %s), ", j, pm->frames[i].chunks[j].offset, pm->frames[i].chunks[j].is_allocated ? "Yes" : "No");
        }
        printf("\n");
    }
}

// Function to print allocated pages and their chunks in virtual memory
void printAllocatedVirtualMemory(const VirtualMemory* vm) {
    int allocatedPagesFound = 0;
    for (unsigned long long i = 0; i < NUM_PAGES; i++) {
        if (vm->pages[i].is_allocated) {
            printf("\nAllocated Virtual Page: %d\n", vm->pages[i].id);
            // Print details about allocated chunks within this page
            for (int j = 0; j < PAGE_SIZE / KB; j++) {
                if (vm->pages[i].chunks[j].is_allocated) {
                    printf("\tAllocated Chunk: %d (Offset: %d)\n", j, vm->pages[i].chunks[j].offset);
                }
            }
            allocatedPagesFound++;
        }
    }
    if (allocatedPagesFound == 0) {
        printf("\nNo allocated pages in virtual memory.\n");
    }
}

// Function to print allocated frames and their chunks in physical memory
void printAllocatedFrameMemory(const PhysicalMemory* pm) {
    int allocatedFramesFound = 0;
    for (unsigned long long i = 0; i < NUM_FRAMES; i++) {
        if (pm->frames[i].is_allocated) {
            printf("\nAllocated Physical Frame: %d\n", pm->frames[i].id);
            // Print details about allocated chunks within this frame
            for (int j = 0; j < FRAME_SIZE / KB; j++) {
                if (pm->frames[i].chunks[j].is_allocated) {
                    printf("\tAllocated Chunk: %d (Offset: %d)\n", j, pm->frames[i].chunks[j].offset);
                }
            }
            allocatedFramesFound++;
        }
    }
    if (allocatedFramesFound == 0) {
        printf("\nNo allocated frames in physical memory.\n");
    }
}