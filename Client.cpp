#include <iostream>
#include <stdio.h>
#include <string.h>
#include <limits>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h> // for close()
#include <vector>   // for std::vector

using namespace std;

#define BUFFER_LEN 1024
#define NAME_LEN 20

char name[NAME_LEN + 1]; // Client's name

// Function to receive messages and print them out
void* handle_recv(void* data) {
    int client_socket = *(int*)data;
    std::vector<char> message_buffer; // Use std::vector
    char buffer[BUFFER_LEN + 1] = {0};

    while (int buffer_len = recv(client_socket, buffer, BUFFER_LEN, 0)) {
        if (buffer_len <= 0) break; // Exit if server is offline

        for (int i = 0; i < buffer_len; i++) {
            message_buffer.push_back(buffer[i]);
            if (buffer[i] == '\n') {
                cout << "Received message: " << string(message_buffer.begin(), message_buffer.end()); // Debug information
                message_buffer.clear();
            }
        }
        memset(buffer, 0, sizeof(buffer));
    }

    cout << "The Server has been shutdown!" << endl;
    return nullptr;
}

int main() {
    int client_sock;
    if ((client_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    char server_ip[16];
    int server_port;

    cout << "Please enter IP address of the server: ";
    cin >> server_ip;
    cout << "Please enter port number of the server: ";
    cin >> server_port;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    addr.sin_port = htons(server_port);
    addr.sin_addr.s_addr = inet_addr(server_ip);

    if (connect(client_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(client_sock);
        return -1;
    }

    cout << "Connected to the server!" << endl;

    // Get client name
    cout << "Please enter your name: ";
    cin.getline(name, NAME_LEN);
    int name_len = strlen(name);

    if (send(client_sock, name, name_len, 0) < 0) {
        perror("send");
        close(client_sock);
        return -1;
    }

    pthread_t recv_thread;
    pthread_create(&recv_thread, nullptr, handle_recv, (void*)&client_sock);

    while (true) {
        char message[BUFFER_LEN + 1];
        cin.getline(message, BUFFER_LEN);
        int n = strlen(message);

        if (n == 0) {
            cout << "You need to input at least one word!" << endl;
            continue;
        }

        // Prepend the username to the message
        string full_message = string(name) + ": " + string(message) + "\n";

        if (send(client_sock, full_message.c_str(), full_message.length(), 0) < 0) {
            perror("send");
            close(client_sock);
            return -1;
        } else {
            cout << "Sent message: " << full_message; // Debug information
        }
    }

    pthread_cancel(recv_thread);
    shutdown(client_sock, SHUT_RDWR);
    close(client_sock);
    return 0;
}
