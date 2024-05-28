#include "orderBook.h"
#include "order.h"
#include "portfolio.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <mpi.h>
#include <omp.h>
#include <vector>
#include <functional>


void merge(std::vector<Order*>& arr, int left, int mid, int right, std::function<bool(Order*, Order*)> comp) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    // Create temporary arrays
    std::vector<Order*> L(n1), R(n2);

    // Copy data to temp arrays L[] and R[]
    for (int i = 0; i < n1; i++)
        L[i] = arr[left + i];
    for (int j = 0; j < n2; j++)
        R[j] = arr[mid + 1 + j];

    // Merge the temp arrays back into arr[left..right]
    int i = 0; // Initial index of first subarray
    int j = 0; // Initial index of second subarray
    int k = left; // Initial index of merged subarray

    while (i < n1 && j < n2) {
        if (comp(L[i], R[j])) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    // Copy the remaining elements of L[], if there are any
    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    // Copy the remaining elements of R[], if there are any
    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
}


void merge_sort(std::vector<Order*>& arr, int left, int right, std::function<bool(Order*, Order*)> comp) {
    if (left < right) {
        // Same as (left + right)/2, but avoids overflow for large left and right
        int mid = left + (right - left) / 2;

        // Sort first and second halves
        #pragma omp parallel sections
        {
            #pragma omp section
            merge_sort(arr, left, mid, comp);

            #pragma omp section
            merge_sort(arr, mid + 1, right, comp);
        }


        merge(arr, left, mid, right, comp);
    }
}


OrderBook::OrderBook(int max_num_orders) : max_num_orders(max_num_orders) {
    orders = new Order[max_num_orders];
}


//
// File format, each line:
// int       int      double      int
// trader_id stock_id offer_price quantity
bool debugging = false;

bool OrderBook::WriteRemainingOrdersToFile(std::string fname) {
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::string localOrdersStr;
    for (const auto& order : remainingOrders) {
        localOrdersStr += std::to_string(order->getTraderId()) + ","
                          + std::to_string(order->getStockId()) + ","
                          + std::to_string(order->getOfferPrice()) + ","
                          + std::to_string(order->getQuantity()) + "\n";
    }

    // Calculate local size
    int localSize = localOrdersStr.size();

    // Gather sizes from all ranks
    std::vector<int> orderSizes(size);
    MPI_Allgather(&localSize, 1, MPI_INT, orderSizes.data(), 1, MPI_INT, MPI_COMM_WORLD);

    // Compute the offset for this rank's data
    int offset = 0;
    for (int i = 0; i < rank; i++) {
        offset += orderSizes[i];
    }

    // Open MPI file for writing
    MPI_File mpiFile;
    MPI_File_open(MPI_COMM_WORLD, fname.c_str(), MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &mpiFile);

    // Write local orders to the file at the computed offset
    MPI_File_write_at(mpiFile, offset, localOrdersStr.c_str(), localSize, MPI_CHAR, MPI_STATUS_IGNORE);

    // Close the file
    MPI_File_close(&mpiFile);
    return true;
}

void OrderBook::addOrder(int idx, int trader_id, int stock_id, double offer_price, int quantity) {
    orders[idx].setTraderId(trader_id);
    orders[idx].setStockId(stock_id);
    orders[idx].setOfferPrice(offer_price);
    orders[idx].setQuantity(quantity);
    return;
}

Portfolio* OrderBook::matchOrders(Portfolio* portfolios, int num_portfolios) {
    int num_orders = 0;
    int num_flops = 0;

    #pragma omp parallel for reduction(+:num_orders)
    for (int order_idx = 0; order_idx < max_num_orders; order_idx++) {
        
        if (orders[order_idx].getQuantity() != 0) {
            #pragma omp atomic
            num_orders++;
        } 
    }
    std::cout << "=============================================================================" << std::endl;
    std::cout << "Starting to match orders. Total orders: " << num_orders << std::endl << std::endl;
    std::map<int,std::vector<Order*>> buyOrders;
    std::map<int,std::vector<Order*>> sellOrders;
    std::map<int,std::vector<Order*>>::iterator it; // stock_id, order

    // sorting orders into buy and sell based on their stock_id

    #pragma omp parallel
    {
        std::map<int, std::vector<Order*>> threadLocalBuyOrders, threadLocalSellOrders;
        #pragma omp for 
        for(int i=0; i<max_num_orders; i++) {
            Order* currentOrder = &orders[i];
            if (currentOrder->getQuantity() == 0) {
                continue;
            }
            int stockId = currentOrder->getStockId();
            if (currentOrder->getQuantity() < 0) { // BUY
                threadLocalBuyOrders[stockId].push_back(currentOrder);
            } else if (currentOrder->getQuantity() > 0) { // SELL
                threadLocalSellOrders[stockId].push_back(currentOrder);
            }
        }

        #pragma omp critical
        {
            for (auto &pair : threadLocalBuyOrders) {
                buyOrders[pair.first].insert(buyOrders[pair.first].end(), pair.second.begin(), pair.second.end());
            }
            for (auto &pair : threadLocalSellOrders) {
                sellOrders[pair.first].insert(sellOrders[pair.first].end(), pair.second.begin(), pair.second.end());
            }
        }
    }
    
    int totalBuyOrders = 0, totalSellOrders = 0;
    for (const auto& pair : buyOrders) {
        totalBuyOrders += pair.second.size();
    }
    for (const auto& pair : sellOrders) {
        totalSellOrders += pair.second.size();
    }

    std::cout << "Categorized " << totalBuyOrders << " buy orders and "
            << totalSellOrders << " sell orders for matching." << std::endl << std::endl;


    for (auto& pair : buyOrders) {
        merge_sort(pair.second, 0, pair.second.size() - 1, 
        [](Order* a, Order* b) {
            return a->getQuantity() > b->getQuantity();
        });
    }

    for (auto& pair : sellOrders) {
        merge_sort(pair.second, 0, pair.second.size() - 1,
            [](Order* a, Order* b) {
                return a->getQuantity() < b->getQuantity();
            });
    }

    // the goal is to sort buyOrders & sellOrders into matchedOrders
    // and the unmatched once remain in buyOrders & sellOrders
    int count = 0;

    std::vector<int> buyOrderKeys;
    for (const auto& pair : buyOrders) {
        buyOrderKeys.push_back(pair.first);
    }

    #pragma omp parallel for schedule(dynamic, 2) 
    for (size_t idx = 0; idx < buyOrderKeys.size(); idx++) {
        #pragma omp critical 
        {
            count++;
            std::cout << count << std::endl;
        }
        
        int key = buyOrderKeys[idx];
        // go through stock id-s availble for buy
        auto it = sellOrders.find(key);
        
        if(it == sellOrders.end()) {
            continue; // no such stock_id availble
        } 

        // process orders
        std::vector<Order*> &ordersToBuy = buyOrders[key];
        std::vector<Order*> &ordersToSell = it->second;
        std::vector<std::pair<Order,Order>> matched;  // local vector for current stock_id matches

        // #pragma omp parallel for num_threads(omp_get_max_threads() / 4) schedule(dynamic)
        for(size_t b=0; b<ordersToBuy.size(); b++){
            for(size_t s=0; s<ordersToSell.size(); s++){

                if(ordersToBuy[b]->getQuantity() == 0) {
                    break;
                }

                if(ordersToSell[s]->getQuantity() == 0 || ordersToSell[s]->getOfferPrice() > ordersToBuy[b]->getOfferPrice()) {
                    continue;
                }

                // logic for matching orders
                int quantityToBuy = std::abs(ordersToBuy[b]->getQuantity());
                int quantityToSell = ordersToSell[s]->getQuantity();

                Order buy = *ordersToBuy[b];
                Order sell = *ordersToSell[s];
                if (quantityToBuy >= quantityToSell) {
                    // Adjust quantities based on the transaction
                    buy.setQuantity(sell.getQuantity());
                    ordersToBuy[b]->setQuantity(ordersToBuy[b]->getQuantity() + sell.getQuantity());
                    ordersToSell[s]->setQuantity(0);
                } else {
                    // ?
                    sell.setQuantity(buy.getQuantity());
                    ordersToSell[s]->setQuantity(ordersToSell[s]->getQuantity() - quantityToBuy);
                    ordersToBuy[b]->setQuantity(0);
                }

                // Sanity check: In `matched`, sell quantity must be equal to buy quantity. 
                if (buy.getQuantity() != sell.getQuantity()) {
                    std::cout << "ERROR! Actual buy quantity and actual sell quantity for this matched record is not equal. Respectively: "
                            << buy.getQuantity() << ", " << sell.getQuantity() << std::endl << std::endl;
                }
                // #pragma omp critical
                matched.push_back(std::make_pair(sell, buy)); // Copy data

                // Adjust PNL 
                int tradeQuantity = sell.getQuantity(); // actual quantity traded
                double tradePrice = sell.getOfferPrice(); // Regard as not matched if buy.getOfferPrice() > sell.getOfferPrice(). Actual tradePrice should always be sell.getOfferPrice(). 
                double cashExchanged = tradePrice * tradeQuantity;
                num_flops += 1;
                // For the buyer, subtract the cash spent
                bool buyer_indicator = false; 
                bool seller_indicator = true; 
                for (int p = 0; p < num_portfolios; p++) {
                    Portfolio portfolio = portfolios[p];
                    if (portfolio.getTraderId() == ordersToBuy[b]->getTraderId()) {
                        // #pragma omp critical
                        portfolio.adjustPnL(-cashExchanged); 
                        num_flops += 1;
                        buyer_indicator = true; 
                        break;
                    }
                }
                for (int p = 0; p < num_portfolios; p++) {
                    Portfolio portfolio = portfolios[p];
                    if (portfolio.getTraderId() == ordersToSell[s]->getTraderId()) {
                        // #pragma omp critical
                        portfolio.adjustPnL(cashExchanged); 
                        num_flops += 1;
                        seller_indicator = true; 
                        break;
                    }
                }
                // Sanity check
                if (!(buyer_indicator && seller_indicator)) {
                    std::cout << "Buyer indicator: " << buyer_indicator << ", Seller indicator: " << seller_indicator << std::endl << std::endl;
                }
                
            } 

        }
        
        #pragma omp critical
        matchedOrders.emplace(key, matched);
            
    } // i, stock_id

    // Update the local portfolios (i.e. trader info) using MPI communications
    double* local_pnls = new double[num_portfolios];
    double* global_pnls = new double[num_portfolios];

    // Extract local PnLs from the portfolios
    for (int i = 0; i < num_portfolios; i++) {
        local_pnls[i] = portfolios[i].getPnL();
    }

    // Perform the MPI Allreduce operation to sum PnLs across all processes
    MPI_Allreduce(local_pnls, global_pnls, num_portfolios, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

    // Update local portfolio objects with the global PnL values
    for (int i = 0; i < num_portfolios; i++) {
        portfolios[i].setPnL(global_pnls[i]);
    }

    // Clean up
    delete[] local_pnls;
    delete[] global_pnls;

    // Iterate through all entries in matchedOrders to sum up the sizes of all matched vectors
    int totalMatchedOrders = 0; // Initialize counter for total matched orders
    for (const auto& entry : matchedOrders) {
        const auto& matchesForStock = entry.second; // entry.second is the vector of matched orders for the stock
        totalMatchedOrders += matchesForStock.size(); // Add the number of matches for this stock to the total
    }

    std::cout << "=============================================================================" << std::endl;
    std::cout << "Total number of matched orders across all stock IDs: " << totalMatchedOrders << std::endl;


    // Remove finished orders. 
    for(auto b = buyOrders.begin(); b!=buyOrders.end(); b++){
        //std::vector<Order*> buy = b->second
        for(size_t i=0; i<b->second.size(); i++){
            if(b->second[i]->getQuantity()==0) continue;
            remainingOrders.push_back(b->second[i]);
        }
    }

    for(auto s = sellOrders.begin(); s!=sellOrders.end(); s++){
        //std::vector<Order*> sell = s->second
        for(size_t i=0; i<s->second.size(); i++){
            if(s->second[i]->getQuantity()==0) continue;
            remainingOrders.push_back(s->second[i]);
        }
    }


    std::cout << "=============================================================================" << std::endl;
    std::cout << "Total Remaining Unmatched Orders: " << remainingOrders.size() << std::endl << std::endl;
    
    return portfolios;
}
