## Cores
There are 4 cores in the simulator. All the simulators run simultaneously. They share same memory, same caches. 

## Pipeline Stages
5 stages of pipeline are implemented. Instruction Fetch, Instruction Decode, Execute, Memory Stage, Write Back.
Among these, Instruction Fetch is common for all cores and the rest 4 stages are core specific.

### Instruction Fetch
This stage is common for all cores. It fetches the instructions written in the assembly code. It accesses the instruction cache. If miss, it accesses the unified L2 cache. If miss here too, it accesses the instruction memory. It then pushes the instruction into the if_id register.

### Instruction Decode
- This stage decodes the type of instruction and gets data from the required registers. 
- It first retrieves the instruction from the if_id register. It separates the opcode, registers and immediate values and gets data as required. 
- The hazards are checked in this stage. If found, stall is added. 
- The branch instructions are also checked here and dealt separately. If branch instruction found, it will lead to a stall right away. 
- It will then push the required data into if_ex register.

### Execute
- This stage perfroms arithmetic operations on the data retrieved from the if_ex register. 
- First, it checks for the branch instructions. Whether taken or not, it will flush the pipeline and starts from the obtained pc. 
- If not branch instruction, it will perform the required arithmetic operation. It also stalls if there is variable latency for the arithmetic operations.
- It then sends the required data to ex_mem register.

### Memory Stage
- This stage accesses the memory and loads/stores it.
- If the instruction is either load or store, it will access the L1 cache. If found, it will retrieve the data. If not, it will access the L2 cache. If missed again, it will access the memory cache.
- It then sends the required data to mem_wb register.
- If the instruction is not a memory instruction, just move to Write Back Stage.

### Write Back
- This stage writes the output in the registers of the cores.
- It will retrieve the register number from the data passed onto the mem_wb register.
- It will then write the retrieved data into the particulat register.

## Caches
These are implemented to reduce the memory access time. All the caches are shared by every core and are not core specific.

Three main important parts of caches:
- Tag = (address / blockSize) * blockSize : Used to check if the required data is present in the particular set
- Index = ((address / blockSize) % blocks) : Used to search the particular set of the cache
- Offset = (address % blockSize) + 2 : Used to search the required data through the cache line


- The first element of the cache line stores the tag, the second element stores the value required for the replacement policies, and the rest of the elements are the data. 
- The caches follow spatial locality and temporal locality.

### L1 Data Cache
- This is the cache which stores some data from the memory.
- It has very low latency compared to the memory.
- When the memory is asked to be accessed in the assembly code, the memory stage first checks here. If found, L1 cache sends back the data to the core. If not, it checks in L2 cache.

### L1 Instruction Cache
- This is the cache which stores pc for the instructions.
- It has same latency as the L1 instruction cache.
- When the instruction is to fetched, the instruction fetch stage first checks here. If found, this will send back the instruction to the core. If not, it checks in L2 cache.

### L2 Cache
- This is the unified cache, i.e, it stores both data and pc.
- This cache is divided into two halfs. One half is for data and the other half is for pc.
- When it's a miss in either of the L1 cache, L2 cache is checked.
- If found, it sends the whole cache line to the L1 cache onto the corresponding set. If L1 cache is full, it will evict a block, based on the replacement policy, and the accessed block is stored in the empty block created. The evicted block is stored in L2 cache.
- If not found, it access the memory and gets the required cache line into the L2 cache and repeats the process above. 

### ScratchPadMemory
- This is not exactly a cache, but acts like one.
- The data stored in this is controlled by the programmer.
- Special instructions like lw_spm and sw_spm are used to access the ScratchPadMemory.
- If new data is to be stored in the ScratchPadMemory, it evicts the data inside and gets the required data from the memory.

## Sync Instruction
- This instruction is used to stall the cores, which have finished executing, to wait for the cores which haven't executed untill the particular point.
- When a core reaches the sync instruction, it will send a signal to system. This core is now stalled. If all the four cores have sent the signal to the system, it will now remove the stalls.

## Special Instructions Added
- lw_spm : Same as lw, but access the elements in SPM
- sw_spm : Same as sw, but access the elements in SPM
- evict : Evicts the data in SPM from a provided index to another provided index
- get : Loads the data from memory using the address present in the provided register
- .spm : Initializes the SPM with the provided indices of the memory variables