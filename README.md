order book c++
websocket data injestion

┌────────────┐     ┌────────────┐     ┌──────────────┐     ┌────────────┐
│ Data Feed  │ --> │ Ring Buffer│ --> │ Order Book   │ --> │ Output API │
│ (Binance)  │     │ (Lock-free)│     │ (In-memory)  │     │ (CLI / GUI)│
└────────────┘     └────────────┘     └──────────────┘     └────────────┘

test.cpp:
    kraken websocket test
    binance didn't work for some reason

daytime.cpp:
    simple api call for timestamp

kraken.cpp:
    working websocket connection to kraken

make:
make
make clean
exec: ./kraken

test compile:
g++ -std=c++20 -O2 -Wall -Wextra \
-I/opt/homebrew/include \
-L/opt/homebrew/lib \
-o kraken kraken.cpp OrderBook.cpp RingBuffer.cpp main.cpp \
-lboost_system -lboost_json -lssl -lcrypto -pthread

buy orders: highest to lowest
sell orders: lowest to highest