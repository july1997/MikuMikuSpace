#include "ModelManager.h"



ModelManager::ModelManager()
{
    dl = new WebDownloader();
}


ModelManager::~ModelManager()
{
}

void ModelManager::setup()
{
    modelhandles[0] = MV1LoadModel(default_model.c_str());
}

string ModelManager::getDefaultModelName()
{
    return default_model_name;
}

int ModelManager::findModel(int mode_id)
{
    // モデルが読み込まれているか検索
    auto itr = modelhandles.find(mode_id);

    if (itr != modelhandles.end())
    {
        return modelhandles[mode_id];
    }
    else
    {
        return -1;
    }
}

bool ModelManager::createConfig(int mode_id, string json_string)
{
    std::ofstream ofs(config_dir + to_string(mode_id) + ".json");

    if (ofs.fail()) { return 0; }

    string err;
    auto json = json11::Json::parse(json_string, err);

    if (err != "") { return 0; }

    json11::Json config = json11::Json::object(
    {
        {        "model_id", json["model_id"].int_value() },
        {      "model_name", json["model_name"].string_value() },
        { "model_file_name", json["model_file_name"].string_value()},
        { "model_file_pash", model_load_dir + "/" + to_string(json["model_id"].int_value())},
        {      "model_type", to_string(json["model_type"].int_value())},
    });
    ofs << config.dump() << std::endl;
    ofs.close();
    return 1;
}

int ModelManager::getModel(int mode_id)
{
    int fnd = findModel(mode_id);

    if (fnd != -1)
    {
        return fnd;
    }

    // configファイルに存在するか確認
    std::ifstream ifs(config_dir + to_string(mode_id) + ".json");

    if (ifs.fail())
    {
        return -1;
    }

    std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    string err;
    auto json = json11::Json::parse(str, err);

    // 非同期読み込み設定に変更
    //SetUseASyncLoadFlag(TRUE);

    if (json["model_type"].int_value() == 0)
    {
        //mv1以外で変更
        MV1SetLoadModelAnimFilePath("System/Motion/");
    }

    modelhandles[mode_id] = MV1LoadModel((model_load_dir + "/" + json["model_file_name"].string_value()).c_str());
    MV1SetLoadModelAnimFilePath(NULL);
    SetUseASyncLoadFlag(FALSE);
    return modelhandles[mode_id];
}

bool ModelManager::isLoading(int mode_id)
{
    return CheckHandleASyncLoad(modelhandles[mode_id]) == true;
}

int ModelManager::downloadModel(int mode_id, string access_key, int *modelHandle)
{
    task.push_back(modelHandle);
    access_keys.push_back(access_key);
    mode_ids.push_back(mode_id);
    return 0;
}

void ModelManager::update()
{
    //1件づつダウンロード
    if (task.size() != 0)
    {
        if (!dl->getIsOpen() && !start)
        {
            dl->httpsOpen(MMS_Server);
            int r = dl->httpGET(
                        server_baseurl.c_str(),
                        MMS_Server,
                        model_save_dir.c_str(),
                        (to_string(mode_ids[0]) + string(".zip")).c_str(),
                        "accesskey=" + access_keys[0]
                    );

            if (r == 0)
            {
                dl->StartDownload();
            }
            else
            {
                //失敗
                task.erase(task.begin());
                access_keys.erase(access_keys.begin());
                mode_ids.erase(mode_ids.begin());
            }

            finish = false;
            start = true;
        }

        if (!finish)
        {
            if (!dl->getIsOpen())
            {
                if (dl->getDownloadSize() != dl->getReadSize())
                {
                    DrawFormatString(0, 0, -1, "%d / %d", dl->getDownloadSize(), dl->getReadSize());
                }
                else
                {
                    finish = true;
                }
            }
        }
        else
        {
            //ファイルができるまで待つ
            WaitTimer(100);
            unzipper zip;
            zip.openZip(model_load_dir + "/" + to_string(mode_ids[0]) + string(".zip"), model_load_dir + "/");
            zip.unzip();
            // 完了
            *task[0] = getModel(mode_ids[0]);
            finish = false;
            start = false;
            task.erase(task.begin());
            access_keys.erase(access_keys.begin());
            mode_ids.erase(mode_ids.begin());
        }
    }
}

bool ModelManager::isDownloading(int mode_id)
{
    auto itr = std::find(mode_ids.begin(), mode_ids.end(), mode_id);
    size_t index = std::distance(mode_ids.begin(), itr);

    if (index != mode_ids.size())   // 発見できたとき
    {
        return 1;
    }
    else   // 発見できなかったとき
    {
        return 0;
    }
}