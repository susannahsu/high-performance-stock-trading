#ifndef PORTFOLIO_UTILS_H
#define PORTFOLIO_UTILS_H

#include <vector>
#include "portfolio.h"
#include "stock.h"

void distributeSharesRandomly(Portfolio* portfolios, int num_portfolios, Stock* stocks, int num_stocks);

#endif // PORTFOLIO_UTILS_H
