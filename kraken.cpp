#include "kraken.hpp"

bool KrakenWebSocketClient::connect()
{
    try
    {
        std::cout << "Connecting to Kraken WebSocket...\n";

        if (!SSL_set_tlsext_host_name(ws.next_layer().native_handle(), "ws.kraken.com"))
        {
            std::cerr << "Failed to set SNI hostname\n";
            return false;
        }

        auto const results = resolver.resolve("ws.kraken.com", "443");
        auto ep = net::connect(get_lowest_layer(ws), results);

        ws.next_layer().handshake(ssl::stream_base::client);

        ws.set_option(websocket::stream_base::decorator(
            [](websocket::request_type &req)
            {
                req.set(beast::http::field::user_agent, "kraken-websocket-client");
                req.set(beast::http::field::host, "ws.kraken.com");
            }));

        ws.handshake("ws.kraken.com", "/");

        connected = true;
        std::cout << "Connected to Kraken WebSocket\n";
        std::cout << "Connected to: " << ep.address() << ":" << ep.port() << "\n";

        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << " Connection failed: " << e.what() << "\n";
        connected = false;
        return false;
    }
}

void KrakenWebSocketClient::subscribe(const std::string &channel, const std::vector<std::string> &pairs,
                                      const json::object &options = {})
{
    if (!connected)
    {
        std::cerr << "Not connected to WebSocket\n";
        return;
    }

    try
    {
        json::object subscription_msg = {
            {"event", "subscribe"},
            {"pair", json::array(pairs.begin(), pairs.end())},
            {"subscription", json::object{{"name", channel}}}};

        if (!options.empty())
        {
            auto &sub = subscription_msg["subscription"].as_object();
            for (const auto &[key, value] : options)
            {
                sub[key] = value;
            }
        }

        std::string msg = json::serialize(subscription_msg);
        ws.write(net::buffer(msg));

        std::cout << "Subscribed to " << channel << " for " << pairs.size() << " pairs\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Subscription failed: " << e.what() << "\n";
    }
}

void KrakenWebSocketClient::subscribe_to_orderbook(const std::vector<std::string> &pairs, int depth = 10)
{
    json::object options = {{"depth", depth}};
    subscribe("book", pairs, options);
    std::cout << "Order book depth: " << depth << " levels\n";
}

void KrakenWebSocketClient::set_message_callback(std::function<void(const std::string &)> callback)
{
    message_callback = callback;
}

void KrakenWebSocketClient::start_message_loop()
{
    running = true;
    std::cout << "Starting message processing loop...\n";

    while (running && connected)
    {
        try
        {
            ws.read(buffer);

            std::string message = beast::buffers_to_string(buffer.data());
            buffer.clear();

            messages_received++;

            if (is_heartbeat(message))
            {
                last_heartbeat = steady_clock::now();
                continue;
            }

            if (message_callback)
            {
                message_callback(message);
            }
            else
            {
                std::cout << message << "\n";
            }
        }
        catch (const beast::system_error &e)
        {
            if (e.code() == websocket::error::closed)
            {
                std::cout << "WebSocket connection closed\n";
                connected = false;
                break;
            }
            std::cerr << " WebSocket error: " << e.what() << "\n";
            break;
        }
        catch (const std::exception &e)
        {
            std::cerr << " Message processing error: " << e.what() << "\n";
        }
    }

    running = false;
    std::cout << "Message loop stopped\n";
}

void KrakenWebSocketClient::stop()
{
    running = false;
    if (connected)
    {
        try
        {
            ws.close(websocket::close_code::normal);
            connected = false;
            std::cout << "WebSocket connection closed gracefully\n";
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error closing connection: " << e.what() << "\n";
        }
    }
}

void KrakenWebSocketClient::send_message(const std::string &message)
{
    if (!connected)
    {
        std::cerr << "Not connected to WebSocket\n";
        return;
    }

    try
    {
        ws.write(net::buffer(message));
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to send message: " << e.what() << "\n";
    }
}

void KrakenWebSocketClient::print_stats() const
{
    auto uptime = std::chrono::duration_cast<std::chrono::seconds>(
                      steady_clock::now() - start_time)
                      .count();

    auto heartbeat_age = std::chrono::duration_cast<std::chrono::seconds>(
                             steady_clock::now() - last_heartbeat)
                             .count();

    std::cout << "\nConnection Statistics:\n";
    std::cout << "Uptime: " << uptime << " seconds\n";
    std::cout << "Messages received: " << messages_received << "\n";
    std::cout << "Last heartbeat: " << heartbeat_age << " seconds ago\n";
    std::cout << "Connected: " << (connected ? "Yes" : "No") << "\n";
}

bool KrakenWebSocketClient::is_connected() const { return connected; }
uint64_t KrakenWebSocketClient::get_message_count() const { return messages_received; }

bool KrakenWebSocketClient::is_heartbeat(const std::string &message)
{
    try
    {
        auto parsed = json::parse(message);
        if (parsed.is_object())
        {
            auto obj = parsed.as_object();
            if (obj.contains("event"))
            {
                std::string event = obj["event"].as_string().c_str();
                return event == "heartbeat";
            }
        }
    }
    catch (...)
    {
    }
    return false;
}
