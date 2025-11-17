#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <semaphore.h>
#include <atomic>

#include "json.hpp"
#include "Group.hpp"
#include "User.hpp"
#include "public.hpp"
#include "config.hpp"
#include "public.hpp"

using json = nlohmann::json;

int clientfd;

User g_currentUser(0, " ", " ", " ");

std::vector<User> g_currentUserFriendLists;

std::vector<Group> g_currentUserGroupLists;

bool got = false;

sem_t rwsem;

std::atomic_bool is_login{false};

void showCurrentUserData();

void readTaskHandler(int clientfd);

std::string getCurrentTime();

void startMenu();
void mainMenu();

void help(int, std::string);
void chat(int, std::string);
void addFriend(int, std::string);
void createGroup(int, std::string);
void addGroup(int, std::string);
void groupChat(int, std::string);
void loginOut(int, std::string);

void QuitHander(int)
{
    json js;
    js["msg_id"] = 0;
    js["id"] = g_currentUser.getId();
    std::string msg = js.dump();
    std::thread send_thread(send, clientfd, msg.c_str(), strlen(msg.c_str()) + 1, 0);
    send_thread.join();

    exit(0);
}

int main(int argc, char **argv)
{
    signal(SIGINT, QuitHander);

    char *ip = IP;
    uint16_t port = PORT;
    if (argc < 2)
    {
    }
    else
    {
        ip = argv[1];
        port = atoi(argv[2]);
    }

    clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        std::cerr << "socket create error" << std::endl;
        exit(-1);
    }
    
    sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    if (connect(clientfd, (sockaddr *)&server, sizeof(sockaddr_in)) == -1)
    {
        perror("connect server error");
        close(clientfd);
        exit(-1);
    }

    sem_init(&rwsem, 0, 0);

    std::thread read(readTaskHandler, clientfd);
    read.detach();

    for (;;)
    {
        if (is_login == false)
        {
            startMenu();
        }
        else
        {
            mainMenu();
        }
    }
}

void showCurrentUserData()
{
    std::cout << "=====================login user====================" << std::endl;
    std::cout << "current login user => id : " << g_currentUser.getId() << "name : " << g_currentUser.getName() << std::endl;
    std::cout << "---------------------friend list-------------------" << std::endl;
    if (!g_currentUserFriendLists.empty())
    {
        for (User &user : g_currentUserFriendLists)
        {
            std::cout << "id : " << user.getId() << "name : " << user.getName() << "state : " << user.getState() << std::endl;
        }
    }
    std::cout << "----------------------group list--------------------" << std::endl;
    if (!g_currentUserGroupLists.empty())
    {
        for (Group &group : g_currentUserGroupLists)
        {
            std::cout << "group id : " << group.getId() << "name : " << group.getName() << " desc : " << group.getDesc() << std::endl;
            for (GroupUser &user : group.getUsers())
            {
                std::cout << "id : " << user.getId() << "name : " << user.getName() << "state : " << user.getState() << "role : "
                          << user.getRole() << std::endl;
            }
        }
    }
    std::cout << "=====================================================" << std::endl;
}

void readTaskHandler(int clientfd)
{
    while (true)
    {
        char buffer[1024] = {0};
        int len = recv(clientfd, buffer, 1024, 0);
        json js = json::parse(buffer);

        if (len == -1 || len == 0)
        {
            close(clientfd);
            exit(-1);
        }
        debug(buffer);

        if (js["msg_id"].get<int>() == LOGIN_MSG_ACK)
        {
            if (js["errno"].get<int>() != 0) // 登录失败
            {
                sem_post(&rwsem);
                // got = true;
                debug(js["error_msg"]);
            }
            else // 登录成功
            {
                sem_post(&rwsem);
                is_login = true;
                // got = true;

                g_currentUser.setId(js["id"].get<int>());
                g_currentUser.setName(js["name"]);

                debug("解析个人信息完成");

                // 记录用户好友信息
                if (js.contains("friend"))
                {
                    std::vector<std::string> vec = js["friend"];
                    for (std::string &str : vec)
                    {
                        json js = json::parse(str);
                        User user(js["id"].get<int>(), js["name"], js["state"], 0);
                        g_currentUserFriendLists.push_back(user);
                    }
                }
                debug("解析好友信息完成");

                // 记录用户群组列表信息
                if (js.contains("groups"))
                {
                    std::vector<std::string> vec = js["groups"];
                    for (std::string &groupStr : vec)
                    {
                        json groupJs = json::parse(groupStr);
                        Group group(groupJs["id"].get<int>(), groupJs["group_name"], groupJs["group_desc"]);
                        std::vector<std::string> vec2 = groupJs["users"];
                        for (std::string &userStr : vec2)
                        {
                            json js = json::parse(userStr);
                            GroupUser groupUser(js["id"].get<int>(), js["name"], js["state"], js["role"]);
                            group.getUsers().push_back(groupUser);
                        }
                        g_currentUserGroupLists.push_back(group);
                    }
                }
                debug("解析群组信息完成");
                showCurrentUserData();

                if (js.contains("offline_msg"))
                {
                    std::vector<std::string> vec = js["offline_msg"];
                    for (std::string &str : vec)
                    {
                        json js = json::parse(str);
                        if (ONE_CHAT_MSG == js["msg_id"].get<int>())
                        {
                            std::cout << js["time"].get<std::string>() << " [ " << js["id"] << " ] "
                                      << js["name"].get<std::string>() << " said : " << js["msg"].get<std::string>() << std::endl;
                            continue;
                        }
                        else if (GROUP_CHAT_MSG == js["msg_id"].get<int>())
                        {
                            std::cout << "群消息[" << js["group_id"] << " : " << js["time"].get<std::string>() << " [ " << js["id"] << " ] "
                                      << js["name"].get<std::string>() << " said : " << js["msg"].get<std::string>() << std::endl;
                            continue;
                        }
                    }
                }
            }
        }

        if(js["msg_id"].get<int>()==REG_MSG_ACK)
        {
            if (js["errno"].get<int>() != 0)
            {
                std::cout << js["name"] << " is already exist,register error !" << std::endl;
                sem_post(&rwsem);
            }
            else
            {
                std::cout << js["name"] << " register success,user_id : " << js["id"] << ", do not forget it" << std::endl;
                sem_post(&rwsem);
            }
        }

        if (ONE_CHAT_MSG == js["msg_id"].get<int>())
        {
            std::cout << js["time"].get<std::string>() << " [ " << js["id"] << " ] "
                      << js["name"].get<std::string>() << " said : " << js["msg"].get<std::string>() << std::endl;
            continue;
        }
        else if (GROUP_CHAT_MSG == js["msg_id"].get<int>())
        {
            std::cout << "群消息[" << js["group_id"] << " : " << js["time"].get<std::string>() << " [ " << js["id"] << " ] "
                      << js["name"].get<std::string>() << " said : " << js["msg"].get<std::string>() << std::endl;
            continue;
        }
    }
}

std::unordered_map<std::string, std::string> commandMap =
    {
        {"help", "显示所以支持的命令"},
        {"chat", "一对一收发消息,chat:{friend_id}:{message}"},
        {"addFriend", "添加好友,addfriend:{friend_id}"},
        {"createGroup", "创建群组,creategroup:{group_name}:{group_desc}"},
        {"addGroup", "加入群组,addgroup:{group_id}"},
        {"groupChat", "群聊,groupchat:{group_id}:message"},
        {"loginOut", "注销"}};
std::unordered_map<std::string, std::function<void(int, std::string)>> commandHandlerMap =
    {
        {"help", help},
        {"chat", chat},
        {"addfriend", addFriend},
        {"creategroup", createGroup},
        {"addgroup", addGroup},
        {"groupchat", groupChat},
        {"loginout", loginOut}};

void startMenu()
{

    std::cout << "================" << std::endl;
    std::cout << "1.login" << std::endl;
    std::cout << "2.register" << std::endl;
    std::cout << "3.quit" << std::endl;
    std::cout << "================" << std::endl;
    std::cout << "Choice : ";
    int choice = 0;
    std::cin >> choice;
    std::cin.get();

    switch (choice)
    {
    case 1:
    {
        int id = 0;
        char pwd[50] = {0};
        std::cout << "user_id : ";
        std::cin >> id;
        std::cin.get();
        std::cout << "password : ";
        std::cin.getline(pwd, 50);

        json js;
        js["msg_id"] = LOGIN_MSG;
        js["id"] = id;
        js["password"] = pwd;
        std::string request = js.dump();

        int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
        if (len == -1)
        {
            debug("send login msg error : ");
            debug(request);
        }
    }

        sem_wait(&rwsem);
        // while (!got)
        // {
        // }
        // got = false;
        break;

    case 2:
    {
        char name[50] = {0};
        char pwd[50] = {0};
        std::cout << "userName : " << std::endl;
        std::cin.getline(name, 50);
        std::cout << "userPassword : " << std::endl;
        std::cin.getline(pwd, 50);

        json js;
        js["msg_id"] = REG_MSG;
        js["name"] = name;
        js["password"] = pwd;
        std::string request = js.dump();

        int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
        if (len == -1)
        {
            debug("send error:");
            debug(request);
        }
        sem_wait(&rwsem);
    }
    break;

    case 3:

        close(clientfd);
        sem_destroy(&rwsem);
        exit(0);

    default:
        std::cout << "no command found" << std::endl;
        break;
    }
}

void mainMenu()
{
    help(0, "xxx");

    char buffer[1024] = {0};
    for (;;)
    {
        std::cin.getline(buffer, 1024);
        std::string commandbuf(buffer);
        std::string command; // 存储命令
        int idx = commandbuf.find(":");
        if (idx == -1)
        {
            command = commandbuf;
        }
        else
        {
            command = commandbuf.substr(0, idx);
        }
        auto it = commandHandlerMap.find(command);
        if (it == commandHandlerMap.end())
        {
            std::cerr << "invalid command!" << std::endl;
            continue;
        }

        it->second(clientfd, commandbuf.substr(idx + 1, commandbuf.size() - idx));
        if (is_login == false)
        {
            break;
        }
    }
}

void help(int, std::string)
{
    std::cout << "command list:>>>>" << std::endl;
    for (auto &p : commandMap)
    {
        std::cout << p.first << " : " << p.second << std::endl;
    }
    std::cout << std::endl;
}

void chat(int fd, std::string str)
{
    int idx = str.find(":");
    if (idx == -1)
    {
        std::cerr << "invalid command!!" << std::endl;
        return;
    }

    int friend_id = atoi(str.substr(0, idx).c_str());
    std::string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msg_id"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["to"] = friend_id;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    std::string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1)
    {
        std::cerr << "send chat message error -> " << buffer << std::endl;
    }
}
void addFriend(int fd, std::string str)
{
    int friend_id = atoi(str.c_str());
    json js;
    js["msg_id"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friend_id"] = friend_id;
    std::string buffer = js.dump();

    int len = send(fd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1)
    {
        std::cerr << "send addfriend msg error -> " << buffer << std::endl;
    }
}
void createGroup(int fd, std::string str)
{
    int idx = str.find(":");
    if (idx == -1)
    {
        std::cerr << "command invalid!!!" << std::endl;
        return;
    }

    std::string group_name = str.substr(0, idx);
    std::string group_desc = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msg_id"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["group_name"] = group_name;
    js["group_desc"] = group_desc;
    std::string buffer = js.dump();

    int len = send(fd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1)
    {
        std::cerr << "send createGroup msg error -> " << buffer << std::endl;
    }
}
void addGroup(int fd, std::string str)
{
    int group_id = atoi(str.c_str());
    json js;
    js["msg_id"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["group_id"] = group_id;
    std::string buffer = js.dump();

    int len = send(fd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1)
    {
        std::cerr << "send addGroup msg error -> " << buffer << std::endl;
    }
}
void groupChat(int fd, std::string str)
{
    int idx = str.find(":");
    if (idx == -1)
    {
        std::cerr << "invalid command!!" << std::endl;
        return;
    }

    int group_id = atoi(str.substr(0, idx).c_str());
    std::string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msg_id"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["group_id"] = group_id;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    std::string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1)
    {
        std::cerr << "send GroupChat message error -> " << buffer << std::endl;
    }
}
void loginOut(int, std::string)
{
    json js;
    js["msg_id"] = 10;
    js["id"] = g_currentUser.getId();
    std::string msg = js.dump();
    send(clientfd, msg.c_str(), strlen(msg.c_str()) + 1, 0);

    is_login = false;
    g_currentUserFriendLists.clear();
    g_currentUserGroupLists.clear();
}

std::string getCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm *local_time = std::localtime(&time);
    std::ostringstream oss;
    oss << std::put_time(local_time, "%Y-%m-%d %H:%M:%S");

    return oss.str();
}