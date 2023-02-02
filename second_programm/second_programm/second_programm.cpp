#include <string>
#include <iostream>

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
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

    std::cout << "Ожидает соединения...\n";
#ifdef _WIN32
    SOCKET sock = socket(AF_INET, SOCK_STREAM, NULL);
    if (connect(sock, (SOCKADDR*)&address, sizeof(address)) != 0) {
        std::cout << "Соединение отсутствует\n";
    }
    else  std::cout << "Соединение установлено\n";
#else
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    int connection = connect(sock, (struct sockaddr*)&address, sizeof(address));
    if (connection >= 0)
        std::cout << "Соединение установлено\n";
#endif // _WIN32
    int value;
    while (true) {
        if (recv(sock, (char*)&value, sizeof(int), 0) == 4) {
            if (checkValue(std::to_string(value))) std::cout << value << "\n";
            else std::cerr << "Число должно быть кратно 32 или должно состоять из более чем 2-х символов\n";
        }
        else {

#ifdef _WIN32
            sock = socket(AF_INET, SOCK_STREAM, 0);
            if (connect(sock, (SOCKADDR*)&address, sizeof(address)) == 0)
#else
            close(sock);
            sock = socket(AF_INET, SOCK_STREAM, 0);
            connection = connect(sock, (struct sockaddr*)&address, sizeof(address));
            if (connection >= 0)
#endif
                std::cout << "Соединение установлено\n";
        }
    }
    return 0;
}
