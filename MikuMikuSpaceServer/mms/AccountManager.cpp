#include "AccountManager.hpp"

AccountManager::AccountManager()
{
}

AccountManager::~AccountManager()
{
}

int AccountManager::addAccount()
{
    accounts.push_back(std::shared_ptr<Account>(new Account()));
    miss.push_back(0);
    old_pos.push_back(startPos);
    old_dir.push_back("0 0 0");
    std::vector<unsigned int> l;
    sendList.push_back(l);
    logined.push_back(0);
    dos.push_back(0);
    help.push_back(0);
    accounts[accounts.size() - 1]->startReceive();
    //accounts[accounts.size()-1]->startUDPReceive();
    accountsNumber++;
    return 0;
}

int AccountManager::deleteAccount(size_t l)
{
    try
    {
        //accounts[l]->~Account();
        accounts.erase(accounts.begin() + l);
        miss.erase(miss.begin() + l);
        old_pos.erase(old_pos.begin() + l);
        old_dir.erase(old_dir.begin() + l);
        sendList.erase(sendList.begin() + l);
        logined.erase(logined.begin() + l);
        dos.erase(dos.begin() + l);
        help.erase(help.begin() + l);
    }
    catch (...)
    {
        std::cout << "DELETE FAILURE" << endl;
        return -1;
    }

    return 0;
}

bool AccountManager::findID(unsigned int ID)
{
    try
    {
        for (int i = 0; i < loginnun; i++)
        {
            if (accounts[i]->myID == ID) { return 1; }
        }
    }
    catch (...)
    {
        return 0;
    }

    return 0;
}

bool AccountManager::login(int account, string str)
{
    try
    {
        if (miss[account] < 3)
        {
            if (!logined[account])
            {
                string strs = str;
                string mail = str.substr(0, strs.find(" "));
                strs.erase(0, mail.length() + 1);
                string str;
                int id = 0;
                bool k = data.login(mail, strs, &id, &str);

                if (id != 0 && k)
                {
                    if (!findID(id))
                    {
                        string es("SUCCESS " + to_string(id) + " " + str);
                        accounts[account]->encryptByAes(es);
                        accounts[account]->send(3, "LOGIN", es);
                        accounts[account]->myID = id;
                        accounts[account]->name = str;
                        string sp(startPos);
                        accounts[account]->encryptByAes(sp);
                        accounts[account]->send(3, "SETPOS", sp);
                        std::cout << "Login : id " << id << " name " << str << std::endl;
                        loginnun++;
                        logined[account] = 1;
                        return 1;
                    }
                }
            }
        }
        else
        {
            string sp("MISTAKE_TOO_MUCH ");
            accounts[account]->encryptByAes(sp);
            accounts[account]->send(3, "LOGIN", sp);
            return 1;
        }

        string sp("FAILURE ");
        accounts[account]->encryptByAes(sp);
        accounts[account]->send(3, "LOGIN", sp);
        miss[account]++;
        return 1;
    }
    catch (...)
    {
        return 0;
    }
}

bool AccountManager::regst(int account, string str)
{
    try
    {
        if (miss[account] < 3)
        {
            string strs = str;
            string mail = strs.substr(0, strs.find(" "));
            strs.erase(0, mail.length() + 1);
            string pass = strs.substr(0, strs.find(" "));
            strs.erase(0, pass.length() + 1);
            string result;
            int id = 0;
            bool k = data.regst(mail, pass, strs, &id);

            if (id != 0 && k & !logined[account])
            {
                result = "SUCCESS " + to_string(id);
                accounts[account]->myID = id;
                accounts[account]->name = strs;
                std::cout << "Regst : id " << id << " name " << strs << std::endl;
                loginnun++;
                logined[account] = 1;
            }
            else
            {
                result = "FAILURE ";
                miss[account]++;
            }

            string es(result);
            accounts[account]->encryptByAes(es);
            accounts[account]->send(3, "REGST", es);
            string sp(startPos);
            accounts[account]->encryptByAes(sp);
            accounts[account]->send(3, "SETPOS", sp);
            return 1;
        }
        else
        {
            string sp("MISTAKE_TOO_MUCH ");
            accounts[account]->encryptByAes(sp);
            accounts[account]->send(3, "REGST", sp);
            return 1;
        }
    }
    catch (...)
    {
        return 0;
    }
}

bool AccountManager::pos(int account, string str)
{
    try
    {
        //座標確認
        string pos = str, oldPos = old_pos[account];
        bool illegal = 0;

        for (int i = 0; i < 3; i++)
        {
            float np = stod(pos.substr(0, pos.find(" ")));
            pos.erase(0, pos.find(" ") + 1);
            float op = stod(oldPos.substr(0, oldPos.find(" ")));
            oldPos.erase(0, oldPos.find(" ") + 1);

            if (np < op + 700.f && np > op - 700.f)
            {
                if (i == 1 && np < -100.f) { illegal = 1; }
            }
            else
            {
                illegal = 1;
            }
        }

        if (illegal == 0)
        {
            for (int i = 0; i < sendList[account].size(); i++)
            {
                if (sendList[account][i] < accounts.size() - 1)
                {
                    if (logined[sendList[account][i]] == 1)
                    {
                        string sp(to_string(accounts[account]->myID) + " " + str);
                        accounts[sendList[account][i]]->encryptByAes(sp);
                        accounts[sendList[account][i]]->send(3, "POS", sp);
                    }
                }
            }

            old_pos[account] = str;
            //ping用
            string ok("OK");
            accounts[account]->encryptByAes(ok);
            accounts[account]->send(3, "POS", ok);
        }
        else
        {
            string sp(startPos);
            accounts[account]->encryptByAes(sp);
            accounts[account]->send(3, "SETPOS", sp);
            old_pos[account] = startPos;
        }
    }
    catch (...)
    {
        return 0;
    }

    return 1;
}

bool AccountManager::dir(int account, string str)
{
    try
    {
        string dir = str, oldDir = old_dir[account];
        bool illegal = 0;

        for (int i = 0; i < 2; i++)
        {
            float np = stod(dir.substr(0, dir.find(" ")));
            dir.erase(0, dir.find(" ") + 1);
            float op = stod(oldDir.substr(0, oldDir.find(" ")));
            oldDir.erase(0, oldDir.find(" ") + 1);

            if (np > op + 3.f && np < op - 3.f)
            {
                illegal = 1;
            }
        }

        float np = stod(dir);
        float op = stod(oldDir);

        if (np > op + 3.f && np < op - 3.f)
        {
            illegal = 1;
        }

        if (illegal == 0)
        {
            for (int i = 0; i < sendList[account].size(); i++)
            {
                if (sendList[account][i] < accounts.size() - 1)
                {
                    if (logined[sendList[account][i]] == 1)
                    {
                        string sp(to_string(accounts[account]->myID) + " " + str);
                        accounts[sendList[account][i]]->encryptByAes(sp);
                        accounts[sendList[account][i]]->send(3, "DIR", sp);
                    }
                }
            }
        }
        else
        {
            std::cout << "FAILURE" << endl;
        }
    }
    catch (...)
    {
        return 0;
    }

    return 1;
}

bool AccountManager::sendlist(int account)
{
    try
    {
        if (logined[account] == 1)
        {
            sendList[account].clear();

            for (int i = 0; i < loginnun; i++)
            {
                if (i != account)
                {
                    string pos = old_pos[account];
                    string oldPos = old_pos[i];
                    bool nr = 0;

                    for (int j = 0; j < 3; j++)
                    {
                        float np = stod(pos.substr(0, pos.find(" ")));
                        pos.erase(0, pos.find(" ") + 1);
                        float op = stod(oldPos.substr(0, oldPos.find(" ")));
                        oldPos.erase(0, oldPos.find(" ") + 1);

                        if (np < op + 500.f && np > op - 500.f)
                        {
                            nr = 1;
                        }
                        else
                        {
                            nr = 0;
                        }
                    }

                    if (nr) { sendList[account].push_back(i); }
                }
            }

            if (sendList[account].size() != 0)
            {
                string usrs = "";

                for (int i = 0; i < sendList[account].size(); i++)
                {
                    usrs.append(to_string(accounts[sendList[account][i]]->myID) + " " + 
                    accounts[sendList[account][i]]->name + " " + old_pos[sendList[account][i]]);

                    string model = data.getModelData(accounts[sendList[account][i]]->myID);
                    if(model != ""){
                        usrs.append(model + " ");
                    } else {
                        usrs.append("0 ");
                    }
                }

                accounts[account]->encryptByAes(usrs);
                accounts[account]->send(3, "LIST", usrs);
            }
            else
            {
                string usrs = "0";
                accounts[account]->encryptByAes(usrs);
                accounts[account]->send(3, "LIST", usrs);
            }
        }
    }
    catch (...)
    {
        return 0;
    }

    return 1;
}


void AccountManager::update()
{
    size_t timer = 0;

    while (1)
    {
        for (int l = 0; l < accountsNumber; l++)
        {
            if (accounts[l]->getMessageSize() > 0)//tcp
            {
                string str = accounts[l]->getMessage();
                dos[l] += str.size();

                if (dos[l] > 1000) { accounts[l]->end = 1; }

                if (str != string("end") && !accounts[l]->end)
                {
                    string id = std::to_string((unsigned char)str[0]);

                    if (id == "1")//plane
                    {
                        string mess = str.substr(1, str.find(" "));
                        str.erase(0, mess.length() + 1);

                        if (mess == "GET ")
                        {
                            //rsaキーを送る
                            accounts[l]->sendRsaPublicKey();
                            accounts[l]->signature();
                        }
                    }
                    else if (id == "2")//rsa
                    {
                        string mess = str.substr(1, str.find(" "));
                        str.erase(0, mess.length() + 1);
                        string dmess = accounts[l]->decryptionByRsa(str);

                        if (mess == "AES ")
                        {
                            //aes共通鍵を受け取る
                            string key = dmess.substr(0, dmess.find(" "));
                            dmess.erase(0, key.length() + 1);
                            help[l] = 0;

                            if (accounts[l]->makeAESKey(key, dmess))
                            {
                                string s(" ");
                                accounts[l]->send(1, "KEY", s);
                                cout << key.size() << " : " << dmess.size() << endl;
                                cout << key << " : " << dmess << endl;
                                accounts[l]->end = 1;
                            }
                        }
                    }
                    else if (id == "3")//aes
                    {
                        if (!help[l])
                        {
                            string mess = str.substr(1, str.find(" "));
                            str.erase(0, mess.length() + 1);
                            string dmess(str);
                            accounts[l]->decryptionByAes(dmess);

                            //cout << "Log : " << mess << dmess << endl;

                            if (mess == "LOGIN ")
                            {
                                if (!login(l, dmess))
                                {
                                    accounts[l]->end = 1;
                                }
                            }
                            else if (mess == "REGST ")
                            {
                                if (!regst(l, dmess))
                                {
                                    accounts[l]->end = 1;
                                }
                            }
                            else if (mess == "POS ")
                            {
                                if (!pos(l, dmess))
                                {
                                    string s = "";
                                    accounts[l]->send(1, "HELP", s);
                                    help[l] = 1;
                                    cout << "HELP" << endl;
                                }
                            }
                            else if (mess == "STATE ")
                            {
                                for (int i = 0; i < sendList[l].size(); i++)
                                {
                                    if (sendList[l][i] < accounts.size() - 1)
                                    {
                                        if (logined[sendList[l][i]] == 1)
                                        {
                                            string sp(to_string(accounts[l]->myID) + " " + dmess);
                                            accounts[sendList[l][i]]->encryptByAes(sp);
                                            accounts[sendList[l][i]]->send(3, "STATE", sp);
                                        }
                                    }
                                }
                            }
                            else if (mess == "DIR ")
                            {
                                if (!dir(l, dmess))
                                {
                                    string s = "";
                                    accounts[l]->send(1, "HELP", s);
                                    help[l] = 1;
                                    cout << "HELP" << endl;
                                }
                            }
                            else if (mess == "RES ")
                            {
                                //for (int i = 0; i < sendList[l].size(); i++) {
                                //	if (sendList[l][i] < accounts.size() - 1) {
                                //		if (logined[sendList[l][i]] == 1) {
                                //			string sp(to_string(accounts[l]->myID) + " " + dmess);
                                //			accounts[sendList[l][i]]->encryptByAes(sp);
                                //			accounts[sendList[l][i]]->send(3, "RES", sp);
                                //		}
                                //	}
                                //}
                                //全体チャット
                                for (int i = 0; i < accountsNumber; i++)
                                {
                                    string sp(to_string(accounts[l]->myID) + " " + dmess);
                                    accounts[i]->encryptByAes(sp);
                                    accounts[i]->send(3, "RES", sp);
                                }
                            }
                            else if (mess == "UP_M ")
                            {
                                /*string err;
                                auto json = json11::Json::parse(dmess, err);
                                int model_id = data.regstModel(accounts[l]->myID, json["model_name"].string_value(), json["model_file_name"].string_value(), json["model_type"].int_value());

                                if (model_id != -1)
                                {
                                    //アクセスキーを発行
                                    string key = data.createAccessKey(model_id);
                                    accounts[l]->encryptByAes(key);
                                    accounts[l]->send(3, "ACCESS", key);
                                }*/
                            }
                            else if (mess == "DL ")
                            {
                                string err;
                                auto json = json11::Json::parse(dmess, err);
                                int model_id = json["model_id"].int_value();
                       
                                cout << model_id << endl;
                                string model_data = data.getModelDataDetail(model_id);

                                string key = data.createAccessKey(model_id);
                                cout << key << endl;

                                string contens("{\"access_key\":\"" + key + "\"," + model_data + "}");

                                accounts[l]->encryptByAes(contens);
                                accounts[l]->send(3, "ACCESS", contens);
                            }
                        }
                    }
                }
                else
                {
                    accounts[l]->end = 1;
                }

                if (accounts[l]->end)
                {
                    if (logined[l] == 1) { loginnun--; }

                    accountsNumber--;
                    deleteAccount(l);
                    break;
                }
            }
        }

        if (timer == 5000)
        {
            for (int l = 0; l < loginnun; l++)
            {
                sendlist(l);
                dos[l] = 0;
            }

            timer = 0;
        }

        timer++;
        //早すぎるとエラーが出るので
        boost::this_thread::sleep(boost::posix_time::milliseconds(1));
    }
}


int AccountManager::connection()
{
    bool startup = 0;
    std::thread t;

    while (1)
    {
        addAccount();

        if (!startup)
        {
            t = std::thread(&AccountManager::update, this);
            t.detach();
            startup = 1;
        }
    }
}
