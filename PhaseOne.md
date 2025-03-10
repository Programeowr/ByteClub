## Getting Started
- Clone the repository
- Open phase1 folder
- Open simCoa.cpp
- Run "g++ simCoa.cpp -o simCoa" in terminal
- Run "./simCoa" in terminal

## Features
- Can execute basic assembly instructions
  
	a. add, sub, mul, div (arithmetic)

	b. addi (immediate values)

	c. muli (added this immediate instruction on our own)

	d. lw, sw (accessing the memory)

	e. li (initializing)

	f. la (load address for variables)
- Contains four cores which share 4KB memory among themselves
- Can execute .data instructions to store data in memory
- Special registers (x31) containing coreID.

## Limitations
- Cannot find syntax error
- Can only add .word variables in the .data section
- No empty lines to be added in between the instructions in the assembly code
