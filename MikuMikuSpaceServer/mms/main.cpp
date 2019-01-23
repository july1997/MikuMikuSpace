#include <iostream>

#include "AccountManager.hpp"

using namespace std;

//文字コード : UTF-8

int main()
{
    cout << MMOW_version << endl;
    AccountManager manager;
    manager.connection();
    //別スレッドで実行
    std::thread t([&manager]() { manager.connection(); });

    while (1)
    {
    }

    return 0;
}