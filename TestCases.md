## Case 1a
Below code is the psuedocode for the strided array addition without using SPM

```cpp
for(int count = 0; count < 10; count++){
    for(int i = 0; i < 100; i++){
        sum[CID] += a[i * x]; //partial sum, x = 100
    }
}

if(CID == 0){
    for(int i = 1; i < 4; i++){
        sum[0] += sum[i];
    }
    print(sum[0]);
}
```

For simplicity, I have reduced the maximum count to 10
The assembly code for this is in TestCase1.txt and the configurations is in config1.txt

Given Configurations:
L1 Data Cache size = 400 Bytes
SPM size = 400 Bytes
Directly Mapped L1 Cache

Assumed Configurations:
BlockSize = 40 Bytes (10 words)
L1 Instruction Cache Size = 400 Bytes
L2 Cache Size = 800 Bytes
L1 Cache Latency = 2
L2 Cache Latency = 5
Memory Latency = 50

After you run the code with the given configurations:

### Observations
Excluding the instructions to sum the partial sums, the average values are :
Clock Cycles = 30642
Total Stalls = 27729
Instructions = 2909
IPC = 0.094
Memory Accesses = 750
L1 Cache Hits = 380
L1 Cache Miss = 370
L1 Instruction Hits = 2900
L1 Instruction Miss = 2
L2 Cache Hits = 0
L2 Cache Misses = 360


### Conclusions
- After observing the values, we can conclude that there are no hits in L2 caches.
- There's supposed to be no hits in L1 caches as well. The hits in L1 caches are due to storing in the addresses which were missed before. That's why they have similar number of hits and misses.
- IPC is very low

## Case 2b
The same cpp pseudocode and the configurations with only change in the associativity of the L1 cache. The cache is now fully associative.

The assembly code for this is in TestCase1.txt and the configurations is in config2.txt
After you run the code with the given configurations:

### Observations
Excluding the instructions to sum the partial sums, the average values are :
Clock Cycles = 24496
Total Stalls = 21583
Instructions = 2909
IPC = 0.118
Memory Accesses = 750
L1 Cache Hits = 500
L1 Cache Miss = 250
L1 Instruction Hits = 2900
L1 Instruction Miss = 2
L2 Cache Hits = 0
L2 Cache Misses = 252

### Conclusions
- After observing the values, we can conclude that this is faster than the case of direct mapped cache
- Initially, the address will be a miss in L1 cache but it will be a hit when you write the address. It will also be a hit when you store the partial sum
- As expected from the above case, there are no hits in L2 cache
- IPC is better than the above case

## Case 2
Below is the pseudocode for the strided array addition with using SPM

```cpp
for(int i = 0; i < 100; i++){
    spm[i] = a[i * x];  //x = 100
}

for(int count = 0; count < 10; count++){
    for(int i = 0; i < 100; i++){
        sum[CID] += spm[i];
    }
}

if(CID == 0){
    for(int i = 1; i < 4; i++){
        sum[0] += sum[i];
    }
    print(sum[0]);
}
```

For simplicity, I have reduced the maximum count to 10
The assembly code for this is in TestCase2.txt and the configurations is in config2.txt

Given Configurations:
L1 Data Cache size = 400 Bytes
SPM size = 400 Bytes

Assumed Configurations:
BlockSize = 40 Bytes (10 words)
L1 Instruction Cache Size = 400 Bytes
L2 Cache Size = 800 Bytes
L1 Cache Latency = 2
L2 Cache Latency = 5
Memory Latency = 50

After you run the code with the given configurations:

### Observations
Excluding the instructions to sum the partial sums, the average values are :
Clock Cycles = 11496
Total Stalls = 8833
Instructions = 2659
IPC = 0.231
Memory Accesses = 500
L1 Cache Hits = 500
L1 Cache Miss = 0
L1 Instruction Hits = 2645
L1 Instruction Miss = 2
L2 Cache Hits = 0
L2 Cache Misses = 2

### Conclusions
- After observing the values, we can conclude that this is faster than the above two cases
- The IPC is high compared to the case without using SPM
- The Memory Access are only to store the partial sum in the memory and all of them are hits since only 4 words are used


## Why is Fully Associative Cache better than Direct Mapped Cache in Strided Array Addition?
In the strided array addition, we choose x such that on every access to the next element, the address of the element marks to the same index (in cache) as the element before.
So, in the case of direct mapped cache, we are basically not using the cache to its fullest capacity. Only one block of the cache is being used. This results in increase in miss rate of L1 Data cache and increases the number of stalls due to which IPC and the performance falls down.
In the case of fully associative, we will be using all the blocks of the cache. So, the miss rate decreases as compared to the direct mapped cache. So performance increases.

## Is the performance same for using Fully Associative Cache and SPM in Strided Array Addition?
In our code, no, the performance is not the same.
This is because in our simulator, we are using a special instruction called .spm which directly initializes the SPM with the required values. (This is assuming the bandwidth is sufficient to load 100 words at a time)
The performance will be same when the spm is initialized, one element by one element, by accessing the main memory

## Why are the latencies of SPM far lower than the caches?
The latencies of SPM are lower because:
- Fixed and Faster access, we can access the SPM just by searching the index instead of doing the offset, index and tag method like in cache
- The programmer knows what he wants to access, so there will be no misses
- There are no replacement and eviction protocols in SPM in contrast to caches

## Is there a performance benefit if SPM is used? (ignoring the latencies and the sizes)
Yes, SPM performs better than Caches because:
- Better Control, the programmer can control the data he wishes to access
- No cache misses and eviction protocols
- Algorithms can be optimized by using SPM (like the partial sum)

## Case 3
Below code is the psuedocode for the strided array addition using SPM but the accesses exceed the size

```cpp
for(int i = 0; i < 50; i++){
    spm[i] = a[i * x];  //x = 100
    }


for(int count = 0; count < 10; count++){
    for(int i = 0; i < 50; i++){
        sum[CID] += spm[i];
    }
}

for(int i = 50; i < 100; i++){
    spm[i] = a[i * x];
}

for(int count = 0; count < 10; count++){
    for(int i = 0; i < 50; i++){
        sum[CID] += spm[i];
    }
}

if(CID == 0){
    for(int i = 1; i < 4; i++){
        sum[0] += sum[i];
    }
    print(sum[0]);
}
```

For simplicity, I have reduced the maximum count to 10, also size of spm is reduced to 200 bytes, meaning only 50 elements to be accessed.
Now, we are going to access the first 50 elements and process them. Then evict the elements in SPM and bring the next 50 required elements and process them.
The assembly code for this is in TestCase3.txt and the configurations is in config3.txt


Given Configurations:
L1 Data Cache size = 200 Bytes
SPM size = 200 Bytes

Assumed Configurations:
BlockSize = 40 Bytes (10 words)
L1 Instruction Cache Size = 400 Bytes
L2 Cache Size = 800 Bytes
L1 Cache Latency = 2
L2 Cache Latency = 5
Memory Latency = 50

After you run the code with the given configurations:

### Observations
Excluding the instructions to sum the partial sums, the average values are :
Clock Cycles = 16351
Total Stalls = 13436
Instructions = 2911
IPC = 0.178
Memory Accesses = 520
L1 Cache Hits = 520
L1 Cache Miss = 0
L1 Instruction Hits = 2883
L1 Instruction Miss = 5
L2 Cache Hits = 0
L2 Cache Misses = 5

### Conclusion
- After observing the values, we can conclude that this is obviously slower than the above case where the size is same as the number of elements to be accessed
- The execution is much more slower than expected because the simulator has a special instruction "get", which brings the data, one element by one element, from the memory. This can be speeded up since the bandwidth is usually larger than just carrying one word a time
- This is still faster than Case 1 where 100 elements are accessed using cache

### Note
All these simulation is done when data forwarding is enabled. If you want to disable it, change the enabled configuraion in config files from "enable = y" to "enable = n"