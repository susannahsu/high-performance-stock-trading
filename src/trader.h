#ifndef TRADER_H
#define TRADER_H

class Trader {
private:
    int trader_id;
    double pnl;
    double risk_tolerance;

public:
    Trader() : trader_id(0), pnl(0.0), risk_tolerance(0.0) {}
    Trader(int id, double pnl, double risk_tolerance);

    int getTraderId() const;
    void setTraderId(int trader_id);
    double getPnl() const;
    void setPnl(double newPnl);
    double getRiskTolerance() const;
    void setRiskTolerance(double newRiskTolerance);
};

#endif // TRADER_H
