﻿#include "Scene.h"



Scene::Scene(std::shared_ptr<NetworkManager> network_)
{
	network = network_;
	//画像の読み込み
	ui.loadBackgroundImage("System/Picture/background.jpg");
	font3 = ui.loadDxFont("System/Font/Koruri Light4.dft");
	font2 = ui.loadDxFont("System/Font/Koruri Light3.dft");
	font = ui.loadDxFont("System/Font/Koruri Light2.dft");
}

Scene::~Scene()
{
}

bool Scene::update()
{
	auto r = updater.downloadRelese();

	if (updater.checkUpdate(r) == 1)
	{
		ui.drawBackgroundImage();
		ui.drawString(font2, u8"アップデートの確認中", 640, 280, 1);
		ui.drawString(font, u8"新しいバージョンが見つかりました。アップデートを行います", 640, 320, 1);
		fps.displayFps(1280 - 20, 0);
		fps.controlClearDrawScreenFps();
		WaitKey();
		//アップデーターを起動
		updater.lunchUpdater();
		return 1;
	}

	return 0;
}


bool Scene::connect()
{
	bool error = 0;

	if (!network->connect()) { error = 1; }

	while (ProcessMessage() == 0 && error == 1)
	{
		if (CheckHitKey(KEY_INPUT_ESCAPE) != 0) { return 0; }

		ui.drawBackgroundImage();
		ui.drawString(font2, u8"サーバメンテナンス中です", 640, 280, 1, GetColor(237, 28, 36));
		fps.displayFps(1280 - 20, 0);
		fps.controlClearDrawScreenFps();
	}

	return 1;
}

bool Scene::login()
{
	if (!connect()) { return 0; }

	int box = ui.makeBoxRoundedBar(400, 170, 440, 295, GetColor(145, 145, 145), 13882323, 36);
	int input = ui.makeInputBox(260, 30, GetColor(150, 150, 150), 36, "", 0, 1);
	ui.activateInputBox(input);
	int input2 = ui.makeInputBox(260, 30, GetColor(150, 150, 150), 36, "", 0, 1);
	int ok = ui.makeButton(u8"ログイン", font, 13882323);
	int signup = ui.makeButton(u8"新規登録", font, 13882323);
	string serverDetail = network->getServerDetail();
	bool warning = 0, toomis = 0, error = 0, connectionerrror = 0;

	while (ProcessMessage() == 0)
	{
		if (CheckHitKey(KEY_INPUT_ESCAPE) != 0) { return 0; }

		if (!network->CorrectVersion)
		{
			MessageBox(GetMainWindowHandle(), "MikuMikuOpenWorld のバージョンが違います", "error", MB_OK);
			return 0;
		}

		ui.drawBackgroundImage();
		ui.drawString(font3, MMS_version, 0, 0, 0, GetColor(238, 238, 238));
		ui.drawString(font2, u8"ログイン", 650, 200, 1);
		ui.updateBox(box);
		ui.updateSize(box, 400, 150, GetColor(145, 145, 145), 13882323, 36);
		ui.drawBox(box, 128);
		ui.drawStringToBox(box, u8"E-mail", ui.getBoxWidth(box) / 2 - 120, ui.getBoxHeight(box) / 2 - 40, font, 1);
		ui.drawStringToBox(box, u8"パスワード", ui.getBoxWidth(box) / 2 - 140, ui.getBoxHeight(box) / 2, font, 1);
		ui.drawInputBoxToBox(box, input, ui.getBoxWidth(box) / 2 - 90, ui.getBoxHeight(box) / 2 - 40, font, 128);
		ui.drawInputBoxToBox(box, input2, ui.getBoxWidth(box) / 2 - 90, ui.getBoxHeight(box) / 2, font, 128, 1, 0, 1);
		ui.updateInputBox(input);
		ui.updateInputBox(input2);
		ui.drawButtonToBox(box, ok, ui.getBoxWidth(box) - 110, ui.getBoxHeight(box) - 40, 72);
		ui.drawButtonToBox(box, signup, ui.getBoxWidth(box) - 220, ui.getBoxHeight(box) - 40, 72);
		ui.drawString(font, "Server : " + serverDetail, 1040, 690);

		//ボタンを押した
		if (ui.updateButton(ok) == 1 || ui.updateInputBox(ui.getNowActiveKey()) == 1)
		{
			std::string mail = ui.getInputString(input);
			std::string pass = ui.getInputString(input2);

			if (illegalChara(mail) && illegalChara(pass) && mail.find("@") != string::npos && !toomis)
			{
				int j = 0, time = 0;

				if (!step)
				{
					network->startCommunication();
					j = network->getid();

					while (j == 0 && ProcessMessage() == 0)
					{
						j = network->getid();
						time++;

						if (time > 1200) { j = -3; }

						fps.controlWaitFps();
					}
				}

				if (j == 1) { step = 1; }
				else { connectionerrror = 1; }

				if (step)
				{
					WaitTimer(500);
					network->login(mail, pass);
					j = network->getid();

					while (j == 0 && ProcessMessage() == 0)
					{
						j = network->getid();
						time++;

						if (time > 1200) { j = -3; }

						fps.controlWaitFps();
					}

					if (j > 0)
					{
						break;
					}
					else
					{
						if (j == -1) { warning = 1; }

						if (j == -2) { toomis = 1; }

						if (j == -3) { error = 1; }

						ui.activateInputBox(input);
						ui.setInputString(input, ""), ui.setInputString(input2, "");
					}
				}
			}
			else { ui.activateInputBox(input), ui.setInputString(input, ""), ui.setInputString(input2, ""), warning = 1; }
		}
		else if (ui.updateButton(signup))//登録ボタンを押した
		{
			std::string mail = ui.getInputString(input);
			std::string pass = ui.getInputString(input2);

			if (illegalChara(mail) && illegalChara(pass) && mail.find("@") != string::npos && !toomis)
			{
				int r = regist(mail, pass);

				if (r == 1)
				{
					tutorialf = 1;
					break;
				}
				else
				{
					if (r == 0) { warning = 1; }

					if (r == -1) { toomis = 1; }

					if (r == -2) { error = 1; }

					ui.activateInputBox(input);
					ui.setInputString(input, ""), ui.setInputString(input2, "");
				}
			}
			else { ui.activateInputBox(input), ui.setInputString(input, ""), ui.setInputString(input2, ""), warning = 1; }
		}

		if (error)
		{
			ui.drawString(font, u8"通信エラーが発生しました", 640, 250, 1, GetColor(237, 28, 36));
		}
		else if (toomis)
		{
			ui.drawString(font, u8"一定以上間違えました。最初からやり直してください", 640, 250, 1, GetColor(237, 28, 36));
		}
		else if (warning)
		{
			ui.drawString(font, u8"入力されたメールアドレスやパスワードが正しくありません", 640, 250, 1, GetColor(237, 28, 36));
		}
		else if (connectionerrror)
		{
			ui.drawString(font, u8"接続できません", 640, 250, 1, GetColor(237, 28, 36));
		}

		fps.displayFps(1280 - 20, 0);
		fps.controlClearDrawScreenFps();
	}

	ui.deleteBox(box);
	ui.deleteBox(input);
	ui.deleteBox(input2);
	ui.deleteBox(ok);
	ui.deleteBox(signup);
	network->startSend();
	return 1;
}

int Scene::regist(std::string mail, std::string pass)
{
	int box = ui.makeBoxRoundedBar(400, 170, 440, 295, GetColor(145, 145, 145), 13882323, 36);
	int input = ui.makeInputBox(360, 30, GetColor(150, 150, 150), 36, "");
	ui.activateInputBox(input);
	int ok = ui.makeButton(u8"OK", font, 13882323);
	bool warning = 0, connectionerrror = 0;

	while (ProcessMessage() == 0)
	{
		if (CheckHitKey(KEY_INPUT_ESCAPE) != 0) { return 0; }

		ui.drawBackgroundImage();
		ui.drawString(font2, u8"新規登録画面", 640, 200, 1);
		ui.updateBox(box);
		ui.updateSize(box, 400, 150, GetColor(145, 145, 145), 13882323, 36);
		ui.drawBox(box, 128);
		ui.drawStringToBox(box, u8"名前を入力", ui.getBoxWidth(box) / 2, 30, font, 1);
		ui.drawInputBoxToBox(box, input, ui.getBoxWidth(box) / 2 - 180, ui.getBoxHeight(box) / 2, font, 128);
		ui.drawButtonToBox(box, ok, ui.getBoxWidth(box) - 60, ui.getBoxHeight(box) - 48, 72);

		if (ui.updateButton(ok) == 1 || ui.updateInputBox(input) == 1)
		{
			warning = 0;
			std::string name = ui.getInputString(input);

			if (Specialsymbol(name) && Invalidname(name))
			{
				int j = 0, time = 0;

				if (!step)
				{
					network->startCommunication();
					j = network->getid();

					while (j == 0 && ProcessMessage() == 0)
					{
						j = network->getid();
						time++;

						if (time > 1200) { j = -3; }

						fps.controlWaitFps();
					}
				}

				if (j == 1) { step = 1; }
				else { connectionerrror = 1; }

				if (step)
				{
					WaitTimer(500);

					if (network->signUp(mail, pass, name))
					{
						j = network->getid();

						while (j == 0 && ProcessMessage() == 0)
						{
							j = network->getid();
							time++;

							if (time > 1200) { j = -3; }

							if (j != 0)
							{
								if (j > 0) { return 1; }

								if (j == -1) { return 0; }

								if (j == -2) { return -1; }

								if (j == -3) { return -2; }
							}

							fps.controlWaitFps();
						}
					}
					else
					{
						ui.activateInputBox(input);
						ui.setInputString(input, ""), warning = 1;
					}
				}
			}
			else { ui.activateInputBox(input), ui.setInputString(input, ""), warning = 1; }
		}

		if (warning)
		{
			ui.drawString(font, u8"名前を正しく入力してください", 640, 250, 1, GetColor(237, 28, 36));
		}
		else if (connectionerrror)
		{
			ui.drawString(font, u8"接続できません", 640, 250, 1, GetColor(237, 28, 36));
		}

		fps.displayFps(1280 - 20, 0);
		fps.measureFps();
	}

	ui.deleteBox(box);
	ui.deleteBox(input);
	ui.deleteBox(ok);
	return 1;
}

bool Scene::illegalChara(std::string str)
{
	if (str == "") { return 0; }

	int c = 0;

	for (int i = 0; i < str.size(); i++)
	{
		if (str[i] == '@') { c++; }
	}

	if (c > 1) { return 0; }

	//正規表現
	std::regex  re(u8"^[0-9A-Za-z]+@*.*_*-*$");

	if (std::regex_match(str, re) == 0)
	{
		return 0;
	}

	return 1;
}

bool Scene::Specialsymbol(std::string str)
{
	if (str == "") { return 0; }

	//正規表現
	std::regex  re(u8"^'*$");

	if (std::regex_match(str, re) != 0)
	{
		return 0;
	}

	return 1;
}
bool Scene::Invalidname(std::string str)
{
	//使用禁止の名前
	if (str.find(u8"まんこ") != string::npos) { return 0; }

	return 1;
}

void Scene::loading()
{

	if (network->isCharaDownload) {
		while (network->model_manager->countTask() == 0)WaitTimer(1);
		do
		{
			ui.drawBackgroundImage();
			ui.drawString(font2, u8"モデルのダウンロード中です…", 640, 200, 1);

			network->model_manager->update();

			fps.displayFps(1280 - 20, 0);
			fps.controlClearDrawScreenFps();
		} while (ProcessMessage() == 0 && network->model_manager->countTask() != 0);
	}

	do
	{
		ui.drawBackgroundImage();
		ui.drawString(font2, u8"ロード中です…", 640, 200, 1);

		fps.displayFps(1280 - 20, 0);
		fps.controlClearDrawScreenFps();
	} while (ProcessMessage() == 0 && GetASyncLoadNum() != 0);
}

bool Scene::chat()
{
	if (network->getChatMassegeSize() > 0)
	{
		chatmessege.push_back(network->getChatMassege());
	}

	if (!load)
	{
		chatbox = ui.makeBoxRoundedBar(600, 200, 340, 510, GetColor(145, 145, 145), 13882323, 36);
		chatinputbox = ui.makeInputBox(300, 30, GetColor(120, 120, 120), 60, "", 1);
		sousin = ui.makeButton(u8"送信", font, GetColor(120, 120, 120));
		ui.setScroll(chatbox, 45);
		load = 1;
	}

	ui.updateBox(chatbox);
	ui.updateSize(chatbox, 100, 50, GetColor(145, 145, 145), 13882323, 36);
	ui.drawBox(chatbox, 206);
	ui.drawStringToBox(chatbox, u8"コメント", ui.getBoxWidth(chatbox) / 2, 2, font, 1);

	if (ui.getBoxWidth(chatbox) - 100 > 100) { ui.changeBoxRounded(chatinputbox, ui.getBoxWidth(chatbox) - 100, 30, GetColor(120, 120, 120)); }

	ui.drawInputBoxToBox(chatbox, chatinputbox, 20, ui.getBoxHeight(chatbox) - 40, font, 168);
	ui.drawButtonToBox(chatbox, sousin, 70, 45, 72, 1);

	if (ui.updateInputBox(chatinputbox, messe) == 1 || ui.updateButton(sousin) == 1)
	{
		messe = ui.getInputString(chatinputbox);

		if (messe != "")
		{
			if (messe.find("/yotube ") != std::string::npos) 
			{
				messe.erase(0, 8);
				if(messe.find("https://www.youtube.com/watch?") != std::string::npos)network->sendMovieRequest(messe);
			}
			else
			{
				if (network->sendChat(messe) != -1) { chatmessege.push_back(network->getname() + " : " + messe); }
			}
		}

		ui.setInputString(chatinputbox, "");
		ui.activateInputBox(-1);
	}

	int scroll = ui.updateScroll(chatbox, 20);
	int max = ui.getBoxHeight(chatbox) / 20 - 4;

	if (max < 0)
	{
		max = chatmessege.size();
		scroll = 0;
	}
	else
	{
		if (max + scroll >= chatmessege.size()) { scroll = chatmessege.size() - max; }

		if (scroll < 0) { max = chatmessege.size(), scroll = 0; }

		if (scroll == 0 && follow == 0)if (max <= chatmessege.size()) { scroll = chatmessege.size() - max, follow = 1; }
	}

	for (int i = 0; i < max; i++)
	{
		ui.drawStringToBox(chatbox, chatmessege[i + scroll], 20, 24 + 18 * (i), font, 0, -1, 75);
	}

	if (ui.getNowActiveKeyI() == chatinputbox) { inp = 1; }
	else { inp = 0; }

	return inp;
}

void Scene::drawName()
{
	//自分の名前
	//スクリーン座標に変換
	VECTOR ahead = ConvWorldPosToScreenPos(VGet(network->cahara->getPos().x, network->cahara->getPos().y + 15.f, network->cahara->getPos().z));
	ui.drawString(font, network->getname(), (int)ahead.x, (int)ahead.y, 1);

	if (network->getid() == 1) { ui.drawString(font, u8"管理人", (int)ahead.x, (int)ahead.y - 25, 1, GetColor(0, 255, 64)); }

	//他社の名前
	for (size_t i = 0; i < network->getMultiplayerSize(); i++)
	{
		unsigned char ID = network->players->getPlayerID(i);
		VECTOR p = network->players->getPos(ID);

		if (CheckCameraViewClip(p) == FALSE)
		{
			VECTOR pp = ConvWorldPosToScreenPos(VGet(p.x, p.y + 15.f, p.z));
			ui.drawString(font, network->players->getName(ID), (int)pp.x, (int)pp.y, 1);

			if (ID == 1) { ui.drawString(font, u8"管理人", (int)pp.x, (int)pp.y - 25, 1, GetColor(0, 255, 64)); }
		}
	}
}

void Scene::tutorial()
{
	if (tutorialf)
	{
		if (!tutoriall)
		{
			tutorialbox = ui.makeBoxRoundedBar(800, 450, 240, 35, GetColor(175, 175, 175), 13882323, 36);
			tutoriall = 1;
		}

		ui.updateBox(tutorialbox);
		ui.updateSize(tutorialbox, 800, 450, GetColor(175, 175, 175), 13882323, 36);
		ui.drawBox(tutorialbox, 186);
		ui.drawStringToBox(tutorialbox, u8"MikuMikuOpenWorld へようこそ！\n　  操作方法をご説明いたします", ui.getBoxWidth(tutorialbox) / 2, 50, font2, 1);
		ui.drawStringToBox(tutorialbox, u8"視点操作 : 右クリック+ドラック　ホイール \nキャラ操作 : Wを押し続けて前進 S後進 A左 D右\n左Shift + W 走る　左Shift長押し + W ダッシュ\n", ui.getBoxWidth(tutorialbox) / 2, 150, font, 1);
		ui.drawStringToBox(tutorialbox, u8"コメント送信 : 下のコメント欄の入力欄をクリック後\n入力し送信ボタンかエンターキーを押す", ui.getBoxWidth(tutorialbox) / 2, 250, font, 1);
		ui.drawStringToBox(tutorialbox, u8"ウィンドウ操作 : タブをクリックでウィンドウを移動\n各ウィンドウの下、右下、右をクリックでウィンドウサイズが変更可", ui.getBoxWidth(tutorialbox) / 2, 320, font, 1);
		ui.drawStringToBox(tutorialbox, u8"何かのキーを押すとこのウィンドウは閉じます", ui.getBoxWidth(tutorialbox) / 2, 400, font, 1);

		if (CheckHitKeyAll() != 0) { tutorialf = 0; }
	}
}