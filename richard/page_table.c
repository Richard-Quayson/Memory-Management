#include <stdio.h>  // For printf
#include "page_table.h"
#include "virtual_memory.h"


// Global variables defined in main.c
extern int page_faults;
extern int num_accesses;
extern Process* processes[MAX_PROCESSES];
extern int processCount;


// SecondaryPageTable allocation function
SecondaryPageTable* allocateSecondaryPageTable(int memorySize) {
    int numEntries = (memorySize + PAGE_SIZE - 1) / PAGE_SIZE; // Number of pages needed
    SecondaryPageTable* spt = (SecondaryPageTable*)malloc(sizeof(SecondaryPageTable));
    if (!spt) return NULL;

    spt->entries = (PageTableEntry*)malloc(numEntries * sizeof(PageTableEntry));
    if (!spt->entries) {
        free(spt);
        return NULL;
    }

    spt->size = memorySize;
    for (int i = 0; i < numEntries; ++i) {
        spt->entries[i].frame_num = -1; // Initially, no frame is allocated
        spt->entries[i].is_valid = false; // Mark as invalid initially
        for (int j = 0; j < PAGE_SIZE / KB; ++j) {
            spt->entries[i].chunks[j] = -1; // Mark all chunks as unallocated
        }
    }
    return spt;
}

// Function to find and allocate a page in virtual memory, returning the page ID
int allocatePage(VirtualMemory* vm) {
    for (int i = 0; i < NUM_PAGES; i++) {
        if (!vm->pages[i].is_allocated) {
            vm->pages[i].is_allocated = true;
            return i; // Return the ID of the allocated page
        }
    }
    return -1; // Indicate failure to allocate a page
}

// Function to allocate chunks within a given page and mark them as allocated
void allocateChunksInPage(VirtualMemory* vm, int pageID, int chunksNeeded, PageTableEntry* entry) {
    Page* page = &vm->pages[pageID];
    int chunkIndex = 0;
    for (int i = 0; i < chunksNeeded && i < PAGE_SIZE / CHUNK_SIZE; i++) {
        if (!page->chunks[i].is_allocated) {
            page->chunks[i].is_allocated = true;
            entry->chunks[chunkIndex++] = i; // Assign chunk ID and increment chunkIndex
        }
    }
}

// Updated to reflect new allocation logic
Process* create_process(int id, int memory_size, VirtualMemory* vm) {
    if (!vm || memory_size <= 0 || memory_size > vm->remaining_memory) {
        printf("\nInsufficient virtual memory to create process.\n");
        return NULL;
    }

    Process* process = (Process*)malloc(sizeof(Process));
    if (!process) return NULL;

    process->id = id;
    process->memory_size = memory_size;
    vm->remaining_memory -= memory_size; // Deduct the allocated memory from the remaining virtual memory

    int numSecondaryTables = (memory_size + SECONDARY_TABLE_SIZE - 1) / SECONDARY_TABLE_SIZE;
    process->mpt = (MasterPageTable*)malloc(sizeof(MasterPageTable));
    if (!process->mpt) {
        free(process);
        return NULL;
    }

    process->mpt->tables = (SecondaryPageTable**)malloc(numSecondaryTables * sizeof(SecondaryPageTable*));
    process->mpt->count = numSecondaryTables;

    // Allocate memory and initialize PageTableEntries
    for (int i = 0; i < numSecondaryTables; ++i) {
        int tableSize = (i < numSecondaryTables - 1) ? SECONDARY_TABLE_SIZE : memory_size - i * SECONDARY_TABLE_SIZE;
        process->mpt->tables[i] = allocateSecondaryPageTable(tableSize);

        // allocate PageTableEntries
        int remaining_memory = memory_size;
        for (int i = 0; i < numSecondaryTables; ++i) {
            int tableSize = (i < numSecondaryTables - 1) ? SECONDARY_TABLE_SIZE : memory_size - i * SECONDARY_TABLE_SIZE;
            process->mpt->tables[i] = allocateSecondaryPageTable(tableSize);

            int remaining_process_size = tableSize; // Memory remaining to be allocated in this secondary table
            int allocated_memory = 0; // Track allocated memory in the current secondary table

            // Calculate the number of pages (PageTableEntries) needed for this SecondaryPageTable
            int numPagesNeeded = (tableSize + PAGE_SIZE - 1) / PAGE_SIZE;

            for (int pageIndex = 0; pageIndex < numPagesNeeded && remaining_process_size > 0; ++pageIndex) {
                int pageID = allocatePage(vm);
                if (pageID == -1) {
                    printf("Failed to allocate enough virtual memory for the process.\n");
                    // Cleanup code omitted for brevity
                    return NULL;
                }

                // Initialize PageTableEntry for the current page
                PageTableEntry* entry = &process->mpt->tables[i]->entries[pageIndex];
                entry->page_num = pageID; // Set the page number
                entry->frame_num = -1; // Assuming no physical frame is allocated yet
                entry->is_valid = true; // Mark as valid since we're allocating memory for it

                // Calculate how many chunks are needed for this secondary table
                int chunksNeeded = tableSize / CHUNK_SIZE + (tableSize % CHUNK_SIZE != 0);

                // Allocate chunks within the allocated page
                allocateChunksInPage(vm, pageID, chunksNeeded, entry);
                chunksNeeded -= PAGE_SIZE / CHUNK_SIZE; // Update the number of chunks needed

                // Update remaining_memory
                remaining_memory -= PAGE_SIZE;
                if (remaining_memory <= 0) break; // Stop if we've allocated enough memory
            }

        }
    }

    printf("\nProcess %d created successfully with %d bytes of memory.\n", id, memory_size);
    return process;
}

Process* findProcessById(int pid) {
    Process* selectedProcess = NULL;
    for (int i = 0; i < processCount; i++) {
        if (processes[i]->id == pid) {
            selectedProcess = processes[i];
            break; // Exit the loop once the process is found
        }
    }
    return selectedProcess; // Return the found process or NULL if not found
}

// Helper function to print the chunks array for a PageTableEntry
void printChunks(const int chunks[], int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        if (chunks[i] != -1) { // Assuming -1 indicates an unused slot in the array
            printf("%d", chunks[i]);
            if (i < size - 1 && chunks[i + 1] != -1) {
                printf(", ");
            }
        }
    }
    printf("]");
}

void printProcess(const Process* process) {
    if (process == NULL) {
        printf("Process not found.\n");
        return;
    }

    printf("Process {\n");
    printf("    id: %d,\n", process->id);
    printf("    memory_size: %d bytes,\n", process->memory_size);
    printf("    MasterPageTable {\n");

    for (int i = 0; i < process->mpt->count; i++) {
        printf("        %d: SecondaryPageTable {\n", i + 1);
        printf("            entries: [\n");
        for (int j = 0; j < process->mpt->tables[i]->size / PAGE_SIZE; j++) { // Adjust based on actual structure and needs
            printf("                {\n");
            printf("                    page_num: %d,\n", process->mpt->tables[i]->entries[j].page_num);
            printf("                    frame_num: %d,\n", process->mpt->tables[i]->entries[j].frame_num);
            printf("                    is_valid: %s,\n", process->mpt->tables[i]->entries[j].is_valid ? "true" : "false");
            printf("                    chunks: ");
            printChunks(process->mpt->tables[i]->entries[j].chunks, PAGE_SIZE / KB);
            printf("\n                }");
            if (j < process->mpt->tables[i]->size / PAGE_SIZE - 1) printf(",");
            printf("\n");
        }
        printf("            ],\n");
        printf("            size: %d bytes\n", process->mpt->tables[i]->size);
        printf("        },\n");
    }

    printf("    }\n");
    printf("}\n");
}

int findFreeFrame(PhysicalMemory* pm) {
    for (int i = 0; i < NUM_FRAMES; i++) {
        if (!pm->frames[i].is_allocated) {
            return i;
        }
    }
    return -1; // Indicate failure to find a free frame
}

void allocatePagesToPhysicalMemory(Process* process, PhysicalMemory* pm) {
    if (!process || !pm) return;

    for (int i = 0; i < process->mpt->count; i++) { // Iterate through secondary page tables
        SecondaryPageTable* spt = process->mpt->tables[i];
        for (int j = 0; j < (spt->size + PAGE_SIZE - 1) / PAGE_SIZE; j++) { // Iterate through page table entries
            PageTableEntry* entry = &spt->entries[j];
            if (entry->is_valid) {
                int frameID = findFreeFrame(pm); // Assume this function finds a free frame and returns its ID, -1 if none found
                if (frameID != -1) {
                    entry->frame_num = frameID;
                    pm->frames[frameID].is_allocated = true; // Mark frame as allocated
                    // Copy chunk allocation details to the physical frame
                    for (int chunk = 0; chunk < PAGE_SIZE / KB; chunk++) {
                        if (entry->chunks[chunk] != -1) {
                            pm->frames[frameID].chunks[chunk].is_allocated = true;
                        }
                    }
                }
            }
        }
    }

    // update remaining_memory
    pm->remaining_memory = pm->remaining_memory - process->memory_size;
    printf("\nPages allocated to physical memory for process %d.\n", process->id);
}

void deallocatePagesFromPhysicalMemory(Process* process, PhysicalMemory* pm) {
    if (!process || !pm) return;

    for (int i = 0; i < process->mpt->count; i++) { // Iterate through secondary page tables
        SecondaryPageTable* spt = process->mpt->tables[i];
        for (int j = 0; j < (spt->size + PAGE_SIZE - 1) / PAGE_SIZE; j++) { // Iterate through page table entries
            PageTableEntry* entry = &spt->entries[j];
            if (entry->frame_num != -1) {
                // Clear the physical frame
                Frame* frame = &pm->frames[entry->frame_num];
                frame->is_allocated = false;
                for (int chunk = 0; chunk < PAGE_SIZE / KB; chunk++) {
                    frame->chunks[chunk].is_allocated = false;
                }
                // Reset PageTableEntry
                entry->frame_num = -1;
                for (int chunk = 0; chunk < PAGE_SIZE / KB; chunk++) {
                    entry->chunks[chunk] = -1; // Clear chunk allocation details
                }
            }
        }
    }

    // update remaining_memory
    pm->remaining_memory = pm->remaining_memory + process->memory_size;
    printf("\nPages deallocated from physical memory for process %d.\n", process->id);
}

// Function to access a process's frame in physical memory
int accessMemory(Process* process, int page_id) {
    num_accesses++;  // Increment the number of memory access attempts

    // Iterate through the MasterPageTable to find the PageTableEntry for the given page_id
    for (int i = 0; i < process->mpt->count; i++) {
        SecondaryPageTable* spt = process->mpt->tables[i];
        for (int j = 0; j < (spt->size + PAGE_SIZE - 1) / PAGE_SIZE; j++) {
            PageTableEntry* entry = &spt->entries[j];
            if (entry->page_num == page_id) {  // Found the corresponding PageTableEntry
                if (entry->frame_num == -1) {  // Page fault occurs if frame_num is -1
                    page_faults++;  // Increment the global page_faults counter
                    printf("Page fault occurred for page ID %d in process ID %d.\n", page_id, process->id);
                    return -1;
                } else {
                    // Successfully accessed the page in physical memory
                    printf("Successfully accessed frame %d for page ID %d in process ID %d.\n", entry->frame_num, page_id, process->id);
                }
                return 0;  // Exit after handling the page access
            }
        }
    }

    // If the page_id was not found in any PageTableEntry, it's considered an invalid access
    printf("Invalid page ID %d access attempt in process ID %d.\n", page_id, process->id);
}

// Function to display memory management statistics
void displayStatistics(VirtualMemory* vm, PhysicalMemory* pm) {
    // Calculate hit rate as the ratio of successful accesses to total accesses
    float hitRate = (num_accesses - page_faults) / (float)num_accesses * 100;

    // Calculate the total and remaining memory in both virtual and physical memory spaces
    int totalVirtualMemory = NUM_PAGES * PAGE_SIZE;
    int usedVirtualMemory = totalVirtualMemory - vm->remaining_memory;

    int totalPhysicalMemory = NUM_FRAMES * FRAME_SIZE;
    int usedPhysicalMemory = totalPhysicalMemory - pm->remaining_memory;

    // Display the statistics
    printf("\nMemory Management Statistics:\n");
    printf("Number of allocated pages in virtual memory: %llu\n", NUM_PAGES - (vm->remaining_memory / PAGE_SIZE));
    printf("Number of frames in physical memory: %llu\n", NUM_FRAMES - (pm->remaining_memory / FRAME_SIZE));
    printf("Number of accesses in physical memory: %d\n", num_accesses);
    printf("Number of page faults: %d\n", page_faults);
    printf("Hit rate: %.2f%%\n", hitRate);
    printf("Total memory used in virtual memory: %d bytes\n", usedVirtualMemory);
    printf("Remaining memory in virtual memory: %d bytes\n", vm->remaining_memory);
    printf("Total memory used in physical memory: %d bytes\n", usedPhysicalMemory);
    printf("Remaining memory in physical memory: %d bytes\n", pm->remaining_memory);
}