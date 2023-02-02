#include <string>
#include <iostream>

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#pragma warning(disable: 4996)
#pragma comment(lib, "ws2_32.lib")

//Проверка на количество символов и кратность 32
bool checkValue(std::string s) {
    if (s.length() > 2 || std::stoi(s) % 32 == 0) return true;
    return false;
}

int main(int argc, char* args[])
{
    setlocale(LC_ALL, "ru");

#ifdef _WIN32
    WSAData wsaData;
    WORD DLLVersion = MAKEWORD(2, 1);
    if (WSAStartup(DLLVersion, &wsaData) != 0) {
        std::cerr << "Неудача...\n";
        exit;
    }
#endif

#ifdef _WIN32
    SOCKADDR_IN address;
#else
    struct sockaddr_in address;
#endif // _WIN32
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    int sizeofaddress = sizeof(address);
    address.sin_port = htons(18666);
    address.sin_family = AF_INET;
#ifdef _WIN32
    SOCKET connection = socket(AF_INET, SOCK_STREAM, NULL);
    if (connect(connection, (SOCKADDR*)&address, sizeof(address)) != 0) {
        std::cout << "Ожидает соединения...\n";
    }
    else  std::cout << "Соединение установлено\n";
#else
    int connection = socket(AF_INET, SOCK_STREAM, NULL);
    std::cout << connection;
    int a = connect(connection, (struct sockaddr*)&address, sizeof(address));
    if (a != 0) {
        std::cout << "Ожидает соединения...\n";
    }
    else  std::cout << "СоединеНие установлено\n" << a << "h";
#endif // _WIN32
    int value;
    while (true) {
        if (recv(connection, (char*)&value, sizeof(int), NULL) == 4) {
            if (checkValue(std::to_string(value))) std::cout << value << "\n";
            else std::cerr << "Число должно быть кратно 32 или должно состоять из более чем 2-х символов\n";
        }
        else {
            connection = socket(AF_INET, SOCK_STREAM, NULL);
#ifdef _WIN32
            if (connect(connection, (SOCKADDR*)&address, sizeof(address)) == 0)
#else
            if (connect(connection, (struct sockaddr*)&address, sizeof(address)) == 0)
#endif
                std::cout << "Соединение установлено\n";
        }
    }
    std::cout << "ok";
    return 0;
}
