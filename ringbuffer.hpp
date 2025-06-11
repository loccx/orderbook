#include <array>
#include <iostream>
#include <iomanip>

constexpr int LEVELS = 10;

struct Order {
    double price = 0.0;
    double amount = 0.0;
    double timestamp = 0.0;
};

class RingBuffer {
private:
    std::array<Order, LEVELS> buffer;
    int head = 0;
    int size = 0;
    bool isBid = false;

public:
    RingBuffer(bool bid) : isBid(bid) {}

    const Order& at(int index) const;

    void insertOrUpdate(double price, double amount, double timestamp);
    void print() const;

private:
    void insertSorted(const Order& order);
    void removeAt(int index);
};