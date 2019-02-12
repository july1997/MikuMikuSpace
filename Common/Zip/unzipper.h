#pragma once
#include "DxLib.h"

#include "ioapi.h"
#include "unzip.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

#pragma comment(lib, "zlibstat.lib")
using namespace std;

namespace ziputils
{
    class unzipper
    {
        public:
            unzipper();
            ~unzipper();

            bool openZip(string path, string outfiledir);

			// 解凍開始（zipファイルは自動で消えます）
            void startunzip();

            size_t getNoFilewSize();
            size_t getZipFileSize();

            bool isComplete();

            void unzip();

			bool removeDirectory(std::string fileName);

        private:

            unzFile hUnzip;
            size_t zipfilesize, nowfilesize = 0.0;
            string openFilePath, strTargetPath;

            bool installed = 0;

            bool IsFileExist(const string &strFilename);

            bool CreateDirectoryReflex(const string &strPath);

    };
};
