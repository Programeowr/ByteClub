#include <bits/stdc++.h>
#include <sstream>
#include <ctime>
using namespace std;

class DisableCores{
    public:

    vector<int> registers;
    int coreID;
    int pc;
    unordered_map<string, pair<int,int>> mp;

    queue<string> if_id;
    queue<tuple<string, int, int, int>> id_ex;
    queue<tuple<string, int, int>> ex_mem;
    queue<tuple<string, int, int>> mem_wb;
    queue<tuple<string, int>> wb_if;
    queue<tuple<string, int, int, string>> branch;

    bool completed = false;
    bool stall = false;
    bool branchStall = false;
    bool coreInstruction = false;

    bool fetchCompleted = false;
    bool decodeCompleted = false;
    bool executeCompleted = false;
    bool memCompleted = false;
    bool writeCompleted = false;
    bool sync = false;

    int count = 0;
    int latencyStall = 0;
    int stallDuration = 0;
    int branchDuration = 0;
    int stallCount = 0;
    int branchStallCount = 0;
    int cacheStallCount = 0;
    int arithmeticStallCount = 0;
    int instructionStallCount = 0;
    int spmStallCount = 0;
    int syncStallCount = 0;
    int otherStallCount = 0;
    int instructionsCount = 0;
    int cycles = -1;

    int cacheStall = 0;
    int memStall = 0;
    int instructionStall = 0;
    int spmStall = 0;
    int cacheOneHit = 0;
    int instructionCacheHit = 0;
    int cacheTwoHit = 0;
    int cacheOneMiss = 0;
    int instructionCacheMiss = 0;
    int cacheTwoMiss = 0;
    int memAccess = 0;
    int spmAccess = 0;
    int syncStall = 0;

    DisableCores(int cid){
        registers.resize(32, 0);
        pc = 0;
        coreID = cid;
        registers[0] = 0;
        registers[31] = cid;
    }

    DisableCores(const DisableCores&) = delete;
    DisableCores& operator=(const DisableCores&) = delete;

    DisableCores(DisableCores&& other) noexcept
        : coreID(other.coreID), registers(std::move(other.registers)), pc(other.pc) {}

    DisableCores& operator=(DisableCores&& other) noexcept {
        if (this != &other) {
            coreID = other.coreID;
            registers = std::move(other.registers);
            pc = other.pc;
        }
        return *this;
    }

    bool checkStall(string instruction){
            istringstream ss(instruction);
            string opcode;
            int rd = -1;
            int rs1 = -1;
            int rs2 = -1;
            int r_rs1 = -1;
            int r_rs2 = -1;
    
            ss >> opcode;
    
            if(opcode == "add" || opcode == "sub" || opcode == "mul" || opcode == "div"){
                    string RD, RS1, RS2;
                    ss >> RD >> RS1 >> RS2;
                    rd = stoi(RD.substr(1));
                    rs1 = stoi(RS1.substr(1));
                    rs2 = stoi(RS2.substr(1));
            }
    
                if(opcode == "lw" || opcode == "lw_spm"){
                    string RD, DATA;
                    ss >> RD >> DATA;
    
                    rd = stoi(RD.substr(1));
                    rs1 = stoi(DATA.substr(0, DATA.find('(')));
                    rs2 = stoi(DATA.substr(DATA.find('(') + 2, DATA.find(')') - DATA.find('(') - 2));

                    rs1 = rs2;
                }
    
                if(opcode == "sw" || opcode == "sw_spm"){
                    string RD, DATA;
                    ss >> RD >> DATA;
        
                    rd = stoi(RD.substr(1));
                    rs1 = stoi(DATA.substr(0, DATA.find('(')));
                    rs2 = stoi(DATA.substr(DATA.find('(') + 2, DATA.find(')') - DATA.find('(') - 2));

                    rs1 = rd;
                }
        
                if(opcode == "bne" || opcode == "beq" || opcode == "bge" || opcode == "blt"){
                    string RS1, RS2, Label;
                    ss >> RS1 >> RS2 >> Label;

                    if(RS1 == "cid")    return false;
        
                    rs1 = stoi(RS1.substr(1));
                    rs2 = stoi(RS2.substr(1));
                }

                if(opcode == "addi" || opcode == "muli"){
                    string RD, RS, VAL;
                    ss >> RD >> RS >> VAL;

                    rs1 = stoi(RS.substr(1));
                    rs2 = rs1;
                }

                if(!wb_if.empty()){
                    auto [opcode_one, rd_one] = wb_if.front();
                    if((rd_one == rs1 || rd_one == rs2) && (opcode_one != "sw") && (opcode_one != "sw_spm")){
                        stallDuration = 1;
                        stallCount += 1;
                        cout << rd_one << endl;
                        return true;
                    }
                }

                if(!mem_wb.empty()){
                    auto [opcode_two, rd_two, result_two] = mem_wb.front();
                    if((rd_two == rs1 || rd_two == rs2) && (opcode_two != "sw") && (opcode_two != "sw_spm")){
                        stallDuration = 2;
                        stallCount += 2;
                        cout << rd_two << endl;
                        return true;
                    }
                }

                if(!ex_mem.empty()){
                    auto [opcode_three, rd_three, mem_three] = ex_mem.front();
                    if((rd_three == rs1 || rd_three == rs2) && (opcode_three != "sw") && (opcode_three != "sw_spm")){
                        stallDuration = 3;
                        stallCount += 3;
                        cout << rd_three << endl;
                        return true;
                    }
                }

                return false;
    }

    void getEvictSpm(string opcode, vector<int>& memory, vector<int>& scratchPadMem, int result, int rd, int cacheOneLatency, int memoryLatency, int spmSize){
        if(opcode == "evict"){
            spmStall = cacheOneLatency;
            scratchPadMem.erase(scratchPadMem.begin() + rd, scratchPadMem.begin() + result);
        } else {
            cacheStall = memoryLatency;
            for(int i = 0; i < rd; i++){
                if(scratchPadMem.size() < spmSize){
                    scratchPadMem.push_back(memory[result]);
                    result++;
                }
                
            }
        }
        
    }

    int accessSpm(string opcode, vector<int>& memory, vector<int>& scratchPadMem, int result, int rd, int cacheOneLatency){
        int address = result / 4;
        int mem;

        spmStall = cacheOneLatency;
        if(opcode == "lw_spm"){
            mem = scratchPadMem[address];
            return mem;
        } else {
            scratchPadMem[address] = rd;
            return rd;
        }
    } 

    int checkCacheTwo(string opcode, vector<vector<int>>& cacheOne, vector<vector<int>>& cacheTwo, vector<int>& memory, int cacheTwoSize, int blockSize, int associativityOne, int associativityTwo, int result, int value, char replacement, int cacheTwoLatency, int memoryLatency, int numOne, int tagOne, bool full){
        bool hit = false;
        bool fullTwo = true;
        int mem;
        int hitIndex;
        int address = result / 4;
        int blocks = (cacheTwo.size() / associativityTwo) / 2;
        int index = (address / blockSize) % blocks;
        int tag = (address / blockSize) * blockSize;
        int offset = (address % blockSize) + 2;
        int num = index * associativityTwo;

        for(int i = 0; i < associativityTwo; i++){
            if(cacheTwo[num + i][0] == tag){
                hit = true;
                hitIndex = i;
                cacheTwoHit++;

                mem = cacheTwo[num + i][offset];    //If hit, access the cacheTwo
                cacheTwo[num + i][1] = 0;
                cacheStall = cacheTwoLatency;
                int j;

                if(full){    
                    if(replacement == 'l'){
                        j = leastRecentlyUsed(cacheOne, numOne, associativityOne);
                    } else {
                        j = notRecentlyUsed(cacheOne, numOne, associativityOne);
                    }
                    replaceCacheBlocks(cacheOne, cacheTwo, num + i, numOne + j);
                } else {
                    j = bringCacheTwo(cacheOne, cacheTwo, numOne, num + i, blockSize, associativityOne);
                }
                cacheOne[numOne + j][0] = tagOne;
                cacheOne[numOne + j][1] = 0; 

                if(opcode == "sw"){
                    memStall = 5;
                    cacheOne[numOne + j][offset] = value;       //Write value in cacheOne
                    cacheTwo[num + i][offset] = value;         //Write value in cacheTwo
                    memory[address] = value;                //Write value in memory
                }
                break;
            }
        }

        for(int i = 0; i < associativityTwo; i++){
            if(cacheTwo[num + i][0] == -10){
                fullTwo = false;
                break;
            }
        }
        replacementPolicy(cacheTwo, replacement, num, associativityTwo, hit, fullTwo, hitIndex);

        if(!hit){
            cacheStall = memoryLatency;
            cacheTwoMiss++;
            //Check if any block is empty
            bool replaced = false;
            for(int i = 0; i < associativityTwo; i++){                    
                if(cacheTwo[num + i][0] == -10){
                    replaced = true;
                    bringMemory(cacheTwo, memory, blockSize, num + i, tag);

                    mem = cacheTwo[num + i][offset];
                    int j;
                    if(full){
                        if(replacement == 'l'){
                            j = leastRecentlyUsed(cacheOne, numOne, associativityOne);
                        } else {
                            j = notRecentlyUsed(cacheOne, numOne, associativityOne);
                        }

                        for(int k = 0; k < blockSize; k++){
                            cacheOne[numOne + j][k + 2] = cacheTwo[num + i][k + 2];
                        }
                    } else{
                        j = bringCacheTwo(cacheOne, cacheTwo, numOne, num + i, blockSize, associativityOne);
                    }

                    cacheOne[numOne + j][0] = tagOne;
                    cacheOne[numOne + j][1] = 0; 
                    
                    if(opcode == "sw"){
                        cacheOne[numOne + j][offset] = value;
                        cacheTwo[num + i][offset] = value;
                        memory[address] = value;
                    }
                    break;
                } 
            }

            //If not empty, use replacement policy
            if(!replaced){
                int i;
                if(replacement == 'l'){
                    i = leastRecentlyUsed(cacheTwo, num, associativityTwo);
                } else {
                    i = notRecentlyUsed(cacheTwo, num, associativityTwo);
                }
                
                bringMemory(cacheTwo, memory, blockSize, num + i, tag);

                mem = cacheTwo[num + i][offset];

                int j;
                if(full){
                    if(replacement == 'l'){
                        j = leastRecentlyUsed(cacheOne, numOne, associativityOne);
                    } else {
                        j = notRecentlyUsed(cacheOne, numOne, associativityOne);
                    }

                    for(int k = 0; k < blockSize; k++){
                        cacheOne[numOne + j][k + 2] = cacheTwo[num + i][k + 2];
                    }
                } else{
                    j = bringCacheTwo(cacheOne, cacheTwo, numOne, num + i, blockSize, associativityOne);
                }

                cacheOne[numOne + j][0] = tagOne;
                cacheOne[numOne + j][1] = 0; 
                
                if(opcode == "sw"){
                    cacheOne[numOne + j][offset] = value;
                    cacheTwo[num + i][offset] = value;
                    memory[address] = value;
                }
            }
        }

        return mem;        
    }

    int checkCacheOne(string opcode, vector<vector<int>>& cacheOne, vector<vector<int>>& cacheTwo, vector<int>& memory, int cacheOneSize, int cacheTwoSize, int blockSize, int associativityOne, int associativityTwo, int result, int value, char replacement, int cacheOneLatency, int cacheTwoLatency, int memoryLatency){
        bool hit = false;
        bool full = true;
        int mem;
        int hitIndex;
        int address = result / 4;
        int blocks = cacheOne.size() / associativityOne;
        int index = (address / blockSize) % blocks;
        int tag = (address / blockSize) * blockSize;
        int offset = (address % blockSize) + 2;
        int num = index * associativityOne;

        //Check if tag is hit
        for(int i = 0; i < associativityOne; i++){
            if(cacheOne[num + i][0] == tag){
                hit = true;
                hitIndex = i;
                cacheOneHit++;

                mem = cacheOne[num + i][offset];    //If hit, access the cacheOne
                cacheOne[num + i][1] = 0;           //For Replacement Policy
                cacheStall = cacheOneLatency;

                if(opcode == "sw"){
                    memStall = 5;
                    cacheOne[num + i][offset] = value;         //Write value in cacheOne
                    writeCacheTwo(cacheOne, cacheTwo, value, cacheTwoSize, blockSize, associativityTwo, address);   //Write value in cacheTwo
                    memory[address] = value;                //Write value in memory
                }
                break;
            }
        }

        for(int i = 0; i < associativityOne; i++){
            if(cacheOne[num + i][0] == -10){
                full = false;
                break;
            }
        }

        replacementPolicy(cacheOne, replacement, num, associativityOne, hit, full, hitIndex);

        // If Cache Miss
        if(!hit){
            cacheOneMiss++;
            
            mem = checkCacheTwo(opcode, cacheOne, cacheTwo, memory, cacheTwoSize, blockSize, associativityOne, associativityTwo, result, value, replacement, cacheTwoLatency, memoryLatency, num, tag, full);
            // Check Cache Two
        }

        return mem;
    }

    void replaceCacheBlocks(vector<vector<int>>& cacheOne, vector<vector<int>>& cacheTwo, int i, int j){
        
        cacheOne[j].swap(cacheTwo[i]);
        return;
    }

    void writeCacheTwo(vector<vector<int>>& cacheOne, vector<vector<int>>& cacheTwo, int value, int cacheTwoSize, int blockSize, int associativityTwo, int address){

        int blocks = (cacheTwo.size() / associativityTwo) / 2;
        int index = (address / blockSize) % blocks;
        int tag = (address / blockSize) * blockSize;
        int offset = (address % blockSize) + 2;
        int num = index * associativityTwo;

        for(int i = 0; i < associativityTwo; i++){
            if(cacheTwo[num + i][0] == tag){
                cacheTwo[num + i][offset] = value;
            }
        }

        return;
    }

    void bringMemory(vector<vector<int>>& cache, vector<int>& memory, int blockSize, int num, int tag){
        cache[num][0] = tag;
        cache[num][1] = 0;

        for(int i = 0; i < blockSize; i++){
            cache[num][i + 2] = memory[tag + i];
        }
    }

    int bringCacheTwo(vector<vector<int>>& cacheOne, vector<vector<int>>& cacheTwo, int numOne, int numTwo, int blockSize, int associativityOne){

        for(int i = 0; i < associativityOne; i++){
            if(cacheOne[numOne + i][0] == -10){
                for(int j = 0; j < blockSize; j++){
                    cacheOne[numOne + i][2 + j] = cacheTwo[numTwo][2 + j];
                }
                return i;
            }
        }
        return 0;
    }

    int leastRecentlyUsed(vector<vector<int>>& cacheOne, int num, int associativity){
        if(associativity == 1){
            return 0;
        }
        pair<int,int> lru = {0, cacheOne[num][1]};

        for(int i = 1; i < associativity; i++){
            if(cacheOne[num + i][1] > lru.second){
                lru = {i, cacheOne[num + i][1]};
            }
        }
        return lru.first;
    }

    int notRecentlyUsed(vector<vector<int>>& cacheOne, int num, int associativity){
        if(associativity == 1){
            return 0;
        }
        for(int i = 0; i < associativity; i++){
            if(cacheOne[num + i][1] == 1){
                return i;
            }
        }

        return 0;
    } 

    void replacementPolicy(vector<vector<int>>& cache, char replacement, int num, int associativity, bool hit, bool full, int j){
        if(replacement == 'l'){
            for(int i = 0; i < associativity; i++){
                if(cache[num + i][0] != -10){
                    cache[num + i][1]++;
                }
            }
        } else {
            if(hit){
                cache[num + j][1] = 0;
            } else{
                if(full){
                    int check = 0;
                    for(int i = 0; i < associativity; i++){
                        if(cache[num + i][1] == 0)  check++;
                    }

                    if(check == associativity){
                        for(int i = 0; i < associativity; i++){
                            cache[num + i][1] = 1;
                        }
                    }
                }
            }
        }
    }

    void writeBack(vector<int>& memory){
        if(memCompleted){
            writeCompleted = true;
        }
        if(mem_wb.empty())  return;

            auto [opcode, rd, value] = mem_wb.front();
            mem_wb.pop();
    
            if(opcode != "sw" && opcode != "sw_spm"){
                registers[rd] = value;
            }

            wb_if.push({opcode, rd});
    }

    void memoryStage(vector<int>& memory, vector<vector<int>>& cacheOne, vector<vector<int>>& cacheTwo, vector<int>& scratchPadMem, int cacheOneSize, int cacheTwoSize, int spmSize, int blockSize, int associativityOne, int associativityTwo, char replacement, int cacheOneLatency, int cacheTwoLatency, int memoryLatency){
        if(executeCompleted){
            memCompleted = true;
        }
        if(ex_mem.empty())  return;
            auto [opcode, rd, result] = ex_mem.front();
            ex_mem.pop();
    
            int mem = result;
            if(opcode == "lw"){
                memAccess++;
                mem = checkCacheOne(opcode, cacheOne, cacheTwo, memory, cacheOneSize, cacheTwoSize, blockSize, associativityOne, associativityTwo, result, rd, replacement, cacheOneLatency, cacheTwoLatency, memoryLatency);
            }

            if(opcode == "sw"){
                memAccess++;
                mem = checkCacheOne(opcode, cacheOne, cacheTwo, memory, cacheOneSize, cacheTwoSize, blockSize, associativityOne, associativityTwo, result, rd, replacement, cacheOneLatency, cacheTwoLatency, memoryLatency);
            }

            if(opcode == "lw_spm" || opcode == "sw_spm"){
                spmAccess++;
                mem = accessSpm(opcode, memory, scratchPadMem, result, rd, cacheOneLatency);
            }

            else if(opcode == "evict" || opcode == "get"){
                getEvictSpm(opcode, memory, scratchPadMem, result, rd, cacheOneLatency, memoryLatency, spmSize);
                return;

            }

            mem_wb.push({opcode, rd, mem});
    }

    void execute(vector<string>& program, unordered_map<string,int>& labels, unordered_map<string,int>& latency){
        if(decodeCompleted){
            executeCompleted = true;
        }
        if(id_ex.empty() && branch.empty())   return;

            if (!branch.empty()) {
                auto [opcode, r_rs1, r_rs2, label] = branch.front();

                int i = 0;

                bool branchTaken = false;
                if (opcode == "bne") {
                    if (r_rs1 != r_rs2) {
                        branchTaken = true;
                        i = labels[label];
                        pc = 4 * i;
                        count = i;
                    }
                }
    
                else if(opcode == "beq"){
                    if (r_rs1 == r_rs2) {
                        branchTaken = true;
                        i = labels[label];
                        pc = 4 * i;
                        count = i;
                    }
                }
    
                else if(opcode == "blt"){
                    if (r_rs1 < r_rs2) {
                        branchTaken = true;
                        i = labels[label];
                        pc = 4 * i;
                        count = i;
                    }
                }

                else if(opcode == "bge"){
                    if (r_rs1 >= r_rs2) {
                        branchTaken = true;
                        i = labels[label];
                        pc = 4 * i;
                        count = i;
                    }
                }
    
                else if(opcode == "jal"){
                    branchTaken = true;
                    registers[1] = pc;
                    
                    i = labels[label];
                    pc = 4 * i;
                    count = i;
                }
    
                else if(opcode == "j"){
                    branchTaken = true;
                    i = labels[label];
                    pc = 4 * i;
                    count = i;
                }

                else if(opcode == "jr"){
                    branchTaken = true;
                    pc = r_rs1;
                    count = r_rs1;
                }

                else if(opcode == "jalr"){
                    branchTaken = true;
                    int temp = stoi(label);

                    registers[r_rs1] = pc;
                    pc = temp + r_rs2;
                    count = pc;

                }

                if(branchTaken){
                    count++;
                    pc += 4;
                    
                    fetchCompleted = false;
                    decodeCompleted = false;
                    executeCompleted = false;
                    memCompleted = false;
                    writeCompleted = false;
                }
    
                branch.pop();  // Remove only once
                return;  // Exit function instead of looping again            
            }
            
    
            auto [opcode, rd, r_rs1, r_rs2] = id_ex.front();
            
            int result;

            if(opcode == "lw" || opcode == "lw_spm"){
                result = r_rs1 + r_rs2;
            }
    
            else if(opcode == "sw" || opcode == "sw_spm"){
                result = r_rs1 + r_rs2;
            }

            else if(opcode == "la" || opcode == "li"){
                result = r_rs1;
            }

            else if(opcode == "evict"){
                result = r_rs1;
            }

            else if(opcode == "get"){
                result = r_rs1 + r_rs2;
            }

            if(latencyStall == 0){
                latencyStall = latency[opcode];
            }
            
            if(latencyStall > 1){
                latencyStall--;
                return;
            }

            latencyStall = 0;

            if(opcode == "add" || opcode == "addi"){
                result = r_rs1 + r_rs2;
            }
        
            else if(opcode == "sub"){
                result = r_rs1 - r_rs2;
            }

            else if(opcode == "mul" || opcode == "muli"){
                result = r_rs1 * r_rs2;
            }

            else if(opcode == "div"){
                if(r_rs2 != 0){
                    result = r_rs1 / r_rs2;
                } else{
                    cout << "Can't divide by zero" << endl;
                    return;
                }
            }
    
            id_ex.pop();
            ex_mem.push({opcode, rd, result});
    }

    void instructionDecode(){
        if(fetchCompleted){
            decodeCompleted = true;
        }

        if(if_id.empty())   return;
            string instruction = if_id.front();
            if_id.pop();

            if(checkStall(instruction)){
                stall = true;
                return;
            }
    
            istringstream ss(instruction);
            string opcode;
            
            int rd = -1;
            int rs1 = -1;
            int rs2 = -1;
            int r_rs1 = -1;
            int r_rs2 = -1;
    
            ss >> opcode;
    
            if(opcode == "bne" || opcode == "beq" || opcode == "blt" || opcode == "bge"){
                
                branchStall = true;
                branchDuration = 2;
                string RS1, RS2, Label;
                ss >> RS1 >> RS2 >> Label;

                if(RS1 == "cid" || RS2.size() == 1){
                    coreInstruction = true;
                    r_rs1 = coreID;
                    r_rs2 = stoi(RS2);
                } else{
                    rs1 = stoi(RS1.substr(1));
                    rs2 = stoi(RS2.substr(1));
        
                    r_rs1 = registers[rs1];
                    r_rs2 = registers[rs2];
                }
    
                branch.push({opcode, r_rs1, r_rs2, Label});
                return;
            }
    
            if(opcode == "jal" || opcode == "j"){
                string Label;
                ss >> Label;
    
                branchStall = true;
                branchDuration = 2;
                branch.push({opcode, r_rs1, r_rs2, Label});
                return;
            }
    
            if(opcode == "jr"){
                string RD;
                ss >> RD;
    
                rd = stoi(RD.substr(1));
                rd = registers[rd];

                branchStall = true;
                branchDuration = 2;
                branch.push({opcode, rd, -1, ""});
                return;
            }
    
            if(opcode == "jalr"){
                string RD, DATA;
                ss >> RD >> DATA;
                
                rd = stoi(RD.substr(1));
                rs1 = stoi(DATA.substr(0, DATA.find('(')));
                rs2 = stoi(DATA.substr(DATA.find('(') + 2, DATA.find(')') - DATA.find('(') - 2));

                r_rs2 = registers[rs2];

                string temp = to_string(r_rs2);

                branchStall = true;
                branchDuration = 2;
                branch.push({opcode, rd, rs1, temp});
                return;
            }
    
            if(opcode == "add" || opcode == "sub" || opcode == "mul" || opcode == "div"){
                string RD, RS1, RS2;
                ss >> RD >> RS1 >> RS2;
                rd = stoi(RD.substr(1));
                rs1 = stoi(RS1.substr(1));
                rs2 = stoi(RS2.substr(1));
    
                r_rs1 = registers[rs1];
                r_rs2 = registers[rs2];
            }
    
            else if(opcode == "lw" || opcode == "lw_spm"){
                string RD, DATA;
                ss >> RD >> DATA;
    
                rd = stoi(RD.substr(1));
                rs1 = stoi(DATA.substr(0, DATA.find('(')));
                rs2 = stoi(DATA.substr(DATA.find('(') + 2, DATA.find(')') - DATA.find('(') - 2));
    
                r_rs1 = rs1;
                r_rs2 = registers[rs2];
            }
    
            else if(opcode == "sw" || opcode == "sw_spm"){
                string RD, DATA;
                ss >> RD >> DATA;
    
                rd = stoi(RD.substr(1));
                rs1 = stoi(DATA.substr(0, DATA.find('(')));
                rs2 = stoi(DATA.substr(DATA.find('(') + 2, DATA.find(')') - DATA.find('(') - 2));
    
                rd = registers[rd];
                r_rs1 = rs1;
                r_rs2 = registers[rs2];
            }

            else if(opcode == "la"){
                string RD, LABEL;
                ss >> RD >> LABEL;

                rd = stoi(RD.substr(1));
                r_rs1 = mp[LABEL].first;
                r_rs2 = mp[LABEL].second;
            }

            else if(opcode == "li"){
                string RD, VAL;
                ss >> RD >> VAL;

                rd = stoi(RD.substr(1));
                r_rs1 = stoi(VAL);
            }

            else if(opcode == "addi" || opcode == "muli"){
                string RD, RS, VAL;
                ss >> RD >> RS >> VAL;

                rd = stoi(RD.substr(1));
                rs1 = stoi(RS.substr(1));
                r_rs2 = stoi(VAL);

                r_rs1 = registers[rs1];
            }

            else if(opcode == "sync"){
                sync = true;
                return;
            }

            else if(opcode == "print"){
                string RD, Label;
                ss >> RD >> Label;

                rd = stoi(RD.substr(1));
                rd = registers[rd];

                cout << Label << " " << rd << endl;
                return;
            }

            else if(opcode == "evict"){
                string NUM1, NUM2;
                ss >> NUM1 >> NUM2;
                rd = stoi(NUM1);
                r_rs1 = stoi(NUM2);
                r_rs2 = r_rs1;
            }

            else if(opcode == "get"){
                string LABEL, NUM1, NUM2;
                ss >> LABEL >> NUM1 >> NUM2;

                rd = stoi(NUM2);
                r_rs1 = mp[LABEL].first;
                r_rs2 = stoi(NUM1);
            }
            
            else {
                return;
            }
            
            id_ex.push({opcode, rd, r_rs1, r_rs2});    
    }

    void clearEverything(){
        while(!if_id.empty()){
            cout << "Not empty" << endl;
            if_id.pop();
        }   
        while(!id_ex.empty()){
            cout << "Not empty" << endl;
            id_ex.pop();
        }  
        while(!ex_mem.empty()){
            cout << "Not empty" << endl;
            ex_mem.pop();
        }   
        while(!mem_wb.empty()){
            cout << "Not empty" << endl;
            mem_wb.pop();
        }   
        while(!wb_if.empty()){
            cout << "Not empty" << endl;
            wb_if.pop();
        }   
    }

    void printRegisters(){
        cout << "Registers of core " << coreID << endl; 
        for(int i = 0; i < 32; i++){
            cout << registers[i] << " ";
        }
        cout << endl;
    }
};

class DisableSimulator{
    public:

    vector<int> memory;
    float clock;
    vector<DisableCores> cores;
    vector<string> program;
    vector<vector<int>> cacheOne;
    vector<vector<int>> cacheTwo;
    vector<vector<int>> instructionCache;
    vector<int> scratchPadMem;

    bool completed = false;
    unordered_map<string, int> labels;
    unordered_map<string, int> latency;

    int blockSize;
    int cacheOneSize;
    int cacheTwoSize;
    int spmSize;
    int associativityOne;
    int associativityTwo;
    char replacement;
    int cacheOneLatency;
    int cacheTwoLatency;
    int memoryLatency;
    int instructionCacheSize;

    DisableSimulator(){
        memory.resize(4096 / 4);
        cores.emplace_back(0);
        cores.emplace_back(1);
        cores.emplace_back(2);
        cores.emplace_back(3);
    }

    void instructionFetch(vector<string>& program, int cid, int index) {
        while(!cores[cid].wb_if.empty()){
            cores[cid].wb_if.pop();
        }

        if(cores[cid].stall || cores[cid].instructionStall > 0){
            return;
        }   
        if(index > program.size()){
            return;
        }
        if(index == program.size()){
            cores[cid].fetchCompleted = true;
            return;
        }

        cores[cid].instructionsCount++;
        if(program[index].find(' ') == string::npos){
            return;
        }
        while(!cores[cid].if_id.empty()){
            cores[cid].if_id.pop();
        }

        checkInstructionCache(cid, index);

        return;    
    }

    void runProgram(int index) {
    // Initialize caches just like before
    cacheOne.resize(cacheOneSize);
    for(auto &row : cacheOne) {
        row.resize(blockSize + 2, -10);
    }

    instructionCache.resize(instructionCacheSize);
    for(auto &row : instructionCache) {
        row.resize(blockSize + 2, -10);
    }

    cacheTwo.resize(cacheTwoSize);
    for(auto &row : cacheTwo) {
        row.resize(blockSize + 2, -10);
    }

    // Initialize all cores
    for(int i = 0; i < 4; i++) {
        cout << "Core " << i << " Started" << endl;
        cores[i].count = index;
        cores[i].clearEverything();
    }

    // Continue until all cores have completed their execution
    bool allCompleted = false;
    while(!allCompleted) {
        allCompleted = true;
        if(cores[0].sync && cores[1].sync && cores[2].sync && cores[3].sync){
            cores[0].sync = false;
            cores[1].sync = false;
            cores[2].sync = false;
            cores[3].sync = false;
        }
        // Run one cycle for each core before moving to the next core
        for(int i = 0; i < 4; i++) {
            // Skip if this core has completed
            if(cores[i].writeCompleted) {
                continue;
            }
            
            // At least one core is still running
            allCompleted = false;
            
            // Process one cycle for this core
            cores[i].cycles++;
            cout << "Clock cycle " << cores[i].cycles << " on Core " << i << endl;

            if(cores[i].sync){
                cout << "Waiting for other cores" << endl;
                cores[i].syncStall++;
                cores[i].stallCount++;
                continue;
            }
            
            // Core pipeline stages
            cores[i].writeBack(memory);

            if(cores[i].cacheStall > 0) {
                cores[i].cacheStall--;
                cores[i].cacheStallCount++;
                cores[i].stallCount++;
                continue; // Skip to next core
            }

            if(cores[i].spmStall > 0){
                cores[i].spmStall--;
                cores[i].spmStallCount++;
                cores[i].stallCount++;
                continue;
            }
            
            cores[i].memoryStage(memory, cacheOne, cacheTwo, scratchPadMem, cacheOneSize, cacheTwoSize, spmSize,
                                blockSize, associativityOne, associativityTwo, replacement,
                                cacheOneLatency, cacheTwoLatency, memoryLatency);

            cores[i].execute(program, labels, latency);

            if(cores[i].latencyStall > 0) {
                cores[i].arithmeticStallCount++;
                cores[i].stallCount++;
                continue; // Skip to next core
            }

            cores[i].instructionDecode();

            if(cores[i].stall) {
                cores[i].stallDuration--;
                if(cores[i].stallDuration > 0) {
                    cores[i].otherStallCount++;
                    cores[i].stallCount++;
                    continue; // Skip to next core
                }
                cores[i].stall = false;
                cores[i].count--;
            }

            if(cores[i].branchStall) {
                cores[i].branchDuration--;
                if(cores[i].branchDuration > 0) {
                    cores[i].branchStallCount++;
                    cores[i].stallCount++;
                    continue; // Skip to next core
                }
                cores[i].branchStall = false;
                int temp = cores[i].count;
            }

            if(cores[i].instructionStall > 0) {
                cores[i].instructionStall--;
                cores[i].instructionStallCount++;
                cores[i].stallCount++;
                continue; // Skip to next core
            }

            int temp = cores[i].count;
            instructionFetch(program, i, temp);

            temp++;
            cores[i].count++;
        }
    }
    
    // All cores have finished
    for(int i = 0; i < 4; i++) {
        cout << "Core " << i << " finished" << endl;
    }

}        
    
    int checkData(){
        istringstream ss(program[0]);
        string opcode;

        ss >> opcode;
        
        if(opcode == ".data"){
            int index = 0;
            int sIndex = 0;
            int it;
            for(it = 1; it != find(program.begin(), program.end(), ".text") - program.begin(); it++){
                vector<string> words;
                istringstream ss(program[it]);
                string word;

                while(ss >> word){
                    words.push_back(word);
                }

                string label = words[0];
                label.pop_back();

                string variable = words[1];
                variable.erase(variable.begin());

                int temp = index;

                if(variable == "word"){
                    for(int j = 2; j < words.size(); j++){
                        int val = stoi(words[j].substr(2));
                        memory[index] = val;
                        index++;
                    }
                }

                cores[0].mp[label] = make_pair(temp, index);
                cores[1].mp[label] = make_pair(temp, index);
                cores[2].mp[label] = make_pair(temp, index);
                cores[3].mp[label] = make_pair(temp, index);

                temp = sIndex;
                if(variable == "spm"){
                    string base = words[2];
                    int num = stoi(words[3]);
                    pair<int,int> p = cores[0].mp[base];
                    int value = p.first;

                    for(int j = 0; j < num && value < p.second; j++){
                        scratchPadMem[sIndex] = memory[value];
                        value++;
                        sIndex++;
                    }
                }

                cores[0].mp[label] = make_pair(temp, index);
                cores[1].mp[label] = make_pair(temp, index);
                cores[2].mp[label] = make_pair(temp, index);
                cores[3].mp[label] = make_pair(temp, index);
            }
            return it;
        } else {
            return 0;
        }
    }

    
    void checkInstructionCache(int cid, int i){
        
        int blocks = instructionCache.size() / associativityOne;
        int offset = (i % blockSize) + 2;
        int index = (i / blockSize) % blocks;
        int tag = (i / blockSize) * blockSize;
        int num = index * associativityOne;

        bool hit = false;
        bool full = true;
        int hitIndex;
        int pc;
        

        for(int j = 0; j < associativityOne; j++){
            if(instructionCache[num + j][0] == tag){
                hit = true;
                hitIndex = j;
                cores[cid].instructionCacheHit++;

                pc = instructionCache[num + j][offset];
                instructionCache[num + j][1] = 0;
                cores[cid].instructionStall = cacheOneLatency;

                cores[cid].if_id.push(program[pc]);
                break;
            }
        }

        for(int j = 0; j < associativityOne; j++){
            if(instructionCache[num + j][0] == -10){
                full = false;
                break;
            }
        }

        cores[cid].replacementPolicy(instructionCache, replacement, num, associativityOne, hit, full, hitIndex);  

        
        if(!hit){
            
            cores[cid].instructionCacheMiss++;

            checkInstructionCacheTwo(cid, i, full, num, tag);
        }

        return;
    }

    void checkInstructionCacheTwo(int cid, int i, bool fullOne, int numOne, int tagOne){
        int blocks = (cacheTwo.size() / associativityTwo) / 2;
        int offset = (i % blockSize) + 2;
        int index = ((i / blockSize) % blocks) + blocks;
        int tag = (i / blockSize) * blockSize;
        int num = index * associativityTwo;

        bool hit = false;
        bool fullTwo = true;
        int hitIndex;
        int pc;

        for(int j = 0; j < associativityTwo; j++){
            if(cacheTwo[num + j][0] == tag){
                hit = true;
                hitIndex = j;
                cores[cid].cacheTwoHit++;

                pc = cacheTwo[num + j][offset];
                cores[cid].if_id.push(program[pc]);
                cacheTwo[num + j][1] = 0; 
                cores[cid].instructionStall = cacheTwoLatency - 1;
                int k = 0;

                if(fullOne){
                    if(replacement == 'l'){
                        k = cores[cid].leastRecentlyUsed(instructionCache, numOne, associativityOne);
                    } else {
                        k = cores[cid].notRecentlyUsed(instructionCache, numOne,associativityOne);
                    }
                    cores[cid].replaceCacheBlocks(instructionCache, cacheTwo, numOne + k, num + j);
                } else {
                    k = cores[cid].bringCacheTwo(instructionCache, cacheTwo, numOne, num + j, blockSize, associativityOne);
                }

                instructionCache[numOne + k][0] = tagOne;
                instructionCache[numOne + k][1] = 0;

                break;
            }
        }
        
        for(int j = 0; j < associativityTwo; j++){
            if(cacheTwo[num + j][0] == -10){
                fullTwo = false;
                break;
            }
        }

        cores[cid].replacementPolicy(cacheTwo, replacement, num, associativityTwo, hit, fullTwo, hitIndex);

        if(!hit){
            cores[cid].instructionStall = memoryLatency - 1;
            cores[cid].cacheTwoMiss++;

            bool replaced = false;
            for(int j = 0; j < associativityTwo; j++){
                if(cacheTwo[num + j][0] == -10){
                    replaced = true;
                    bringInstructions(num + j, tag);

                    pc = cacheTwo[num + j][offset];
                    cores[cid].if_id.push(program[pc]);
                    int k = 0;

                    if(fullOne){
                        if(replacement == 'l'){
                            k = cores[cid].leastRecentlyUsed(instructionCache, numOne, associativityOne);
                        } else {
                            k = cores[cid].notRecentlyUsed(instructionCache, numOne, associativityOne);
                        }
                        
                        for(int l = 0; l < blockSize; l++){
                            instructionCache[numOne + k][l + 2] = cacheTwo[num + j][l + 2];
                        }

                    } else {
                        k = cores[cid].bringCacheTwo(instructionCache, cacheTwo, numOne, num + j, blockSize, associativityOne);
                    }
    
                    instructionCache[numOne + k][0] = tagOne;
                    instructionCache[numOne + k][1] = 0;

                    break;
                }
            }

            if(!replaced){
                int j = 0;
                if(replacement == 'l'){
                    j = cores[cid].leastRecentlyUsed(cacheTwo, num, associativityTwo);
                } else {
                    j = cores[cid].notRecentlyUsed(cacheTwo, num, associativityTwo);
                }

                bringInstructions(num + j, tag);

                pc = cacheTwo[num + j][offset];
                cores[cid].if_id.push(program[pc]);

                int k = 0;
                if(fullOne){
                    if(replacement == 'l'){
                        k = cores[cid].leastRecentlyUsed(instructionCache, numOne, associativityOne);
                    } else {
                        k = cores[cid].notRecentlyUsed(instructionCache, numOne,associativityOne);
                    }
                    
                    for(int l = 0; l < blockSize; l++){
                        instructionCache[numOne + k][l + 2] = cacheTwo[num + j][l + 2];
                    }

                } else {
                    k = cores[cid].bringCacheTwo(instructionCache, cacheTwo, numOne, num + j, blockSize, associativityOne);
                }

                instructionCache[numOne + k][0] = tagOne;
                instructionCache[numOne + k][1] = 0;
            }
        }

        return;
    }

    void bringInstructions(int num, int tag){
        cacheTwo[num][0] = tag;
        cacheTwo[num][1] = 0;

        for(int i = 0; i < blockSize; i++){
            cacheTwo[num][i + 2] = tag;
            tag = tag + 1;
        }

        return;
    }

    void run(){

        for(int i = 0; i < 4; i++){
            for(int j = 0; j < 32; j++){
                cores[i].registers[j] = 0;
            }
        }

        scratchPadMem.resize(spmSize);
        int index = checkData();
        if(index != 0)  index++;

        runProgram(index);

        for(int i = 0; i < 4; i++){
            cores[i].printRegisters();
        }

        cout << endl;
        printCacheOne();
        cout << endl;
        printCacheTwo();
        cout << endl;
        printInstructionCache();
        cout << endl;
        printSPM();
        cout << endl;
        printMemory();
        cout << endl;
        
        for(int i = 0; i < 4; i++){
            cout << "Core " << i << endl;
            cout << "Clock cycles : " << cores[i].cycles << endl;
            cout << "Total Stalls : " << cores[i].stallCount << endl;
            cout << "Branch Stalls : " << cores[i].branchStallCount << endl;
            cout << "Cache Stalls : " << cores[i].cacheStallCount << endl;
            cout << "Arithmetic Stalls : " << cores[i].arithmeticStallCount << endl;
            cout << "Instruction Stalls : " << cores[i].instructionStallCount << endl;
            cout << "SPM Stalls : " << cores[i].spmStallCount << endl;
            cout << "Sync Stalls : " << cores[i].syncStallCount << endl;
            cout << "Other Stalls (RAW): " << cores[i].otherStallCount << endl << endl;
            cout << "Instructions : " << cores[i].instructionsCount << endl;
            float temp = (float)cores[i].instructionsCount / cores[i].cycles;
            cout << "IPC : " << temp << endl << endl;
            cout << "Memory Accesses : " << cores[i].memAccess << endl;
            cout << "SPM Access : " << cores[i].spmAccess << endl; 
            cout << "L1 Data Cache Hits : " << cores[i].cacheOneHit << "                    ";
            cout << "L1 Cache Misses : " << cores[i].cacheOneMiss << endl;
            cout << "L1 Instruction Cache Hits : " << cores[i].instructionCacheHit << "           ";
            cout << "L1 Instruction Cache Miss : " << cores[i].instructionCacheMiss << endl;
            cout << "L2 Cache Hits : " << cores[i].cacheTwoHit << "                         ";
            cout << "L2 Cache Misses : " << cores[i].cacheTwoMiss << endl << endl;
        }
    }

    void printCacheOne(){
        cout << "L1D Cache : " << endl;
        for(int i = 0; i < cacheOne.size(); i++){
            cout << "Tag : " << cacheOne[i][0] << " | " << "Rep : " << cacheOne[i][1] << " | ";
            for(int j = 2; j < blockSize + 2; j++){
                cout << cacheOne[i][j] << " ";
            }
            cout << endl;
        }
    }

    void printInstructionCache(){
        cout << "L1I Cache : " << endl;
        for(int i = 0; i < instructionCache.size(); i++){
            cout << "Tag : " << instructionCache[i][0] << " | " << "Rep : " << instructionCache[i][1] << " | ";
            for(int j = 2; j < blockSize + 2; j++){
                cout << instructionCache[i][j] << " ";
            }
            cout << endl;
        }
    }

    void printSPM(){
        cout << "SPM : ";
        for(int i = 0; i < scratchPadMem.size(); i++){
            if(i % 5 == 0)  cout << endl;
            cout << "Address " << 4*i << " : " << scratchPadMem[i] << " | ";
            
        } 
        cout << endl;
    }

    void printCacheTwo(){
        cout << "L2D Cache : " << endl;
        for(int i = 0; i < cacheTwo.size(); i++){
            cout << "Tag : " << cacheTwo[i][0] << " | " << "Rep : " << cacheTwo[i][1] << " | ";
            for(int j = 2; j < blockSize + 2; j++){
                cout << cacheTwo[i][j] << " ";
            }
            cout << endl;
        }
    }

    void printMemory(){
        cout << "Memory : ";
        for(int i = 0; i < 1024; i++){
            if(i % 5 == 0)  cout << endl;
            cout << "Address " << 4*i << " : " << memory[i] << " | ";
            
        } 
        cout << endl;

        return;
    }

};