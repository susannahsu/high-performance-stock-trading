#include <iostream>
#include <fstream>
#include <random>
#include <iomanip> 

/*
stocks.csv
...
stock_id, price, vol_rating
...
types:
int, double, double
*/

int main() {
    using namespace std;

    int numStocks = 100000;   // change here to generate different kinds of datasets: 5, 10, 20, 50, 100, 500

    // price range for stocks
    double minPrice = 0.1;
    double maxPrice = 200.0;
    double minVolRating = 0.1;
    double maxVolRating = 1.0;

    unsigned seed = 205;
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> priceDist(minPrice, maxPrice); // for prices
    std::uniform_real_distribution<double> volDist(minVolRating, maxVolRating);

    ofstream stocksFile("stocks_" + std::to_string(numStocks) + ".csv");
    if (stocksFile.is_open()) {
        stocksFile << "stock_id,price,vol_rating\n"; // Header
        for (int i = 1; i <= numStocks; ++i) { // Adjusted to start at 1
            double price = priceDist(rng);
            double volRating = volDist(rng); // Generate volatility rating

            // Write to file
            stocksFile << std::fixed << std::setprecision(2);
            stocksFile << i << "," << price << "," << volRating << "\n";
        }
        stocksFile.close();
    } else {
        cerr << "Unable to open file";
        return -1;
    }

    return 0;
}