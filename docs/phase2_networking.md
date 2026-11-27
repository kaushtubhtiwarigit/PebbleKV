# Phase 2: TCP Networking

## Overview

Phase 2 adds network access to PebbleKV. Clients can now connect over TCP/IP instead of just using the local command-line interface.

## Why Networking Matters

Databases are almost always accessed over a network. This phase transforms PebbleKV from a local toy into something that resembles a real database server.

## Architecture

### Server Component (`server.h`, `server.cpp`)

**Responsibilities:**
1. Listen for incoming TCP connections on a port
2. Accept client connections
3. Read commands from clients
4. Execute commands using the database
5. Send responses back to clients
6. Handle disconnections gracefully

**Key Design Decisions:**

#### Winsock2 vs POSIX Sockets

We use **conditional compilation** to support both:
- Windows: Winsock2 (`winsock2.h`, `ws2_32.lib`)
- Linux/Unix: POSIX sockets (`sys/socket.h`, `netinet/in.h`)

```cpp
#ifdef _WIN32
    #include <winsock2.h>
    typedef SOCKET socket_t;
#else
    #include <sys/socket.h>
    typedef int socket_t;
#endif
```

This makes the code portable between platforms.

#### Sequential Client Handling

Phase 2 handles **one client at a time**:

```
Client 1 connects → Server processes all Client 1 commands → Client 1 disconnects
                 → Client 2 connects → Server processes Client 2...
```

**Why not concurrent?**
- Simpler to implement and understand
- No race conditions or synchronization needed
- Good for learning the networking basics
- Phase 3 will add multi-client support with threads

#### Text-Based Protocol

Commands and responses are plain text with newline delimiters:

```
Client sends: "SET name John\n"
Server responds: "OK\n"

Client sends: "GET name\n"
Server responds: "John\n"
```

**Why text protocol?**
- Easy to debug (use telnet or netcat)
- Human-readable
- Simple to parse
- Redis uses a similar approach (RESP protocol)

**Drawbacks:**
- Not the most efficient (binary would be faster)
- No pipelining support (send multiple commands at once)
- Future phases can optimize if needed

### Client Component (`client.cpp`)

A simple TCP client for testing:
- Connects to server
- Sends commands from stdin
- Prints responses to stdout
- Handles disconnection

**Not required for the database to work** - any TCP client (telnet, netcat, custom code) can connect.

## Network Flow

### Server Startup

```
1. Create socket          → socket()
2. Bind to port          → bind()
3. Listen for connections → listen()
4. Accept connections     → accept() [blocks]
```

### Client Connection

```
1. Create socket          → socket()
2. Connect to server      → connect()
3. Send/receive data      → send() / recv()
4. Close connection       → closesocket()
```

### Request-Response Cycle

```
┌──────────┐                              ┌──────────┐
│  Client  │                              │  Server  │
└────┬─────┘                              └────┬─────┘
     │                                         │
     │  "SET key value\n"                     │
     │────────────────────────────────────────▶│
     │                                         │
     │                                    Parse command
     │                                    Execute on DB
     │                                    Format response
     │                                         │
     │             "OK\n"                      │
     │◀────────────────────────────────────────│
     │                                         │
```

## Implementation Details

### Reading Lines from Socket

We read **one byte at a time** until we hit '\n':

```cpp
bool Server::readLine(socket_t socket, std::string& line) {
    line.clear();
    char ch;
    while (true) {
        int bytes = recv(socket, &ch, 1, 0);
        if (bytes <= 0) return false;  // Connection closed
        if (ch == '\n') break;
        if (ch == '\r') continue;      // Handle Windows \r\n
        line += ch;
    }
    return true;
}
```

**Why byte-by-byte?**
- Simple and correct
- Handles variable-length commands
- No buffering complexity
- Performance is acceptable for educational purposes

**Production optimization:**
- Use buffered reads (read chunks, scan for '\n')
- Reduces syscall overhead
- We'll consider this in Phase 7 if needed

### Sending Responses

We loop until all bytes are sent:

```cpp
bool Server::sendResponse(socket_t socket, const std::string& response) {
    const char* data = response.c_str();
    size_t total = response.length();
    size_t sent = 0;
    
    while (sent < total) {
        int bytes = send(socket, data + sent, total - sent, 0);
        if (bytes == SOCKET_ERROR_VALUE) return false;
        sent += bytes;
    }
    return true;
}
```

**Why loop?**
- `send()` is not guaranteed to send all bytes in one call
- Especially for large messages
- Must track progress and retry

### Error Handling

**Connection errors:**
- `recv()` returns 0 → Client closed connection gracefully
- `recv()` returns -1 → Network error or client crash
- `send()` fails → Client disconnected mid-response

**Server continues running** even if a client has issues.

### Signal Handling

Ctrl+C sends `SIGINT`:

```cpp
void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        g_server->stop();
    }
}
```

This allows graceful shutdown:
- Close server socket
- Reject new connections
- Exit cleanly

## Windows-Specific Details

### Winsock Initialization

Windows requires initializing the Winsock library:

```cpp
WSADATA wsaData;
WSAStartup(MAKEWORD(2, 2), &wsaData);
```

Must be called **before any socket operations**.

### Linking ws2_32.lib

Windows sockets need the Winsock library:

```
g++ ... -lws2_32
```

### DELETE Macro Conflict

Windows headers define `DELETE` as a macro (for Windows API).

**Solution:** Renamed `CommandType::DELETE` to `CommandType::DEL`

This is a common issue when mixing system headers with application code.

## Testing Phase 2

### Manual Testing

**Terminal 1:**
```
pebblekv_server.exe
```

**Terminal 2:**
```
pebblekv_client.exe
SET key value
GET key
...
```

### With telnet

```
telnet localhost 6379
SET test 123
GET test
DELETE test
quit
```

### Automated Testing

`test_phase2.ps1` starts server, runs commands through client, verifies responses, stops server.

## Performance Characteristics

| Operation | Overhead | Notes |
|-----------|----------|-------|
| SET       | +network round-trip | ~0.1-1ms on localhost |
| GET       | +network round-trip | Same |
| DELETE    | +network round-trip | Same |

**Network adds latency** but throughput is still good for single client.

**Next Phase:** Concurrency will enable handling multiple clients simultaneously.

## Security Considerations (Not Implemented)

Phase 2 has **no security**:
- No authentication - anyone can connect
- No encryption - data sent in plaintext
- No rate limiting - client can flood server
- No input validation beyond command parsing
- Binds to all interfaces (0.0.0.0) - accessible from network

**For learning purposes this is acceptable.** Production databases need:
- TLS/SSL encryption
- Username/password or token authentication
- Connection limits
- Input validation
- Firewall rules

## Common Issues and Solutions

### Issue: "Failed to bind to port 6379"

**Cause:** Port already in use or no permission

**Solutions:**
- Use a different port: `pebblekv_server.exe 7000`
- Kill existing process: `taskkill /F /IM pebblekv_server.exe`
- Use port > 1024 (non-privileged)

### Issue: Client hangs, server not responding

**Cause:** Server not running or firewall blocking

**Solutions:**
- Start server first
- Check firewall settings
- Try `127.0.0.1` explicitly (not `localhost`)

### Issue: "Failed to initialize Winsock"

**Cause:** Windows Winsock library issue

**Solution:**
- Restart Windows (usually fixes it)
- Check if `ws2_32.dll` exists in System32

### Issue: Commands work but responses garbled

**Cause:** Line ending mismatch (Windows \r\n vs Unix \n)

**Solution:**
- Server handles both by skipping '\r'
- Already implemented in `readLine()`

## What We Learned

1. **Socket programming basics** - listen, accept, send, recv
2. **Platform differences** - Windows vs POSIX sockets
3. **Protocol design** - text-based request/response
4. **Error handling** - network failures, disconnections
5. **Blocking I/O** - simple but limits concurrency

## Next Steps: Phase 3

Phase 3 will add **concurrent client support**:
- Multiple clients can connect simultaneously
- Use threads (one per client)
- Add mutex to protect shared database
- Handle race conditions

**Why threads?**
- Simple and understandable
- Good for learning concurrency basics
- Scales to dozens of clients
- Later phases could explore async I/O (epoll/IOCP) for thousands of clients

---

*Phase 2 complete! You now have a network-accessible key-value database.*
