#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include "order.h"
#include <vector>
#include <map>
#include <string>
#include <memory>
#include "portfolio.h"
#include <omp.h>

class OrderBook {
public:
        OrderBook(int max_num_orders);
        Order* orders;
        int max_num_orders;
        bool ReadOrdersFromFile(std::string fname);        
        bool WriteRemainingOrdersToFile(std::string fname);        
        void addOrder(int idx, int trader_id, int stock_id, double offer_price, int quantity);
        Portfolio* matchOrders(Portfolio* portfolios, int num_portfolios);
private:
        std::map<int,std::vector<std::pair<Order,Order>>> matchedOrders;
        std::vector<Order*> remainingOrders;
};

#endif // ORDERBOOK_H