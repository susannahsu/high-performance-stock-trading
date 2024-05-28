#include "portfolio.h"
#include <iostream>
#include <mpi.h>
#include <omp.h>

Portfolio::Portfolio(int id, double pnl, double risk_tolerance)
    : trader_id(id), pnl(pnl), risk_tolerance(risk_tolerance) {}

int Portfolio::getTraderId() const {
    return trader_id;
}

double Portfolio::getPnL() const {
    return pnl;
}

double Portfolio::getRiskTolerance() const {
    return risk_tolerance;
}

int Portfolio::getShares(int stock_id) const {
    auto it = stockHoldings.find(stock_id);
    if (it != stockHoldings.end()) {
        return it->second;
    }
    return 0; // Return 0 if the stock_id is not found
}

void Portfolio::assignShares(int stock_id, int shares) {
    stockHoldings[stock_id] += shares; // Increment shares for flexibility
}

void Portfolio::adjustPnL(double amount) {
    pnl += amount;
}

void Portfolio::setPnL(double amount) {
    pnl = amount;
}

void Portfolio::printPortfolio() const {
    std::cout << "Trader ID: " << trader_id << ", PNL: " << pnl << ", Risk Tolerance: " << risk_tolerance << std::endl;
    for (const auto& pair : stockHoldings) {
        std::cout << "\tStock ID: " << pair.first << ", Shares: " << pair.second << std::endl;
    }
}
