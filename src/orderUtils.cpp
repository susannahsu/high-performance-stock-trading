#include "orderUtils.h"
#include "order.h"
#include "orderBook.h"
#include <random>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cmath> // For std::abs
#include <mpi.h>
#include <omp.h>

OrderBook generateAndWriteOrders(Portfolio* portfolios, int num_portfolios, Stock* stocks, int num_stocks, const std::string& filename) {

    OrderBook order_book = OrderBook(num_portfolios * num_stocks);
    std::ofstream outFile(filename);
    if (!outFile) {
        std::cerr << "Failed to open " << filename << " for writing.\n";
        return order_book;
    }
    
    // std::default_random_engine generator(205); // For reproducibility
    std::normal_distribution<double> priceAdjustment(0.0, 0.3);
    std::vector<std::vector<std::string>> localOutputs(omp_get_max_threads());
    int num_flops = 0;
    #pragma omp parallel
    {
        int threadNum = omp_get_thread_num(); 
        auto& localOutput = localOutputs[threadNum]; // Reference to the thread's stringstream
        std::default_random_engine generator(threadNum); // Thread-local random generator
        std::normal_distribution<double> priceAdjustment(0.0, 0.3);

        #pragma omp for 
        for (int i = 0; i < num_portfolios; ++i) {
            for (int j = 0; j < num_stocks; ++j) {
                int idx = i * num_stocks + j;
                auto& portfolio = portfolios[i];
                auto& stock = stocks[j];
                double basePrice = stock.getPrice();
                double volatility = stock.getVolatility();
                double riskTolerance = portfolio.getRiskTolerance();
                int availableShares = portfolio.getShares(stock.getStockId());
                
                if (availableShares == 0) continue; // Skip if no shares to sell
                
                // Calculate offer price
                double offerPrice = basePrice * (1 + priceAdjustment(generator) * (1 + riskTolerance * volatility));
                num_flops += 5;
                
                // Determine quantity
                int quantity = std::round(availableShares * riskTolerance * (1 - volatility));
                num_flops += 1;
                if (quantity == 0) continue; // Ensure the quantity is not zero
                
                // Random decision to buy or sell, but not both
                bool isBuyOrder = std::uniform_int_distribution<int>(0, 1)(generator);
                num_flops += 2;
                if (isBuyOrder) {
                    order_book.addOrder(idx, portfolio.getTraderId(), 
                        stock.getStockId(), offerPrice, -std::abs(quantity));
                        
                    localOutput.emplace_back(std::to_string(portfolio.getTraderId()) + "," +
                                             std::to_string(stock.getStockId()) + "," +
                                             std::to_string(offerPrice) + "," +
                                             std::to_string(-std::abs(quantity)) + "\n");
                } else {
                    order_book.addOrder(idx, portfolio.getTraderId(), stock.getStockId(), offerPrice, std::abs(quantity));
                    localOutput.emplace_back(std::to_string(portfolio.getTraderId()) + "," +
                                             std::to_string(stock.getStockId()) + "," +
                                             std::to_string(offerPrice) + "," +
                                             std::to_string(std::abs(quantity)) + "\n");
                }
                
            }
        }
    }
    // std::cout << "Flops: " << num_flops << '\n';
    std::stringstream ss;
    
    // Concatenate all local strings into the file
    for (const auto& localOutput : localOutputs) {
        for (const auto& line : localOutput) {
            ss << line;
        }
    }
    std::string data = ss.str();
    int dataSize = data.size();

    int world_size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Gather sizes of each local data to calculate offsets
    int* sizes = new int[world_size];
    MPI_Allgather(&dataSize, 1, MPI_INT, sizes, 1, MPI_INT, MPI_COMM_WORLD);

    // Calculate offsets for each process
    int* offsets = new int[world_size];
    offsets[0] = 0;
    for (int i = 1; i < world_size; i++) {
        offsets[i] = offsets[i - 1] + sizes[i - 1];
    }

    // std::cout << "local generation size: " << dataSize << std::endl;
    MPI_File fh;
    MPI_File_open(MPI_COMM_WORLD, filename.c_str(), MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);

    // Write each process's local data at the calculated offset
    MPI_File_write_at(fh, offsets[rank], data.c_str(), dataSize, MPI_CHAR, MPI_STATUS_IGNORE);

    MPI_File_close(&fh);

    delete[] sizes;
    delete[] offsets;

    return order_book;
}
