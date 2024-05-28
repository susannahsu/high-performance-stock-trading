#ifndef ORDERUTILS_H
#define ORDERUTILS_H

#include "orderBook.h"
#include "portfolio.h"
#include "stock.h"
#include <vector>
#include <string>

OrderBook generateAndWriteOrders(Portfolio* portfolios, int num_portfolios, Stock* stocks, int num_stocks, const std::string& filename);

#endif // ORDERUTILS_H
