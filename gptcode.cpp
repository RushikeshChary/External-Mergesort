#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <queue>
#include <string>
using namespace std;

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
    // if (!block.data.empty()) {
    //     totalTransfers += 2; // One read and one write per block
    // }
    return block;
}

// Helper function to write a block of data to a temporary file
void writeBlock(const Block &block, const string &filename) {
    ofstream output(filename);
    for (int val : block.data) {
        output << val << "\n";
    }
    // if (!block.data.empty()) {
    //     totalTransfers += 2; // One read and one write per block
    // }
}

// Initial phase: Create sorted runs
vector<string> createInitialRuns(const string &inputFile, int blockSize, int memoryBlocks) {
    ifstream input(inputFile);
    vector<string> runFiles;
    int runCount = 0;

    while (!input.eof()) {
        Block block = readBlock(input, blockSize * memoryBlocks);
        for(int i = 0;i < memoryBlocks;i++)
        {
            vector<int> disk_block = readBlock(input,blockSize).data;
            totalTransfers++;
            block.data.insert(block.data.end(),disk_block.begin(),disk_block.end());
        }
        totalSeeks++; // One seek per run
        if (block.data.empty()) break;

        // Sort the block
        sort(block.data.begin(), block.data.end());

        // Write to a new temporary file
        string runFile = "run_" + to_string(runCount++) + ".txt";
        writeBlock(block, runFile);
        runFiles.push_back(runFile);
    }

    input.close();
    return runFiles;
}

// Merge phase: Perform (m-1)-way merge and produce intermediate runs
vector<string> mergeRuns(const vector<string> &runFiles, int blockSize, int memoryBlocks) {
    int mergeFactor = memoryBlocks - 1; // m-1 way merge
    vector<string> newRunFiles;
    int pass = 0;

    vector<string> currentRuns = runFiles;

    while (currentRuns.size() >= memoryBlocks) {
        int runCount = 0;

        for (size_t i = 0; i < currentRuns.size(); i += mergeFactor) {
            // Determine the subset of runs to merge
            vector<string> subset(currentRuns.begin() + i, currentRuns.begin() + min(i + mergeFactor, currentRuns.size()));

            // Output file for the merged result
            string mergedFile = "pass_" + to_string(pass) + "_run_" + to_string(runCount++) + ".txt";
            newRunFiles.push_back(mergedFile);

            ofstream output(mergedFile);

            // Priority queue to merge (min-heap)
            auto cmp = [](pair<int, int> a, pair<int, int> b) { return a.first > b.first; };
            priority_queue<pair<int, int>, vector<pair<int, int>>, decltype(cmp)> minHeap(cmp);

            vector<ifstream> inputs;
            vector<int> buffers(subset.size());

            // Open all selected run files
            for (const string &file : subset) {
                ifstream input(file);
                if (input.is_open()) {
                    inputs.push_back(move(input));
                }
            }

            // Initialize the heap with the first element of each run
            for (int j = 0; j < inputs.size(); ++j) {
                if (inputs[j] >> buffers[j]) {
                    minHeap.emplace(buffers[j], j);
                    totalTransfers += 2; // Read and write per block
                    totalSeeks++;       // Seek for each block
                }
            }

            // Perform the merge
            while (!minHeap.empty()) {
                auto [value, idx] = minHeap.top();
                minHeap.pop();

                // Write the smallest value to the output file
                output << value << "\n";
                totalTransfers += 2; // Read and write per block
                totalSeeks++;       // Seek for each block

                // Read the next value from the corresponding run
                if (inputs[idx] >> buffers[idx]) {
                    minHeap.emplace(buffers[idx], idx);
                    totalTransfers += 2; // Read and write per block
                    totalSeeks++;       // Seek for each block
                }
            }

            // Close all files
            output.close();
            for (auto &input : inputs) input.close();
        }

        // Add any remaining runs to newRunFiles
        for (size_t j = (currentRuns.size() / mergeFactor) * mergeFactor; j < currentRuns.size(); ++j) {
            newRunFiles.push_back(currentRuns[j]);
        }

        currentRuns = newRunFiles;
        newRunFiles.clear();
        pass++;

        cout << "Pass " << pass << ": Total Seeks = " << totalSeeks << ", Total Transfers = " << totalTransfers << "\n";
    }

    // Add the last set of runs in the correct order
    newRunFiles.insert(newRunFiles.end(), currentRuns.begin(), currentRuns.end());

    return newRunFiles;
}

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
    vector<string> finalRuns = mergeRuns(runFiles, b / k, m);

    // Final merged output
    for (string run_file : finalRuns) {
        cout << "Merging completed. Final output written to: " << run_file << "\n";
    }

    cout << "Total Disk Seeks: " << totalSeeks << ", Total Disk Transfers: " << totalTransfers << "\n";

    return 0;
}
