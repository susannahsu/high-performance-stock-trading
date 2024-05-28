#ifndef PORTFOLIO_H
#define PORTFOLIO_H

#include <map>
#include <vector>

class Portfolio {
private:
    int trader_id;
    double pnl;
    double risk_tolerance;
    std::map<int, int> stockHoldings; // Key: stock_id, Value: quantity owned

public:
    Portfolio() : trader_id(0), pnl(0.0), risk_tolerance(0.0) {}
    Portfolio(int id, double pnl, double risk_tolerance);

    // Accessors
    int getTraderId() const;
    double getPnL() const;
    double getRiskTolerance() const;
    int getShares(int stock_id) const;

    // Mutators
    void assignShares(int stock_id, int shares);
    void adjustPnL(double amount);
    void setPnL(double amount);

    // Utility
    void printPortfolio() const;
};

#endif // PORTFOLIO_H
