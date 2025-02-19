#include <bits/stdc++.h>
#include <sstream>
using namespace std;

class Cores{
    public:

    vector<int> registers;
    int pc;
    int coreID;
    unordered_map<string, pair<int,int>> mp;

    Cores(int cid){
        registers.resize(32, 0);
        pc = 0;
        coreID = cid;
        registers[0] = 0;
    }
    
    void execute(vector<string>& program, vector<int>& memory, int& clock){
        for(int i = 0; i < program.size(); i++){
            vector<string> words;
            istringstream ss(program[i]);
            string word;

            while(ss >> word){
                words.push_back(word);
                cout << word << " ";
            }
            cout << endl;

            string opcode = words[0];

            if(opcode == ".data"){
                i = executeData(opcode, i+1, program, memory);
            }

            if(opcode == "add"){
                int rd = stoi(words[1].substr(1));
                int rs1 = stoi(words[2].substr(1));
                int rs2 = stoi(words[3].substr(1));
                
                if(rd == 0){
                    cout << "Error : You can't rewrite x0" << endl;
                    return;
                }

                registers[rd] = registers[rs1] + registers[rs2];
            }

            else if(opcode == "sub"){
                int rd = stoi(words[1].substr(1));
                int rs1 = stoi(words[2].substr(1));
                int rs2 = stoi(words[3].substr(1));
                
                if(rd == 0){
                    cout << "Error : You can't rewrite x0" << endl;
                    return;
                }

                registers[rd] = registers[rs1] - registers[rs2];
            }

            else if(opcode == "mul"){
                int rd = stoi(words[1].substr(1));
                int rs1 = stoi(words[2].substr(1));
                int rs2 = stoi(words[3].substr(1));
                
                if(rd == 0){
                    cout << "Error : You can't rewrite x0" << endl;
                    return;
                }

                registers[rd] = registers[rs1] * registers[rs2];
            }

            else if(opcode == "div"){
                int rd = stoi(words[1].substr(1));
                int rs1 = stoi(words[2].substr(1));
                int rs2 = stoi(words[3].substr(1));
                
                if(rd == 0){
                    cout << "Error : You can't rewrite x0" << endl;
                    return;
                }

                registers[rd] = registers[rs1] / registers[rs2];
            }

            else if(opcode == "addi"){
                int rd = stoi(words[1].substr(1));
                int rs = stoi(words[2].substr(1));
                int val = stoi(words[3]);
                
                if(rd == 0){
                    cout << "Error : You can't rewrite x0" << endl;
                    return;
                }
                
                registers[rd] = registers[rs] + val;
            }

            else if(opcode == "muli"){
                int rd = stoi(words[1].substr(1));
                int rs = stoi(words[2].substr(1));
                int val = stoi(words[3]);
                
                if(rd == 0){
                    cout << "Error : You can't rewrite x0" << endl;
                    return;
                }
                
                registers[rd] = registers[rs] * val;
            }

            else if(opcode == "la"){
                int rd = stoi(words[1].substr(1));
                string variable = words[2];

                registers[rd] = mp[variable].first * 4;
            }

            else if(opcode == "lw"){
                int rd = stoi(words[1].substr(1));
                int offset = stoi(words[2].substr(0, words[2].find('(')));
                int rs = stoi(words[2].substr(words[2].find('(') + 2, words[2].find(')') - words[2].find('(') - 2));
                
                if(rd == 0){
                    cout << "Error : You can't rewrite x0" << endl;
                    return;
                }
                
                int off = (registers[rs] + offset);
                int address = off / 4 + 256 * coreID;
                registers[rd] = memory[address];
            }

            else if(opcode == "sw"){
                int rs = stoi(words[1].substr(1));
                int offset = stoi(words[2].substr(0, words[2].find('(')));
                int rd = stoi(words[2].substr(words[2].find('(') + 2, words[2].find(')') - words[2].find('(') - 2));

                int off = registers[rd] + offset;
                int address = off / 4 + 256 * coreID;
                memory[address] = registers[rs];
            }

            else if(opcode == "bne"){
                int rs1 = stoi(words[1].substr(1));
                int rs2 = stoi(words[2].substr(1));
                string label = words[3] + ":";

                if(registers[rs1] != registers[rs2]){
                    int temp = find(program.begin(), program.end(), label) - program.begin();
                    i = temp;
                    pc = 4*i;
                }
            }

            else if(opcode == "beq"){
                int rs1 = stoi(words[1].substr(1));
                int rs2 = stoi(words[2].substr(1));
                string label = words[3] + ":";

                if(registers[rs1] == registers[rs2]){
                    int temp = find(program.begin(), program.end(), label) - program.begin();
                    i = temp;
                    pc = 4*i;
                }
            }

            else if(opcode == "bge"){
                int rs1 = stoi(words[1].substr(1));
                int rs2 = stoi(words[2].substr(1));
                string label = words[3] + ":";

                if(registers[rs1] >= registers[rs2]){
                    int temp = find(program.begin(), program.end(), label) - program.begin();
                    i = temp;
                    pc = 4*i;
                }
            }

            else if(opcode == "ble"){
                int rs1 = stoi(words[1].substr(1));
                int rs2 = stoi(words[2].substr(1));
                string label = words[3] + ":";

                if(registers[rs1] <= registers[rs2]){
                    int temp = find(program.begin(), program.end(), label) - program.begin();
                    i = temp;
                    pc = 4*i;
                }
            }

            else if(opcode == "jal"){
                int rl = stoi(words[1].substr(1));
                string label = words[2] + ":";
                
                if(rl == 0){
                    cout << "Error : You can't rewrite x0" << endl;
                    return;
                }
                

                registers[rl] = pc;
                int temp = find(program.begin(), program.end(), label) - program.begin();
                i = temp;
                pc = 4*i;
            }

            else if(opcode == "jalr"){
                int rl = stoi(words[1].substr(1));
                int offset = stoi(words[2].substr(0, words[2].find('(')));
                int rj = stoi(words[2].substr(words[2].find('(') + 2, words[2].find(')') - words[2].find('(') - 2));
                
                if(rl == 0){
                    cout << "Error : You can't rewrite x0" << endl;
                    return;
                }
                
                
                int address = offset + rj;
                registers[rl] = pc;
                i = registers[address];
                pc = 4*i; 
            }

            else if(opcode == "j"){
                string label = words[1] + ":";

                int temp = find(program.begin(), program.end(), label) - program.begin();
                i = temp;
                pc = 4*i;
            }

            else if(opcode == "jr"){
                int r = stoi(words[1].substr(1));

                i = registers[r];
                pc = 4*i;
            }

            else if(opcode == "li"){
                int rd = stoi(words[1].substr(1));
                int val = stoi(words[2]);
                
                if(rd == 0){
                    cout << "Error : You can't rewrite x0" << endl;
                    return;
                }
                
                registers[rd] = val;
            }

            printRegisters(opcode, memory);
            pc += 4;
            clock++;
        }
    }

    int executeData(string opcode, int i, vector<string>& program, vector<int>& memory){
        int index = 0;
        int it;
        for(it = i; it != find(program.begin(), program.end(), ".text") - program.begin(); it++){
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
                    memory[index + 256*coreID] = val;
                    index++;
                }
            }

            mp[label] = make_pair(temp, index);
        }
        return it;
    }

    void printRegisters(string opcode, vector<int>& memory){
        cout << "Core - " << coreID << " : " << opcode << endl;
        for(int i = 0; i < 32; i++){
            cout << registers[i] << " ";
        }
        cout << endl;

        return;
    }

    void printMemory(vector<int>& memory){
        cout << "Core : " << coreID << " : Memory : " << endl;
        for(int i = 0; i < 256; i++){
            cout << memory[i + 256*coreID] << " ";
        } 
        cout << endl;

        return;
    }
};

class Simulator{
    public:

    vector<int> memory;
    int clock;
    vector<Cores> cores;
    vector<string> program;

    Simulator(){
        memory.resize(4096 / 4);
        clock = 0;
        cores = {Cores(0), Cores(1), Cores(2), Cores(3)};
    }

    void run(){
            for(int i = 0; i < 4; i++){
                cores[i].execute(program, memory, clock);
                cores[i].printMemory(memory);
            }
            cout << "Clock cycles : " << clock << endl;    
    }

};

int main(){

    Simulator sim;

    ifstream file("bubbleSort.txt"); // Open the file
    vector<string> lines;
    string line;

    if(file.is_open()){
        cout << "File opened" << endl; // Check if the file opened successfully
        while(getline(file, line)){
            lines.push_back(line);
            cout << line << endl; // Store each line in the vector
        }
        file.close(); // Close the file
    } else{
        cerr << "Error: Could not open the file!" << endl;
    }

    sim.program = lines;
    
    cout << endl;
    sim.run();

    
}