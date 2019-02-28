#pragma once

#include <iostream>
#include <sstream>
#include <memory>
#include <regex>
#include <vector>
#include <random>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <mysql_error.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>

#include "mysql_config.hpp"

//#pragma comment(lib, "mysqlcppconn.lib")

using namespace std;

class dataBase
{
        sql::Driver *driver;
        std::shared_ptr<sql::Connection> con;

        bool illegalChara(std::string str);

        vector<char> seed;
        string createRandomString(int size);
        std::mt19937 mt;

        std::shared_ptr<sql::ResultSet> query(std::string query);
        int update(std::string query);

        void checkConnection();

    public:
        dataBase();
        ~dataBase();

        bool login(string mail, string pass, int *id, string *name);

        bool regst(string mail, string pass, string name, int *id);

        int regstModel(int user_id, string model_name, string model_file_name, int model_type);

        string createAccessKey(int model_id);

        string getModelData(int user_id);

        string getModelDataDetail(int model_id);

        bool checkBanIP(string IP);
};
