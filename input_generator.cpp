#include <iostream>
#include <fstream>
#include <cstdlib>  // for rand() and srand()
#include <ctime>    // for time()

int main() {
    const std::string filename = "random_integers.txt";
    const int numberOfIntegers = 100; // change this to generate more or fewer integers
    const int minValue = 1;
    const int maxValue = 1000;

    std::ofstream outFile(filename);
    
    if (!outFile) {
        std::cerr << "Error opening file for writing!" << std::endl;
        return 1;
    }

    std::srand(std::time(nullptr)); // seed the random number generator

    for (int i = 0; i < numberOfIntegers; ++i) {
        int randomNumber = minValue + std::rand() % (maxValue - minValue + 1);
        outFile << randomNumber << std::endl;
    }

    outFile.close();
    std::cout << "File '" << filename << "' generated with " 
              << numberOfIntegers << " random integers." << std::endl;

    return 0;
}
