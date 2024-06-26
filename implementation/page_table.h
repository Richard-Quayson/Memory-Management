#include <stdbool.h> // For bool type
#include <stdlib.h> // For dynamic allocation
#include "memory_config.h" // For memory configuration
#include "physical_memory.h"


#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#define SECONDARY_TABLE_SIZE (4 * MB)


typedef struct PageTableEntry {
    int page_num;               // ID of the page in the virtual memory
    int frame_num;              // ID in a Frame in the physical memory
    bool is_valid;              // Indicates if the entry is valid
    int chunks[PAGE_SIZE / KB]; // List of chunk IDs used to store the process
} PageTableEntry;

typedef struct SecondaryPageTable {
    PageTableEntry* entries;    // Dynamic array of PageTableEntry
    int size;                   // Size of this secondary page table, in bytes
} SecondaryPageTable;

typedef struct MasterPageTable {
    SecondaryPageTable** tables; // Array of pointers to SecondaryPageTable
    int count;                   // Number of SecondaryPageTable pointers
} MasterPageTable;

typedef struct Process {
    int id;
    int memory_size;       // Total memory size of the process
    MasterPageTable* mpt;  // Pointer to the MasterPageTable
} Process;

Process* create_process(int id, int memory_size, VirtualMemory* vm);
Process* findProcessById(int pid);
void printProcess(const Process* process);
int findFreeFrame(PhysicalMemory* pm);
void allocatePagesToPhysicalMemory(Process* process, PhysicalMemory* pm);
void deallocatePagesFromPhysicalMemory(Process* process, PhysicalMemory* pm);
int accessMemory(Process* process, int page_id);
void translateVirtualToPhysicalAddress(PhysicalMemory* pm, char* virtualAddress, int processId);
void displayStatistics(VirtualMemory* vm, PhysicalMemory* pm);
void requestAdditionalMemory(int processId, unsigned int additionalMemorySize, VirtualMemory* vm, PhysicalMemory* pm);
void freeVirtualPage(int pageID, VirtualMemory* vm);
void freePhysicalFrame(int frameID, PhysicalMemory* pm);
void destroy_process(int processId, VirtualMemory* vm, PhysicalMemory* pm);

#endif // PAGE_TABLE_H