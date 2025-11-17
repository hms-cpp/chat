#include <iostream>
#include <thread>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <atomic>
#include <semaphore.h>

#include "json.hpp"
#include "config.hpp"
#include "public.hpp"
#include "User.hpp"

using json = nlohmann::json;
using std::cin;
using std::cout;
using std::endl;

int clientfd;

User g_currentUser(0, " ", " ", " ");

User g_marchedUser(0, " ", " ", " ");

std::atomic_bool is_login{false};

bool game_start = false;

sem_t rwsem;

int pan[3][3];

void readTaskHandler(int clientfd);

void startMenu();

void mainMenu();

void fresh();

void QuitHander(int)
{

    exit(0);
}

int main(int argc, char **argv)
{
    signal(SIGINT, QuitHander);

    sem_init(&rwsem, 0, 0);

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

    std::thread read(readTaskHandler, clientfd);
    read.detach();

    for (;;)
    {
        if (!is_login || g_marchedUser.getId() == 0)
        {
            startMenu();
        }
        else
        {
            mainMenu();
        }
    }
}
void mainMenu()
{
    int pos = 0;
    for (;;)
    {
        fresh();
        cin >> pos;
        int x = (pos / 10) - 1;
        int y = (pos % 10) - 1;

        if (pos == 99)
        {
            exit(0);
        }
        if (x >= 0 && x < 3 && y >= 0 && y < 3 && pan[x][y] == 0)
        {
            pan[x][y] = g_currentUser.getId();
            json js;
            js["msg_id"] = ONE_CHAT_MSG;
            js["id"] = g_currentUser.getId();
            js["to"] = g_marchedUser.getId();
            js["pos"] = pos;
            std::string buffer = js.dump();

            int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
            if (len == -1)
            {
                std::cerr << "send chat message error -> " << buffer << std::endl;
            }
        }
        else
        {
            cout << "invalid position" << endl;
        }
    }
}
void startMenu()
{

    std::cout << "================" << std::endl;
    std::cout << "1.login" << std::endl;
    std::cout << "2.register" << std::endl;
    std::cout << "3.quit" << std::endl;
    std::cout << "4.march" << std::endl;
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

    case 4:
    {
        json js;
        js["msg_id"] = MARCH_MSG;
        js["id"] = g_currentUser.getId();
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

    default:
        std::cout << "no command found" << std::endl;
        break;
    }
}

void fresh()
{
   // system("clear"); 
    cout << "====";
    cout << g_currentUser.getId();
    cout << " vs ";
    cout << g_marchedUser.getId();
    cout << "====" << endl;
    
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (pan[i][j] == 0)
            {
                cout << "0 ";
            }
            else if (pan[i][j] == g_currentUser.getId())
            {
                cout << "# ";
            }
            else if (pan[i][j] == g_marchedUser.getId())
            {
                cout << "@ ";
            }
        }
        cout << endl;
    }
    cout << "输入位置(如11表示第一行第一列): ";
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

        cout << buffer;

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
            }
            continue;
        }

        if (js["msg_id"].get<int>() == REG_MSG_ACK)
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
            continue;
        }

        if (js["msg_id"].get<int>() == MARCH_MSG)
        {
            g_marchedUser.setId(js["marched_id"].get<int>());
            memset(pan, 0, sizeof(pan));
            sem_post(&rwsem);
            mainMenu();
            continue;
        }

        if (js["msg_id"].get<int>() == ONE_CHAT_MSG)
        {
            int pos = js["pos"].get<int>();
            pan[(pos / 10) - 1][(pos % 10) - 1] = g_marchedUser.getId();
            fresh();
            continue;
        }
    }
}