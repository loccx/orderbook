#include "orderbook.hpp"

void OrderBook::updateBid(double price, double amount, double timestamp) {
    bids.insertOrUpdate(price, amount, timestamp);
}

void OrderBook::updateAsk(double price, double amount, double timestamp) {
    asks.insertOrUpdate(price, amount, timestamp);
}

void OrderBook::printBook() const {
    std::system("clear");
    std::cout << "Order Book:\n";
    std::cout << "============\n";
    bids.print();
    asks.print();
}