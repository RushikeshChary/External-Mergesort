#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <queue>
#include <string>
using namespace std;


template<typename T1, typename T2> // cin >> pair<T1, T2>
istream& operator>>(istream &istream, pair<T1, T2> &p) { return (istream >> p.first >> p.second); }
template<typename T> // cin >> vector<T>
istream& operator>>(istream &istream, vector<T> &v){for (auto &it : v)cin >> it;return istream;}
template<typename T1, typename T2> // cout << pair<T1, T2>
ostream& operator<<(ostream &ostream, const pair<T1, T2> &p) { return (ostream << p.first << ' ' << p.second); }
template<typename T> // cout << vector<T>
ostream& operator<<(ostream &ostream, const vector<T> &c) { for (auto &it : c) cout << it << ' '; return ostream; }

// Structure to represent a block of data
struct Block {
    vector<int> data; // Data in the block
};

int totalSeeks = 0;
int totalTransfers = 0;

// Helper function to read a block of data from the input file
Block readBlock(ifstream &input, int blockSize) {
    Block block;
    int value;
    for (int i = 0; i < blockSize; ++i) {
        if(input >> value)
            block.data.push_back(value);
        else break;
    }
    if (!block.data.empty()) {
        totalTransfers ++;
    }
    return block;
}

// Helper function to write a block of data to a temporary file
void writeBlock(const Block &block, ofstream &output) {
    
    for (int val : block.data) {
        output << val << "\n";
    }
    if (!block.data.empty()) {
        totalTransfers ++;
    }
}

// Initial phase: Create sorted runs
vector<string> createInitialRuns(const string &inputFile, int blockSize, int memoryBlocks) {
    ifstream input(inputFile);
    vector<string> runFiles;
    int runCount = 0;

    while (!input.eof()) {
        
        vector<Block> blocks;
        totalSeeks++; // One seek per read.
        for(int i = 0;i < memoryBlocks;i++)
        {
            Block block;
            block = readBlock(input,blockSize);
            blocks.push_back(block);
        }

        // Sort the blocks

        for(Block& b : blocks)
        {
            // cout<<b.data<<endl;
            sort(b.data.begin(),b.data.end());
            // cout<<b.data<<endl;
        }

        // Write to a new temporary file
        string runFile = "run_" + to_string(runCount++) + ".txt";
        ofstream output(runFile);
        totalSeeks++; // One seek per write.
        for(Block& b : blocks)
        {
            writeBlock(b, output);
        }
        runFiles.push_back(runFile);
        output.close();
    }
    cout<<"After initial sorted run phase:\n";
    cout << "Total Disk Seeks: " << totalSeeks << ", Total Disk Transfers: " << totalTransfers << "\n";
    input.close();
    return runFiles;
}

// Merge phase: Perform (m-1)-way merge and produce intermediate runs


int main(int argc, char *argv[]) {
    if (argc != 6) {
        cerr << "Usage: ./program input-file n k b m\n";
        return 1;
    }

    string inputFile = argv[1];
    int n = stoi(argv[2]);  // Total number of keys
    int k = stoi(argv[3]);  // Size of each key in bytes
    int b = stoi(argv[4]);  // Disk block size in bytes
    int m = stoi(argv[5]);  // Available memory in blocks

    cout << "Running external merge sort on " << inputFile << " with parameters:\n";
    cout << "n = " << n << ", k = " << k << ", b = " << b << ", m = " << m << "\n";

    // Step 1: Create initial sorted runs
    vector<string> runFiles = createInitialRuns(inputFile, b / k, m);
    cout << "Created " << runFiles.size() << " initial runs.\n";

    // Step 2: Merge runs in (m-1) way
    // vector<string> finalRuns = mergeRuns(runFiles, b / k, m);

    // Final merged output
    // for (string run_file : finalRuns) {
    //     cout << "Merging completed. Final output written to: " << run_file << "\n";
    // }

    cout << "Total Disk Seeks: " << totalSeeks << ", Total Disk Transfers: " << totalTransfers << "\n";

    return 0;
}
