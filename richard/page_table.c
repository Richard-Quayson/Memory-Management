#include <stdio.h>              // For printf
#include <stdlib.h>             // For dynamic memory allocation
#include <string.h>             // For string manipulation
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

// Function to create a process and allocate memory for it in virtual memory
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
                int frameID = findFreeFrame(pm); // This function finds a free frame and returns its ID, -1 if none found
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

// Function to translate all virtual addresses of a process to physical addresses
void translateVirtualToPhysicalAddress(PhysicalMemory* pm, char* virtualAddress, int processId) {
    int pageId, offset;
    char physicalAddress[20];       // Assuming this is large enough for the physical address format

    // Validate and parse the virtual address
    if (sscanf(virtualAddress, "0vp%ds%d", &pageId, &offset) != 2) {
        printf("Invalid virtual address format.\n");
        return;
    }

    Process* process = findProcessById(processId);
    if (!process) {
        printf("Process with ID %d not found.\n", processId);
        return;
    }

    // Lookup the page in the process's page table to find its frame number
    int frameNum = -1;
    for (int i = 0; i < process->mpt->count; i++) {
        for (int j = 0; j < (process->mpt->tables[i]->size + PAGE_SIZE - 1) / PAGE_SIZE; j++) {
            if (process->mpt->tables[i]->entries[j].page_num == pageId) {
                frameNum = process->mpt->tables[i]->entries[j].frame_num;
                break;
            }
        }
        if (frameNum != -1) break;
    }

    // If the frame number is -1, the page is not in physical memory
    if (frameNum == -1) {
        page_faults++;      // Assume entire process loading counts as one page fault for simplicity
        printf("Page ID %d not found in physical memory for process ID %d.\n", pageId, processId);
        printf("Do you want to allocate the page to physical memory? (y/n): ");
        char choice;
        scanf(" %c", &choice);
        if (choice == 'y' || choice == 'Y') {
            allocatePagesToPhysicalMemory(process, pm);
        } else {
            return;
        }

        // Print the physical memory allocation after handling the page fault
        printf("\nPhysical Memory after handling page fault:\n---------------------------");
        printAllocatedFrameMemory(pm);

        printf("\n");       // newline
        // Print the physical address for the pages in the process
        for (int i = 0; i < process->mpt->count; i++) {
            for (int j = 0; j < (process->mpt->tables[i]->size + PAGE_SIZE - 1) / PAGE_SIZE; j++) {
                if (process->mpt->tables[i]->entries[j].frame_num != -1) {
                    printf("Physical address for virtual address '0vp%ds%d' of process ID %d: 0pf%ds%d\n", process->mpt->tables[i]->entries[j].page_num, offset, processId, process->mpt->tables[i]->entries[j].frame_num, offset);
                }
            }
        }

    } else {
        // Print the physical address for the given virtual address
        printf("Physical address for virtual address '%s' of process ID %d: 0pf%ds%d\n", virtualAddress, processId, frameNum, offset);
    }
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

void requestAdditionalMemory(int processId, unsigned int additionalMemorySize, VirtualMemory* vm, PhysicalMemory* pm) {
    Process* process = findProcessById(processId);
    if (process == NULL) {
        printf("Process with ID %d not found.\n", processId);
        return;
    }

    if (additionalMemorySize > vm->remaining_memory || additionalMemorySize > pm->remaining_memory) {
        printf("Insufficient virtual or physical memory available.\n");
        return;
    }

    // get a free secondary page table that is not full
    SecondaryPageTable* spt = NULL;
    for (int i = 0; i < process->mpt->count; i++) {
        if (process->mpt->tables[i]->size < SECONDARY_TABLE_SIZE) {
            spt = process->mpt->tables[i];
            break;
        }
    }

    // if no free secondary page table is found, allocate a new one
    if (spt == NULL) {
        int newTableSize = (additionalMemorySize < SECONDARY_TABLE_SIZE) ? additionalMemorySize : SECONDARY_TABLE_SIZE;
        spt = allocateSecondaryPageTable(newTableSize);
        if (spt == NULL) {
            printf("Failed to allocate memory for the new secondary page table.\n");
            return;
        }
        process->mpt->tables[process->mpt->count++] = spt;
    }

    // allocate memory for the new pages
    int remainingMemory = additionalMemorySize;

    // loop should begin from the length of the entries array in the secondary page table
    for (int i = spt->size / PAGE_SIZE; i < (spt->size + additionalMemorySize + PAGE_SIZE - 1) / PAGE_SIZE; i++) {
        if (remainingMemory <= 0) break; // Stop if we've allocated enough memory
        if (spt->entries[i].is_valid) continue; // Skip already allocated pages
        int pageID = allocatePage(vm);
        if (pageID == -1) {
            printf("Failed to allocate enough virtual memory for the process.\n");
            return;
        }

        // Initialize PageTableEntry for the current page
        PageTableEntry* entry = &spt->entries[i];
        entry->page_num = pageID; // Set the page number
        entry->frame_num = -1; // Assuming no physical frame is allocated yet
        entry->is_valid = true; // Mark as valid since we're allocating memory for it

        // Calculate how many chunks are needed for this secondary table
        int chunksNeeded = spt->size / CHUNK_SIZE + (spt->size % CHUNK_SIZE != 0);

        // Allocate chunks within the allocated page
        allocateChunksInPage(vm, pageID, chunksNeeded, entry);
        chunksNeeded -= PAGE_SIZE / CHUNK_SIZE; // Update the number of chunks needed

        // Update remainingMemory
        remainingMemory -= PAGE_SIZE;
    }

    // Update process memory size
    process->memory_size += additionalMemorySize;

    // Update SecondaryPageTable size
    spt->size += additionalMemorySize;

    // Deallocate process memory from physical memory
    deallocatePagesFromPhysicalMemory(process, pm);

    // Allocate memory in physical memory
    allocatePagesToPhysicalMemory(process, pm);

    // Deduct the allocated memory from the remaining virtual and physical memory
    vm->remaining_memory -= additionalMemorySize; // Assuming this simplification for demonstration
    pm->remaining_memory -= additionalMemorySize; // This might need more complex management in a real scenario

    printf("Additional memory allocated to process ID %d. Total memory: %u bytes.\n", processId, process->memory_size);
}

void freeVirtualPage(int pageID, VirtualMemory* vm) {
    if (vm == NULL || pageID < 0 || pageID >= NUM_PAGES) {
        printf("Invalid virtual memory or page ID.\n");
        return;
    }

    // Mark the page as free in the virtual memory structure
    vm->pages[pageID].is_allocated = false;

    // Update the remaining memory in the virtual memory structure
    vm->remaining_memory += PAGE_SIZE;
}

void freePhysicalFrame(int frameID, PhysicalMemory* pm) {
    if (pm == NULL || frameID < 0 || frameID >= NUM_FRAMES) {
        printf("Invalid physical memory or frame ID.\n");
        return;
    }

    // Mark the frame as free in the physical memory structure
    pm->frames[frameID].is_allocated = false;

    // Update the remaining memory in the physical memory structure
    pm->remaining_memory += FRAME_SIZE;
}

// Function to destroy a process and free its resources
void destroy_process(int processId, VirtualMemory* vm, PhysicalMemory* pm) {
    Process* process = findProcessById(processId);
    if (process == NULL) {
        printf("Process with ID %d not found.\n", processId);
        return;
    }

    // Deallocate virtual memory
    for (int i = 0; i < process->mpt->count; i++) {
        SecondaryPageTable* spt = process->mpt->tables[i];
        for (int j = 0; j < (spt->size + PAGE_SIZE - 1) / PAGE_SIZE; j++) {
            PageTableEntry entry = spt->entries[j];
            freeVirtualPage(entry.page_num, vm); // Free the virtual page
            if (entry.frame_num != -1) {
                freePhysicalFrame(entry.frame_num, pm); // Free the corresponding frame in physical memory
            }
            free(spt->entries); // Free the dynamic memory for page table entries
        }
        free(process->mpt->tables[i]); // Free the secondary page table itself
    }
    free(process->mpt->tables); // Free the array of secondary page tables
    free(process->mpt); // Free the master page table structure
    free(process); // Free the process structure itself

    printf("Process ID %d destroyed and resources freed.\n", processId);
}