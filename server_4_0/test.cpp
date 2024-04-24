#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <mutex>
#include <fstream>
#include <iomanip>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 8888
#define NUM_CLIENTS 5000// 要创建的客户端数量

std::atomic<int> successfulConnections(0);
std::atomic<int> successfulRequests(0);
std::atomic<int> failedConnections(0);
std::mutex mutex;
std::chrono::steady_clock::time_point testStartTime;
std::chrono::steady_clock::time_point testEndTime;

void testConnection() {
    int clientSock = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSock == -1) {
        // std::cerr << "Failed to create client socket" << std::endl;
        failedConnections++;
        return;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_ADDRESS, &serverAddr.sin_addr);

    if (connect(clientSock, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
        std::lock_guard<std::mutex> lock(mutex);
        // std::cerr << "Failed to connect to server: " << strerror(errno) << std::endl;
        failedConnections++;
    }
    else {
        std::lock_guard<std::mutex> lock(mutex);
        // std::cout << "Successfully connected to server" << std::endl;
        successfulConnections++;

        // 发送HTTP GET请求
        std::string httpRequest = "GET /index.html HTTP/1.1\r\nHost: " + std::string(SERVER_ADDRESS) + "\r\nConnection: close\r\n\r\n";
        if (send(clientSock, httpRequest.c_str(), httpRequest.size(), 0) == -1) {
            //std::cerr << "Failed to send HTTP GET request: " << strerror(errno) << std::endl;
            close(clientSock);
            return;
        }

        // 接收服务器响应
        char buffer[1024];
        ssize_t bytesRead;
        std::ofstream outputFile("index.html", std::ios::out | std::ios::binary);
        while ((bytesRead = recv(clientSock, buffer, sizeof(buffer), 0)) > 0) {
            outputFile.write(buffer, bytesRead);
        }

        if (bytesRead == -1) {
            //std::cerr << "Failed to receive server response: " << strerror(errno) << std::endl;
        }
        else {
            successfulRequests++;
        }

        outputFile.close();
    }

    close(clientSock);
}

int main() {
    std::vector<std::thread> threads;

    testStartTime = std::chrono::steady_clock::now();
    std::cout << "Testing..." << std::endl;
    for (int i = 0; i < NUM_CLIENTS; ++i) {
        threads.emplace_back(testConnection);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    testEndTime = std::chrono::steady_clock::now();

    int totalConnections = successfulConnections + failedConnections;
    double successRate = (double)successfulConnections / totalConnections * 100;

    auto testDuration = std::chrono::duration_cast<std::chrono::milliseconds>(testEndTime - testStartTime).count();

    double connectionsPerSecond = (double)successfulConnections / testDuration * 1000; // 转换为每秒连接数
    double requestsPerSecond = (double)successfulRequests / testDuration * 1000; // 转换为每秒请求数

    std::cout << "Total connections attempted: " << NUM_CLIENTS << std::endl;
    std::cout << "Successful connections: " << successfulConnections << std::endl;
    std::cout << "Failed connections: " << failedConnections << std::endl;
    std::cout << "Successful requests: " << successfulRequests << std::endl;
    std::cout << "Total test duration: " << testDuration << " milliseconds" << std::endl;
    std::cout << "Connections per second: " << std::fixed << std::setprecision(2) << connectionsPerSecond << std::endl;
    std::cout << "Requests per second: " << std::fixed << std::setprecision(2) << requestsPerSecond << std::endl;

    return 0;
}
