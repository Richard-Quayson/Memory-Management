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
    printf("7. Address Translation\n");
    printf("8. Display Statistics\n");
    printf("9. Rrquest memory\n");
    printf("10. Destroy Process\n");
    printf("11. Print Allocated Virtual Memory\n");
    printf("12. Print Allocated Physical Memory\n");
    printf("13. Print Virtual Memory\n");
    printf("14. Print Physical Memory\n");
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
                    int accessResult = accessMemory(process, pageId); // This function internally increments num_accesses

                    // result of access is -1 if page fault occurs, 
                    // handle it by asking the user if they want to allocate the page to physical memory
                    // if yes, call allocatePagesToPhysicalMemory function

                    if (accessResult == -1) {
                        printf("Do you want to allocate the page to physical memory? (y/n): ");
                        char choice;
                        scanf(" %c", &choice);
                        if (choice == 'y' || choice == 'Y') {
                            allocatePagesToPhysicalMemory(process, pm);
                        } else {
                            break;
                        }
                    } else {
                        break;
                    }
                    
                    // print the physical memory allocation after the handling of page fault
                    printf("\nPhysical Memory after handling page fault:\n---------------------------");
                    printAllocatedFrameMemory(pm);

                } else {
                    printf("\nProcess ID %d not found.\n", pid3);
                }
                break;

            case 7:     // address translation
                // enter process id
                printf("\nEnter Process ID: ");
                int pid4;
                scanf("%d", &pid4);

                // enter the virtual address in the form 0vp<page_id>s<offset> e.g. 0vp01s0 for page 1 and offset 0
                printf("Enter the virtual address in the form 0vp<page_id>s<offset> e.g. 0vp01s0 for page 1 and offset 0: ");
                char virtualAddress[20];
                scanf("%s", virtualAddress);

                // call the function to translate the virtual address to physical address
                translateVirtualToPhysicalAddress(pm, virtualAddress, pid4);
                num_accesses++; // increment the number of accesses
                break;

            case 8:     // display statistics
                displayStatistics(vm, pm);
                break;

            case 9:     // Request memory
                printf("Enter process ID: ");
                int pid5;
                scanf("%d", &pid5);

                // request additional memory for the process
                printf("Enter the additional memory size (in bytes): ");
                unsigned int additionalMemorySize;
                scanf("%u", &additionalMemorySize);

                requestAdditionalMemory(pid5, additionalMemorySize, vm, pm);
                break;

            case 10:    // Destroy Process
                printf("Enter process ID: ");
                int pid6;
                scanf("%d", &pid6);
                // get the index of the process in the processes array
                int index;
                for (index = 0; index < processCount; index++) {
                    if (processes[index]->id == pid6) {
                        break;
                    }
                }
                destroy_process(pid6, vm, pm);
                // remove the process from the processes array
                for (int i = 0; i < processCount - 1; i++) {
                    processes[i] = processes[i + 1];
                }
                processCount--;
                break;

            case 11:     // Print Allocated Virtual Memory
                printAllocatedVirtualMemory(vm);
                break;

            case 12:     // Print Allocated Physical Memory
                printAllocatedFrameMemory(pm);
                break;
                
            case 13:     // Print Virtual Memory
                printVirtualMemory(vm);
                break;

            case 14:     // Print Physical Memory
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