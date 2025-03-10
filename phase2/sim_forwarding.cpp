#include <bits/stdc++.h>
#include <sstream>
#include <ctime>
using namespace std;

class EnableCores{
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

    int count = 0;
    int latencyStall = 0;
    int stallDuration = 0;
    int branchDuration = 0;
    int stallCount = 0;
    int instructionsCount = 0;
    int cycles = -1;

    EnableCores(int cid){
        registers.resize(32, 0);
        pc = 0;
        coreID = cid;
        registers[0] = 0;
        registers[31] = cid;
    }

    EnableCores(const EnableCores&) = delete;
    EnableCores& operator=(const EnableCores&) = delete;

    EnableCores(EnableCores&& other) noexcept
        : coreID(other.coreID), registers(std::move(other.registers)), pc(other.pc) {}

    EnableCores& operator=(EnableCores&& other) noexcept {
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
            string label;
    
            ss >> opcode;
    
            if(opcode == "add" || opcode == "sub" || opcode == "mul" || opcode == "div"){
                    string RD, RS1, RS2;
                    ss >> RD >> RS1 >> RS2;
                    rd = stoi(RD.substr(1));
                    rs1 = stoi(RS1.substr(1));
                    rs2 = stoi(RS2.substr(1));

                    r_rs1 = registers[rs1];
                    r_rs2 = registers[rs2];
            }
    
                if(opcode == "lw"){
                    string RD, DATA;
                    ss >> RD >> DATA;
    
                    rd = stoi(RD.substr(1));
                    rs1 = stoi(DATA.substr(0, DATA.find('(')));
                    rs2 = stoi(DATA.substr(DATA.find('(') + 2, DATA.find(')') - DATA.find('(') - 2));
                    
                    r_rs1 = rs1;
                    r_rs2 = registers[rs2];

                    rs1 = rs2;
                }
    
                if(opcode == "sw"){
                    string RD, DATA;
                    ss >> RD >> DATA;
        
                    rd = stoi(RD.substr(1));
                    rs1 = stoi(DATA.substr(0, DATA.find('(')));
                    rs2 = stoi(DATA.substr(DATA.find('(') + 2, DATA.find(')') - DATA.find('(') - 2));

                    r_rs1 = rs1;
                    r_rs2 = registers[rs2];

                    rs1 = rd;
                    rs2 = rd;

                    rd = registers[rd];
                }
        
                if(opcode == "bne" || opcode == "beq" || opcode == "bge" || opcode == "blt"){
                    string RS1, RS2, Label;
                    ss >> RS1 >> RS2 >> Label;

                    if(RS1 == "cid")    return false;
        
                    rs1 = stoi(RS1.substr(1));
                    rs2 = stoi(RS2.substr(1));

                    r_rs1 = registers[rs1];
                    r_rs2 = registers[rs2];
                    label = Label;
                }

                if(opcode == "addi" || opcode == "muli"){
                    string RD, RS, VAL;
                    ss >> RD >> RS >> VAL;

                    rd = stoi(RD.substr(1));
                    rs1 = stoi(RS.substr(1));
                    rs2 = rs1;

                    r_rs1 = registers[rs1];
                    r_rs2 = stoi(VAL);
                }

                if(!mem_wb.empty()){
                    auto [opcode_two, rd_two, result_two] = mem_wb.front();

                    if((rd_two == rs1 && rd_two == rs2) && opcode_two != "sw"){
                        if(opcode == "sw"){
                            id_ex.push({opcode, result_two, r_rs1, r_rs2});
                            stallDuration = 1;
                            stallCount += 1;
                            return true;
                        }

                        if(opcode == "lw"){
                            id_ex.push({opcode, rd, r_rs1, result_two});
                            stallDuration = 1;
                            stallCount += 1;
                            return true;
                        }

                        if(opcode == "bne" || opcode == "beq" || opcode == "blt" || opcode == "bge"){
                            branch.push({opcode, result_two, result_two, label});
                            branchStall = true;
                            branchDuration = 2;
                            stallCount += 2;
                            return true;
                        }

                        id_ex.push({opcode, rd, result_two, result_two});
                        stallDuration = 1;
                        stallCount += 1;
                        return true;
                    }
                    
                    if((rd_two == rs1) && opcode_two != "sw"){
                        if(opcode == "bne" || opcode == "beq" || opcode == "blt" || opcode == "bge"){
                            r_rs1 = result_two;
                            branchStall = true;
                            branchDuration = 2;
                            
                        } else{
                            id_ex.push({opcode, rd, result_two, r_rs2});
                            stallDuration = 1;
                            stallCount += 1;
                            return true;
                        }

                        
                    }

                    if((rd_two == rs2) && opcode_two != "sw"){
                        if(opcode == "bne" || opcode == "beq" || opcode == "blt" || opcode == "bge"){
                            r_rs2 = result_two;
                            branchStall = true;
                            branchDuration = 2;
                        } else{
                            id_ex.push({opcode, rd, r_rs1, result_two});
                            stallDuration = 1;
                            stallCount += 1;
                            return true;
                        }    
                    }
                }

                if(!ex_mem.empty()){
                    auto [opcode_three, rd_three, mem_three] = ex_mem.front();
                    if((rd_three == rs1 && rd_three == rs2) && (opcode_three != "sw")){
                        if(opcode_three == "lw"){
                            stallDuration = 2;
                            stallCount += 1;
                            return true;
                        }

                        if(opcode == "sw"){
                            id_ex.push({opcode, mem_three, r_rs1, r_rs2});
                            stallDuration = 1;
                            stallCount += 1;
                            return true;
                        }

                        if(opcode == "lw"){
                            id_ex.push({opcode, rd, r_rs1, mem_three});
                            stallDuration = 1;
                            stallCount += 1;
                            return true;
                        }

                        if(opcode == "bne" || opcode == "beq" || opcode == "blt" || opcode == "bge"){
                            branch.push({opcode, mem_three, mem_three, label});
                            branchStall = true;
                            branchDuration = 2;
                            stallCount += 2;
                            if_id.pop();
                            return true;
                        }

                        id_ex.push({opcode, rd, mem_three, mem_three});
                        stallDuration = 1;
                        stallCount += 1;
                        return true;
                    }

                    if((rd_three == rs1) && (opcode_three != "sw")){
                        if(opcode_three == "lw"){
                            stallDuration = 2;
                            stallCount += 1;
                            return true;
                        }

                        if(opcode == "bne" || opcode == "beq" || opcode == "blt" || opcode == "bge"){
                            r_rs1 = mem_three;
                            branchStall = true;
                            branchDuration = 2;
                        } else{
                            id_ex.push({opcode, rd, mem_three, r_rs2});
                            stallDuration = 1;
                            stallCount += 1;
                            return true;
                        }

                        
                    }

                    if((rd_three == rs2) && (opcode_three != "sw")){
                        if(opcode_three == "lw"){
                            stallDuration = 2;
                            stallCount += 1;
                            return true;
                        }
                        if(opcode == "bne" || opcode == "beq" || opcode == "blt" || opcode == "bge"){
                            r_rs2 = mem_three;
                            branchStall = true;
                            branchDuration = 2;    
                        } else{
                            id_ex.push({opcode, rd, r_rs1, mem_three});
                            stallDuration = 1;
                            stallCount += 1;
                            return true;
                        }    
                    }    
                }

                if(branchStall){
                    branch.push({opcode, r_rs1, r_rs2, label});
                    stallCount += 2;
                    if_id.pop();
                    return true;
                }

                return false;
    }

    void writeBack(vector<int>& memory){
        if(memCompleted){
            writeCompleted = true;
        }
        if(mem_wb.empty())  return;

            auto [opcode, rd, value] = mem_wb.front();
            mem_wb.pop();
    
            if(opcode == "sw"){
                memory[value] = rd;
            }
            else{
                registers[rd] = value;
            }
            wb_if.push({opcode, rd});
    }

    void memoryStage(vector<int>& memory){
        if(executeCompleted){
            memCompleted = true;
        }
        if(ex_mem.empty())  return;
            auto [opcode, rd, result] = ex_mem.front();
            ex_mem.pop();

            int mem = result;
            if(opcode == "lw"){
                mem = memory[result / 4];
            }
            if(opcode == "sw"){
                mem = result / 4;
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

            if(opcode == "lw"){
                result = r_rs1 + r_rs2;
            }
    
            else if(opcode == "sw"){
                result = r_rs1 + r_rs2;
            }

            else if(opcode == "la" || opcode == "li"){
                result = r_rs1;
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
    
            id_ex.pop();
            ex_mem.push({opcode, rd, result});
    }

    void instructionDecode(){
        if(fetchCompleted){
            decodeCompleted = true;
        }

        if(if_id.empty())   return;
            string instruction = if_id.front();
            

            if(checkStall(instruction)){
                stall = true;
                return;
            }

            if_id.pop();
    
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

                if(RS1 == "cid"){
                    cout << RS2 << endl;
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
    
            else if(opcode == "lw"){
                string RD, DATA;
                ss >> RD >> DATA;
    
                rd = stoi(RD.substr(1));
                rs1 = stoi(DATA.substr(0, DATA.find('(')));
                rs2 = stoi(DATA.substr(DATA.find('(') + 2, DATA.find(')') - DATA.find('(') - 2));
    
                r_rs1 = rs1;
                r_rs2 = registers[rs2];
            }
    
            else if(opcode == "sw"){
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

            else if(opcode == "print"){
                string RD, Label;
                ss >> RD >> Label;

                rd = stoi(RD.substr(1));
                rd = registers[rd];

                cout << Label << " " << rd << endl;
                writeCompleted = true;
                return;
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

class EnableSimulator{
    public:

    vector<int> memory;
    float clock;
    vector<EnableCores> cores;
    vector<string> program;

    bool completed = false;
    unordered_map<string, int> labels;
    unordered_map<string, int> latency;

    EnableSimulator(){
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

        if(cores[cid].stall){
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

        cores[cid].if_id.push(program[index]);

        return;    
    }

    void runProgram(int index){
            for(int i = 0; i < 4; i++){
                cout << "Core " << i << "Started" << endl;
                int temp = index;
                cores[i].count = index;
                cores[i].clearEverything();
                while(!cores[i].writeCompleted){
                    cores[i].cycles++;
                    cout << "Clock cycle " << cores[i].cycles << endl;
                    cores[i].writeBack(memory);
                    cores[i].memoryStage(memory);
                    cores[i].execute(program, labels, latency);

                    if(cores[i].latencyStall > 0){
                        continue;
                    }

                    cores[i].instructionDecode();

                    if(cores[i].stall){
                        cores[i].stallDuration--;
                        if(cores[i].stallDuration > 0){
                            _sleep(clock);
                            continue;
                        }
                        cores[i].stall = false;
                    }

                    if(cores[i].branchStall){
                        cores[i].branchDuration--;
                        if(cores[i].branchDuration > 0){
                            _sleep(clock);
                            continue;
                        }
                        cores[i].branchStall = false;
                        temp = cores[i].count;
                    }

                    instructionFetch(program, i, temp);
                    temp++;
                    cores[i].count++;
                    _sleep(clock);
                }
                cout << i << " finished" << endl;
            }
    }
        
    int checkData(){
        istringstream ss(program[0]);
        string opcode;

        ss >> opcode;
        
        if(opcode == ".data"){
            int index = 0;
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
            }
            return it;
        } else {
            return 0;
        }
    }

    void run(){

        for(int i = 0; i < 4; i++){
            for(int j = 0; j < 6; j++){
                cores[i].registers[j] = j;
            }
        }

        int index = checkData();
        if(index != 0)  index++;

        runProgram(index);

        for(int i = 0; i < 4; i++){
            cores[i].printRegisters();
        }

        printMemory();
        
        for(int i = 0; i < 4; i++){
            cout << "Core " << i << endl;
            cout << "Clock cycles : " << cores[i].cycles << endl;
            cout << "Stalls : " << cores[i].stallCount << endl;
            cout << "Instructions : " << cores[i].instructionsCount << endl;
            float temp = (float)cores[i].instructionsCount / cores[i].cycles;
            cout << "IPC : " << temp << endl << endl; 
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
