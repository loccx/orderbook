#include "ringbuffer.hpp"

const Order& RingBuffer::at(int index) const {
    return this->buffer[(head + index) % LEVELS];
};

void RingBuffer::insertOrUpdate(double price, double amount, double timestamp) {
    for (int index = 0; index < size; index++) {
        Order& currentOrder = this->buffer[(head + index) % LEVELS];
        if (currentOrder.price == price) {
            if (amount == 0.0) {
                removeAt(index);
            }
            else {
                currentOrder.amount = amount;
                currentOrder.timestamp = timestamp;
            }
            return;
        }
    }

    if (amount == 0.0) return;

    if (size < LEVELS) {
        insertSorted({price, amount, timestamp});
    }
    else {
        const Order& worst = at(size - 1);
        bool moreAggressive = isBid ? (price > worst.price) : (price < worst.price);
        if (moreAggressive) {
            removeAt(size - 1);
            insertSorted({price, amount, timestamp});
        }
    }
}

void RingBuffer::insertSorted(const Order& order) {
    int pos = 0;
    for (; pos < size; ++pos) {
        const Order& currentOrder = at(pos);
        if ((isBid && currentOrder.price > order.price) || (!isBid && currentOrder.price < order.price))
            break;
    }

    for (int i = size; i > pos; i--) {
        buffer[(head + i) % LEVELS] = at(i - 1);
    }

    buffer[(head + pos) % LEVELS] = order;
    size++;
}

void RingBuffer::removeAt(int index) {
    for (int i = index; i < size - 1; ++i) {
        this->buffer[(head + index) % LEVELS] = at(index + 1);
    }
    size--;
}

void RingBuffer::print() const {
    for (int i = 0; i < size; i++) {
        const auto& order = at(i);
        std::cout << std::fixed << std::setprecision(2)
                << (isBid ? "[BID] " : "[ASK] ")
                << "Price: " << order.price << ' ' << "Amount: " << order.amount << '\n';
    }
    std::cout << '\n';
}