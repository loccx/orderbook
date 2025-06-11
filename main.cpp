#include <iostream>

#include "kraken.hpp"
#include "orderbook.hpp"

int main() {
    std::cout << "Order Book from Kraken Market:\n";
    std::cout << "==========================\n\n";
    
    KrakenWebSocketClient client;
    OrderBook book;
    
    client.set_message_callback([&book](const std::string& message) {
        // std::cout << "Received: " << message << "\n";
        
        try {
            auto parsed = json::parse(message);
            
            if (parsed.is_object()) {
                auto obj = parsed.as_object();
                if (obj.contains("event")) {
                    std::string event = obj["event"].as_string().c_str();
                    // std::cout << "Event: " << event << "\n";
                }
            } else if (parsed.is_array()) { // book update
                // std::cout << "Data update received\n";

                auto arr = parsed.as_array();
                if (arr.size() >= 4 && arr[1].is_object()) {
                    auto update = arr[1].as_object();
                    std::string pair = arr[3].as_string().c_str();

                    // ask
                    if (update.contains("a")) {
                        auto asks = update["a"].as_array();
                        for (const auto& ask_entry : asks) {
                            auto ask = ask_entry.as_array();
                            if (ask.size() >= 3) {
                                double price = std::stod(ask[0].as_string().c_str());
                                double amount = std::stod(ask[1].as_string().c_str());
                                double timestamp = std::stod(ask[2].as_string().c_str());
                                // std::cout << "ask update: " << pair <<
                                // ", price: " << price <<
                                // ", amount: " << amount <<
                                // ", timestamp: " << timestamp << '\n';
                                book.updateAsk(price, amount, timestamp);
                                book.printBook();
                            }
                        }
                    }
                    // bid
                    else if (update.contains("b")) {
                        auto bids = update["b"].as_array();
                        for (const auto& bid_entry : bids) {
                            auto bid = bid_entry.as_array();
                            if (bid.size() >= 3) {
                                double price = std::stod(bid[0].as_string().c_str());
                                double amount = std::stod(bid[1].as_string().c_str());
                                double timestamp = std::stod(bid[2].as_string().c_str());
                                // std::cout << "bid update: " << pair <<
                                // ", price: " << price <<
                                // ", amount: " << amount <<
                                // ", timestamp: " << timestamp << '\n';
                                book.updateBid(price, amount, timestamp);
                                book.printBook();
                            }
                        }
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Parse error: " << e.what() << "\n";
        }
    });
    
    if (!client.connect()) {
        std::cerr << "Failed to connect to Kraken\n";
        return 1;
    }
    
    std::vector<std::string> pairs = {
        "BTC/USD"
    };
    
    client.subscribe_to_orderbook(pairs, 10);
    
    std::thread message_thread([&client]() {
        client.start_message_loop();
    });
    
    std::cout << "\nStarting WebSocket monitoring...\n";
    std::cout << "Press Ctrl+C to stop\n\n";
    
    int iteration = 0;
    while (client.is_connected() && iteration < 1) {  // each iter is 5 seconds
        std::this_thread::sleep_for(std::chrono::seconds(5));
        client.print_stats();
        // std::cout << "\n" << std::string(50, '=') << "\n";
        iteration++;
    }
    
    std::cout << "\nShutting down...\n";
    client.stop();
    
    if (message_thread.joinable()) {
        message_thread.join();
    }

    
    std::cout << "Shutdown complete\n";
    return 0;
}