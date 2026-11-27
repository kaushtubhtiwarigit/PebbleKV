#include "server.h"
#include "command_parser.h"
#include <iostream>
#include <cstring>
#include <algorithm>

namespace pebblekv {

// Static wrapper for thread callback
static void handleClientWrapper(void* server_ptr, socket_t client_socket) {
    Server* server = static_cast<Server*>(server_ptr);
    server->handleClient(client_socket);
}

Server::Server(uint16_t port) 
    : server_socket_(INVALID_SOCKET_VALUE), 
      port_(port), 
      running_(false) {
#ifdef _WIN32
    // Initialize Winsock on Windows
    WSADATA wsaData;
    winsock_initialized_ = (WSAStartup(MAKEWORD(2, 2), &wsaData) == 0);
    if (!winsock_initialized_) {
        std::cerr << "Failed to initialize Winsock\n";
    }
#endif
}

Server::~Server() {
    stop();
#ifdef _WIN32
    if (winsock_initialized_) {
        WSACleanup();
    }
#endif
}

bool Server::start() {
    // Create socket
    server_socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket_ == INVALID_SOCKET_VALUE) {
        std::cerr << "Failed to create socket\n";
        return false;
    }
    
    // Allow address reuse
    int opt = 1;
    setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, 
               reinterpret_cast<char*>(&opt), sizeof(opt));
    
    // Bind to port
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Listen on all interfaces
    server_addr.sin_port = htons(port_);
    
    if (bind(server_socket_, reinterpret_cast<struct sockaddr*>(&server_addr), 
             sizeof(server_addr)) == SOCKET_ERROR_VALUE) {
        std::cerr << "Failed to bind to port " << port_ << "\n";
        closesocket(server_socket_);
        server_socket_ = INVALID_SOCKET_VALUE;
        return false;
    }
    
    // Listen for connections (backlog of 10)
    if (listen(server_socket_, 10) == SOCKET_ERROR_VALUE) {
        std::cerr << "Failed to listen on port " << port_ << "\n";
        closesocket(server_socket_);
        server_socket_ = INVALID_SOCKET_VALUE;
        return false;
    }
    
    running_ = true;
    std::cout << "PebbleKV server started on port " << port_ << "\n";
    std::cout << "Multi-client mode enabled (Phase 3)\n";
    std::cout << "Waiting for connections...\n";
    
    return true;
}

bool Server::enablePersistence(const std::string& wal_filename) {
    // Replay WAL if it exists
    db_.replayWAL(wal_filename);
    
    // Enable WAL for future writes
    bool ok = db_.enableWAL(wal_filename);
    
    // Set memory limit (1000 keys default)
    db_.setMaxSize(1000);
    
    return ok;
}

void Server::stop() {
    running_ = false;
    
    if (server_socket_ != INVALID_SOCKET_VALUE) {
        closesocket(server_socket_);
        server_socket_ = INVALID_SOCKET_VALUE;
    }
    
    // Wait for all client threads to finish
    LockGuard lock(threads_mutex_);
    for (auto* thread : client_threads_) {
        if (thread->joinable()) {
            thread->join();
        }
        delete thread;
    }
    client_threads_.clear();
}

void Server::run() {
    if (!running_) {
        std::cerr << "Server not started. Call start() first.\n";
        return;
    }
    
    while (running_) {
        // Accept client connection
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        socket_t client_socket = accept(server_socket_, 
                                       reinterpret_cast<struct sockaddr*>(&client_addr),
                                       &client_len);
        
        if (client_socket == INVALID_SOCKET_VALUE) {
            if (running_) {
                std::cerr << "Failed to accept connection\n";
            }
            continue;
        }
        
        std::cout << "Client connected (starting new thread)\n";
        
        // Clean up finished threads before starting new one
        cleanupThreads();
        
        // Handle client in new thread
        Thread* thread = new Thread();
        thread->start(reinterpret_cast<void*>(&handleClientWrapper), 
                     reinterpret_cast<void*>(this),
                     reinterpret_cast<void*>(client_socket));
        
        {
            LockGuard lock(threads_mutex_);
            client_threads_.push_back(thread);
        }
    }
}

void Server::handleClient(socket_t client_socket) {
    std::string line;
    
    while (running_) {
        // Read command from client
        if (!readLine(client_socket, line)) {
            break;  // Client disconnected or error
        }
        
        // Skip empty lines
        if (line.empty()) {
            continue;
        }
        
        // Check for quit command
        if (line == "quit" || line == "QUIT" || line == "exit" || line == "EXIT") {
            sendResponse(client_socket, "Goodbye!\n");
            break;
        }
        
        // Process command and send response
        std::string response = processCommand(line);
        if (!sendResponse(client_socket, response + "\n")) {
            break;  // Failed to send, client probably disconnected
        }
    }
    
    closesocket(client_socket);
    std::cout << "Client disconnected\n";
}

std::string Server::processCommand(const std::string& command) {
    Command cmd = CommandParser::parse(command);
    
    switch (cmd.type) {
        case CommandType::SET: {
            if (cmd.args.size() < 2) {
                return "ERROR: SET requires key and value";
            }
            const std::string& key = cmd.args[0];
            // Join remaining args as value (allows values with spaces)
            std::string value;
            for (size_t i = 1; i < cmd.args.size(); ++i) {
                if (i > 1) value += " ";
                value += cmd.args[i];
            }
            db_.set(key, value);
            return "OK";
        }
        
        case CommandType::GET: {
            if (cmd.args.empty()) {
                return "ERROR: GET requires a key";
            }
            auto value = db_.get(cmd.args[0]);
            if (value.has_value()) {
                return value.value();
            }
            return "(nil)";
        }
        
        case CommandType::DEL: {
            if (cmd.args.empty()) {
                return "ERROR: DELETE requires a key";
            }
            bool deleted = db_.del(cmd.args[0]);
            return deleted ? "OK" : "(nil)";
        }
        
        case CommandType::EXPIRE: {
            if (cmd.args.size() < 2) {
                return "ERROR: EXPIRE requires key and seconds";
            }
            const std::string& key = cmd.args[0];
            int64_t seconds = std::atoll(cmd.args[1].c_str());
            
            if (seconds < 0) {
                return "ERROR: Invalid expiration time";
            }
            
            bool ok = db_.expire(key, seconds);
            return ok ? "OK" : "ERROR: Key not found";
        }
        
        case CommandType::TTL: {
            if (cmd.args.empty()) {
                return "ERROR: TTL requires a key";
            }
            int64_t ttl = db_.ttl(cmd.args[0]);
            
            if (ttl == -1) {
                return "No expiration";
            } else if (ttl == -2) {
                return "Key not found or expired";
            } else {
                return std::to_string(ttl) + " seconds";
            }
        }
        
        default:
            return "ERROR: Unknown command";
    }
}

bool Server::readLine(socket_t socket, std::string& line) {
    line.clear();
    char ch;
    
    while (true) {
        int bytes = recv(socket, &ch, 1, 0);
        
        if (bytes <= 0) {
            // Connection closed or error
            return false;
        }
        
        if (ch == '\n') {
            // End of line
            break;
        }
        
        if (ch == '\r') {
            // Skip carriage return (handles Windows \r\n)
            continue;
        }
        
        line += ch;
    }
    
    return true;
}

bool Server::sendResponse(socket_t socket, const std::string& response) {
    const char* data = response.c_str();
    size_t total = response.length();
    size_t sent = 0;
    
    while (sent < total) {
        int bytes = send(socket, data + sent, static_cast<int>(total - sent), 0);
        
        if (bytes == SOCKET_ERROR_VALUE) {
            return false;
        }
        
        sent += bytes;
    }
    
    return true;
}

void Server::cleanupThreads() {
    LockGuard lock(threads_mutex_);
    
    // Remove finished threads
    for (auto it = client_threads_.begin(); it != client_threads_.end(); ) {
        if ((*it)->joinable()) {
            (*it)->join();
            delete *it;
            it = client_threads_.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace pebblekv
