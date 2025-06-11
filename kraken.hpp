#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/json.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>
#include <functional>

#include <openssl/ssl.h>
#include <openssl/err.h>

namespace net = boost::asio;
namespace ssl = net::ssl;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace json = boost::json;

using tcp = net::ip::tcp;
using steady_clock = std::chrono::steady_clock;

class KrakenWebSocketClient {
private:
    net::io_context ioc;
    ssl::context ctx;
    tcp::resolver resolver;
    websocket::stream<beast::ssl_stream<tcp::socket>> ws;
    beast::flat_buffer buffer;
    
    std::atomic<bool> running{false};
    std::atomic<bool> connected{false};
    std::atomic<uint64_t> messages_received{0};
    
    steady_clock::time_point start_time;
    steady_clock::time_point last_heartbeat;
    
    std::function<void(const std::string&)> message_callback;

public:
    KrakenWebSocketClient() 
        : ctx(ssl::context::tlsv12_client), resolver(ioc), ws(ioc, ctx) {
        
        ctx.set_default_verify_paths();
        ctx.set_verify_mode(ssl::verify_none); 
        
        start_time = steady_clock::now();
        last_heartbeat = start_time;
    }
    
    bool connect();
    
    void subscribe(const std::string& channel, const std::vector<std::string>& pairs, 
                   const json::object& options);
    
    void subscribe_to_orderbook(const std::vector<std::string>& pairs, int depth);
    
    void set_message_callback(std::function<void(const std::string&)> callback);
    
    void start_message_loop();
    
    void stop();
    
    void send_message(const std::string& message);
    
    void print_stats() const;
    
    bool is_connected() const;

    uint64_t get_message_count() const;
private:
    bool is_heartbeat(const std::string& message);
};