
#  SYSC4001 A2 — Operating Systems Assignment 2
#  Carleton University | Fall 2025
#  Student: Srivathsan Murali (101287534)
#  Student: Emeka Anonyei (101209704)
#  Repository: github.com/Watson752/SYSC4001_A2


>> OVERVIEW
---------------------------------------------------------------
Implements process scheduling, memory management, and 
system call simulation. Builds on Assignment 1’s interrupt 
simulator with realistic process control primitives.

Project is divided into:
  • Part II – Concurrent Processes in Unix (C)
  
  • Part III – API Simulator (fork & exec) (C++)
  
  • report.pdf – Final analysis and discussion (root directory)

---------------------------------------------------------------
>> REPOSITORY STRUCTURE
---------------------------------------------------------------
```
SYSC4001_A2/
├── report.pdf
│
├── SYSC4001_A2_P2/
│   ├── process1.c
│   ├── process2.c
│   ├── shared_memory.c
│   ├── semaphores.c
│   └── README_P2.md
│
└── SYSC4001_A2_P3/
    ├── build.sh
    ├── interrupts.cpp
    ├── interrupts.hpp
    ├── simulator.exe
    ├── .gitignore
    └── input_files/
        ├── trace.txt
        ├── vector_table.txt
        ├── external_files.txt
        ├── device_table.txt
        ├── execution.txt
        └── system_status.txt
```
NOTE:
  The simulator outputs (`execution.txt`, `system_status.txt`)
  are located inside the 'input_files/' folder.

---------------------------------------------------------------
>> PART I — CONCEPTUAL ANALYSIS
---------------------------------------------------------------
• CPU Scheduling: FCFS, RR(4ms), Preemptive SJF, MLFQ
• Memory Allocation: First Fit, Best Fit, Worst Fit
• Calculations for turnaround time, completion, fragmentation
• Gantt charts and explanations in report.pdf

---------------------------------------------------------------
>> PART II — CONCURRENT PROCESSES (C)
---------------------------------------------------------------
Implements concurrent Unix processes using:
  - fork(), exec(), wait()
  - Shared memory (shmget, shmat, shmdt, shmctl)
  - Semaphores (semget, semop, semctl)

Behavior:
  Process 1 → increments counter & prints multiples of 3
  Process 2 → decrements counter, starts after counter ≥ 100
Both end when shared variable > 500.

# Compile & Run
$ gcc process1.c -o process1
$ gcc process2.c -o process2
$ ./process1 &
$ ./process2 &
$ ps aux | grep process
$ kill <PID>

---------------------------------------------------------------
>> PART III — API SIMULATOR (C++)
---------------------------------------------------------------
Simulates fork() and exec() within a single-CPU OS model.

Core Components:
  • 6 fixed memory partitions (40, 25, 15, 10, 8, 2 MB)
  • PCB table storing PID, partition, size, state
  • External files list loaded from external_files.txt
  • Trace file defining events (FORK, EXEC, CPU, SYSCALL)
  • Output logs: execution.txt & system_status.txt

# Build & Run
```
$ cd SYSC4001_A2_P3
$ chmod +x build.sh
$ ./build.sh
$ ./interrupts.exe
```
```
Example: execution.txt
  0,1,switch to kernel mode
  1,10,context saved
  13,10,cloning the PCB
  24,1,IRET
  ...
```
```
Example: system_status.txt
  time: 247; current trace: EXEC program1, 50
  +------------------------------------------------------+
  | PID | program | partition | size | state             |
  +------------------------------------------------------+
  |  1  | program1 |    4     |  10  | running           |
  |  0  | init     |    6     |   1  | waiting           |
  +------------------------------------------------------+
```
---------------------------------------------------------------
>> TESTING
---------------------------------------------------------------
• Mandatory scenarios: 3 provided in assignment spec
• Custom tests: added for nested FORK/EXEC & partition reuse
All results and explanations included in report.pdf.

---------------------------------------------------------------
>> REPORT
---------------------------------------------------------------
report.pdf (root directory) includes:
  • Explanations of fork & exec behavior
  • PCB and memory allocation analysis
  • Comparison between C (Part II) & C++ (Part III)
  • Simulation result breakdowns and commentary

---------------------------------------------------------------
>> ENVIRONMENT & TOOLS
---------------------------------------------------------------
Languages  :  C (Part II), C++ (Part III)
Compiler   :  GCC / G++ (ISO C++17)
Platform   :  Linux / WSL (Ubuntu)
Editor     :  VS Code / CLion
Build Tool :  build.sh (Bash)

---------------------------------------------------------------
>> REFERENCES
---------------------------------------------------------------
• Silberschatz, Galvin & Gagne — Operating System Concepts

• Linux man pages: fork, exec, shmget, semget

• Carleton SYSC4001 course slides & Brightspace materials

---------------------------------------------------------------
>> SUMMARY
---------------------------------------------------------------
```
✅ Part II — Implemented in C
✅ Part III — Implemented in C++
✅ report.pdf — in root directory
✅ Outputs (execution.txt, system_status.txt) — in input_files/
```


# END OF FILE
