#include <iostream>

#include "ringbuffer.hpp"

class OrderBook {
private:
    RingBuffer bids{true};
    RingBuffer asks{false};

public:
    void updateBid(double price, double amount, double timestamp);
    void updateAsk(double price, double amount, double timestamp);
    void printBook() const;
};