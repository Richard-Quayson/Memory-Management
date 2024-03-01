#include <stdio.h>
#include <stdlib.h> // For dynamic memory allocation and system commands
#include "page_table.h"

#define MAX_PROCESSES 100 // Assume a max number of processes for simplicity
Process* processes[MAX_PROCESSES]; // Array to store processes
int processCount = 0; // Keep track of the number of processes

void menu() {
    printf("\nMenu:\n");
    printf("1. Create Process\n");
    printf("2. List Processes\n");
    printf("3. Print Process Details\n");
    printf("4. Print Virtual Memory\n");
    printf("5. Print Physical Memory\n");
    printf("6. Print Allocated Virtual Memory\n");
    printf("7. Print Allocated Physical Memory\n");
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
                Process* selectedProcess = NULL;
                for (int i = 0; i < processCount; i++) {
                    if (processes[i]->id == pid) {
                        selectedProcess = processes[i];
                        break;
                    }
                }
                printProcess(selectedProcess);
                break;
            case 4:     // Print Virtual Memory
                printVirtualMemory(vm);
                break;
            case 5:     // Print Physical Memory
                printPhysicalMemory(pm);
                break;
            case 6:     // Print Allocated Virtual Memory
                printAllocatedVirtualMemory(vm);
                break;
            case 7:     // Print Allocated Physical Memory
                printAllocatedFrameMemory(pm);
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