#include <iostream>
#include <fstream>
#include <random>
#include <iomanip>

int main() {
    using namespace std;

    int numTraders = 19500; // Number of traders to generate: 10, 20, 50, 100, 200, 500

    // Ranges for pnl and risk tolerance
    double minPnl = 1000.0;
    double maxPnl = 10000.0;
    double minRiskTolerance = 0.1;
    double maxRiskTolerance = 1.0;

    unsigned seed = 205;
    std::mt19937 rng(seed); // Mersenne Twister random number generator
    std::uniform_real_distribution<double> pnlDist(minPnl, maxPnl);
    std::uniform_real_distribution<double> riskToleranceDist(minRiskTolerance, maxRiskTolerance);

    ofstream tradersFile("traders_" + std::to_string(numTraders) + ".csv");
    if (tradersFile.is_open()) {
        tradersFile << "trader_id,pnl,risk_tol\n"; // Header
        for (int i = 1; i <= numTraders; ++i) {
            double pnl = pnlDist(rng);
            double riskTolerance = riskToleranceDist(rng);

            // Use std::fixed and std::setprecision to format the output
            tradersFile << std::fixed << std::setprecision(2);
            tradersFile << i << "," << pnl << "," << riskTolerance << "\n";
        }
        tradersFile.close();
    } else {
        cerr << "Unable to open file";
        return -1;
    }

    return 0;
}
