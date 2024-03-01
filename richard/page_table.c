#include <stdio.h>  // For printf
#include "page_table.h"
#include "virtual_memory.h"


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
        spt->entries[i].isvalid = false; // Mark as invalid initially
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
                entry->isvalid = true; // Mark as valid since we're allocating memory for it

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
            printf("                    frame_num: %d,\n", process->mpt->tables[i]->entries[j].frame_num);
            printf("                    isvalid: %s,\n", process->mpt->tables[i]->entries[j].isvalid ? "true" : "false");
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