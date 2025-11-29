#include <iostream>
#include <string>
#include <cstring>
#include <map>
#include <vector>
#include <sstream>
// 获取时间戳
#include <ctime>
// 网络连接部分
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <cstdint>
// 生成服务器日志
#include <fstream>
#include <iomanip>
// 线程库
#include <thread>
// 消息封装类（服务器专用版本，不依赖Qt）
#include "chatMsg_server.hpp"
// 需要链接WinSock库
// #pragma comment(lib, "ws2_32.lib") 非MSVC编译器无法识别，必须手动链接Winsock库
// 服务端已经用CMake链接


// TODO：获取服务器ip地址的函数

// 设置服务器端口号
const int PORT = 8888;
// 设置listen连接队列长度
const int BACKLOG = 5;


// 为Packet添加Windows socket支持的补充函数
// chatmsg中recv和send是用的qt封装的函数，由于server不依赖qt，只能重新写一个winsock版本的

// 接收数据包函数
bool RecvPacket(SOCKET sock, Packet& packet) {
    // 先接收Header（Header的定义在chatmsg.hpp中）
    char headerBuf[sizeof(Header)]; // 取得包头长度
    int totalReceived = 0; 
    while (totalReceived < sizeof(Header)) { // 循环接收包头
        int n = recv(sock, headerBuf + totalReceived, sizeof(Header) - totalReceived, 0);
        if (n <= 0) return false;
        totalReceived += n;
    }
    
    // 解析header获取body大小
    Header* hdr = reinterpret_cast<Header*>(headerBuf);
    size_t bodySize = n2h16(hdr->field1Len) + n2h16(hdr->field2Len) + 
                      n2h16(hdr->field3Len) + n2h16(hdr->field4Len);
    
    // 接收完整数据包
    std::vector<char> fullPacket(sizeof(Header) + bodySize);
    memcpy(fullPacket.data(), headerBuf, sizeof(Header));
    
    totalReceived = 0;
    while (totalReceived < bodySize) {
        int n = recv(sock, fullPacket.data() + sizeof(Header) + totalReceived, 
                     bodySize - totalReceived, 0);
        if (n <= 0) return false;
        totalReceived += n;
    }
    
    // 使用parseFrom解析
    return packet.parseFrom(fullPacket.data(), fullPacket.size());
}

// 发送数据包函数
bool SendPacket(SOCKET sock, const Packet& packet) {
    // 准备网络字节序的包头
    Header networkHeader;
    memcpy(&networkHeader, packet.data(), sizeof(Header));
    networkHeader.field1Len = h2n16(networkHeader.field1Len);
    networkHeader.field2Len = h2n16(networkHeader.field2Len);
    networkHeader.field3Len = h2n16(networkHeader.field3Len);
    networkHeader.field4Len = h2n16(networkHeader.field4Len);
    
    // 发送header
    if (send(sock, reinterpret_cast<const char*>(&networkHeader), sizeof(Header), 0) != sizeof(Header)) {
        return false;
    }
    
    // 发送各个field
    const char* dataPtr = packet.data() + sizeof(Header);
    size_t bodySize = packet.size() - sizeof(Header);
    if (bodySize > 0) {
        if (send(sock, dataPtr, bodySize, 0) != bodySize) {
            return false;
        }
    }
    return true;
}

// 获取时间戳
std::string TimeStamp() {
    time_t currentTime = time(nullptr); //不是可直接阅读的日历时间
    tm* localTime = localtime(&currentTime); //转换为服务器本地时间
    std::stringstream ss; // 开始生成时间戳
    ss << "[" << std::put_time(localTime, "%Y-%m-%d %H:%M:%S") << "]";
    return ss.str();
}


// 初始化服务器日志
std::ofstream logFile; // 初始化文件句柄

std::string FileNameGen() { // 将日志以当前日期命名,逻辑与获取时间戳函数一致
    time_t currentTime = time(nullptr);
    tm* localTime = localtime(&currentTime);
    std::stringstream fname;
    fname << std::put_time(localTime, "%Y%m%d");
    fname << ".log";
    return fname.str();
}

// 日志写入函数
enum class LogLevel { // 限定日志级别
    FATAL_LEVEL, // 系统崩溃或不可恢复的错误
    ERROR_LEVEL, // 运行时错误，功能受影响但程序仍在运行
    WARN_LEVEL, // 潜在的问题或异常情况
    INFO_LEVEL, // 程序正常运行的关键步骤
    DEBUG_LEVEL, // 详细的调试信息
    TRACE_LEVEL // 追踪细致操作
};

std::string LevelToString(LogLevel level) { // 将日志级别转为字符串
    switch (level) {
        case LogLevel::FATAL_LEVEL: return "FATAL";
        case LogLevel::ERROR_LEVEL: return "ERROR";
        case LogLevel::WARN_LEVEL: return "WARN";
        case LogLevel::INFO_LEVEL: return "INFO";
        case LogLevel::DEBUG_LEVEL: return "DEBUG";
        case LogLevel::TRACE_LEVEL: return "TRACE";
        default: return "UNKNOWN";
    }
}

void WriteLog(LogLevel level, const std::string& message) { //包含两个参数：重要级，消息内容
    if (logFile.is_open()) {
        logFile << TimeStamp() << "[" << LevelToString(level) << "]" << message << std::endl;
        logFile.flush(); //立即刷新缓冲区
    }
}

void InitializeLogFile() { // 生成文件
    const std::string LOG_FOLDER = "log/";
    std::string logfilename = LOG_FOLDER + FileNameGen();
    logFile.open(logfilename, std::ios::out | std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "无法打开日志文件：" << logfilename << std::endl; // 此时无法写入日志（当然）
    } else {
        std::string message = "日志文件初始化成功：" + logfilename;
        WriteLog(LogLevel::INFO_LEVEL, message);
    }
}

void CloseLogFile() {
    if (logFile.is_open()) {
        WriteLog(LogLevel::INFO_LEVEL, "关闭日志文件");
        logFile.close();
    }
}


// 初始化和清理Winsock
bool InitializeWinSock() {
    WSADATA wsadata;
    if (WSAStartup(MAKEWORD(2,2), &wsadata) !=0) {
        std::string errormessage = "WSA启动失败:" + std::to_string(WSAGetLastError());
        WriteLog(LogLevel::FATAL_LEVEL, errormessage);
        return false;
    }
    WriteLog(LogLevel::DEBUG_LEVEL, "WinSock 2.2 初始化成功");
    return true;
}

void CleanupWinSock() {
    WSACleanup();
    WriteLog(LogLevel::DEBUG_LEVEL, "WinSock 清理完成");
}


// 配置服务器地址
int SetupServerAddress(const int port, sockaddr_in& address_info) {
    if (port <= 0 || port > 65535) {
        WriteLog(LogLevel::FATAL_LEVEL, "配置的端口号无效");
        return -1;
    }
    memset(&address_info, 0, sizeof(address_info)); // 清零结构体
    address_info.sin_family = AF_INET; // 设置地址族为IPv4
    address_info.sin_addr.s_addr = htonl(INADDR_ANY); //设置IP地址：监听所有接口（转换为网络字节序）
    address_info.sin_port = htons(port); // 设置端口号（转换为网络字节序）
    return 0;
}


// 用户会话类（注意这个类里不包含id，密码，这个类仅仅用于表示这个用户的网络连接属性）
class ClientSession {
public:
    // 网络属性
    SOCKET socket_fd;
    std::string client_ip;
    unsigned short client_port;
    // 在线状态
    bool is_online;

public:
    // 构造函数：初始化用户属性
    ClientSession(SOCKET fd, const std::string& ip, unsigned short port)
        : socket_fd(fd), 
        client_ip(ip), 
        client_port(port), 
        is_online(false) // 初始未认证

    { // 向日志中记录用户连接
        std::string logmessage = "客户端连接: IP = " + client_ip + ", 端口 = " + std::to_string(client_port);
        WriteLog(LogLevel::INFO_LEVEL, logmessage);
    }

    // 控制在线状态
    void online(bool onlineState) {
        if (onlineState == true) {
            this->is_online = true;
        } else {
            this->is_online = false;
        }
    }

    // 更新socket（用于重连时）
    void updateSocket(SOCKET newSocket) {
        this->socket_fd = newSocket;
        this->is_online = true;
        WriteLog(LogLevel::INFO_LEVEL, "会话对象更新socket: IP = " + client_ip);
    }

};

/*
这个用户会话类的使用逻辑是这样的：我先使用accept连接客户端，然后我会为它分配一个对应这个会话类的线程。这个线程
仅仅是用于处理它与客户端之间的通信的，与他登录什么账号没有关系。验证账号密码，通过id识别会话对象，这些通过下面的map
来实现。
*/

/*
注意：现在一旦服务器关停，所有的账户信息都会被抹除，如果想要永久记录，除非手
动输入，否则可能需要通过构建数据库例如使用SQLite，然而这样工作量就太大了，因此
目前只能先这样了。当然也可以通过读取文件的方式在重启服务器的时候重新创建，然而
那样并不是一个很好的解决方案，如果用户量增多，重新创建用户对象的时间会难以估量
*/



// 存储ID与密码（ID统一为uint8长度）m
std::map<uint8_t, std::string> g_userCredentials = {

};

//将ID与用户对象对应
std::map<uint8_t, ClientSession*> g_userSessions = {

};

// 将IP地址与会话对象对应（用于持久化会话，同一IP重连时重用会话对象）
std::map<std::string, ClientSession*> g_ipToSession = {

};


// 验证登录凭证函数
bool AuthenticateCredential(uint8_t userId, const std::string &inputPassword) {
    if (g_userCredentials.count(userId)) { // 检查map中是否存有该用户ID
        if (g_userCredentials[userId] == inputPassword) { // 如果存在，比较密码
            return true;
        }
    }
    return false;
}


// 工作线程入口函数：为每个客户端分配独立线程处理消息
void HandleClient(ClientSession* sessionPtr) { // 这个会话指针（sessionPtr)作为一个客户端在内存中的唯一代表
    SOCKET clientSocket = sessionPtr->socket_fd;
    std::string clientInfo = sessionPtr->client_ip + ":" + std::to_string(sessionPtr->client_port); // 读取这个连接的ip和端口
    WriteLog(LogLevel::DEBUG_LEVEL, "客户端处理线程启动: " + clientInfo);
    
    // 消息接收循环，持续接收并处理客户端消息
    while (true) {
        Packet receivedPacket; // 创建数据包对象
        
        // 接收完整数据包并进行错误处理。在接收或断开前会阻塞在这一句。
        if (!RecvPacket(clientSocket, receivedPacket)) {
            // 连接断开或接收失败
            WriteLog(LogLevel::INFO_LEVEL, "客户端断开连接: " + clientInfo);
            break;
        }
        
        // 根据消息类型分别处理
        switch (receivedPacket.type()) {
            // 登录请求
            case MsgType::LoginReq: {
                // 从header获取userId，从field2获取密码
                uint8_t userId = receivedPacket.getsendid();
                std::string password = receivedPacket.getField2Str();
                
                WriteLog(LogLevel::INFO_LEVEL, "登录请求 - 用户ID: " + std::to_string(userId) + ", 来源: " + clientInfo);
                
                // 验证登录凭证
                bool authSuccess = AuthenticateCredential(userId, password);
                
                // 构造响应包Loginreturn
                Packet response = Packet::makeLoginRe(authSuccess);
                if (authSuccess) {
                    sessionPtr->authenticate(std::to_string(userId)); // 标记会话为已认证
                    g_userSessions[userId] = sessionPtr; // 记录会话映射
                    WriteLog(LogLevel::INFO_LEVEL, "用户认证成功: " + std::to_string(userId));
                } else {
                    WriteLog(LogLevel::WARN_LEVEL, "用户认证失败: " + std::to_string(userId));
                }
                
                // 发送响应给客户端 - 使用辅助函数
                SendPacket(clientSocket, response);
                break;
            }

            // 创建账号请求
            case MsgType::CreateAcc: {
                // 修改：从header获取userId，从field2获取密码
                uint8_t userId = receivedPacket.getsendid();
                std::string password = receivedPacket.getField2Str();
                
                WriteLog(LogLevel::INFO_LEVEL, "创建账号请求 - 用户ID: " + std::to_string(userId));
                
                // 检查用户ID是否已存在
                bool success = false;
                if (g_userCredentials.count(userId)) {
                    WriteLog(LogLevel::WARN_LEVEL, "账号创建失败：用户ID已存在 - " + std::to_string(userId));
                } else {
                    g_userCredentials[userId] = password;
                    success = true;
                    WriteLog(LogLevel::INFO_LEVEL, "账号创建成功 - 用户ID: " + std::to_string(userId));
                }

                // 使用正确的响应包构造方法
                Packet response = Packet::makeRegiRe(success);
                SendPacket(clientSocket, response);
                break;
            }

            // 普通聊天消息
            case MsgType::NormalMsg: { 
                // 检查用户是否已认证
                if (!sessionPtr->is_authenticated) {
                    WriteLog(LogLevel::WARN_LEVEL, "未认证用户尝试发送消息: " + clientInfo);
                    break;
                }
                
                // 修改：从header获取收发信息，从field1获取内容
                uint8_t senderId = receivedPacket.getsendid();
                uint8_t receiverId = receivedPacket.getrecvid();
                std::string content = receivedPacket.getField1Str();
                
                WriteLog(LogLevel::INFO_LEVEL, 
                         "收到消息 - 发送者ID: " + std::to_string(senderId) + 
                         ", 接收者ID: " + std::to_string(receiverId) + 
                         ", 内容: " + content);
                
                // 转发消息给接收者
                if (g_userSessions.count(receiverId)) {
                    ClientSession* targetSession = g_userSessions[receiverId];
                    // 直接转发原始数据包
                    SendPacket(targetSession->socket_fd, receivedPacket);
                    WriteLog(LogLevel::DEBUG_LEVEL, "消息已转发给用户ID: " + std::to_string(receiverId));
                } else {
                    // TODO：如果用户不在线，由服务器暂存消息，等到客户端上线的时候再发送
                    // 应该为每一个用户建立一个暂存消息库，每次用户的在线状态变为在线的时候就把这些暂存消息一股脑发给这个用户
                    WriteLog(LogLevel::WARN_LEVEL, "接收者不在线: " + std::to_string(receiverId));
                }
                break;
            }

            // 创建群组请求
            case MsgType::CreateGrope: { // chatmsg里是grope所以就grope吧
                if (!sessionPtr->is_authenticated) {
                    WriteLog(LogLevel::WARN_LEVEL, "未认证用户尝试创建群组");
                    break;
                }
                
                WriteLog(LogLevel::INFO_LEVEL, "创建群组请求 - 发起者: " + sessionPtr->username);
                
                // TODO: 处理群组创建逻辑
                // 应该新建一个群组类，包含这个群组的id和成员列表，每次收到群聊消息，先检查这个群存不存在（应该不会不存在），
                // 接下来把这条消息转发给这个群组除了发送人以外的其他所有成员（发送人本地应该有这条消息记录）
                // 暂时返回成功
                Packet response = Packet::makeCreGroRe(true);
                SendPacket(clientSocket, response);
                break;
            }
            
            // 添加好友请求
            case MsgType::AddFriendReq: {
                if (!sessionPtr->is_authenticated) {
                    WriteLog(LogLevel::WARN_LEVEL, "未认证用户尝试添加好友");
                    break;
                }
                
                uint8_t requesterId = receivedPacket.getsendid();
                uint8_t targetId = receivedPacket.getrecvid();
                
                WriteLog(LogLevel::INFO_LEVEL, 
                         "添加好友请求 - 请求者ID: " + std::to_string(requesterId) + 
                         ", 目标ID: " + std::to_string(targetId));
                
                // 检查目标用户是否在线
                if (g_userSessions.count(targetId)) {
                    // 转发好友请求给目标用户
                    ClientSession* targetSession = g_userSessions[targetId];
                    SendPacket(targetSession->socket_fd, receivedPacket);
                    WriteLog(LogLevel::DEBUG_LEVEL, "好友请求已转发给用户ID: " + std::to_string(targetId));
                } else {
                    // 目标用户不在线，直接返回失败
                    Packet response = Packet::makeAddFriendRe(requesterId, targetId, false);
                    SendPacket(clientSocket, response);
                    WriteLog(LogLevel::WARN_LEVEL, "目标用户不在线: " + std::to_string(targetId));
                }
                break;
            }
            
            case MsgType::AddFriendRe: { // 添加好友响应（新增）
                // B用户接受了A的好友请求，需要转发给A
                uint8_t originalRequesterId = receivedPacket.getsendid();
                bool accepted = receivedPacket.success();
                
                WriteLog(LogLevel::INFO_LEVEL, 
                         "好友请求响应 - 原请求者ID: " + std::to_string(originalRequesterId) + 
                         ", 结果: " + (accepted ? "接受" : "拒绝"));
                
                // 转发响应给原请求者
                if (g_userSessions.count(originalRequesterId)) {
                    ClientSession* requesterSession = g_userSessions[originalRequesterId];
                    SendPacket(requesterSession->socket_fd, receivedPacket);
                    WriteLog(LogLevel::DEBUG_LEVEL, "好友响应已转发给用户ID: " + std::to_string(originalRequesterId));
                }
                break;
            }
            default: {
                WriteLog(LogLevel::WARN_LEVEL, 
                         "未知消息类型: " + std::to_string(static_cast<int>(receivedPacket.type())));
                break;
            }
        }
    }
    
    // 清理工作：关闭socket，标记为离线（但不删除会话对象，以便下次重连重用）
    WriteLog(LogLevel::INFO_LEVEL, "客户端断开连接: " + clientInfo);
    sessionPtr->online(false); // 标记为离线
    closesocket(clientSocket);
    // 注意：不再 delete sessionPtr，保留会话对象供下次重连使用
    WriteLog(LogLevel::DEBUG_LEVEL, "会话对象已标记为离线，保留以供重连: " + sessionPtr->client_ip);
}
// 注：消息封装和解包功能已由 chatMsg.hpp 中的 Packet 类实现
// Packet 类提供了更完善的消息封装、网络字节序转换、以及接收功能


int main() {
    // 初始化日志文件
    InitializeLogFile();

    // 初始化WinSock
    if (!InitializeWinSock()) {
        CloseLogFile();
        return 1;
    }

    // 创建Socket
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        std::string errormessage = "Socket 创建失败:" + std::to_string(WSAGetLastError());
        WriteLog(LogLevel::FATAL_LEVEL, errormessage);
        CleanupWinSock();
        CloseLogFile();
        return 1;
    }
    WriteLog(LogLevel::INFO_LEVEL, "Socket 创建成功");

    // 调用配置地址函数
    sockaddr_in service_address;
    int iResult;
    if (SetupServerAddress(PORT, service_address) != 0) {
        WriteLog(LogLevel::FATAL_LEVEL, "服务器地址配置失败");
        closesocket(listenSocket);
        CleanupWinSock();
        CloseLogFile();
        return 1;
    }
    WriteLog(LogLevel::INFO_LEVEL, "服务器地址配置成功");

    // 执行bind
    iResult = bind(listenSocket, (SOCKADDR*)&service_address, sizeof(service_address));

    if (iResult == SOCKET_ERROR) { // 错误处理
        std::string errormessage = "Bind失败: " + std::to_string(WSAGetLastError());
        WriteLog(LogLevel::FATAL_LEVEL, errormessage);
        closesocket(listenSocket);
        CleanupWinSock();
        CloseLogFile();
        return 1;
    }
    WriteLog(LogLevel::INFO_LEVEL, "Socket 成功绑定到端口 " + std::to_string(PORT));

    // 执行listen
    iResult = listen(listenSocket, BACKLOG);

    if(iResult == SOCKET_ERROR) { // 错误处理
        std::string errormessage = "Listen 失败: " + std::to_string(WSAGetLastError());
        WriteLog(LogLevel::FATAL_LEVEL, errormessage);
        closesocket(listenSocket);
        CleanupWinSock();
        CloseLogFile();
        return 1;
    }
    WriteLog(LogLevel::INFO_LEVEL, "服务器开始监听连接，端口: " + std::to_string(PORT));

    // accept循环：持续接受客户端连接并为每个客户端分配独立线程
    while (true) {
        sockaddr_in clientAddress;
        int addressLength = sizeof(clientAddress);
        
        // 等待并接受新的客户端连接（会阻塞直到有连接到来）
        SOCKET clientSocket = accept(listenSocket, 
                                     reinterpret_cast<sockaddr*>(&clientAddress), 
                                     &addressLength);
        
        if (clientSocket == INVALID_SOCKET) {
            // accept失败，记录错误但继续监听下一个连接
            std::string errormessage = "Accept 失败: " + std::to_string(WSAGetLastError());
            WriteLog(LogLevel::ERROR_LEVEL, errormessage);
            continue; // 跳过本次循环，继续等待下一个连接
        }
        
        // 获取客户端 IP地址
        char* clientIPAddress = inet_ntoa(clientAddress.sin_addr);
        unsigned short clientPort = ntohs(clientAddress.sin_port);
        
        std::string clientIP = std::string(clientIPAddress);
        WriteLog(LogLevel::INFO_LEVEL, 
                 "接受新连接 - IP: " + clientIP + ", 端口: " + std::to_string(clientPort));
        
        ClientSession* clientSession = nullptr;
        
        // 检查该IP是否已有会话对象（持久化会话）
        if (g_ipToSession.count(clientIP)) {
            // 重用已有的会话对象
            clientSession = g_ipToSession[clientIP];
            clientSession->updateSocket(clientSocket); // 更新socket和在线状态
            WriteLog(LogLevel::INFO_LEVEL, "重用已有会话对象: " + clientIP);
        } else {
            // 首次连接，创建新的会话对象
            clientSession = new ClientSession(clientSocket, clientIP, clientPort);
            g_ipToSession[clientIP] = clientSession; // 保存到映射中
            WriteLog(LogLevel::INFO_LEVEL, "创建新会话对象: " + clientIP);
        }
        
        // 为这个客户端创建新线程进行处理
        std::thread clientHandlerThread(HandleClient, clientSession);
        clientHandlerThread.detach(); // 分离线程，使其独立运行，主线程不等待
        
        WriteLog(LogLevel::DEBUG_LEVEL, "已为客户端分配独立处理线程");
    }
    
    // 程序正常情况下不会执行到这里（除非手动break跳出循环）
    closesocket(listenSocket);
    CleanupWinSock();
    CloseLogFile();
    
    return 0;
}