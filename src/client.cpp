#include <iostream>
#include <string>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET socket_t;
    #define INVALID_SOCKET_VALUE INVALID_SOCKET
    #define SOCKET_ERROR_VALUE SOCKET_ERROR
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int socket_t;
    #define INVALID_SOCKET_VALUE -1
    #define SOCKET_ERROR_VALUE -1
    #define closesocket close
#endif

bool readLine(socket_t socket, std::string& line) {
    line.clear();
    char ch;
    
    while (true) {
        int bytes = recv(socket, &ch, 1, 0);
        
        if (bytes <= 0) {
            return false;
        }
        
        if (ch == '\n') {
            break;
        }
        
        if (ch == '\r') {
            continue;
        }
        
        line += ch;
    }
    
    return true;
}

bool sendLine(socket_t socket, const std::string& line) {
    std::string message = line + "\n";
    const char* data = message.c_str();
    size_t total = message.length();
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

int main(int argc, char* argv[]) {
    std::string host = "127.0.0.1";
    uint16_t port = 6379;
    
    // Parse command line arguments
    if (argc > 1) {
        host = argv[1];
    }
    if (argc > 2) {
        port = static_cast<uint16_t>(std::atoi(argv[2]));
    }
    
#ifdef _WIN32
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock\n";
        return 1;
    }
#endif
    
    // Create socket
    socket_t sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET_VALUE) {
        std::cerr << "Failed to create socket\n";
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    
    // Connect to server
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(host.c_str());
    
    if (server_addr.sin_addr.s_addr == INADDR_NONE) {
        std::cerr << "Invalid address: " << host << "\n";
        closesocket(sock);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    
    if (connect(sock, reinterpret_cast<struct sockaddr*>(&server_addr), 
                sizeof(server_addr)) == SOCKET_ERROR_VALUE) {
        std::cerr << "Failed to connect to " << host << ":" << port << "\n";
        std::cerr << "Make sure the server is running.\n";
        closesocket(sock);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    
    std::cout << "Connected to PebbleKV at " << host << ":" << port << "\n";
    std::cout << "Type commands (or 'quit' to exit)\n\n";
    
    // Interactive client loop
    std::string line;
    while (true) {
        std::cout << "pebblekv> ";
        
        if (!std::getline(std::cin, line)) {
            break;
        }
        
        if (line.empty()) {
            continue;
        }
        
        // Send command to server
        if (!sendLine(sock, line)) {
            std::cerr << "Failed to send command\n";
            break;
        }
        
        // Check if we're quitting
        if (line == "quit" || line == "QUIT" || line == "exit" || line == "EXIT") {
            std::string response;
            readLine(sock, response);
            std::cout << response << "\n";
            break;
        }
        
        // Read response from server
        std::string response;
        if (!readLine(sock, response)) {
            std::cerr << "Failed to read response (server disconnected?)\n";
            break;
        }
        
        std::cout << response << "\n";
    }
    
    closesocket(sock);
#ifdef _WIN32
    WSACleanup();
#endif
    
    return 0;
}
