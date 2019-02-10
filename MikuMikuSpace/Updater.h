#pragma once

#include "DxLib.h"

#include "../Common/WebDownloader.h"
#include "../Common/Version.h"
#include "../Common/Server.h"

#include "../Common/Utility/json11.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

class Updater
{
        WebDownloader* dl;

        DWORD dwD;
        HANDLE hHandle;

    public:
        Updater();
        ~Updater();

        std::string downloadRelese();
        int checkUpdate(std::string relese);

        int getDownloadSize();
        int getReadSize();

        void lunchUpdater();
};

