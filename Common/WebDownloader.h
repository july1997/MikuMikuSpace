#pragma once
#include "DxLib.h"
#include "Logger.h"

#include <fstream>
#include <thread>
#include <string>

#include <Wininet.h>
#pragma comment(lib, "Wininet.lib")

//https://the-united-front.blogspot.jp/2014/05/wininetinternetreadfilehtml.html 様参考
//-106のエラーはセキュリティソフトに弾かれてる可能性あり
//
//返り値一覧
//
//-106:ERR_INTERNET_DISCONNECTED        インターネットに接続されていない。
//-403:"403 Forbidden"                                サーバーにリクエストが拒否された。
//-404:"404 Not Found"                                指定したURLにドキュメントが存在しない。
//-1

class WebDownloader
{
    private:
        HINTERNET hInternet;
        HINTERNET hHttpSession;
        HINTERNET hHttpRequest;
        DWORD RequiredBufSize, RequiredBufSize_Size;
        DWORD ReadSize;        //一回の操作で読みだされたサイズ
        DWORD ReadSize_sum;        //読み出しサイズの合計
        DWORD dwReadSize;
        FILE *file;
        char *Buf_main;
        bool isOpen = 0;
        std::string Directory;
        std::string Name;
        HINTERNET hRequest;
        HANDLE hFile;
        DWORD filesize;//ファイル全体のサイズ
        DWORD sum;//送ったファイルサイズ

        int WebDownloading();
        int WebPutting();

    public:
        WebDownloader();
        ~WebDownloader();

        //非同用
        int httpOpen(const char *Server_Host_Address);
        int httpsOpen(const char *Server_Host_Address);

        int httpGET(const char *FileLocation, const char *Referer, std::string SaveDirectory, std::string SaveName, std::string query = "");
        int httpPOST(const char *FileLocation, const char *Referer, const char *SaveDirectory, const char *SaveName, const char *poststr);
        BOOL httpPUT(std::string upFile, std::string query = "");

        //容量の大きいファイルに使用
        int StartDownload();
        int StartPutting();

        //同期用
        std::string getMessege();

        //取得系
        int getDownloadSize();
        int getReadSize();
        bool getIsOpen();
        int getPutSize();
        int getPuttedSize();
};

