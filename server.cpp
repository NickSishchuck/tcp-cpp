#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <chrono>
#include <memory>
#include <algorithm>
#include <errno.h>

using namespace std;

// Global variable for signal handling
atomic<bool> g_shutdown_requested(false);

class TCPServer {
private:
    int listening_socket;
    atomic<bool> running;
    mutex client_mutex;
    vector<unique_ptr<thread>> client_threads;
    const int PORT = 54000;
    const int MAX_CLIENTS = 100;
    const int BUFFER_SIZE = 4096;

public:
    TCPServer() : listening_socket(-1), running(false) {
    }

    ~TCPServer() {
        stop();
    }

    bool start() {
        // Create socket with error checking
        listening_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (listening_socket == -1) {
            cerr << "Failed to create socket: " << strerror(errno) << endl;
            return false;
        }

        // Enable address reuse to avoid "Address already in use" errors
        int opt = 1;
        if (setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            cerr << "Failed to set socket options: " << strerror(errno) << endl;
            close(listening_socket);
            return false;
        }

        // Setup address structure
        sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        server_addr.sin_addr.s_addr = INADDR_ANY;

        // Bind socket
        if (bind(listening_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
            cerr << "Failed to bind socket: " << strerror(errno) << endl;
            close(listening_socket);
            return false;
        }

        // Start listening
        if (listen(listening_socket, MAX_CLIENTS) == -1) {
            cerr << "Failed to listen: " << strerror(errno) << endl;
            close(listening_socket);
            return false;
        }

        running = true;
        cout << "Server started on port " << PORT << endl;
        cout << "Waiting for connections..." << endl;

        return true;
    }

    void run() {
        if (!running) {
            cerr << "Server not started!" << endl;
            return;
        }

        while (running && !g_shutdown_requested) {
            sockaddr_in client_addr;
            socklen_t client_size = sizeof(client_addr);

            // Accept connection - this will fail when socket is closed by signal handler
            int client_socket = accept(listening_socket, (sockaddr*)&client_addr, &client_size);

            if (client_socket == -1) {
                if (running && !g_shutdown_requested) {
                    cerr << "Failed to accept connection: " << strerror(errno) << endl;
                }
                // If running is false, this means we're shutting down, so break
                break;
            }

            // Get client information
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
            int client_port = ntohs(client_addr.sin_port);

            cout << "New connection from " << client_ip << ":" << client_port << endl;

            // Create thread to handle client
            {
                lock_guard<mutex> lock(client_mutex);
                client_threads.emplace_back(make_unique<thread>(&TCPServer::handleClient, this, client_socket, string(client_ip), client_port));
            }

            // Clean up finished threads
            cleanupThreads();
        }
    }

    void stop() {
        if (!running) return;

        running = false;

        // Close listening socket to interrupt accept()
        if (listening_socket != -1) {
            shutdown(listening_socket, SHUT_RDWR);
            close(listening_socket);
            listening_socket = -1;
        }

        // Wait for client threads to finish (with reasonable timeout)
        {
            lock_guard<mutex> lock(client_mutex);
            for (auto& thread_ptr : client_threads) {
                if (thread_ptr && thread_ptr->joinable()) {
                    thread_ptr->detach(); // Let them finish naturally
                }
            }
            client_threads.clear();
        }

        cout << "Server stopped." << endl;
    }

private:
    void handleClient(int client_socket, string client_ip, int client_port) {
        char buffer[BUFFER_SIZE];
        string client_id = client_ip + ":" + to_string(client_port);

        // Send welcome message
        string welcome = "Welcome to  TCP Server!\n";
        send(client_socket, welcome.c_str(), welcome.length(), 0);

        while (running) {
            memset(buffer, 0, BUFFER_SIZE);

            // Receive data with timeout
            int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

            if (bytes_received <= 0) {
                if (bytes_received == 0) {
                    cout << "Client " << client_id << " disconnected" << endl;
                } else {
                    cout << "Error receiving from client " << client_id << ": " << strerror(errno) << endl;
                }
                break;
            }

            // Null-terminate the received data
            buffer[bytes_received] = '\0';
            string message(buffer);

            cout << "From " << client_id << ": " << message;

            // Process commands
            if (message.find("/quit") == 0) {
                string goodbye = "Goodbye!\n";
                send(client_socket, goodbye.c_str(), goodbye.length(), 0);
                break;
            } else if (message.find("/time") == 0) {
                auto now = chrono::system_clock::now();
                auto time_t = chrono::system_clock::to_time_t(now);
                string time_str = "Server time: " + string(ctime(&time_t));
                send(client_socket, time_str.c_str(), time_str.length(), 0);
            } else if (message.find("/help") == 0) {
                string help = "Available commands:\n/help - Show this help\n/time - Get server time\n/quit - Disconnect\n";
                send(client_socket, help.c_str(), help.length(), 0);
            } else {
                // Echo the message back with a prefix
                string response = "Echo: " + message;
                send(client_socket, response.c_str(), response.length(), 0);
            }
        }

        cout << "Closing connection to " << client_id << endl;
        close(client_socket);
    }

    void cleanupThreads() {
        lock_guard<mutex> lock(client_mutex);

        // Remove finished threads
        client_threads.erase(
            remove_if(client_threads.begin(), client_threads.end(),
                [](const unique_ptr<thread>& t) {
                    return t && !t->joinable();
                }),
            client_threads.end()
        );
    }
};

// Signal handler function
void signal_handler(int) {
    cout << "\nShutdown signal received..." << endl;
    g_shutdown_requested = true;
}

int main() {
    TCPServer server;

    if (!server.start()) {
        return -1;
    }

    // Setup signal handler for graceful shutdown
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    cout << "Server running. Press Ctrl+C to stop." << endl;

    // Start server in a separate thread so signal handling works properly
    thread server_thread(&TCPServer::run, &server);

    // Monitor for shutdown signal
    while (!g_shutdown_requested) {
        this_thread::sleep_for(chrono::milliseconds(100));
    }

    cout << "Shutting down server..." << endl;
    server.stop();

    // Wait for server thread to finish
    if (server_thread.joinable()) {
        server_thread.join();
    }

    cout << "Program exited." << endl;
    return 0;
}
