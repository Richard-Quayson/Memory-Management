# Memory Management System (Paging Implementation)

This is a simple memory management system implemented in C. It provides functionalities for managing virtual and physical memory, allocating and deallocating pages, accessing memory, and handling page faults.

## Features

- **Virtual Memory Management**: Defines virtual memory space size, page size, and number of pages.
- **Physical Memory Management**: Initializes physical memory space and manages frame allocation.
- **Page Table Management**: Manages page tables for each process.
- **Page Fault Handling**: Detects and handles page faults by allocating pages to physical memory on demand.
- **Memory Access**: Allows accessing memory within a process and handles page faults gracefully.
- **Statistics Display**: Displays statistics such as page faults and memory accesses.

## Getting Started

To compile the program, move into the implementation directory:

```bash
cd implementation
```

and run the following command:


```bash
gcc -o main main.c physical_memory.c page_table.c virtual_memory.c 
```

To run the program, execute the compiled binary:

```bash
./main
```

## Usage
Upon running the program, a menu will be displayed with various options to interact with the memory management system. Here are the available options:

```bash
1. Create Process
2. List Processes
3. Print Process Details
4. Allocate Pages to Physical Memory
5. Deallocate Pages from Physical Memory
6. Access Memory
7. Display Statistics
8. Print Allocated Virtual Memory
9. Print Allocated Physical Memory
10. Print Virtual Memory
11. Print Physical Memory
-1. Exit
```

Follow the on-screen instructions to perform different operations.

## Contributors
- Asher Chakupa
- Fredrick Njoki
- Richard Quayson