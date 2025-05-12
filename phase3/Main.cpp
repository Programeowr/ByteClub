#include <bits/stdc++.h>
#include "sim_no_forwarding.cpp"
#include "sim_forwarding.cpp"

using namespace std;

map<string, string> parseConfigFile(const string& configFileName) {
    map<string, string> config;
    ifstream configFile(configFileName);
    
    if (!configFile.is_open()) {
        cerr << "Error: Could not open configuration file: " << configFileName << endl;
        return config;
    }
    
    string line;
    while (getline(configFile, line)) {
        // Skip empty lines
        if (line.empty() || line.find_first_not_of(" \t\r\n") == string::npos) {
            continue;
        }
        
        // Find the equals sign
        size_t equalsPos = line.find('=');
        if (equalsPos != string::npos) {
            // Extract key and value
            string key = line.substr(0, equalsPos);
            string value = line.substr(equalsPos + 1);
            
            // Trim whitespace
            key.erase(0, key.find_first_not_of(" \t\r\n"));
            key.erase(key.find_last_not_of(" \t\r\n") + 1);
            
            value.erase(0, value.find_first_not_of(" \t\r\n"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);
            
            // Store in config map
            config[key] = value;
        }
    }
    
    configFile.close();
    return config;
}

// Function to get value from config with different types
template <typename T>
T getConfigValue(const map<string, string>& config, const string& key, const T& defaultValue) {
    auto it = config.find(key);
    if (it != config.end()) {
        stringstream ss(it->second);
        T value;
        if (ss >> value) {
            return value;
        }
    }
    return defaultValue;
}

// Specialization for char type (to handle 'y', 'n', etc.)
template <>
char getConfigValue<char>(const map<string, string>& config, const string& key, const char& defaultValue) {
    auto it = config.find(key);
    if (it != config.end() && !it->second.empty()) {
        return it->second[0]; // Return first character
    }
    return defaultValue;
}
 

int main(){
    char enable = 'Y';
    char replacement = 'l';
    int add = 1, sub = 1, mul = 2, div = 10;
    int cacheOneSize = 1024, cacheTwoSize = 4096, blockSize = 16;
    int associativityOne = 2, associativityTwo = 4;
    int instructionCacheSize = 1024, spmSize = 1024;
    int cacheOneLatency = 1, cacheTwoLatency = 5, memoryLatency = 100;
    
    // Parse configuration file
    map<string, string> config = parseConfigFile("config.txt");
    
    // Get values from config
    enable = getConfigValue(config, "enable", enable);
    add = getConfigValue(config, "add", add);
    sub = getConfigValue(config, "sub", sub);
    mul = getConfigValue(config, "mul", mul);
    div = getConfigValue(config, "div", div);
    cacheOneSize = getConfigValue(config, "cacheOneSize", cacheOneSize);
    instructionCacheSize = getConfigValue(config, "instructionCacheSize", instructionCacheSize);
    cacheTwoSize = getConfigValue(config, "cacheTwoSize", cacheTwoSize);
    spmSize = getConfigValue(config, "spmSize", spmSize);
    associativityOne = getConfigValue(config, "associativityOne", associativityOne);
    associativityTwo = getConfigValue(config, "associativityTwo", associativityTwo);
    blockSize = getConfigValue(config, "blockSize", blockSize);
    cacheOneLatency = getConfigValue(config, "cacheOneLatency", cacheOneLatency);
    cacheTwoLatency = getConfigValue(config, "cacheTwoLatency", cacheTwoLatency);
    memoryLatency = getConfigValue(config, "memoryLatency", memoryLatency);
    replacement = getConfigValue(config, "replacement", replacement);

    ifstream file("TestCase1.txt"); // Open the file
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

        sim.latency["add"] = add;
        sim.latency["addi"] = add;
        sim.latency["sub"] = sub;
        sim.latency["mul"] = mul;
        sim.latency["muli"] = mul; 
        sim.latency["div"] = div;

        sim.cacheOneSize = cacheOneSize / blockSize;
        sim.cacheTwoSize = cacheTwoSize / blockSize;
        sim.spmSize = spmSize / 4;
        sim.associativityOne = associativityOne;
        sim.associativityTwo = associativityTwo;
        sim.blockSize = blockSize / 4;
        sim.cacheOneLatency = cacheOneLatency;
        sim.cacheTwoLatency = cacheTwoLatency;
        sim.memoryLatency = memoryLatency;
        sim.replacement = replacement;
        sim.instructionCacheSize = instructionCacheSize / blockSize;

        cout << endl;
        sim.run();

    }
    
    if(enable == 'y'){
        EnableSimulator sim;
        cout << "Simulator Enabled" << endl;
        sim.program = lines;
    
        for(int i = 0; i < sim.program.size(); i++){
            if(sim.program[i].find(' ') == string::npos){
                sim.labels[sim.program[i]] = i;
            }
        }

        sim.latency["add"] = add;
        sim.latency["addi"] = add;
        sim.latency["sub"] = sub;
        sim.latency["mul"] = mul;
        sim.latency["muli"] = mul;
        sim.latency["div"] = div;

        sim.cacheOneSize = cacheOneSize / blockSize;
        sim.cacheTwoSize = cacheTwoSize / blockSize;
        sim.spmSize = spmSize / 4;
        sim.associativityOne = associativityOne;
        sim.associativityTwo = associativityTwo;
        sim.blockSize = blockSize / 4;
        sim.cacheOneLatency = cacheOneLatency;
        sim.cacheTwoLatency = cacheTwoLatency;
        sim.memoryLatency = memoryLatency;
        sim.replacement = replacement;
        sim.instructionCacheSize = instructionCacheSize / blockSize;

        cout << endl;
        sim.run();
    }
    
}