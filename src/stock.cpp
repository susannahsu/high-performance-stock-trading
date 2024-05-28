#include "stock.h"
#include <mpi.h>
#include <omp.h>

Stock::Stock(int id, double initialPrice, double volatility)
    : stock_id(id), price(initialPrice), vol_rating(volatility) {}

int Stock::getStockId() const {
    return stock_id;
}

double Stock::getPrice() const {
    return price;
}

void Stock::updatePrice(double newPrice) {
    price = newPrice;
}

double Stock::getVolatility() const {
    return vol_rating;
}
