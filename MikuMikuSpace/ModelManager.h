#pragma once
#include "DxLib.h"

#include "Model.h"
#include "Bullet_physics.h"

#include "../Common/WebDownloader.h"
#include "../Common/Server.h"
#include "../Common/Utility/json11.hpp"

#include "../Common/Fps.h"
#include "../Common/Zip/unzipper.h"

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>

using namespace std;
using namespace ziputils;

// モデルデータの管理とダウンロード、読込してモデルハンドルを返すクラス
class ModelManager
{
        std::map<int, int> modelhandles;
        vector<int*> task;
        vector<string> access_keys;
        vector<int> mode_ids;

        WebDownloader *dl;
        bool finish = false, start = false;

        const string model_save_dir = u8"./Models";
        const string model_load_dir = u8"Models";
        const string config_dir = "Models/.config/";
        const string mmd_anime_dir = "System/Motion";

        const string default_model = u8"System/Model/Tda式初音ミクV4X_Ver1.00/Tda式初音ミクV4X_Ver1.00/Tda式初音ミクV4X.mv1";
        const string default_model_name = u8"Tda式初音ミクV4X";

        const string server_baseurl = "/access";

        int findModel(int mode_id);

    public:
        ModelManager();
        ~ModelManager();

        void setup();
        string getDefaultModelName();

        //戻り値：モデルハンドル -1：エラー
        int getModel(int mode_id);

        //モデルの設定情報作成
        bool createConfig(int mode_id, string json_string);

        //戻り値：0 成功
        int downloadModel(int mode_id, string access_key, int *modelHandle);

        void update();

        // モデルが読み込み中か
        bool isLoading(int mode_id);

        // モデルがダウンロード中か
        bool isDownloading(int mode_id);

		int countTask();
};

