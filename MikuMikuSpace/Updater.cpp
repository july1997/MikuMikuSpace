#include "Updater.h"

Updater::Updater()
{
}


Updater::~Updater()
{
}

std::string Updater::downloadRelese()
{
    dl = new WebDownloader();
    //https://api.github.com/repos/july1997/MikuMikuOpenWorld/releases
    dl->httpsOpen(MMOW_Server);
    int r = dl->httpGET(
                (char *)"/releases",
                (char *)MMOW_Server,
                (char *)"./", //.exeと同じディレクトリなら「"./"」
                (char *)""                     //拡張子も必要なので注意
            );
    std::string relese = dl->getMessege();
    std::ofstream outputfile("System/Update/releases.txt");
    outputfile << relese;
    outputfile.close();
    return relese;
}

int Updater::getDownloadSize()
{
    return dl->getDownloadSize();
}

int Updater::getReadSize()
{
    return dl->getReadSize();
}

int Updater::checkUpdate(std::string relese)
{
    delete dl;

    if (relese == "") { return 0; }

    std::string err;
    auto json = json11::Json::parse(relese, err);

    if (json["releases"]["version"] != MMOW_version)
    {
        return 1;
    }

    return 0;
}

void Updater::lunchUpdater()
{
    SHELLEXECUTEINFO sei;
    memset(&sei, 0, sizeof(sei));
    sei.cbSize = sizeof(sei);
    sei.hwnd = NULL;
    sei.lpVerb = _T("open");
    sei.lpFile = "Updater.exe";
    sei.nShow = SW_SHOWNORMAL;

    //標準出力を獲得するならCreateProcessを使う
    if (ShellExecuteEx(&sei) == FALSE)
    {
        //失敗した場合のエラー処理
        MessageBox(GetMainWindowHandle(), "ShellExecuteEx Failed.", "error", MB_OK);//エラー表示
    }
}