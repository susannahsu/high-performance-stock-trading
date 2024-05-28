#include <iostream>
#include <fstream>
#include <iomanip> 
#include "dataLoader.h"
#include "portfolio.h"
#include "portfolioUtils.h"
#include "order.h"
#include "orderUtils.h"
#include "orderBook.h" 
#include <chrono>
#include <string>
#include <sstream>
#include <cassert>
#include <mpi.h>
#include <omp.h>
// #include "papi.h"
#define assertm(exp, msg) assert(((void)msg, exp))
using namespace std::chrono;

int main(int argc, char *argv[]) {
    // Initialize PAPI
    // if (PAPI_library_init(PAPI_VER_CURRENT) != PAPI_VER_CURRENT) {
    //     std::cerr << "PAPI library initialization error!" << std::endl;
    //     return 1;
    // }

    // Setup PAPI
    const int CacheLineSize = 64; // Cache line size in bytes
    // int events[3] = {PAPI_L1_DCM, PAPI_L2_DCM, PAPI_L3_TCM};
    long long counters[3];
    // int event_set = PAPI_NULL;
    // PAPI_create_eventset(&event_set);
    // PAPI_add_events(event_set, events, 3);

    // Load traders and stocks
    int num_traders = 10000;
    int num_stocks = 500; 
    
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
    if (provided < MPI_THREAD_FUNNELED) {
        std::cout << "ERROR: The MPI library does not have full thread support" << '\n';
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int world_size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int max_local_stocks = num_stocks / world_size + 1;   // Adjust based on the maximum expected number of stocks
    int max_local_traders = num_traders;   // Adjust based on the maximum expected number of traders

    Stock* local_stocks = new Stock[max_local_stocks];
    Trader* traders = new Trader[max_local_traders];
    Portfolio* portfolios = new Portfolio[max_local_traders];

    MPI_File fh;
    MPI_Status status;

    // Open the CSV file with MPI
    std::string stock_filename = "../data/stocks_" + std::to_string(num_stocks) + ".csv";
    MPI_File_open(MPI_COMM_WORLD, stock_filename.c_str(), MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);

    // Determine file size
    MPI_Offset file_size;
    MPI_File_get_size(fh, &file_size);

    // Calculate partition size for each process
    MPI_Offset start_idx = (file_size / world_size) * rank;
    MPI_Offset end_idx = (rank == world_size - 1) ? file_size : (start_idx + file_size / world_size);

    // Read a bit extra to not cut off lines in the middle
    end_idx += (rank != world_size - 1) ? 20 : 0;  // Additional bytes, adjust as necessary

    // Allocate buffer and read data
    std::cout << start_idx << ", " << end_idx << std::endl;
    int bufsize = end_idx - start_idx + 1;
    char *buffer = new char[bufsize];
    MPI_File_read_at_all(fh, start_idx, buffer, bufsize - 1, MPI_CHAR, &status);

    // Null terminate the buffer to make it a valid C-style string
    buffer[bufsize - 1] = '\0';

    // Adjust start to skip partial initial line unless at the beginning of the file
    std::string data_str(buffer);
    if (start_idx != 0) {
        size_t first_newline = data_str.find('\n');
        data_str = data_str.substr(first_newline + 1);
    }

    // Adjust end to remove partial final line unless at the end of the file
    if (rank != world_size - 1) {
        size_t last_newline = data_str.rfind('\n');
        if (last_newline != std::string::npos) {
            data_str = data_str.substr(0, last_newline);
        }
    }

    auto start = high_resolution_clock::now();
    // auto stocks = readStocksFromFile("../data/stocks_" + std::to_string(num_stocks) + ".csv");

    int local_stock_count = 0;
    std::stringstream ss(data_str);
    std::string line;
    while (getline(ss, line)) {
        
        std::stringstream linestream(line);
        int stock_id;
        double price, vol_rating;
        char delim; // Variable to consume the commas
        
        linestream >> stock_id >> delim >> price >> delim >> vol_rating;
        if (local_stock_count < max_local_stocks) {
            local_stocks[local_stock_count++] = Stock(stock_id, price, vol_rating);
        }
    }
    
    // Clean up
    MPI_Barrier(MPI_COMM_WORLD);
    delete[] buffer;
    MPI_File_close(&fh);
    
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    std::string time_message = "Read " + std::to_string(num_stocks) + " stocks. Takes " + std::to_string(duration.count()) + " micro-seconds.";
    std::cout << time_message << std::endl;
    
    start = high_resolution_clock::now();
    std::string trader_filename = "../data/traders_"+ std::to_string(num_traders) + ".csv";
    std::ifstream trader_file(trader_filename);
    std::string trader_line;

    int local_trader_count = 0;
    getline(trader_file, trader_line); // Skip the header line
    
    while (getline(trader_file, trader_line)) {
        std::stringstream trader_linestream(trader_line);
        int trader_id;
        double pnl, risk_tolerance;
        char trader_delim; // Variable to consume the commas
        
        trader_linestream >> trader_id >> trader_delim >> pnl >> trader_delim >> risk_tolerance;
        if (!trader_linestream.fail()) {
            traders[local_trader_count++] = Trader(trader_id, pnl, risk_tolerance);
        } 
    }

    stop = high_resolution_clock::now();
    duration = duration_cast<microseconds>(stop - start);
    time_message = "Read " + std::to_string(num_traders) + " traders. Takes " + std::to_string(duration.count()) + " micro-seconds.";
    std::cout << time_message << std::endl;

    start = high_resolution_clock::now();

    for (int i = 0; i < max_local_traders; i++) {
        Trader trader = traders[i]; 
        portfolios[i] = Portfolio(trader.getTraderId(), trader.getPnl(), trader.getRiskTolerance());
    }
    
    // PAPI_start(event_set);
    distributeSharesRandomly(portfolios, local_trader_count, local_stocks, local_stock_count);
    // PAPI_stop(event_set, counters);

    // long long total_memory_accessed = (counters[0] + counters[1] + counters[2]) * CacheLineSize;
    // std::cout << "Total memory accessed in bytes for distributeSharesRandomly: " << total_memory_accessed << std::endl;

    // PAPI_reset(event_set);
    
    std::string orderFileName = "../data/orders_" + std::to_string(num_stocks) + "_" + std::to_string(num_traders) + ".csv";
    // PAPI_start(event_set);
    OrderBook book = generateAndWriteOrders(portfolios, local_trader_count, local_stocks, local_stock_count, orderFileName);
    // PAPI_stop(event_set, counters);
    stop = high_resolution_clock::now();

    // total_memory_accessed = (counters[0] + counters[1] + counters[2]) * CacheLineSize;
    // std::cout << "Total memory accessed in bytes for generateAndWriteOrders: " << total_memory_accessed << std::endl;

    duration = duration_cast<microseconds>(stop - start);
    time_message = "Order generation with " + std::to_string(num_stocks) + " stocks and " + std::to_string(num_traders) + " traders. Takes " + std::to_string(duration.count()) + " micro-seconds.";
    std::cout << time_message << std::endl;

    // PAPI_reset(event_set);
      
    // PAPI_start(event_set);
    start = high_resolution_clock::now();
    portfolios = book.matchOrders(portfolios, local_trader_count);
    stop = high_resolution_clock::now();
    // PAPI_stop(event_set, counters);

    // total_memory_accessed = (counters[0] + counters[1] + counters[2]) * CacheLineSize;
    // std::cout << "Total memory accessed in bytes for total_memory_accessed: " << total_memory_accessed << std::endl;

    duration = duration_cast<microseconds>(stop - start);
    time_message = "Matching orders takes " + std::to_string(duration.count()) + " micro-seconds.";
    std::cout << time_message << std::endl;

    if(!book.WriteRemainingOrdersToFile("../tests/t_" + std::to_string(num_traders) + "_s_" + std::to_string(num_stocks) + ".csv")){        
        std::cout << "can't write\n";
    }

    // PAPI_cleanup_eventset(event_set);
    // PAPI_destroy_eventset(&event_set);
    // PAPI_shutdown();

    MPI_Finalize();

    return 0;
}
