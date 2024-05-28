#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include "trader.h"
#include "stock.h"
#include <mpi.h>
#include <omp.h>

void readTradersFromFile(const std::string& filename, Trader* traders) {

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return; // Exit if file cannot be opened
    }
    std::string line;
    int local_trader_count = 0;
    getline(file, line); // Skip the header line
    
    while (getline(file, line)) {
        
        std::stringstream linestream(line);
        int trader_id;
        double pnl, risk_tolerance;
        char delim; // Variable to consume the commas
        linestream >> trader_id >> delim >> pnl >> delim >> risk_tolerance;
        if (!linestream.fail()) {
            traders[local_trader_count++] = Trader(trader_id, pnl, risk_tolerance);
            // traders[local_trader_count].setPnl(pnl);
            // traders[local_trader_count].setRiskTolerance(risk_tolerance);
            // local_trader_count++;
        } 
    }
}

