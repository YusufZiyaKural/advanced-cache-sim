\# Cache Simulator



This project is a flexible, configurable \*\*Cache Memory Simulator\*\* written in C++. It reads memory access traces and simulates the behavior of a cache memory hierarchy based on user-defined configurations. It reports detailed statistics including hit/miss rates, AMAT (Average Memory Access Time), and miss classifications (Compulsory, Capacity, Conflict).



\## Features



\* \*\*Associativity:\*\* Supports Direct Mapped, N-Way Set Associative, and Fully Associative caches.

\* \*\*Replacement Policies:\*\* LRU (Least Recently Used), FIFO (First-In First-Out), and Random.

\* \*\*Write Policies:\*\* Write-Back and Write-Through.

\* \*\*Allocation Policies:\*\* Write-Allocate and No-Write-Allocate.

\* \*\*Detailed Reporting:\*\* Calculates total loads/stores, hit rates, and breaks down misses into Compulsory, Capacity, and Conflict types.



\## Prerequisites



\* A C++ compiler (GCC/G++ recommended).

\* Standard C++ libraries (no external dependencies required).



\## Compilation Instructions



To compile the source code, open your terminal and run the following command:



```bash

g++ -o cache\_sim cache\_sim.cpp



```



This will create an executable file named `cache\_sim` (or `cache\_sim.exe` on Windows).



\## Usage



The simulator requires two command-line arguments: a \*\*configuration file\*\* and a \*\*trace file\*\*.



\*\*Syntax:\*\*



```bash

./cache\_sim <config\_file> <trace\_file>



```



\*\*Example:\*\*



```bash

./cache\_sim config.txt trace.txt



```



\### Input File Formats



\#### 1. Configuration File (`config.txt`)



This file defines the cache parameters using `KEY=VALUE` pairs.



\* \*\*CACHE\_SIZE:\*\* Total size of the cache in bytes.

\* \*\*BLOCK\_SIZE:\*\* Size of a single block in bytes.

\* \*\*ASSOCIATIVITY:\*\* Number of ways (Set size). Use `0` for Fully Associative.

\* \*\*REPLACEMENT\_POLICY:\*\* `LRU`, `FIFO`, or `RANDOM`.

\* \*\*WRITE\_POLICY:\*\* `WRITE\_BACK` or `WRITE\_THROUGH`.

\* \*\*WRITE\_ALLOCATE:\*\* `1` for Write-Allocate, `0` for No-Write-Allocate.



\*\*Example `config.txt`:\*\*



```text

CACHE\_SIZE=4096

BLOCK\_SIZE=16

ASSOCIATIVITY=4

REPLACEMENT\_POLICY=LRU

WRITE\_POLICY=WRITE\_BACK

WRITE\_ALLOCATE=1



```



\#### 2. Trace File (`trace.txt`)



Contains the sequence of memory accesses. Each line should contain the operation type (`R` for Read, `W` for Write) followed by the hexadecimal address.



\*\*Example `trace.txt`:\*\*



```text

R 0x0040

W 0x00A4

R 0x00A8

W 0x1000



```



\## Output



After execution, the program prints the simulation statistics to the standard output:



```text

Total loads: 2

Total stores: 2

Load hits: 1

Load misses: 1

Store hits: 0

Store misses: 2

Total hit rate: 25.00%

Compulsory misses: 3

Capacity misses: 0

Conflict misses: 0

Memory reads: 3

Memory writes: 1

AMAT: 76 cycles



```



---



\*\*Note:\*\* The AMAT calculation assumes a fixed base formula (`1 + Miss Rate \* 100`).

