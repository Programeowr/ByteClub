# ByteClub
Rule 1: You do not talk about ByteClub.

Rule 2: You DO NOT TALK ABOUT ByleClub.

Risc-V Simulator


## Overview
Our project is based on simulator which contains four processors. It simulates a multi-core environment where it can execute assembly instructions based on the lines of Ripes.

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

## Limitations
- Cannot find syntax error
- Can only add .word variables in the .data section
- No empty lines to be added in between the instructions in the assembly code

## Meeting Minutes
Refer to MeetingMinutes.md for all the minutes of the meeting for each meeting held.
