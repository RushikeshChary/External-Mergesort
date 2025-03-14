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
    size_t index = 0;

    bool empty() const
    {
        return index >= data.size();
    }
    void reset()
    {
        data.clear();
        index = 0;
    }
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

        // Read blocks into memory
        for (int i = 0; i < memoryBlocks; i++) {
            Block block = readBlock(input, blockSize);
            if (block.data.empty()) break; // Handle end of file
            blocks.push_back(block);
        }

        // Collect all data from blocks into a single vector
        vector<int> allData;
        for (const Block &b : blocks) {
            allData.insert(allData.end(), b.data.begin(), b.data.end());
        }

        // Global sort across all blocks
        sort(allData.begin(), allData.end());

        // Write sorted data to a new temporary file
        string runFile = "run_" + to_string(runCount++) + ".txt";
        ofstream output(runFile);
        totalSeeks++; // One seek per write.

        // Split the sorted data back into blocks and write
        for (size_t i = 0; i < allData.size(); i += blockSize) {
            Block block;
            block.data.assign(allData.begin() + i,
                              allData.begin() + min(i + blockSize, allData.size()));
            writeBlock(block, output);
        }

        runFiles.push_back(runFile);
        output.close();
    }

    cout << "After initial sorted run phase:\n";
    cout << "Total Disk Seeks: " << totalSeeks << ", Total Disk Transfers: " << totalTransfers << "\n";
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

            vector<ifstream> inputs(subset.size());
            vector<Block> buffers(subset.size());

            // Open all selected run files and load the first block
            for (int j = 0; j < subset.size(); ++j) {
                inputs[j].open(subset[j]);
                if (inputs[j].is_open()) {
                    buffers[j] = readBlock(inputs[j], blockSize);
                    totalSeeks++;
                    if (!buffers[j].empty()) {
                        minHeap.emplace(buffers[j].data[buffers[j].index++], j);
                    }
                }
            }

            // Output buffer to store merged data
            Block outputBuffer;

            // Perform the merge
            while (!minHeap.empty()) {
                auto [value, idx] = minHeap.top();
                minHeap.pop();

                // Store the smallest value in the output buffer
                outputBuffer.data.push_back(value);

                // Write the output buffer if it's full
                if (outputBuffer.data.size() == blockSize) {
                    writeBlock(outputBuffer,output);
                    totalSeeks++;
                    outputBuffer.reset();
                }

                // Check if the current block is exhausted, load the next block if available
                if (buffers[idx].empty() && !inputs[idx].eof()) {
                    buffers[idx] = readBlock(inputs[idx], blockSize);
                    totalSeeks++;
                }

                // If there are still elements in the block, add the next value to the heap
                if (!buffers[idx].empty()) {
                    minHeap.emplace(buffers[idx].data[buffers[idx].index++], idx);
                }
            }

            // Write any remaining data in the output buffer
            if (!outputBuffer.empty()) {
                writeBlock(outputBuffer,output);
                totalSeeks++;
                outputBuffer.reset();
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
    //concatenate these files to get final output.
    // string finalOutputFile = "sorted_" + inputFile;
    // ofstream output(finalOutputFile);
    // for (string run_file : finalRuns) {
    //     ifstream input(run_file);
    //     string line;
    //     while (getline(input, line)) {
    //         output << line << "\n";
    //     }
    //     input.close();
    //     remove(run_file.c_str());
    // }
    // output.close();

    cout << "Total Disk Seeks: " << totalSeeks << ", Total Disk Transfers: " << totalTransfers << "\n";

    return 0;
}
