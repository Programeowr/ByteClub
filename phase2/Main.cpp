#include <bits/stdc++.h>
#include "sim_no_forwarding.cpp"
#include "sim_forwarding.cpp"

using namespace std;

int main(){
    char enable;

    int add, sub, mul, div;
    float time;
    cout << "Enable Forwarding? y (or) n" << endl;
    cin >> enable;

    cout << "Provide the length of 1 clock cycle (in ms)" << endl;
    cin >> time;
    cout << "Provide the latency for the below arithmetic instructions " << endl;
    cout << "add and addi : " << endl;
    cin >> add;

    cout << "sub : " << endl;
    cin >> sub;

    cout << "mul and muli : " << endl;
    cin >> mul;

    cout << "div : " << endl;
    cin >> div;

    ifstream file("ArrayAddition.txt"); // Open the file
    vector<string> lines;
    string line;

    if(file.is_open()){
        cout << "File opened" << endl; // Check if the file opened successfully
        while(getline(file, line)){
            lines.push_back(line);
        }
        file.close(); // Close the file
    } else{
        cerr << "Error: Could not open the file!" << endl;
    }

    if(enable == 'n'){
        DisableSimulator sim;
        sim.program = lines;
    
        for(int i = 0; i < sim.program.size(); i++){
            if(sim.program[i].find(' ') == string::npos){
                sim.labels[sim.program[i]] = i;
            }
        }

        sim.clock = time;
        sim.latency["add"] = add;
        sim.latency["addi"] = add;
        sim.latency["sub"] = sub;
        sim.latency["mul"] = mul;
        sim.latency["muli"] = mul;
        sim.latency["div"] = div;

        cout << endl;
        sim.run();

    }
    
    if(enable == 'y'){
        EnableSimulator sim;
        sim.program = lines;
    
        for(int i = 0; i < sim.program.size(); i++){
            if(sim.program[i].find(' ') == string::npos){
                sim.labels[sim.program[i]] = i;
            }
        }

        sim.clock = time;
        sim.latency["add"] = add;
        sim.latency["addi"] = add;
        sim.latency["sub"] = sub;
        sim.latency["mul"] = mul;
        sim.latency["muli"] = mul;
        sim.latency["div"] = div;

        cout << endl;
        sim.run();
    }
    
}