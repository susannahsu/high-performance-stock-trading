#ifndef DATALOADER_H
#define DATALOADER_H

#include "trader.h"
#include "stock.h"
#include <vector>

void readTradersFromFile(const std::string& filename, Trader* traders);

#endif // DATALOADER_H
