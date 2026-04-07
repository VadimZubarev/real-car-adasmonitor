#include <iostream>
#include <obd_parser.h>

int main() {
    OBDParser parser;
    
    try {
        parser.parseCSVFile("../data/data.csv");
        int slow, normal, aggressive, unknown;
        int n = 5;

        for (int i = 0; i < n; i++) {
            parser.printRecord(i);
        }

        parser.getStyleStatistics(n, slow, normal, aggressive, unknown);
        
        std::cout << "SLOW: " << slow << std::endl;
        std::cout << "NORMAL: " << normal << std::endl;
        std::cout << "AGGRESSIVE: " << aggressive << std::endl;
        std::cout << "UNKNOWN: " << unknown << std::endl;
        
        // parser.printAllRecords();        
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}