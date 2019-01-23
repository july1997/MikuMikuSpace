#pragma once
#include "../../Common/Version.h"

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <memory>

#include <boost/asio.hpp>
#include <boost/array.hpp>

#pragma comment( lib, "cryptlib.lib" )
#include <aes.h>
#include <rsa.h>
#include <dh.h>
#include <modes.h>
#include <osrng.h>
#include <hex.h>
#include <osrng.h>
#include <base64.h>
#include <files.h>
#include <pssr.h>
#include <random>
#include <bitset>

using namespace std;
namespace asio = boost::asio;
namespace ip = asio::ip;
using boost::asio::ip::udp;

class Account
{
    protected:

        asio::io_service *io_service;
        ip::tcp::socket sock;
        udp::socket udpsock;
        ip::address IP;

        std::vector<std::string> message;
        std::vector<std::string> command;
        std::vector<std::string> bigmessage;

        std::thread t;
        int receiveLoop();

        std::thread u;
        int receiveUDPLoop();

        CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption enc; //aes
        CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption dec;
        CryptoPP::RSA::PrivateKey privateKey;
        CryptoPP::RSA::PublicKey publicKey;

        bool sendRSAKey = 0;
    public:
        Account();
        ~Account();

        string name;
        string modelname;
        unsigned int myID;
        bool end = 0;

        //TCP
        int startReceive();

        int send(unsigned char Command, string method, string &Buffer);

        //UDP
        int startUDPReceive();

        int sendUDP(unsigned char PlayerID, string &Buffer);

        //暗号化
        int sendRsaPublicKey();
        std::string decryptionByRsa(std::string &str);
        std::string encryptByRsa(std::string &str);

        //署名と同時にバージョン確認
        void signature();

        int makeAESKey(std::string &Key_s, std::string &iv_s);
        void encryptByAes(std::string &str);
        void decryptionByAes(std::string &str);

        //get
        size_t getMessageSize();
        std::string getMessage();

        size_t getCommandSize();
        std::string getCommand();

        //変換
        std::string convertBufferToString(boost::asio::streambuf& buffer);
};

