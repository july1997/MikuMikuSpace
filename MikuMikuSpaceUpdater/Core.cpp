#include "Core.h"
#include "DxLib.h"


Core::Core()
{
}


Core::~Core()
{
}


size_t Core::setup()
{
	//画面モード
	SetGraphMode(1280, 720, 32);
	ChangeWindowMode(1);
	SetMainWindowText("MikuMikuOpenWorld Updater");//ウィンドウモード
	//　ウインドウモードの時にウインドウのサイズを自由に変更出来るように設定する
	SetWindowSizeChangeEnableFlag(TRUE);
	SetMultiThreadFlag(TRUE);// マルチスレッドに適したモードで起動する
	//ログを出力しない
	SetOutApplicationLogValidFlag(FALSE);
	// DirectX11を使用する。
	SetUseDirect3DVersion(DX_DIRECT3D_11);
	//Zバッファの深度
	SetZBufferBitDepth(24);
	//TCPで送信するデータの先頭に４バイト付けない
	SetUseDXNetWorkProtocol(FALSE);
	SetAlwaysRunFlag(TRUE);//ウインドウがアクティブではない状態でも処理を続行する
	//SetWaitVSyncFlag(FALSE);//垂直同期を待たない
	//多重起動
	SetDoubleStartValidFlag(TRUE);
	//UTF8
	SetUseCharCodeFormat(DX_CHARCODEFORMAT_UTF8);

	if (DxLib_Init() == -1) { return -1; }

	//SetWindowUserCloseEnableFlag(FALSE);//閉じるボタンを押しても閉じない
	SetMouseDispFlag(TRUE);//マウスカーソルを表示
	SetDrawScreen(DX_SCREEN_BACK);//裏画面に描画設定
	// フルスクリーンウインドウの切り替えでリソースが消えるのを防ぐ。
	SetChangeScreenModeGraphicsSystemResetFlag(FALSE);
	// Zバッファを有効にする。
	SetUseZBuffer3D(TRUE);
	// Zバッファへの書き込みを有効にする。
	SetWriteZBuffer3D(TRUE);
	return 0;
}

void Core::load()
{
	log.Initialize("MikuMikuSpaceUpdater.txt");

	//画像の読み込み
	ui.loadBackgroundImage("System/Picture/background.jpg");
	font3 = ui.loadDxFont("System/Font/Koruri Light4.dft");
	font2 = ui.loadDxFont("System/Font/Koruri Light3.dft");
	font = ui.loadDxFont("System/Font/Koruri Light2.dft");

	box = ui.makeBoxRoundedBar(400, 40, 440, 290, GetColor(145, 145, 145), GetColor(145, 145, 145), 0);
	inbox = ui.makeBoxRoundedBar(390, 30, 445, 295, GetColor(195, 216, 37), GetColor(195, 216, 37), 0);
}

bool Core::donwloadUpdater()
{
	//ファイルの読み込み
	std::ifstream fin("System/Update/releases.txt");
	if (!fin)
	{
		return 0;
	}

	std::stringstream strstream;
	strstream << fin.rdbuf();
	fin.close();
	//ファイルの内容をstringに入れる
	std::string data(strstream.str());
	std::string err;
	auto json = json11::Json::parse(data, err);
	if (err != "")return 0;

	ver = json["releases"]["version"].string_value();
	majorupdate = json["releases"]["majorupdate"].bool_value();

	//ダウンロード----------------------------------------------------------------------------------
	std::string url = "/releases";
	std::string relesenote = json["releases"]["note"].string_value();
	WebDownloader* dl = new WebDownloader();
	dl->httpsOpen(MMS_Server);
	int r = dl->httpGET(
		(char *)url.c_str(),
		(char *)MMS_Server,
		(char *)"./", //.exeと同じディレクトリなら「"./"」
		(char *)"updater.zip",                     //拡張子も必要なので注意
		"hash=" + json["releases"]["hash"].string_value()
	);

	if (r == 0) { dl->StartDownload(); }

	//ダウンロード
	while (ProcessMessage() == 0)
	{
		if (CheckHitKey(KEY_INPUT_ESCAPE) != 0) { return 0; }

		ui.drawBackgroundImage();
		ui.drawString(font3, u8" Update to " + ver, 0, 0, 0, GetColor(238, 238, 238));

		if (r == 0)
		{
			ui.drawString(font2, u8"ダウンロードしています", 640, 250, 1);
			ui.drawString(font, u8"更新履歴", 640, 400, 1);
			ui.drawString(font, relesenote, 640, 420, 1);
			ui.drawBox(box, 128);
			ui.changeBoxRoundedBar(inbox, e, 30, GetColor(195, 216, 37), GetColor(195, 216, 37), 0);
			ui.drawBox(inbox, 128);
			e = (double)dl->getReadSize() / (double)dl->getDownloadSize() * (double)390;
			ui.drawString(font, std::to_string(dl->getReadSize()) + u8" / " + std::to_string(dl->getDownloadSize()) + u8" Byte", 640, 335, 1);

			if (dl->getReadSize() >= dl->getDownloadSize())
			{
				WaitTimer(3000);
				return 1;
			}
		}
		else
		{
			ui.drawString(font2, u8"ダウンロードに失敗　エラーコード" + std::to_string(r), 640, 270, GetColor(237, 28, 36));
			return 0;
		}

		//fps.displayFps(1280 - 20, 0);
		fps.measureFps();
	}

	return 0;
}

bool Core::update()
{
	//アップデート----------------------------------------------------------------------------------
	e = 0;
	bool remove = 0;

	if (!majorupdate) { remove = 1; }

	string strZipFilename = "updater.zip", strTargetPath = "";
	unzipper zip;
	if (!zip.openZip(strZipFilename, strTargetPath))
	{
		return 0;
	}
	// 解凍開始
	zip.startunzip();

	while (ProcessMessage() == 0)
	{
		ui.drawBackgroundImage();
		ui.drawString(font3, u8" Update to " + ver, 0, 0, 0, GetColor(238, 238, 238));
		ui.drawString(font2, u8"インストール中です", 640, 250, 1);
		ui.drawBox(box, 128);
		ui.changeBoxRoundedBar(inbox, e, 30, GetColor(195, 216, 37), GetColor(195, 216, 37), 0);
		ui.drawBox(inbox, 128);

		if (!remove)
		{
			zip.removeDirectory("System");
			remove = 1;
		}
		else
		{
			if (zip.isComplete()) {
				return 1;
			}

			ui.drawString(font, std::to_string(zip.getNoFilewSize()) + u8" / " + std::to_string(zip.getZipFileSize()) + u8" Byte", 640, 335, 1);
			e = (double)zip.getNoFilewSize() / (double)zip.getZipFileSize() * (double)390;
		}

		//fps.displayFps(1280 - 20, 0);
		fps.measureFps();
	}

	return 0;
}

bool Core::lunchExe()
{
	//MikuMikuOpenWorldの起動-------------------------------------------------------------------------------------

	struct stat buf;

	while (ProcessMessage() == 0)
	{
		ui.drawBackgroundImage();
		ui.drawString(font3, u8" Update to " + ver, 0, 0, 0, GetColor(238, 238, 238));
		ui.drawString(font2, u8"終了しています", 640, 250, 1);

		//消えるまで待つ
		if (stat("Update.zip", &buf) != 0) { break; }

		//fps.displayFps(1280 - 20, 0);
		fps.measureFps();
	}

	//MikuMikuOpenWorldの起動
	SHELLEXECUTEINFO sei;
	memset(&sei, 0, sizeof(sei));
	sei.cbSize = sizeof(sei);
	sei.hwnd = NULL;
	sei.lpVerb = _T("open");
	sei.lpFile = "MikuMikuSpace.exe";
	sei.nShow = SW_SHOWNORMAL;

	//標準出力を獲得するならCreateProcessを使う
	if (ShellExecuteEx(&sei) == FALSE)
	{
		return 0;
	}

	return 1;
}