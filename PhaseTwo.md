## Getting Started
- Clone the repository
- Open phase2 folder
- Open Main.cpp
- Run "g++ Main.cpp -o Main" in terminal
- Run "./Main" in terminal

## New Features
- Incorporated pipelining
- Common instruction fetch unit for all the 4 cores
- Separate instruction decode, execute, memory, write back unit for all the 4 cores
- Hazards can now happen leading to stalls
- Allows data forwarding from one pipeline to other
- All the 4 cores share the same memory
- Variable Latency for arithmetic instructions (add, addi, sub, mul, muli, div)
- Branch instructions now allow only a particular core to take/not take the branch
- Added print instruction

## Limitations
- Cores run one after the another
- Cannot find syntax error
- Can only add .word variables in the .data section
- Use only decimal numbers in the .data section
- No empty lines to be added in between the instructions in the assembly code

## Things we wanted to implement but couldn't
- User Interface
- Run the pipelining using threads
- Run the 4 cores all at once using threads
- Detection of syntax errors

## Problems Faced
- Implementation of threads arose many problems due to its unpredictable nature
- Only once in three times did the program gave the right output using threads
- Data forwarding was very hard to implement with threads
- Had to restart the whole project again without the threads
