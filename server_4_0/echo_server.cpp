#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <sys/epoll.h>
#include <vector>
#include <functional>
#include <stdexcept>
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <ctime>
#include <memory>
#include <filesystem>
#include <sys/mman.h>
#include "thread_pool.h"
#include "epoll_server.h"
#define SERVER_PORT 8888
#define MAX_EVENTS 512
#define NUM_THREADS 8

using namespace std;
using namespace std::filesystem;

string getTime();
void handleClient(int clientSock, EpollServer& epollServer);
void doHttpResponse(int clientSock, const string& filePath, EpollServer& epollServer);
void sendHeader(int clientSock);
void badRequest(int clientSock);
void notFound(int clientSock);
void notImplemented(int clientSock);
void badGateway(int clientSock);
void getFile(int clientSock, const string& filePath);

int main() {
	// 创建 epoll 实例
	EpollServer epollServer(MAX_EVENTS);

	// 创建服务器套接字
	int serverSock = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSock == -1) {
		cerr << "Failed to create server socket" << endl;
		return 1;
	}

	// 设置服务器套接字为非阻塞模式
	if (fcntl(serverSock, F_SETFL, fcntl(serverSock, F_GETFL) | O_NONBLOCK) == -1) {
		cerr << "Failed to set server socket to non-blocking mode" << endl;
		close(serverSock);
		return 1;
	}

	// 绑定地址和端口
	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(SERVER_PORT);
	if (bind(serverSock, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
		cerr << "Failed to bind server socket" << endl;
		close(serverSock);
		return 1;
	}

	// 监听连接
	if (listen(serverSock, SOMAXCONN) == -1) {
		cerr << "Failed to listen on server socket" << endl;
		close(serverSock);
		return 1;
	}

	// if (listen(serverSock, 128) == -1) {
	// 	cerr << "Failed to listen on server socket" << endl;
	// 	close(serverSock);
	// 	return 1;
	// }

	if (!epollServer.AddFd(serverSock, EPOLLIN | EPOLLET)) {
		cerr << "Failed to add server socket to epoll" << endl;
		close(serverSock);
		return 1;
	}

	cout << "Waiting for client connections..." << endl;

	// 创建线程池
	ThreadPool threadPool(NUM_THREADS);

	// 定义事件循环
	while (true) {
		int numEvents = epollServer.Wait(-1);
		if (numEvents == -1) {
			cerr << "epoll_wait failed: " << strerror(errno) << endl;
			break;
		}

		for (int i = 0; i < numEvents; ++i) {
			int fd = epollServer.GetEventFd(i);
			uint32_t events = epollServer.GetEvents(i);

			if (fd == serverSock) {
				// 接受新的客户端连接
				while (true) {
					struct sockaddr_in clientAddr;
					socklen_t clientAddrLen = sizeof(clientAddr);
					int clientSock = accept(serverSock, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientAddrLen);
					if (clientSock == -1) {
						if (errno == EAGAIN || errno == EWOULDBLOCK) // 已经没有连接请求
							break;
						else {
							cerr << "Failed to accept client connection: " << strerror(errno) << endl;
							continue;
						}
					}

					// 打印时间和客户端连接的IP地址与端口号
					cout << "**************************" << endl;
					cout << "time : " << getTime() << endl;
					cout << "Client connected from: " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << endl;

					// 将新客户端套接字设置为非阻塞模式
					if (fcntl(clientSock, F_SETFL, fcntl(clientSock, F_GETFL) | O_NONBLOCK) == -1) {
						cerr << "Failed to set client socket to non-blocking mode: " << strerror(errno) << endl;
						close(clientSock);
						continue;
					}

					if (!epollServer.AddFd(clientSock, EPOLLIN | EPOLLET)) {
						cerr << "Failed to add client socket to epoll: " << strerror(errno) << endl;
						close(clientSock);
						continue;
					}
				}
			}
			else {
				// 已连接客户端有数据可读
				if (events & EPOLLIN) {
					threadPool.Enqueue([fd, &epollServer]() {
						handleClient(fd, epollServer);
						});
				}
			}
		}
	}

	close(serverSock);
	return 0;
}


string getTime() {
	time_t now = time(nullptr);
	struct tm* timeinfo = localtime(&now);
	char buffer[80];
	strftime(buffer, sizeof(buffer), "%Y/%m/%d %H:%M:%S", timeinfo);
	return string(buffer);
}

void handleClient(int clientSock, EpollServer& epollServer) {
	char buffer[1024];
	ssize_t bytesRead = read(clientSock, buffer, sizeof(buffer));
	if (bytesRead <= 0) {
		if (bytesRead == 0) {
			cout << "Client closed connection" << endl;
		}
		else {
			cerr << "Failed to read from client socket: " << strerror(errno) << endl;
		}
		close(clientSock);
		return;
	}

	string request(buffer, bytesRead);
	string requestedUrl;
	string requestedFilePath = "../html_docs";

	size_t i = 0;
	if (request.substr(0, 3) == "GET") {
		i = request.find(' ') + 1;
		size_t j = request.find(' ', i);
		requestedUrl = request.substr(i, j - i);
		requestedFilePath += requestedUrl;
		cout << "Requested file path: " << requestedFilePath << endl;
		doHttpResponse(clientSock, requestedFilePath, epollServer);
	}
	else {
		cerr << "***400***" << endl;
		badRequest(clientSock);

	}
	// 重置 EPOLLONESHOT 事件
	if (!epollServer.ModFd(clientSock, EPOLLIN | EPOLLET | EPOLLONESHOT)) {
		cerr << "Failed to reset EPOLLONESHOT event for client socket" << endl;
	}
	// You may check the request or response headers here to determine if the connection should be closed.
	// For example, if a request includes "Connection: close", or if you respond with "Connection: close", then close the connection.
	// Here's a simplified example:
	bool closeConnection = request.find("Connection: close") != string::npos;
	if (closeConnection) {
		cout << "Closing connection as requested by client" << endl;
		close(clientSock);
		return;
	}
}

void doHttpResponse(int clientSock, const string& filePath, EpollServer& epollServer) {
	try {
		file_status status = filesystem::status(filePath);
		sendHeader(clientSock);
		if (exists(status) && is_regular_file(status)) {
			getFile(clientSock, filePath);
			cout << "Content size: " << file_size(filePath) << endl;
		}
		else {
			cout << "***404***" << endl;
			notFound(clientSock);
		}
	}
	catch (const exception& ex) {
		cout << "***502***" << endl;
		cerr << "Exception occurred: " << ex.what() << endl;
		badGateway(clientSock);
	}
	return;
}

// ***400**
void badRequest(int clientSock) {
	string filePath = "../html_docs/400.html";
	getFile(clientSock, filePath);

}
// ****404***
void notFound(int clientSock) {
	string filePath = "../html_docs/404.html";
	getFile(clientSock, filePath);

}
// ***501***
void notImplemented(int clientSock) {
	string filePath = "../html_docs/501.html";
	getFile(clientSock, filePath);

}
// ***502***
void badGateway(int clientSock) {
	string filePath = "../html_docs/502.html";
	getFile(clientSock, filePath);

}

void sendHeader(int clientSock) {
	string header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: keep-alive\r\nAccess-Control-Allow-Origin: *\r\n\r\n";
	ssize_t bytesSent = send(clientSock, header.c_str(), header.length(), 0);
	if (bytesSent != static_cast<ssize_t>(header.length())) {
		cerr << "Failed to send header to client socket" << endl;
	}
}
void getFile(int clientSock, const string& filePath) {
	int fd = open(filePath.c_str(), O_RDONLY);
	if (fd == -1) {
		std::cerr << "Failed to open file: " << filePath << std::endl;
		// 发送 404 页面
		return;
	}

	// 获取文件大小
	struct stat fileInfo;
	if (fstat(fd, &fileInfo) == -1) {
		perror("Failed to get file size");
		close(fd);
		// 发送 404 页面
		return;
	}
	off_t fileSize = fileInfo.st_size;

	// 将文件映射到内存中
	char* fileData = static_cast<char*>(mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fd, 0));
	if (fileData == MAP_FAILED) {
		perror("Failed to mmap file");
		close(fd);
		// 发送 404 页面
		return;
	}

	// 将文件内容发送到客户端套接字
	ssize_t bytesSent = send(clientSock, fileData, fileSize, 0);
	if (bytesSent != fileSize) {
		cerr << "Failed to send file to client socket" << endl;
	}

	// 解除文件映射
	munmap(fileData, fileSize);

	// 关闭文件描述符
	close(fd);
}