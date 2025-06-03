#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <thread>

using namespace std;

class TCPClient {
private:
    int sock;
    bool connected;

public:
    TCPClient() : sock(-1), connected(false) {}

    ~TCPClient() {
        disconnect();
    }

    bool connect_to_server(const string& ip = "127.0.0.1", int port = 54000) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
            cerr << "Could not create socket" << endl;
            return false;
        }

        sockaddr_in server;
        server.sin_addr.s_addr = inet_addr(ip.c_str());
        server.sin_family = AF_INET;
        server.sin_port = htons(port);

        if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
            cerr << "Connection failed" << endl;
            close(sock);
            return false;
        }

        connected = true;
        cout << "Connected to server " << ip << ":" << port << endl;

        // Start receiving thread
        thread receive_thread(&TCPClient::receive_messages, this);
        receive_thread.detach();

        return true;
    }

    void send_message(const string& message) {
        if (!connected) {
            cerr << "Not connected to server" << endl;
            return;
        }

        if (send(sock, message.c_str(), message.length(), 0) < 0) {
            cerr << "Send failed" << endl;
            connected = false;
        }
    }

    void disconnect() {
        if (connected) {
            connected = false;
            close(sock);
            cout << "Disconnected from server" << endl;
        }
    }

    void run() {
        string message;
        cout << "Type messages (or 'quit' to exit):" << endl;

        while (connected && getline(cin, message)) {
            if (message == "quit") {
                send_message("/quit\n");
                break;
            }

            message += "\n";
            send_message(message);
        }
    }

private:
    void receive_messages() {
        char buffer[4096];

        while (connected) {
            memset(buffer, 0, 4096);
            int bytes_received = recv(sock, buffer, 4096, 0);

            if (bytes_received <= 0) {
                connected = false;
                cout << "Server disconnected" << endl;
                break;
            }

            cout << "Server: " << buffer;
        }
    }
};

int main() {
    TCPClient client;

    if (!client.connect_to_server()) {
        return -1;
    }

    client.run();

    return 0;
}
