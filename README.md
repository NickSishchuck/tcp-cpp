# TCP Server/Client in C++

A high-performance, multi-threaded TCP server and client implementation C++20. This project demonstrates socket programming, concurrent client handling, and graceful resource management.

## Features

### Server
- **Multi-client support** - Handle multiple simultaneous connections using threading
- **Graceful shutdown** - Proper signal handling (SIGINT/SIGTERM) with resource cleanup
- **Built-in commands** - Interactive command system for clients
- **Robust error handling** - Comprehensive error checking and logging
- **Thread-safe operations** - Safe concurrent access to shared resources
- **Automatic cleanup** - Memory and thread management

### Client
- **Interactive interface** - Real-time message sending and receiving
- **Threaded I/O** - Separate threads for sending and receiving messages
- **Command support** - Built-in commands for server interaction

## Built-in Commands

The server supports several interactive commands:

| Command | Description |
|---------|-------------|
| `/help` | Show available commands |
| `/time` | Get current server time |
| `/quit` | Disconnect from server |

## Requirements

- **C++20 compatible compiler** (GCC 10+, Clang 10+, MSVC 2019+)
- **CMake 3.20+**
- **POSIX-compliant system** (Linux, macOS, WSL)
- **pthread library** (usually included)

## Building

### Using CMake (Recommended)

```bash
# Clone or download the project
git clone <https://github.com/NickSishchuck/tcp-cpp.git>
cd tcp-cpp

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make

```

## Usage

### Starting the Server

```bash
# From build directory
./server

# Or if using custom output directory
./bin/server
```

The server will start listening on port `54000` and display:
```
Server started on port 54000
Waiting for connections...
```

### Connecting Clients

In separate terminal windows, start one or more clients:

```bash
./client
# or
./bin/client
```

Successful connection shows:
```
Connected to server 127.0.0.1:54000
Server: Welcome to Enhanced TCP Server!
Type messages (or 'quit' to exit):
```

### Example Session

**Client Input:**
```
Hello Server!
/time
How are you?
/help
/quit
```

**Server Output:**
```
New connection from 127.0.0.1:45678
From 127.0.0.1:45678: Hello Server!
From 127.0.0.1:45678: /time
From 127.0.0.1:45678: How are you?
From 127.0.0.1:45678: /help
From 127.0.0.1:45678: /quit
Closing connection to 127.0.0.1:45678
```


## Configuration

Default settings can be modified in the source code:

```cpp
// In TCPServer class
const int PORT = 54000;           // Server port
const int MAX_CLIENTS = 100;      // Maximum concurrent clients
const int BUFFER_SIZE = 4096;     // Message buffer size
```

```cpp
// In TCPClient class
string ip = "127.0.0.1";          // Server IP
int port = 54000;                 // Server port
```


## Development

### Adding New Commands

To add a new server command:

1. **Add command handling** in `handleClient()` method:
```cpp
else if (message.find("/newcmd") == 0) {
    string response = "New command response\n";
    send(client_socket, response.c_str(), response.length(), 0);
}
```

2. **Update help text** to include the new command.

### Error Handling

The server includes error handling for:
- Socket creation failures
- Bind/listen errors
- Client connection issues
- Thread management problems
- Signal handling

### Thread Safety

- **Mutex protection** for shared data structures
- **Atomic variables** for shutdown signaling
- **RAII principles** for resource management
- **Smart pointers** for automatic memory management

## Testing

### Basic Functionality Test

1. Start the server
2. Connect multiple clients simultaneously
3. Send messages from different clients
4. Use built-in commands (`/help`, `/time`, `/quit`)
5. Test graceful shutdown (Ctrl+C on server)

### Load Testing

```bash
# Start server
./server &

# Connect multiple clients in background
for i in {1..10}; do
    echo "test message" | ./client &
done

# Monitor server output for concurrent handling
```

## Troubleshooting

### Common Issues

**"Address already in use" error:**
```bash
# Wait a moment and retry, or check for existing processes
lsof -i :54000
kill -9 <process_id>
```

**Permission denied:**
```bash
# Use a port > 1024 or run with elevated privileges
sudo ./server
```

**Client connection refused:**
- Ensure server is running
- Check firewall settings
- Verify IP address and port


## Future Enhancements

- [ ] Database connectivity
- [ ] Protocol buffer message format
- [ ] WebSocket support
- [ ] Load balancing capabilities
- [ ] Authentication system
- [ ] Client reconnection logic

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request
