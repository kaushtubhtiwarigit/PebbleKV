#include "server.h"
#include <iostream>
#include <cstdlib>
#include <csignal>

using namespace pebblekv;

// Global server pointer for signal handler
Server* g_server = nullptr;

// Signal handler for graceful shutdown (Ctrl+C)
void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\nShutting down server...\n";
        if (g_server) {
            g_server->stop();
        }
    }
}

int main(int argc, char* argv[]) {
    // Parse port from command line (default 6379)
    uint16_t port = 6379;
    if (argc > 1) {
        port = static_cast<uint16_t>(std::atoi(argv[1]));
        if (port == 0) {
            std::cerr << "Invalid port number\n";
            return 1;
        }
    }
    
    std::cout << "PebbleKV v0.5 - Phase 5: Expiration & LRU\n";
    std::cout << "==========================================\n\n";
    
    // Create and start server
    Server server(port);
    g_server = &server;
    
    // Enable persistence
    if (!server.enablePersistence("data/pebblekv.wal")) {
        std::cerr << "Warning: Failed to enable WAL (continuing without persistence)\n";
    }
    
    // Setup signal handlers for graceful shutdown
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    if (!server.start()) {
        std::cerr << "Failed to start server\n";
        return 1;
    }
    
    // Run server (blocks until stopped)
    server.run();
    
    std::cout << "Server stopped\n";
    return 0;
}
