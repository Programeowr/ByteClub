#include <bits/stdc++.h>
#include "sim_no_forwarding.cpp"
#include "sim_forwarding.cpp"

using namespace std;

int main(){
    char enable;

    cout << "Enable Forwarding? y (or) n" << endl;
    cin >> enable;

    ifstream file("bubbleSort.txt"); // Open the filey
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

        cout << endl;
        sim.run();
    }
    
}