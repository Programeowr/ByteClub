## Getting Started
- Clone the repository
- Open phase3 folder
- Open Main.cpp
- Run "g++ Main.cpp -o Main" in terminal
- Run "./Main" in terminal

## New Features
- Added L1 Cache : Data and Instruction 
- Added Unified L2 Cache
- Variable Latency for memory accesses at different levels
- Added sync function which stalls cores to wait for other cores to finish executing instructions untill that point
- Added ScratchPadMemory which is controlled by the programmer
- Added special instructions (lw_spm, sw_spm, evict, get) to access ScratchPadMemory
- Added .spm keyword inside .data section to store data directly in ScratchPadMemory

## Updates
- The cores now run simultaneously instead of one after another

## Limitations
- Instruction Cache stores pc, not the whole instruction
- Cannot find syntax error
- Can only add .word variables in the .data section
- Use only decimal numbers in the .data section
- No empty lines to be added in between the instructions in the assembly code

## Things we wanted to implement but couldn't
- User Interface
- Detection of syntax errors
- Core Specific L1 Cache
- Store instructions instead of pc in Instruction Cache

## Problems Faced
- Storing instructions in Instruction Cache was difficult to implement since it was not possible to have int data type (for tag) and string data type (for instructions) in the same vector in cpp.

