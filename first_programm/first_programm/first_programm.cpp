#include <mutex>
#include <queue>
#include <string>
#include <vector>
#include <thread>
#include <iostream>
#include <algorithm>
#include <condition_variable>

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#pragma warning(disable: 4996)
#pragma comment(lib, "ws2_32.lib")

std::mutex mtx;
std::condition_variable cv;

//Общий буффер
std::queue<std::string> buffer;
bool notEmptyBuffer() { return !buffer.empty(); }

//Сумма элементов строки
int SumString(std::string s) {
    int sum = 0, dig = 0;
    for (int i = 0; i < s.length(); i++)
        if (isdigit(s[i])) dig = dig * 10 + (s[i] - 48);
        else { sum += dig; dig = 0; }
    sum += dig;
    return sum;
}

//Проверка строки на цифры
bool CheckValues(std::string s) {
    for (int i = 0; i < s.length(); i++)
        if (!(isdigit(s[i]))) return false;
    return true;
}

//Проверка строки на длину
bool CheckLength(std::string s) {
    return s.length() > 0 ? true : false;
    return s.length() <= 64 ? true : false;
}

//Сортировка значений строки
void SortedString(std::string& s) {
    std::sort(std::begin(s), std::end(s), std::greater<>());
}

//Создание строки из вектора значений
std::string ChangeString(std::string s) {
    std::string str;
    for (int i = 0; i < s.length(); i++) {
        if ((s[i] - 48) % 2 == 0) {
            str.push_back('K');
            str.push_back('B');
        }
        else str.push_back(s[i]);
    }
    return str;
}

//Второй поток
void SecondThread() {

    std::cout << "Второй поток ID: " << std::this_thread::get_id() << "\n";

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
#endif
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    int sizeofaddress = sizeof(address);
    address.sin_port = htons(18666);
    address.sin_family = AF_INET;
#ifdef _WIN32
    SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
    bind(sListen, (SOCKADDR*)&address, sizeof(address));
#else
    int sListen = socket(AF_INET, SOCK_STREAM, 0);
    bind(sListen, (struct sockaddr*)&address, sizeof(address));
#endif
    listen(sListen, SOMAXCONN);

    std::cout << "Поток " << std::this_thread::get_id() << ": Ожидает соединения...\n";
#ifdef _WIN32
    SOCKET connection = accept(sListen, (SOCKADDR*)&address, &sizeofaddress);
#else
    int connection = accept(sListen, NULL, NULL);
#endif
    if (connection < 0) {
        std::cerr << "Поток " << std::this_thread::get_id() << ": Отсутствует соединение\n";
    }
    else  std::cout << "Поток " << std::this_thread::get_id() << ": Соединение установлено\n";

    int summary;
    while (true) {
        std::unique_lock<std::mutex> un_lock(mtx);
        cv.wait(un_lock, notEmptyBuffer);
        std::string data = buffer.front();
        buffer.pop();
        summary = (SumString(data));
#ifdef _WIN32
        if (send(connection, (char*)&summary, sizeof(int), NULL) == SOCKET_ERROR) {
#else
        if (send(connection, (char*)&summary, sizeof(int), MSG_NOSIGNAL) != 4) {
#endif
            std::cerr << "Поток " << std::this_thread::get_id() << ": Отсутствует соединение\n";
#ifdef _WIN32
            connection = accept(sListen, (SOCKADDR*)&address, &sizeofaddress);
            if (connection == 0) std::cout << "Поток " << std::this_thread::get_id() << ": Соединение установлено\n";
#else
            connection = accept(sListen, NULL, NULL);
            if (connection >= 0) std::cout << "Поток " << std::this_thread::get_id() << ": Соединение установлено\n";
#endif
            send(connection, (char*)&summary, sizeof(int), 0);
        }
        std::cout << "Поток " << std::this_thread::get_id() << ": " << data << "\n";
        }
    }

//Первый поток
int main(int argc, char* args[])
{
    setlocale(LC_ALL, "ru");

    std::string str;

    std::cout << "Первый поток ID: " << std::this_thread::get_id() << "\n";

    std::thread second_thread(SecondThread);
    second_thread.detach();

    while (std::getline(std::cin, str)) {
        if (CheckLength(str) && CheckValues(str)) {
            SortedString(str);
            str = ChangeString(str);
            buffer.push(str);
            cv.notify_one();
        }
        else {
            std::cerr << "Поток " << std::this_thread::get_id() << ": Строка должна состоять только из цифр и не должна превышать 64 символов\n";
        }
    }

    return 0;
}
