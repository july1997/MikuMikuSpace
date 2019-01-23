#include "database.hpp"

dataBase::dataBase()
{
    driver = get_driver_instance();

    //乱数シードの初期化
    for (char c = 'a'; c <= 'z'; c++)
    {
        seed.push_back(c);
    }

    for (char c = '0'; c <= '9'; c++)
    {
        seed.push_back(c);
    }

    for (char c = 'A'; c <= 'Z'; c++)
    {
        seed.push_back(c);
    }

    random_device rnd; // 非決定的な乱数生成器を生成
    mt.seed(rnd());
}

dataBase::~dataBase()
{
}

bool dataBase::login(string mail, string pass, int *id, string *name)
{
    if (illegalChara(mail) && illegalChara(pass))
    {
        try
        {
            sql::Connection *con;
            con = driver->connect(HOST, USER, PASSWORD);
            con->setSchema(DATABASE);
            sql::Statement *stmt;
            sql::ResultSet *res;
            stmt = con->createStatement();
            res = stmt->executeQuery("SELECT * FROM user_data WHERE email='" + mail + "' AND password=SHA2(SHA2('" + pass + "', 256), 256)");
            string name_;

            while (res->next())
            {
                *id = res->getInt(1);
                name_ = res->getString("name");
            }

            *name = name_;
            //削除すると実行
            delete res;
            delete stmt;
            delete con;
            return 1;
        }
        catch (sql::SQLException &e)
        {
            cout << "# ERR: " << e.what();
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

bool dataBase::regst(string mail, string pass, string name, int *id)
{
    if (illegalChara(mail) && illegalChara(pass))
    {
        try
        {
            sql::Connection *con;
            con = driver->connect(HOST, USER, PASSWORD);
            con->setSchema(DATABASE);
            sql::Statement *stmt;
            sql::ResultSet *res;
            stmt = con->createStatement();
            res = stmt->executeQuery("SELECT * FROM user_data WHERE email='" + mail + "'");
            int Existence = 0;

            while (res->next())
            {
                Existence = res->getInt(1);
            }

            if (Existence == 0)
            {
                stmt = con->createStatement();
                res = stmt->executeQuery("SELECT MAX(user_id) FROM user_data");
                int lastid;

                while (res->next())
                {
                    lastid = res->getInt(1);
                }

                *id = lastid + 1;
                stmt = con->createStatement();
                stmt->execute("INSERT INTO user_data VALUES(" + to_string(lastid + 1) + ", '" + mail + "', SHA2(SHA2('" + pass + "', 256), 256), '" + name + "')");
                return 1;
            }
            else
            {
                return 0;
            }

            //削除すると実行
            delete res;
            delete stmt;
        }
        catch (sql::SQLException &e)
        {
            cout << "# ERR: " << e.what();
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

bool dataBase::illegalChara(std::string str)
{
    if (str == "")
    {
        return 0;
    }

    //正規表現
    std::regex re("^[0-9A-Za-z]+@*.*_*-*$");

    if (std::regex_match(str, re) == 0)
    {
        return 0;
    }

    return 1;
}

int dataBase::regstModel(int user_id, string model_name, string model_file_name, int model_type)
{
    try
    {
        sql::Connection *con;
        con = driver->connect(HOST, USER, PASSWORD);
        con->setSchema(DATABASE);
        sql::Statement *stmt;
        sql::ResultSet *res;
        stmt = con->createStatement();
        res = stmt->executeQuery("SELECT MAX(model_id) FROM model_data");
        int lastmodelid;

        while (res->next())
        {
            lastmodelid = res->getInt(1);
        }

        stmt = con->createStatement();
        stmt->execute("INSERT INTO model_data VALUES(" + to_string(++lastmodelid) + "," + to_string(user_id) + ",'" +
                      model_name + "','" + "" + "','" + model_file_name + "'," + to_string(model_type) + ")");
        //削除すると実行
        delete res;
        delete stmt;
        delete con;
        return lastmodelid;
    }
    catch (sql::SQLException &e)
    {
        cout << "# ERR: " << e.what();
        return -1;
    }
}

string dataBase::createAccessKey(int model_id)
{
    try
    {
        sql::Connection *con;
        con = driver->connect(HOST, USER, PASSWORD);
        con->setSchema(DATABASE);
        sql::Statement *stmt;
        sql::ResultSet *res;
        stmt = con->createStatement();
        res = stmt->executeQuery("SELECT MAX(access_number) FROM access_key");
        int accessNumber;

        while (res->next())
        {
            accessNumber = res->getInt(1);
        }

        string accesskey = createRandomString(10);
        stmt = con->createStatement();
        stmt->execute("INSERT INTO access_key VALUES('" + accesskey + "'" + "," + to_string(model_id) + "," + to_string(++accessNumber) + ")");
        //削除すると実行
        delete res;
        delete stmt;
        delete con;
        return accesskey;
    }
    catch (sql::SQLException &e)
    {
        cout << "# ERR: " << e.what();
        return "";
    }
}

string dataBase::createRandomString(int size)
{
    string sb;

    for (int i = 0; i < size; i++)
    {
        sb += seed[mt() % seed.size()];
    }

    return sb;
}
