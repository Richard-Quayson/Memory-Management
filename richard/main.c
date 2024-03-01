#include <stdio.h>
#include <stdlib.h> // For dynamic memory allocation and system commands
#include "page_table.h"

Process* processes[MAX_PROCESSES]; // Array to store processes
int processCount = 0; // Keep track of the number of processes
int page_faults;
int num_accesses;

void menu() {
    printf("\nMenu:\n");
    printf("1. Create Process\n");
    printf("2. List Processes\n");
    printf("3. Print Process Details\n");
    printf("4. Allocate Pages to Physical Memory\n");
    printf("5. Deallocate Pages from Physical Memory\n");
    printf("6. Access Memory\n");

    printf("8. Print Allocated Virtual Memory\n");
    printf("9. Print Allocated Physical Memory\n");
    printf("10. Print Virtual Memory\n");
    printf("11. Print Physical Memory\n");
    printf("-1. Exit\n");
    printf("Enter your choice: ");
}

int main() {
    VirtualMemory* vm = initializeVirtualMemory();
    PhysicalMemory* pm = initializePhysicalMemory();

    if (!vm || !pm) {
        printf("Failed to initialize memory structures.\n");
        return 1; // Exit with error
    }

    while (1) {
        menu();
        int choice;
        scanf("%d", &choice);
        getchar(); // skip the newline character after the number

        switch(choice) {
            case 1:     // Create Process
                int id, memorySize;
                printf("Enter process ID: ");
                scanf("%d", &id);

                // ensure that the process ID is unique
                if (findProcessById(id) != NULL) {
                    printf("Process ID %d already exists. Please enter a different ID.\n", id);
                    break;
                }
                
                printf("Enter memory size (in bytes): ");
                scanf("%d", &memorySize);

                Process* newProcess = create_process(id, memorySize, vm);
                if (newProcess != NULL) {
                    processes[processCount++] = newProcess;
                }
                break;

            case 2:     // List Processes
                if (processCount == 0) {
                    printf("\nNo processes created yet.\n");
                } else {
                    printf("\nList of processes:\n");
                    for (int i = 0; i < processCount; i++) {
                        printf("Process ID: %d, Memory Size: %d\n", processes[i]->id, processes[i]->memory_size);
                    }
                }
                break;

            case 3:     // Print Process Details
                printf("Enter process ID: ");
                int pid;
                scanf("%d", &pid);
                Process* selectedProcess = findProcessById(pid);

                if (selectedProcess == NULL) {
                    printf("\nProcess ID %d not found.\n", pid);
                    break;
                }

                printProcess(selectedProcess);
                break;

            case 4:     // Allocate Pages to Physical Memory
                printf("Enter process ID: ");
                int pid1;
                scanf("%d", &pid1);
                Process* selectedProcess1 = findProcessById(pid1);
                
                if (selectedProcess1 == NULL) {
                    printf("\nProcess ID %d not found.\n", pid1);
                    break;
                }

                allocatePagesToPhysicalMemory(selectedProcess1, pm);
                break;

            case 5:     // Deallocate Pages from Physical Memory
                printf("Enter process ID: ");
                int pid2;
                scanf("%d", &pid2);
                Process* selectedProcess2 = findProcessById(pid2);

                if (selectedProcess2 == NULL) {
                    printf("\nProcess ID %d not found.\n", pid2);
                    break;
                }

                deallocatePagesFromPhysicalMemory(selectedProcess2, pm);
                break;            

            case 6:
                printf("\nEnter Process ID: ");
                int pid3;
                scanf("%d", &pid3);
                Process* process = findProcessById(pid3);
                
                if (process != NULL) {
                    // List all pages for the selected process
                    printf("\nListing all pages for Process ID %d:\n", pid3);
                    for (int i = 0; i < process->mpt->count; i++) {
                        for (int j = 0; j < (process->mpt->tables[i]->size + PAGE_SIZE - 1) / PAGE_SIZE; j++) {
                            printf("Page ID: %d\n", process->mpt->tables[i]->entries[j].page_num);
                        }
                    }
                    
                    printf("\nEnter the Page ID you wish to access: ");
                    int pageId;
                    scanf("%d", &pageId);
                    accessMemory(process, pageId); // This function internally increments num_accesses
                } else {
                    printf("\nProcess ID %d not found.\n", pid3);
                }
                break;

            case 7:     // display statistics
                displayStatistics(vm, pm);
                break;

            case 8:     // Print Allocated Virtual Memory
                printAllocatedVirtualMemory(vm);
                break;

            case 9:     // Print Allocated Physical Memory
                printAllocatedFrameMemory(pm);
                break;
                
            case 10:     // Print Virtual Memory
                printVirtualMemory(vm);
                break;

            case 11:     // Print Physical Memory
                printPhysicalMemory(pm);
                break;

            case -1:
                printf("Exiting program.\n");
                return 0; // Exit the program

            default:
                printf("Invalid choice, please try again.\n");
        }
    }

    // Once done, free the allocated memory
    freeMemory(vm, pm);

    return 0;
}