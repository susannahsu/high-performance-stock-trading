#include "portfolioUtils.h"
#include <random>
#include <iostream>
#include <algorithm>
#include <mpi.h>
#include <omp.h>

void distributeSharesRandomly(Portfolio *portfolios, int num_portfolios, Stock *stocks, int num_stocks) {
    // std::default_random_engine generator(205); // Seed for reproducibility
    
    #pragma omp parallel for
    for (int i = 0; i < num_stocks; ++i) {
        std::default_random_engine generator(205 + omp_get_thread_num()); // Make seed dependent on thread number
        int *traderIndices = new int[num_portfolios];
        std::iota(traderIndices, traderIndices + num_portfolios, 0); // Fill with indices 0, 1, ..., portfolios.size() - 1

        int average_share_per_trader = 1;
        int sharesRemaining = average_share_per_trader * num_portfolios;
        while (sharesRemaining > 0) {
            std::shuffle(traderIndices, traderIndices + num_portfolios, generator); // Randomly shuffle trader indices
            for (int idx = 0; idx < num_portfolios; idx++) {
                if (sharesRemaining <= 0) break;

                int maxSharesForTrader = std::min(sharesRemaining, 30); // Limit max shares per trader per round to prevent hoarding
                std::uniform_int_distribution<int> dist(1, maxSharesForTrader); // Distribute between 1 and maxSharesForTrader shares
                int sharesToAssign = dist(generator);

                #pragma omp critical
                portfolios[idx].assignShares(stocks[i].getStockId(), sharesToAssign);
                
                sharesRemaining -= sharesToAssign;
            }
        }
        
        delete[] traderIndices;
    }
}
