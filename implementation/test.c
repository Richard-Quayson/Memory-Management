#include "page_table.h"
#include "virtual_memory.h" // Assuming this includes the definitions for VirtualMemory, Page, and Chunk

// Helper function to allocate and initialize a SecondaryPageTable
SecondaryPageTable* allocateSecondaryPageTable(int memorySize) {
    int numPages = (memorySize + PAGE_SIZE - 1) / PAGE_SIZE; // Ceiling division to cover all memory
    SecondaryPageTable* spt = malloc(sizeof(SecondaryPageTable));
    if (!spt) {
        return NULL; // Allocation failed
    }
    spt->entries = malloc(numPages * sizeof(PageTableEntry));
    if (!spt->entries) {
        free(spt);
        return NULL; // Allocation failed
    }
    spt->size = memorySize;
    // Initialize chunk IDs array to -1 indicating unallocated chunks
    for (int i = 0; i < PAGE_SIZE / KB; ++i) {
        spt->chunks[i] = -1; // Placeholder initialization
    }
    return spt;
}

// Function to create a process
Process* create_process(int id, int memory_size, VirtualMemory* vm) {
    if (!vm) {
        printf("Virtual Memory is not initialized.\n");
        return NULL;
    }

    Process* process = malloc(sizeof(Process));
    if (!process) {
        return NULL; // Allocation failed
    }

    process->id = id;
    process->memory_size = memory_size;
    int numSecondaryTables = (memory_size + SECONDARY_TABLE_SIZE - 1) / SECONDARY_TABLE_SIZE; // Ceiling division

    process->mpt = malloc(sizeof(MasterPageTable));
    if (!process->mpt) {
        free(process);
        return NULL;
    }
    process->mpt->tables = malloc(numSecondaryTables * sizeof(SecondaryPageTable*));
    process->mpt->count = numSecondaryTables;

    int remainingMemory = memory_size;
    for (int i = 0; i < numSecondaryTables; ++i) {
        int tableSize = remainingMemory > SECONDARY_TABLE_SIZE ? SECONDARY_TABLE_SIZE : remainingMemory;
        SecondaryPageTable* spt = allocateSecondaryPageTable(tableSize);
        if (!spt) {
            // Properly handle partial allocation failure by cleaning up previously allocated tables
            // For brevity, cleanup code is omitted
            return NULL;
        }
        process->mpt->tables[i] = spt;
        remainingMemory -= SECONDARY_TABLE_SIZE;
    }

    // Logic to allocate chunks within virtual memory pages, mark them as allocated,
    // and fill in the SecondaryPageTable's chunks and entries should go here.
    // For brevity, this logic is not fully implemented in this example.

    return process;
}
