#pragma once

#include "NetWork.h"

#include "../Common/Fps.h"

#include "Character.h"
#include "MultiPlayers.h"
#include "ModelManager.h"

#include "../Common/WebDownloader.h"
#include "../Common/Server.h"
#include "../Common/Utility/json11.hpp"

class NetworkManager
{
    public:
        NetworkManager(std::shared_ptr<Bullet_physics> bullet_, int world_, std::shared_ptr<Character> cahara_, std::shared_ptr<ModelManager> model_manager_);
        ~NetworkManager();

        bool connect();
        bool login(std::string mail, std::string pass);
        bool signUp(std::string mail, std::string pass, std::string nane);

        bool CorrectVersion = 1;

        int getid();
        string getname();

        void startSend();

        string oldState = "";
        bool sendDir = 0;

        size_t getChatMassegeSize();
        std::string getChatMassege();

        void multiplayerDraw();
        void multiplayerUpdate();

        bool startCommunication();

        size_t getMultiplayerSize();

        std::shared_ptr<Character> cahara;
		bool isCharaDownload = false;
        MultiPlayers *players;

        int sendChat(string str);

        int ping = 0, ping_show = 0;
        void displayPing(int x, int y, int color);

        string getServerDetail();

		std::shared_ptr<ModelManager> model_manager;

    private:

        std::string coneection = std::string(MMS_Server);

        int cpt = 0;
        vector<string> chatmessege;

        Network network;

        std::thread t;
        std::thread t2;

        void setCorrect(bool c);

        void update();

        void sendLoop();

        std::vector<std::string> chat;

        string access_key = "";

        std::map<int, std::pair<int, int>> user_modelid;
};

