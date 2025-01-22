#include <iostream>
#include <stdio.h>
#include <string.h>
#include <limits>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <vector>
#include <unistd.h> // for close()
#include <algorithm> // for std::remove

using namespace std;

#define BUFFER_LEN 1024
#define NAME_LEN 20

vector<int> clients; // List of client sockets
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for thread safety

void* handle_client(void* data) {
    int client_socket = *(int*)data;
    char buffer[BUFFER_LEN + 1] = {0};

    // Receive client's name
    recv(client_socket, buffer, NAME_LEN, 0);
    string client_name(buffer);
    cout << client_name << " has joined the chat." << endl;

    while (int buffer_len = recv(client_socket, buffer, BUFFER_LEN, 0)) {
        if (buffer_len <= 0) break;

        buffer[buffer_len] = '\0';
        string message(buffer);
        cout << "Received message: " << message << endl; // Debug information

        // Echo message back to all clients
        pthread_mutex_lock(&clients_mutex);
        for (int client : clients) {
            if (client != client_socket) {
                if (send(client, message.c_str(), message.length(), 0) < 0) {
                    perror("send"); // Debug information
                } else {
                    cout << "Sent message to client: " << client << endl; // Debug information
                }
            }
        }
        pthread_mutex_unlock(&clients_mutex);
        memset(buffer, 0, sizeof(buffer));
    }

    cout << client_name << " has left the chat." << endl;

    // Remove client from the list
    pthread_mutex_lock(&clients_mutex);
    clients.erase(remove(clients.begin(), clients.end(), client_socket), clients.end());
    pthread_mutex_unlock(&clients_mutex);

    close(client_socket);
    return nullptr;
}

int main() {
    int server_sock;
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    int server_port = 8888; // Ensure this matches the port you use in the client
    addr.sin_port = htons(server_port);

    if (bind(server_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return -1;
    }

    if (listen(server_sock, 5) < 0) {
        perror("listen");
        return -1;
    }

    cout << "Server is listening on port " << server_port << endl;

    while (true) {
        int client_sock;
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        if ((client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_len)) < 0) {
            perror("accept");
            continue;
        }

        // Add new client to the list
        pthread_mutex_lock(&clients_mutex);
        clients.push_back(client_sock);
        pthread_mutex_unlock(&clients_mutex);

        pthread_t client_thread;
        pthread_create(&client_thread, nullptr, handle_client, (void*)&client_sock);
    }

    close(server_sock);
    return 0;
}
