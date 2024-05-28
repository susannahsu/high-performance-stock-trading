#ifndef STOCK_H
#define STOCK_H

class Stock {
private:
    int stock_id;
    double price;
    double vol_rating;

public:
    Stock() : stock_id(0), price(0.0), vol_rating(0.0) {}

    Stock(int id, double price, double vol_rating);

    int getStockId() const;
    double getPrice() const;
    void updatePrice(double newPrice);
    double getVolatility() const;
};

#endif // STOCK_H
