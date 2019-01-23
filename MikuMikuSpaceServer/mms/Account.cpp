#include "Account.hpp"


Account::Account() : io_service(new asio::io_service()), sock(*io_service), udpsock(*io_service)
{
}

Account::~Account()
{
}

int Account::startReceive()
{
	try
	{
		//IPv4のソケットアドレス情報を設定する
		u_short port = 13939;
		ip::tcp::acceptor acceptor(*io_service, ip::tcp::endpoint(ip::tcp::v4(), port));

		//クライアントからの接続を受け入れる
		acceptor.accept(sock);

		IP = sock.local_endpoint().address();

		t = std::thread(&Account::receiveLoop, this);
		t.detach();
	}
	catch (...)
	{
		return -1;
	}

	return 0;
}

int Account::receiveLoop()
{
	//別スレッドで立ち上げ
	try
	{
		//メッセージを送受信
		while (true)
		{
			asio::streambuf receive_buffer;
			boost::system::error_code error;

			asio::read(sock, receive_buffer, asio::transfer_at_least(1), error);
			if (error && error != asio::error::eof)
			{
				break;
			}
			else if (asio::buffer_cast<const char*>(receive_buffer.data()) == string("end"))
			{
				break;
			}

			//文字列へ変換
			std::string restr = convertBufferToString(receive_buffer);
			if (restr == "")break;

			//std::cout << restr << std::endl;

			//dxライブラリは512byte送ると勝手に分割するので注意
			if (restr.size() >= 512) {
				restr.erase(restr.begin() + 512, restr.end());
				bigmessage.push_back(restr);
				//std::cout << "recived big : " << restr.size() << " " << restr << std::endl;
			}
			else {

				if (bigmessage.size() > 0) {
					restr = bigmessage[0] + restr;
					bigmessage.erase(bigmessage.begin());

					//std::cout << "attached : " << restr << std::endl;
				}
				string decoded;
				CryptoPP::StringSource ssk(restr, true /*pump all*/,
					new CryptoPP::Base64Decoder(
						new CryptoPP::StringSink(decoded)
					) // HexDecoder
				); // StringSource

				message.push_back(decoded);
			}
		}
	}
	catch (...)
	{
		return -1;
	}

	if (!end) {
		message.push_back("end");
		cout << "Logout : id " << myID << "  name " << name << endl;
	}

	return 0;
}


std::string Account::convertBufferToString(boost::asio::streambuf& buffer)
{
	std::string result = boost::asio::buffer_cast<const char*>(buffer.data());
	return result;
}

int Account::send(unsigned char Command, string method, string &Buffer)
{
	try
	{
		string Buf = method + " " + Buffer;

		Buf.insert(0, "1");
		Buf[0] = Command;

		string encoded;
		CryptoPP::StringSource ss((const byte*)Buf.data(), Buf.size(), true,
			new CryptoPP::Base64Encoder(
				new CryptoPP::StringSink(encoded)
			) // HexEncoder
		); // StringSource
		encoded.push_back(0);

		//std::cout << encoded << std::endl;

		asio::write(sock, asio::buffer(encoded));
	}
	catch (...)
	{
		return -1;
	}

	return 0;
}



int Account::startUDPReceive()
{
	try
	{
		udpsock.open(boost::asio::ip::udp::v4());
		udpsock.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 23939));

		u = std::thread(&Account::receiveUDPLoop, this);
		u.detach();
	}
	catch (...)
	{
		return -1;
	}

	return 0;
}

int Account::receiveUDPLoop()
{
	//別スレッドで立ち上げ
	try
	{
		boost::array<char, 256> recv_buf;
		udp::endpoint remote_endpoint;
		boost::system::error_code error;

		while (true)
		{
			//お掃除
			for (int i = 0; i < 256; i++)
			{
				recv_buf[i] = 0;
			}

			size_t len = udpsock.receive_from(boost::asio::buffer(recv_buf), remote_endpoint, 0, error);

			if (len != 0)
			{

				string decoded;
				CryptoPP::StringSource ssk(recv_buf.data(), true /*pump all*/,
					new CryptoPP::Base64Decoder(
						new CryptoPP::StringSink(decoded)
					) // HexDecoder
				); // StringSource
				command.push_back(decoded);
			}
			else
			{
				break;
			}
		}
	}
	catch (...)
	{
		return -1;
	}

	return 0;
}

int Account::sendUDP(unsigned char PlayerID, string &Buffer)
{
	try
	{
		udp::resolver resolver(*io_service);
		udp::resolver::query query(udp::v4(), IP.to_string(), "23940");

		udp::socket socket(*io_service);
		socket.open(udp::v4());

		string Buf = Buffer;
		Buf.insert(0, "1");
		Buf[0] = PlayerID;

		string encoded;
		CryptoPP::StringSource ss((const byte*)Buf.data(), Buf.size(), true,
			new CryptoPP::Base64Encoder(
				new CryptoPP::StringSink(encoded)
			) // HexEncoder
		); // StringSource
		encoded.push_back(0);

		socket.send_to(boost::asio::buffer(encoded), udp::endpoint(*resolver.resolve(query)));
	}
	catch (...)
	{
		return -1;
	}

	return 0;
}

int Account::sendRsaPublicKey()
{
	try
	{
		CryptoPP::AutoSeededRandomPool rng;
		CryptoPP::InvertibleRSAFunction params;

		//RSAキーの生成
		params.GenerateRandomWithKeySize(rng, 3072);

		privateKey = CryptoPP::RSA::PrivateKey(params);
		publicKey = CryptoPP::RSA::PublicKey(params);

		// Temporaries
		string spki;
		CryptoPP::StringSink ss(spki);

		// Use Save to DER encode the Subject Public Key Info (SPKI)
		publicKey.Save(ss);

		//cout << spki.size() <<endl;
		//cout << spki << " : " << dmess<<endl;

		//rsa publicキーを送る
		string s("KEY");
		send(1, s, spki);

	}
	catch (...)
	{
		return -1;
	}
	return 0;
}

void Account::signature()
{
	try
	{
		CryptoPP::AutoSeededRandomPool rng;

		string signature;
		CryptoPP::RSASS<CryptoPP::PSS, CryptoPP::SHA1>::Signer signer(privateKey);

		CryptoPP::StringSource ss1(MMOW_version, true,
			new CryptoPP::SignerFilter(rng, signer,
				new CryptoPP::StringSink(signature)
			) // SignerFilter
		); // StringSource

		string s("SIGN");
		send(2, s, signature);
	}
	catch (...)
	{

	}
}

std::string Account::encryptByRsa(std::string &str)
{
	try
	{
		CryptoPP::AutoSeededRandomPool rnd;

		std::string cipher;
		// Encryption
		CryptoPP::RSAES_OAEP_SHA_Encryptor e(publicKey);

		CryptoPP::StringSource ss1(str, true,
			new CryptoPP::PK_EncryptorFilter(rnd, e,
				new CryptoPP::StringSink(cipher)
			) // PK_EncryptorFilter
		); // StringSource

		return cipher;
	}
	catch (...)
	{
		return "";
	}
}

std::string Account::decryptionByRsa(std::string &str)
{
	try
	{
		std::string recovered;
		// Decryption
		CryptoPP::RSAES_OAEP_SHA_Decryptor d(privateKey);
		CryptoPP::AutoSeededRandomPool rndg;

		CryptoPP::StringSource ss2(str, true,
			new CryptoPP::PK_DecryptorFilter(rndg, d,
				new CryptoPP::StringSink(recovered)
			) // PK_DecryptorFilter
		); // StringSource

		return recovered;
	}
	catch (...)
	{
		return "";
	}
}

int Account::makeAESKey(std::string &Key_s, std::string &iv_s)
{
	try
	{
		if (Key_s.size() == 16 && iv_s.size() == 16) {

			byte key[CryptoPP::AES::DEFAULT_KEYLENGTH];
			byte iv[CryptoPP::AES::BLOCKSIZE];

			// 共通鍵とIVを適当な値で初期化
			//CryptoPP::AutoSeededRandomPool prng;
			//prng.GenerateBlock(key, sizeof(key));
			//prng.GenerateBlock(iv, CryptoPP::AES::BLOCKSIZE);

			for(int i = 0; i < CryptoPP::AES::DEFAULT_KEYLENGTH;i++){
				key[i] = Key_s[i];
			}
			for (int i = 0; i < CryptoPP::AES::BLOCKSIZE; i++) {
				iv[i] = iv_s[i];
			}
			
			enc.SetKeyWithIV(key, sizeof(key), iv);
			dec.SetKeyWithIV(key, sizeof(key), iv);


		}
		else {
			return -1;
		}
	}
	catch (...)
	{
		return -1;
	}

	return 0;
}

void Account::encryptByAes(std::string &str)
{
	try
	{
		//in string	と out string　に指定するのは同じstirngでないとダメ
		enc.ProcessData((byte*)str.data(), (byte*)str.data(), str.size());
	}
	catch (...)
	{

	}
}

void Account::decryptionByAes(std::string &str)
{
	try
	{
		//in string	と out string　に指定するのは同じstirngでないとダメ
		dec.ProcessData((byte*)str.data(), (byte*)str.data(), str.size());
	}
	catch (...)
	{

	}
}

size_t Account::getMessageSize()
{
	return message.size();
}

std::string Account::getMessage()
{
	try
	{
		if (!end) {
			string m = message[0];
			message.erase(message.begin());

			return m;
		}
		else {
			return "";
		}
	}
	catch (...)
	{
		return "";
	}
}

size_t Account::getCommandSize()
{
	return command.size();
}

std::string Account::getCommand()
{
	try
	{
		string m = command[0];
		command.erase(command.begin());

		return m;
	}
	catch (...)
	{
		return "";
	}
}