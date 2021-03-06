# Angrave's 2021 Acme CS 241 Exam Prep		
## A.K.A. Preparing for the Final Exam & Beyond CS 241... 

Some of the questions require research (wikibook; websearch; wikipedia). 
It is accepted to work together and discuss answers, but please make an honest attempt first! 
Be ready to discuss and clear confusions & misconceptions in your last discussion section.
The final will also include pthreads, fork-exec-wait questions and virtual memory address translation. 
Be awesome. Angrave.

## 1. C 


1.	What are the differences between a library call and a system call? Include an example of each.

A system call is a function provided by the kernel to enter kernel mode to access the hardware resources. A library call is a function provided by programming libraries. System call examples are fork() exit() wait(). Library call examples are fopen() fclose().

2.	What is the `*` operator in C? What is the `&` operator? Give an example of each.

The * operator is a pointer. It points to a place in memory. char * string = malloc(4). The & operator is for the address. It is the address in the memory. int a = 4; &a // this is the address of a

3.	When is `strlen(s)` != `1+strlen(s+1)` ?

when s + 1 points to a different value from s (like, s was 0 bytes long so by incrementing to s + 1 you are now pointing to a different variable in memory)

4.	How are C strings represented in memory? What is the wrong with `malloc(strlen(s))` when copying strings?

C strings are represented as char pointers. You cannot malloc(strlen(s)) because you need 1 extra byte for the null byte. 

5.	Implement a truncation function `void trunc(char*s,size_t max)` to ensure strings are not too long with the following edge cases.
```
if (length < max)
    strcmp(trunc(s, max), s) == 0
else if (s is NULL)
    trunc(s, max) == NULL
else
    strlen(trunc(s, max)) <= max
    // i.e. char s[]="abcdefgh; trunc(s,3); s == "abc". 
```


6.	Complete the following function to create a deep-copy on the heap of the argv array. Set the result pointer to point to your array. The only library calls you may use are malloc and memcpy. You may not use strdup.

    `void duplicate(char **argv, char ***result);` 
    char ** dup = malloc(sizeof(argv));
    size_t len = sizeof(argv);
    for (size_t i = 0; i < len; i++) {
        char * j = argv[i];
        size_t len2 = strlen(j);
        dup[i] = malloc(len2 + 1);
        for (size_t k = 0; k < len2; k++) {
            dup[i][k] = argv[i][k];
        }
        dup[i][len2] = '\0';
    }
    result = &dup;

7.	Write a program that reads a series of lines from `stdin` and prints them to `stdout` using `fgets` or `getline`. Your program should stop if a read error or end of file occurs. The last text line may not have a newline char.

char * getInput(void) {
    char * line = malloc(100), * linep = line;
    size_t lenmax = 100, len = lenmax;
    int c;

    if(line == NULL)
        return NULL;

    for(;;) {
        c = fgetc(stdin);
        if(c == EOF)
            break;

        if(--len == 0) {
            len = lenmax;
            char * linen = realloc(linep, lenmax *= 2);

            if(linen == NULL) {
                free(linep);
                return NULL;
            }
            line = linen + (line - linep);
            linep = linen;
        }

        if((*line++ = c) == '\n')
            break;
    }
    *line = '\0';
    linep[strlen(linep) - 1] = '\0';
    return linep;
}

## 2. Memory 

1.	Explain how a virtual address is converted into a physical address using a multi-level page table. You may use a concrete example e.g. a 64bit machine with 4KB pages. 

each virtual adress space is divided into fixed-size chunks called pages. The physical address space is divided into frames. Frame size and page size match. OS maintains a page table for each process. Page table specifies the frame in which each of the process's pages is located. At run time, MMU translates virtual addresses to physical using the page table of the running process. 

2.	Explain Knuth's and the Buddy allocation scheme. Discuss internal & external Fragmentation.

the buddy allocation scheme is a memor allocation algorithm that divides memory into partitions to try to satisfy a memory request as suitably as possible. This system makes use of splitting memory into halves to try to give a best fit. The buddy system has little external fragmentation (and allows for compaction of memory with little overhead) Internal fragmentation is still a problem because memory is wasted when the meory requested is a little larger than a small block, but a lot smaller than a large block.

3.	What is the difference between the MMU and TLB? What is the purpose of each?

MMU is a hardware unit which performs the translation of virtual memory address to physical memory address whereas TLB is an associative cache of page table entries and is used to avoid the necessity of accessing the main memory every time a virtual address is mapped. 

4.	Assuming 4KB page tables what is the page number and offset for virtual address 0x12345678  ?

pain.

5.	What is a page fault? When is it an error? When is it not an error?

a page fault is an exception that the mmu raises when a process accesses a memory page without proper preparations.

6.	What is Spatial and Temporal Locality? Swapping? Swap file? Demand Paging?

Spatial locality are items with nearby addresses tend to be referenced close together in time. Temporal locality is when recently referenced items are likely to be referenced in near future. Demand paging is when you only bring in a page when needed. Swap files are files that contain data retrieved from system meory or RAM.

## 3. Processes and Threads 

1.	What resources are shared between threads in the same process?

they share the same (heap) memory and the same open files.

2.	Explain the operating system actions required to perform a process context switch

The state of the currently executing process must be saved so it can be restored when rescheduled for execution. The process state includes all the registers that the process may be using. especially the program counter, plus any other operating system specific data that may be necessary (stored in the PCB). the PCB might be stored on a per-process stack. A handle to the PCB is added to a queue of processes that is ready to now run on. It can then switch context by choosing a process from the ready queue and restoring its PCB. The program counter from the PCB is loaded, and thus execution can continue in the chosen process. Process and thread priority can influence which process is chosen from the ready queue. 

3.	Explain the actions required to perform a thread context switch to a thread in the same process

thread switching is very efficient and much cheaper because it involves switching out only identities and resources such as the program counter, registers, and stack pointers. 

4.	How can a process be orphaned? What does the process do about it?

Orphan processes are those processes that are still running even though their parent process has terminated or finished. You can terminate it, reincarnate it, or set an expiration time.

5.	How do you create a process zombie?

A child process terminates but has not been waited for.

6.	Under what conditions will a multi-threaded process exit? (List at least 4)

1) when a thread calls the exit subroutine
2) a cancelation mechanism is called
3) async cancel safety is called
4) the process calls a pthread exit subroutine

## 4. Scheduling 
1.	Define arrival time, pre-emption, turnaround time, waiting time and response time in the context of scheduling algorithms. What is starvation?  Which scheduling policies have the possibility of resulting in starvation?

arrival time: time when process arrives
preemption: if a new process arrives with less work than the remaining time of current process, then switch to the new process
turnaround time: time taken between the submission of a process for execution and the return of the complete output to the user
waiting time: how much time processes spend in the ready queue witing for their turn to get on the CPU
response time: time spent between the ready state and getting the CPU for the first time
starvation: when a process ready to run for CPU can wait indefinitely because of low priority
Scheduling poliices: overly sipmlistic or poorly designed scheduling algorithims can result in starvation. For example: SJF, Priority Based Scheduling, Multilevel Queue Sheduling and Shortest Remaining Time.

2.	Which scheduling algorithm results the smallest average wait time?

SJF scheduling algorithm

3.	What scheduling algorithm has the longest average response time? shortest total wait time?

SJF (again?)

4.	Describe Round-Robin scheduling and its performance advantages and disadvantages.

It jumps from task to task after a set amount of time has passed. The pros: it's starvation free, it's common, it's preemptive. The cons: more overhead of context switching.

5.	Describe the First Come First Serve (FCFS) scheduling algorithm. Explain how it leads to the convoy effect. 

the FCFS algorithm executes tasks in order of their arrival. The convoy effecct happens in which the whole operating system slows down due to a few slow processes.

6.	Describe the Pre-emptive and Non-preemptive SJF scheduling algorithms. 

Preemptive SJF is when jobs are put into the ready queue as they come. If a new job comes in with shorter burst time than the current job, the 2 will switch. Non-preemptive SJF does not interrupt current job.

7.	How does the length of the time quantum affect Round-Robin scheduling? What is the problem if the quantum is too small? In the limit of large time slices Round Robin is identical to _____?

The length of time quantum affects round robin in how much time passes before switching tasks. If the quantum is too small it will frequently switch jobs (which can become inefficient). If the slices are too large round robin becomes identical to FCFS

8.	What reasons might cause a scheduler switch a process from the running to the ready state?

if there's a preemption.

## 5. Synchronization and Deadlock

1.	Define circular wait, mutual exclusion, hold and wait, and no-preemption. How are these related to deadlock?

Circular wait: each process must be waiting for a resource which is being held by another process, which in turn is waiting for the first process to release the resource.
Mutual exclusion: At least two resource must be held in a non-shareable mode. Otherwise, the processes would not be prevented from using the resource when necessary. Only one process can use the resource at any given instant of time
Hold and wait: a process is currently holding at least one resource and requesting additional resources which are being held by other processes.
No preemption: a resource can be released only voluntarily by the process holding it.

2.	What problem does the Banker's Algorithm solve?

The banker algorithm tests for safety by simulating the allocation of pre determined maximum possible ammounts of all resources, then makes an s-state check to test for possible deadlock conditions and all other pending activities, before deciding whether allocation should be allowed to continue.

3.	What is the difference between Deadlock Prevention, Deadlock Detection and Deadlock Avoidance?

deadlock prevention ensures that at least one of the necessary conditions to cause a deadlock will never occur, while deadlock avoidance ensures that the system will not enter an unsafe state. Deadlock detection simply detects if there is a deadlock.

4.	Sketch how to use condition-variable based barrier to ensure your main game loop does not start until the audio and graphic threads have initialized the hardware and are ready.

5.	Implement a producer-consumer fixed sized array using condition variables and mutex lock.

6.	Create an incorrect solution to the CSP for 2 processes that breaks: i) Mutual exclusion. ii) Bounded wait.

7.	Create a reader-writer implementation that suffers from a subtle problem. Explain your subtle bug.

## 6. IPC and signals

1.	Write brief code to redirect future standard output to a file.

2.	Write a brief code example that uses dup2 and fork to redirect a child process output to a pipe

3.	Give an example of kernel generated signal. List 2 calls that can a process can use to generate a SIGUSR1.

4.	What signals can be caught or ignored?

5.	What signals cannot be caught? What is signal disposition?

6.	Write code that uses sigaction and a signal set to create a SIGALRM handler.

7.	Why is it unsafe to call printf, and malloc inside a signal handler?

## 7. Networking 

1.	Explain the purpose of `socket`, `bind`, `listen`, and `accept` functions

2.	Write brief (single-threaded) code using `getaddrinfo` to create a UDP IPv4 server. Your server should print the contents of the packet or stream to standard out until an exclamation point "!" is read.

3.	Write brief (single-threaded) code using `getaddrinfo` to create a TCP IPv4 server. Your server should print the contents of the packet or stream to standard out until an exclamation point "!" is read.

4.	Explain the main differences between using `select` and `epoll`. What are edge- and level-triggered epoll modes?

5.	Describe the services provided by TCP but not UDP. 

6.	How does TCP connection establishment work? And how does it affect latency in HTTP1.0 vs HTTP1.1?

7.	Wrap a version of read in a loop to read up to 16KB into a buffer from a pipe or socket. Handle restarts (`EINTR`), and socket-closed events (return 0).

8.	How is Domain Name System (DNS) related to IP and UDP? When does host resolution not cause traffic?

9.	What is NAT and where and why is it used? 

## 8. Files 

1.	Write code that uses `fseek`, `ftell`, `read` and `write` to copy the second half of the contents of a file to a `pipe`.

2.	Write code that uses `open`, `fstat`, `mmap` to print in reverse the contents of a file to `stderr`.

3.	Write brief code to create a symbolic link and hard link to the file /etc/password

4.	"Creating a symlink in my home directory to the file /secret.txt succeeds but creating a hard link fails" Why? 

5.	Briefly explain permission bits (including sticky and setuid bits) for files and directories.

6.	Write brief code to create a function that returns true (1) only if a path is a directory.

7.	Write brief code to recursive search user's home directory and sub-directories (use `getenv`) for a file named "xkcd-functional.png' If the file is found, print the full path to stdout.

8.	The file 'installmeplz' can't be run (it's owned by root and is not executable). Explain how to use sudo, chown and chmod shell commands, to change the ownership to you and ensure that it is executable.

## 9. File system 
Assume 10 direct blocks, a pointer to an indirect block, double-indirect, and triple indirect block, and block size 4KB.

1.	A file uses 10 direct blocks, a completely full indirect block and one double-indirect block. The latter has just one entry to a half-full indirect block. How many disk blocks does the file use, including its content, and all indirect, double-indirect blocks, but not the inode itself? A sketch would be useful.

2.	How many i-node reads are required to fetch the file access time at /var/log/dmesg ? Assume the inode of (/) is cached in memory. Would your answer change if the file was created as a symbolic link? Hard link?

3.	What information is stored in an i-node?  What file system information is not? 

4.	Using a version of stat, write code to determine a file's size and return -1 if the file does not exist, return -2 if the file is a directory or -3 if it is a symbolic link.

5.	If an i-node based file uses 10 direct and n single-indirect blocks (1 <= n <= 1024), what is the smallest and largest that the file contents can be in bytes? You can leave your answer as an expression.

6.	When would `fstat(open(path,O_RDONLY),&s)` return different information in s than `lstat(path,&s)`?

## 10. "I know the answer to one exam question because I helped write it"

Create a hard but fair 'spot the lie/mistake' multiple choice or short-answer question. Ideally, 50% can get it correct.
