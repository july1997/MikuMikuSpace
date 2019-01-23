#pragma once

#include "Account.hpp"
#include "database.hpp"

#include <random>
#include <boost/thread.hpp>

#include "json11.hpp"

class AccountManager
{
    std::vector<std::shared_ptr<Account>> accounts;
    std::vector<size_t> miss;
    std::vector<string> old_pos;
    std::vector<string> old_dir;
    std::vector< std::vector<unsigned int> > sendList;
	std::vector<bool> logined;
	std::vector<size_t> dos;
	std::vector<bool> help;

    size_t accountsNumber=0;
    size_t loginnun =0 ;
    string startPos = "0 9 0 0 0 0 1 ";

    dataBase data;

    bool startup = 0;

    int addAccount();
    int deleteAccount(size_t l);

    bool findID(unsigned int ID);

    bool dir(int account,string str);
    bool login(int account,string str);
    bool regst(int account,string str);
    bool pos(int account,string str);
	bool sendlist(int account);

    public:
	AccountManager();
	~AccountManager();

	int connection();

	void update();
};
