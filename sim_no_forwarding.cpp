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

    vector<pair<string, int>> vecStall; 
    queue<tuple<string, int, int, string>> branch;

    mutex m_if_id, m_id_ex, m_ex_mem, m_mem_wb, m_stall, m_branch_stall, m_registers, m_memory, m_program, m_pc;
    condition_variable c_if_id, c_id_ex, c_ex_mem, c_mem_wb, c_wb_if, c_stall, c_branch_stall;

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
    int cycles = 0;

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
    
            if(opcode == "add" || opcode == "sub"){
                    string RD, RS1, RS2;
                    ss >> RD >> RS1 >> RS2;
                    rd = stoi(RD.substr(1));
                    rs1 = stoi(RS1.substr(1));
                    rs2 = stoi(RS2.substr(1));
            }
    
                if(opcode == "lw"){
                    string RD, DATA;
                    ss >> RD >> DATA;
    
                    rd = stoi(RD.substr(1));
                    rs1 = stoi(DATA.substr(0, DATA.find('(')));
                    rs2 = stoi(DATA.substr(DATA.find('(') + 2, DATA.find(')') - DATA.find('(') - 2));

                    rs1 = rs2;
                }
    
                if(opcode == "sw"){
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
                    if((rd_one == rs1 || rd_one == rs2) && opcode_one != "sw"){
                        stallDuration = 1;
                        stallCount += 1;
                        cout << rd_one << endl;
                        return true;
                    }
                }

                if(!mem_wb.empty()){
                    auto [opcode_two, rd_two, result_two] = mem_wb.front();
                    if((rd_two == rs1 || rd_two == rs2) && opcode_two != "sw"){
                        stallDuration = 2;
                        stallCount += 2;
                        cout << rd_two << endl;
                        return true;
                    }
                }

                if(!ex_mem.empty()){
                    auto [opcode_three, rd_three, mem_three] = ex_mem.front();
                    if((rd_three == rs1 || rd_three == rs2) && opcode_three != "sw"){
                        stallDuration = 3;
                        stallCount += 3;
                        cout << rd_three << endl;
                        return true;
                    }
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
            cout << "WB (" << opcode << ")" << endl;
    
            if(opcode == "sw"){
                memory[value] = rd;
                cout << value << memory[value] << endl;
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
            cout << "MEM (" << opcode << ")" << endl;
    
            int mem = result;
            if(opcode == "lw"){
                mem = memory[result / 4];
                cout << (result/4) << memory[result/4] << endl;
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
                cout << "EX (Branch) (" << label << ") count = " << count << endl;

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

                

                cout << branchTaken << " Flushed IF & ID, restarted at PC = " << count << endl;
    
                branch.pop();  // Remove only once
                return;  // Exit function instead of looping again            
            }
            
    
            auto [opcode, rd, r_rs1, r_rs2] = id_ex.front();
            
            cout << "EX (" << opcode << ")" << endl;
            
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
                cout << "Latencyyy = " << latencyStall << endl;
                return;
            }

            cout << "Latency = 0" << endl;
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
            if_id.pop();

            if(checkStall(instruction)){
                cout << "Stall" << endl;
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
            cout << "ID (" << opcode << ")" << endl;
    
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
        
                    unique_lock<mutex> lock_registers(m_registers);
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
    int clock;
    vector<DisableCores> cores;
    vector<string> program;

    bool completed = false;
    unordered_map<string, int> labels;
    unordered_map<string, int> latency;

    DisableSimulator(){
        memory.resize(4096 / 4);
        clock = 0;
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

        cout << "IF" << endl;
        if(program[index].find(' ') == string::npos){
            cout << "label found" << endl;
            return;
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
                            _sleep(10);
                            continue;
                        }
                        cores[i].stall = false;
                        temp--;
                        cores[i].count--;
                    }

                    if(cores[i].branchStall){
                        cores[i].branchDuration--;
                        if(cores[i].branchDuration > 0){
                            _sleep(10);
                            continue;
                        }
                        cores[i].branchStall = false;
                        temp = cores[i].count;
                    }

                    instructionFetch(program, i, temp);
                    temp++;
                    cores[i].count++;
                    _sleep(10);
                    cout << "count = " << cores[i].count << endl;
                }
                cout << i << " finished" << endl;
                cores[i].printRegisters();
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
        cout << "Clock cycles : " << clock << endl;    
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