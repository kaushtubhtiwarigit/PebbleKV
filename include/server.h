#pragma once

#include "database.h"
#include "thread_compat.h"
#include <string>
#include <cstdint>
#include <vector>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET socket_t;
    #define INVALID_SOCKET_VALUE INVALID_SOCKET
    #define SOCKET_ERROR_VALUE SOCKET_ERROR
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    typedef int socket_t;
    #define INVALID_SOCKET_VALUE -1
    #define SOCKET_ERROR_VALUE -1
    #define closesocket close
#endif

namespace pebblekv {

class Server {
public:
    explicit Server(uint16_t port = 6379);
    ~Server();
    
    // Start the server and listen for connections
    // Returns false if server fails to start
    bool start();
    
    // Stop the server and cleanup
    void stop();
    
    // Main server loop - accepts and handles clients concurrently
    void run();
    
    // Enable WAL and replay if exists
    bool enablePersistence(const std::string& wal_filename = "data/pebblekv.wal");

    // Check if server is running
    bool isRunning() const { return running_; }

private:
    // Handle a single client connection (runs in separate thread)
    void handleClient(socket_t client_socket);
    
    // Process a command from client and return response
    std::string processCommand(const std::string& command);
    
    // Read a line from socket (until \n)
    bool readLine(socket_t socket, std::string& line);
    
    // Send response to client
    bool sendResponse(socket_t socket, const std::string& response);
    
    // Clean up finished threads
    void cleanupThreads();
    
    Database db_;
    socket_t server_socket_;
    uint16_t port_;
    AtomicBool running_;
    
    // Thread management
    std::vector<Thread*> client_threads_;
    Mutex threads_mutex_;
    
    #ifdef _WIN32
    bool winsock_initialized_;
    #endif
};

} // namespace pebblekv
