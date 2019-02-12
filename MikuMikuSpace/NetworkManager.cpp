#include "NetworkManager.h"



NetworkManager::NetworkManager(std::shared_ptr<Bullet_physics> bullet_, int world_, std::shared_ptr<Character> cahara_,
                               std::shared_ptr<ModelManager> model_manager_) : players(new MultiPlayers(bullet_, world_))
{
    cahara = cahara_;
    model_manager = model_manager_;
}


NetworkManager::~NetworkManager()
{
}


bool NetworkManager::connect()
{
    if (network.connect(coneection) != -1 && network.makeUDPSocket(coneection) != -1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

bool NetworkManager::startCommunication()
{
    network.setid(0);

    if (network.send(1, "GET", "", 0, 0) != -1)
    {
        network.receive_start();
        //network.UDP_receive_start();
        t = std::thread(&NetworkManager::update, this);
        t.detach();
        return 1;
    }

    return 0;
}

bool NetworkManager::login(std::string mail, std::string pass)
{
    network.setid(0);
    std::string s(mail + " " + pass);
    network.send(3, "LOGIN", s, 0, 1, 0, 1);
    return 1;
}

bool NetworkManager::signUp(std::string mail, std::string pass, std::string nane)
{
    network.setid(0);
    std::string s(mail + " " + pass + " " + nane);
    network.send(3, "REGST", s, 0, 1, 0, 1);
    network.setname(nane);
    return 1;
}

void NetworkManager::update()
{
    FpsManager fps;

    //別スレッドで立ち上げ 60fps
    while (!ProcessMessage())
    {
        if (network.getMassegeSize() > 0)//tcp
        {
            string str = network.getMassege();
            const unsigned char command = str[0];

            if (command == (unsigned char)1)  //Plane
            {
                string mess = str.substr(1, str.find(" "));
                str.erase(0, mess.length() + 1);

                if (mess == "KEY ")
                {
                    if (!cpt)
                    {
                        //rsa公開鍵を受け取る
                        network.getRsaPublicKey(str);
                        //aes共通鍵を暗号化して送る
                        network.makeAESKey();
                        network.setid(1);
                        cpt = 1;
                    }
                    else
                    {
                        MessageBox(GetMainWindowHandle(), "不明なエラー", "error", MB_OK);
                        break;
                    }
                }
                else if (mess == "HELP ")
                {
                    network.makeAESKey();
                }
            }
            else if (command == (unsigned char)2)  //RSA
            {
                string mess = str.substr(1, str.find(" "));
                str.erase(0, mess.length() + 1);

                if (mess == "SIGN ")
                {
                    if (!network.signature(str))
                    {
                        setCorrect(0);
                        break;
                    }
                }
            }
            else if (command == (unsigned char)3)  //AES
            {
                string mess = str.substr(1, str.find(" "));
                str.erase(0, mess.length() + 1);
                //暗号を解く
                string dstr(str);
                network.decryptionByAes(dstr);

                if (mess == "LOGIN ")
                {
                    string result = dstr.substr(0, dstr.find(" "));
                    dstr.erase(0, result.length() + 1);

                    if (result == "SUCCESS")
                    {
                        string id = dstr.substr(0, dstr.find(" "));
                        dstr.erase(0, id.length() + 1);
                        string name = dstr.substr(0, dstr.find(" "));
                        dstr.erase(0, name.length() + 1);

						// モデルID取得
						int model_id = stoi(dstr.substr(0, dstr.find(" ")));
						dstr.erase(0, name.length() + 1);
						if (model_id != 0)
						{
							cahara->setModelID(model_id);
							// moddel_idをmodelmanagerに問い合わせ
							int handle = model_manager->getModel(model_id);

							if (handle != -1)
							{
								cahara->setModel(handle);
							}
							else
							{
								//ダウンロードする必要あり
								network.send(3, "DL", u8"{\"model_id\" :" + std::to_string(model_id) + "}", 0, 1, 0, 1);
								isCharaDownload = true;
							}
						} 
						else 
						{
							cahara->setModelID(-1);
							cahara->setModel(model_manager->getModel(0));
						}
                        network.setid(stoi(id));
                        network.setname(name);
                    }
                    else if (result == "FAILURE") { network.setid(-1); }
                    else if (result == "MISTAKE_TOO_MUCH") { network.setid(-2); }
                }
                else if (mess == "REGST ")
                {
                    string result = dstr.substr(0, dstr.find(" "));
                    dstr.erase(0, result.length() + 1);
                    string id = dstr.substr(0, dstr.find(" "));
                    dstr.erase(0, id.length() + 1);

                    if (result == "SUCCESS")
                    {
                        network.setid(stoi(id));
                    }
                    else if (result == "FAILURE") { network.setid(-1); }
                    else if (result == "MISTAKE_TOO_MUCH") { network.setid(-2); }
                }
                else if (mess == "SETPOS ")
                {
                    float t[7];

                    if (dstr.find(" ") != string::npos)
                    {
                        for (int i = 0; i < 7; i++)
                        {
                            t[i] = stod(dstr.substr(0, dstr.find(" ")));
                            dstr.erase(0, dstr.find(" ") + 1);
                        }
                    }

                    btVector3 bt = btVector3(t[0], t[1], t[2]);
                    btQuaternion bq = btQuaternion(t[3], t[4], t[5], t[7]);
                    cahara->setPosRotBullet(bt, bq);
                }
                else if (mess == "LIST ")
                {
                    try
                    {
                        vector<string> elems;
                        string item;

                        for (char ch : dstr)
                        {
                            if (ch == ' ')
                            {
                                if (!item.empty())
                                {
                                    elems.push_back(item);
                                }

                                item.clear();
                            }
                            else
                            {
                                item += ch;
                            }
                        }

                        if (!item.empty())
                        {
                            elems.push_back(item);
                        }

                        std::vector<unsigned int> usrid;

                        if (elems[0] != "0")
                        {
                            for (int i = 0; i < elems.size() / 10; i++)
                            {
                                unsigned int uid = stoi(elems[i * 10]);
                                usrid.push_back(uid);

                                if (players->findPlayerID(uid) == -1)
                                {
                                    players->addPlayer(uid, elems[i * 10 + 1]);

                                    if (elems[i * 14 + 9] != "0")
                                    {
                                        int model_id = stoi(elems[i * 14 + 9]);
                                        // moddel_idをmodelmanagerに問い合わせ
                                        int handle = model_manager->getModel(model_id);

                                        if (handle != -1)
                                        {
                                            user_modelid[model_id] = std::pair<int, int>(uid, 0);
                                            players->setModel(uid, handle);
                                        }
                                        else
                                        {
                                            user_modelid[model_id] = std::pair<int, int>(uid, 1);
                                            //ダウンロードする必要あり
                                            network.send(3, "DL", u8"{\"model_id\" :" + std::to_string(model_id) + "}", 0, 1, 0, 1);
                                        }

                                        btVector3 bt = btVector3(stod(elems[i * 10 + 2]), stod(elems[i * 10 + 3]), stod(elems[i * 10 + 4]));
                                        btQuaternion bq = btQuaternion(stod(elems[i * 10 + 5]), stod(elems[i * 10 + 6]), stod(elems[i * 10 + 7]), stod(elems[i * 10 + 8]));
                                        players->setPosBullet(uid, bt, bq);
                                    }
                                    else
                                    {
                                        //初期モデル
                                        players->copyModel(uid, model_manager->getModel(0));
                                        players->setup(uid);
                                        players->addPlayerSize();
                                        btVector3 bt = btVector3(stod(elems[i * 10 + 2]), stod(elems[i * 10 + 3]), stod(elems[i * 10 + 4]));
                                        btQuaternion bq = btQuaternion(stod(elems[i * 10 + 5]), stod(elems[i * 10 + 6]), stod(elems[i * 10 + 7]), stod(elems[i * 10 + 8]));
                                        players->setPosBullet(uid, bt, bq);
                                    }
                                }
                            }
                        }
                        else
                        {
                            //誰もいない
                            for (int j = 0; j < players->getMultiplayerSize(); j++)
                            {
                                unsigned int uid = players->getPlayerID(j);
                                players->deletePlayer(uid);
                                auto itr = user_modelid.find(uid);

                                if (itr != user_modelid.end()) { user_modelid.erase(itr); }
                            }
                        }

                        if (usrid.size() != 0)
                        {
                            for (int i = 0; i < players->getMultiplayerSize(); i++)
                            {
                                bool find = 0;
                                size_t findpos = 0;

                                for (int j = 0; j < usrid.size(); j++)
                                {
                                    if (players->getPlayerID(i) == usrid[j]) { find = 1; }
                                }

                                if (!find)
                                {
                                    players->deletePlayer(usrid[i]);
                                    auto itr = user_modelid.find(usrid[i]);

                                    if (itr != user_modelid.end()) { user_modelid.erase(itr); }
                                }
                            }
                        }
                    }
                    catch (...)
                    {
                    }
                }
                else if (mess == "POS ")
                {
                    try
                    {
                        if (dstr != "OK")
                        {
                            string userid = dstr.substr(0, dstr.find(" "));
                            dstr.erase(0, userid.length() + 1);
                            unsigned int uid = stoi(userid);

                            if (players->findPlayerID(uid) != -1)
                            {
                                float t[7];

                                for (int i = 0; i < 7; i++)
                                {
                                    t[i] = atof(dstr.substr(0, dstr.find(" ")).c_str());
                                    dstr.erase(0, dstr.find(" ") + 1);
                                }

                                btVector3 bt = btVector3(t[0], t[1], t[2]);
                                btQuaternion bq = btQuaternion(t[3], t[4], t[5], t[6]);
                                players->setPosBullet(uid, bt, bq);
                            }
                        }
                        else
                        {
                            //ping用
                            ping_show = GetNowCount() - ping;
                        }
                    }
                    catch (...)
                    {
                    }
                }
                else if (mess == "RES ")
                {
                    string userid = dstr.substr(0, dstr.find(" "));
                    dstr.erase(0, userid.length() + 1);
                    unsigned int uid = stoi(userid);

                    if (players->findPlayerID(uid) != -1)
                    {
                        chat.push_back(players->getName(uid) + " : " + dstr);
                    }
                    else if (uid == 0)
                    {
                        chat.push_back(u8"システム : " + dstr);
                    }
                }
                else if (mess == "STATE ")
                {
                    try
                    {
                        string userid = dstr.substr(0, dstr.find(" "));
                        dstr.erase(0, userid.length() + 1);
                        unsigned int uid = stoi(userid);

                        if (players->findPlayerID(uid) != -1)
                        {
                            players->setState(uid, dstr);
                        }
                    }
                    catch (...)
                    {
                    }
                }
                else if (mess == "DIR ")
                {
                    try
                    {
                        string userid = dstr.substr(0, dstr.find(" "));
                        dstr.erase(0, userid.length() + 1);
                        unsigned int uid = stoi(userid);

                        if (players->findPlayerID(uid) != -1)
                        {
                            string dir1 = dstr.substr(0, dstr.find(" "));
                            dstr.erase(0, dir1.length() + 1);
                            string dir2 = dstr.substr(0, dstr.find(" "));
                            dstr.erase(0, dir2.length() + 1);
                            string dir3 = dstr.substr(0, dstr.find(" "));
                            dstr.erase(0, dir3.length() + 1);
                            VECTOR D = VGet(stod(dir1), stod(dir2), stod(dir3));
                            players->setTurningSpeed(uid, stod(dstr));
                            players->setDirection(uid, D);
                        }
                    }
                    catch (...)
                    {
                    }
                }
                else if (mess == "ACCESS ")
                {
                    string err;
                    auto json = json11::Json::parse(dstr, err);
                    model_manager->createConfig(json["model_id"].int_value(), json.dump());
					if (cahara->getModelID() != json["model_id"].int_value())
					{
						int handle = model_manager->downloadModel(json["model_id"].int_value(),
							json["access_key"].string_value(), players->getModelHandle(user_modelid[json["model_id"].int_value()].first));
					}
					else
					{
						// プレイヤーのモデル
						int handle = model_manager->downloadModel(json["model_id"].int_value(),
							json["access_key"].string_value(), cahara->getModelHandlePointer());
					}
                }
            }
            else if (str.substr(0, 6) == "SYSTEM")
            {
                str.erase(0, str.find(" ") + 1);
                chat.push_back(u8"システム : " + network.convertUTF8(str));
            }
        }

        fps.controlWaitFps();
    }
}

void NetworkManager::setCorrect(bool c)
{
    CorrectVersion = c;
}

int NetworkManager::getid()
{
    return network.getid();
}
string NetworkManager::getname()
{
    return network.getname();
}

void NetworkManager::sendLoop()
{
    FpsManager fps;
    size_t counter = 0;

    //別スレッドで立ち上げ 60fps
    while (!ProcessMessage())
    {
        if (counter % 30 == 0)
        {
            //direction
            VECTOR d = cahara->gertDirection();
            float ts = cahara->getTurningSpeed();

            if (VSize(d) != 0 || ts != 0.f)
            {
                string dir1 = to_string(d.x);
                string dir2 = to_string(d.y);
                string die3 = to_string(d.z);
                string sts = to_string(ts);
                string s = (dir1.substr(0, dir1.size() - 3) + " " + dir2.substr(0, dir2.size() - 3) + " " + die3.substr(0, die3.size() - 3) + " " + sts.substr(0, sts.size() - 1));
                network.send(3, "DIR", s, 0, 1, 0, 1);
                sendDir = 0;
            }
            else
            {
                if (!sendDir)
                {
                    string dir1 = to_string(d.x);
                    string dir2 = to_string(d.y);
                    string die3 = to_string(d.z);
                    string sts = to_string(ts);
                    string s = (dir1.substr(0, dir1.size() - 3) + " " + dir2.substr(0, dir2.size() - 3) + " " + die3.substr(0, die3.size() - 3) + " " + sts.substr(0, sts.size() - 1));
                    network.send(3, "DIR", s, 0, 1, 0, 1);
                    sendDir = 1;
                }
            }
        }

        string state = cahara->getState();

        if (state != oldState)
        {
            //state
            oldState = state;
            network.send(3, "STATE", state, 0, 1, 0, 1);
        }

        if (counter >= 300)
        {
            //position
            string posi1 = to_string((int)cahara->getPosBullet().x);
            string posi2 = to_string((int)cahara->getPosBullet().y);
            string posi3 = to_string((int)cahara->getPosBullet().z);
            string posi4 = to_string(cahara->getRotBullet().getX());
            posi4 = posi4.substr(0, posi4.find('.') + 4);
            string posi5 = to_string(cahara->getRotBullet().getY());
            posi5 = posi5.substr(0, posi5.find('.') + 4);
            string posi6 = to_string(cahara->getRotBullet().getZ());
            posi6 = posi6.substr(0, posi6.find('.') + 4);
            string posi7 = to_string(cahara->getRotBullet().getW());
            posi7 = posi7.substr(0, posi7.find('.') + 4);
            std::string s(posi1 + " " + posi2 + " " + posi3 + " " + posi4 + " " + posi5 + " " + posi6 + " " + posi7 + " ");
            network.send(3, "POS", s, 0, 1, 0, 1);
            //ping用
            ping = GetNowCount();
            counter = 0;
        }

        counter++;
        fps.controlWaitFps();
    }
}

void NetworkManager::startSend()
{
    t2 = std::thread(&NetworkManager::sendLoop, this);
    t2.detach();
}

size_t NetworkManager::getChatMassegeSize()
{
    return chat.size();
}

std::string NetworkManager::getChatMassege()
{
    string m = chat[0];
    chat.erase(chat.begin());
    return m;
}

void NetworkManager::multiplayerDraw()
{
    players->draw();
}

void NetworkManager::multiplayerUpdate()
{
    players->playAnimeControl();
    players->update();

    if (user_modelid.size() != 0)
    {
        // まだ読み込みが完了していないモデルを更新
        for (auto usr = user_modelid.begin(); usr != user_modelid.end(); ++usr)
        {
            // 設定完了
            if (usr->second.second == 2) { continue; }

            if (usr->second.second == 0)
            {
                // ロードが終了したか確認
                if (!model_manager->isLoading(usr->first))
                {
                    // セットアップ
                    players->setup(usr->second.first);
                    players->addPlayerSize();
                    usr->second.second = 2;
                }
            }
            else if (usr->second.second == 1)
            {
                // ダウンロードが完了したか確認
                if (!model_manager->isDownloading(usr->first))
                {
                    players->setup(usr->second.first);
                    players->addPlayerSize();
                    usr->second.second = 2;
                }
            }
        }
    }
}

size_t NetworkManager::getMultiplayerSize()
{
    return players->getMultiplayerSize();
}

int NetworkManager::sendChat(string str)
{
    return network.send(3, "RES", str, 0, 1, 0, 1);
}

/*
void NetworkManager::uploadModel(string filepath)
{
    network.send(3, "UP_M", u8"{\"model_name\" : \"どっと式初音ミク_V3\", \"model_file_name\" : \"どっと式初音ミク_V3.mv1\", \"model_type\" : 1}", 0, 1, 0, 1);

    while (access_key.length() == 0);

    WebDownloader p;
    p.httpsOpen("192.168.0.19");
    p.httpPUT("Log.txt", "accesskey=" + access_key);
    p.StartPutting();

    while (p.getPutSize() != p.getPuttedSize());

    string s = p.getMessege();
}

void NetworkManager::downloadModel(int model_id)
{
    network.send(3, "DL", u8"{\"model_id\" :" + std::to_string(model_id) + "}", 0, 1, 0, 1);

    while (access_key.length() == 0);

    WebDownloader *dl = new WebDownloader();
    dl->httpsOpen("119.171.85.139");
    int r = dl->httpGET(
                (char *)"MikuMikuOpenWorld.zip",
                (char *)"119.171.85.139",
                (char *)"./", //.exeと同じディレクトリなら「"./"」
                (char *)"MikuMikuOpenWorld.zip",                     //拡張子も必要なので注意
                "accesskey=" + access_key
            );

    if (r == 0) { dl->StartDownload(); }

    FpsManager fps;

    while (dl->getDownloadSize() != dl->getReadSize())
    {
        DrawFormatString(0, 0, -1, "%d / %d", dl->getDownloadSize(), dl->getReadSize());
        fps.displayFps(1280 - 20, 0);
        fps.controlClearDrawScreenFps();
    }

    return;
}
*/

void NetworkManager::displayPing(int x, int y, int color)
{
    DrawFormatString(x, y, color, "%dms", ping_show);
}

string NetworkManager::getServerDetail()
{
    if (coneection.find("192") != string::npos) { return coneection + " (Local)"; }

    if (coneection.find("119.171.85.139") != string::npos) { return coneection + " (Home)"; }

    return coneection + " (AWS)";
}