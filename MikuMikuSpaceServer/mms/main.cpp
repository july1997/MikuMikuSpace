#include <iostream>
#include <string>

#include "AccountManager.hpp"

using namespace std;

//文字コード : UTF-8

int main()
{
    cout << MMS_version << endl;

    std::shared_ptr<AccountManager> manager(new AccountManager());

    //別スレッドで実行
    std::thread t([manager]() { (*manager).connection(); });
    t.join();

    while (1)
    {
        string inp;
        cin >> inp;

        if(inp == "accounts")
        {
            cout <<  "# Current number of connections : " << manager->getAccountsSize() - 1 << endl;
        } 
        else if(inp == "help")
        {
            cout <<  "# Commands" << endl;
            cout <<  "   accounts : show current number of connections" << endl;
        }
    }

    return 0;
}