#include "database.hpp"

dataBase::dataBase()
{
    driver = get_driver_instance();
    con = std::shared_ptr<sql::Connection>(driver->connect(HOST, USER, PASSWORD));
    con->setSchema(DATABASE);

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

string dataBase::createRandomString(int size)
{
    string sb;

    for (int i = 0; i < size; i++)
    {
        sb += seed[mt() % seed.size()];
    }

    return sb;
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


std::shared_ptr<sql::ResultSet> dataBase::query(std::string query)
{
    auto stmt = std::shared_ptr<sql::Statement>(con->createStatement());
    auto res = std::shared_ptr<sql::ResultSet>(stmt->executeQuery(query));

    return res;
}

int dataBase::update(std::string query)
{
    auto stmt = std::shared_ptr<sql::Statement>(con->createStatement());
    stmt->execute(query);
    return stmt->getUpdateCount();
}

bool dataBase::login(string mail, string pass, int *id, string *name)
{
    if (illegalChara(mail) && illegalChara(pass))
    {
        try
        {
            auto res = query("SELECT * FROM user_data WHERE email='" + mail + "' AND password=SHA2(SHA2('" + pass + "', 256), 256)");
            res->next();

            if(res->rowsCount() != 0)
            {
                *id = res->getInt(1);
                *name = res->getString("name");
                return 1;
            } 

            return 0;
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
            auto res = query("SELECT * FROM user_data WHERE email='" + mail + "'");
            int Existence = 0;

            while (res->next())
            {
                Existence = res->getInt(1);
            }

            if (Existence == 0)
            {
                res = query("SELECT MAX(user_id) FROM user_data");
                int lastid;

                while (res->next())
                {
                    lastid = res->getInt(1);
                }

                *id = lastid + 1;
                int count = update("INSERT INTO user_data VALUES(" + to_string(lastid + 1) + ", '" + mail + "', SHA2(SHA2('" + pass + "', 256), 256), '" + name + "')");
                
                return count == 1;
            }
            else
            {
                return 0;
            }
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

int dataBase::regstModel(int user_id, string model_name, string model_file_name, int model_type)
{
    try
    {
        auto res = query("SELECT MAX(model_id) FROM model_data");
        int lastmodelid;

        while (res->next())
        {
            lastmodelid = res->getInt(1);
        }

        int count = update("INSERT INTO model_data VALUES(" + to_string(++lastmodelid) + "," + to_string(user_id) + ",'" +
                      model_name + "','" + "" + "','" + model_file_name + "'," + to_string(model_type) + ")");

        if(count != 0)return lastmodelid;
        else return -1;
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
        string accesskey = createRandomString(10);
        int count = update("INSERT INTO access_key VALUES( null,'" + accesskey + "'" + "," + to_string(model_id) + ")");
     
        if(count != 0)return accesskey;
        else return "";
    }
    catch (sql::SQLException &e)
    {
        cout << "# ERR: " << e.what();
        return "";
    }
}

string dataBase::getModelData(int user_id)
{
    try
    {
        auto res = query("SELECT model_id FROM model_data WHERE user_id = " + to_string(user_id));
        std::string str = "";

        while (res->next())
        {
            str = res->getString("model_id") ;
        }

        return str;
    }
    catch (sql::SQLException &e)
    {
        cout << "# ERR: " << e.what();
        return "";
    }
}

string dataBase::getModelDataDetail(int model_id)
{
    try
    {
        auto res = query("SELECT * FROM model_data WHERE model_id = " + to_string(model_id));
        std::string str = "";

        while (res->next())
        {
            str =  "\"model_id\":" + res->getString("model_id") + ",";
            str += "\"model_name\":\"" + res->getString("model_name")+ "\",";
            str += "\"model_file_pash\":\"" + res->getString("model_file_pash")+ "\",";
            str += "\"model_file_name\":\"" + res->getString("model_file_name")+ "\",";
            str += "\"model_type\":" + res->getString("model_type");
        }

        return str;
    }
    catch (sql::SQLException &e)
    {
        cout << "# ERR: " << e.what();
        return "";
    }
}