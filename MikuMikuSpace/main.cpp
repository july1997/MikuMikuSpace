#include "DxLib.h"

#include "../Common/Fps.h"
#include "../Common/WebDownloader.h"
#include "../Common/Logger.h"

#include "Model.h"
#include "Camera.h"
#include "Bullet_physics.h"
#include "_dxdebugdraw.h"
#include "Effekseer.h"
#include "NetworkManager.h"
#include "Scene.h"
#include "Character.h"
#include "Core.h"
#include "VstHost.h"
#include "ModelManager.h"
#include "ObjectManager.h"

//#define B2D_DEBUG_DRAW
#ifdef  B2D_DEBUG_DRAW  
DxDebugDraw g_debugdraw;
#endif  

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Core core;

	if (core.setup() == -1) { return -1; }

	//VstHost vst;
	//vst.loadLibrary("C:\\Users\\kamim\\Downloads\\Piano1X64\\Piano One.dll");
	//return 0;
	FpsManager fps;
	//fps.setFps(30.0f);
	Camera camera;
	VECTOR vp = { 0, 10, 0 };
	std::shared_ptr<Bullet_physics> bullet(new Bullet_physics);
	int world = bullet->createWorld(VGet(-0, -0, -0), VGet(10000.f, 10000.f, 10000.f));
	std::shared_ptr<ModelManager> model_manager(new ModelManager());

	//非同期読み込み設定に変更
	SetUseASyncLoadFlag(TRUE);

	Object obj(bullet, world);
	obj.loadModel(u8"System/Object/大画面ディスプレイ/大画面ディスプレイ スタンド付き.mqo",VGet(10,13,8));
	Model stage;
	stage.loadModel(u8"System/Stage/唄川町 ver0.10/models/唄川町Light【軽量版】ver0.10.mv1");
	model_manager->setup();
	Model sky;
	sky.loadModel(u8"System/Skybox/skydome/skybox.mqo");

	//設定を戻す
	SetUseASyncLoadFlag(FALSE);
	std::shared_ptr<Character> chara(new Character(bullet, world));
	chara->setModel(model_manager->getModel(0));

	//デバッグドロー用  
#ifdef B2D_DEBUG_DRAW  
	bullet->getWorld(world)->setDebugDrawer(&g_debugdraw);
	bullet->getWorld(world)->getDebugDrawer()->setDebugMode(btIDebugDraw::DBG_DrawWireframe);

	MV1_REF_POLYGONLIST RefMesh;
	// 参照用メッシュの作成
	MV1SetupReferenceMesh(chara->getModelHandle(), -1, TRUE);
	// 参照用メッシュの取得
	RefMesh = MV1GetReferenceMesh(chara->getModelHandle(), -1, TRUE);
#endif

	//int specCubeTex = LoadGraph(u8"Skybox/GrandCanyon/skybox_32.dds");
	//SetUseTextureToShader(2, specCubeTex);

	//Effekseer_DX effect;
	std::shared_ptr<NetworkManager> network(new NetworkManager(bullet, world, chara, model_manager));
	Scene scene(network);

	network->setScreen(obj.getModelHandle(0), 0);

	if (!scene.update())
	{
		if (scene.login())
		{
			scene.loading();
			stage.setScale(12.f);
			bullet->createGroundBodytoMesh(stage.getModelHandle(), world);
			chara->setup();
			sky.setScale(40.f);
			obj.setModelScale(0,0.1);
			obj.setPos(0, VGet(-208,10,-173));
			obj.setRot(0, VGet(0.0f, - 130.0f * DX_PI_F / 180.0f, 0.0f));

			while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
			{
				camera.MouseCamera(stage.getModelHandle(), chara->getPos(), vp);
				//camera.CameraAdditionally( chara.getPos(), chara.getRot(), vp);
				//camera.MouseCamera(stage.getModelHandle(), MV1GetFramePosition(chara->getModelHandle(), MV1SearchFrame(chara->getModelHandle(), u8"頭")));
				//yotube.update();
				chara->playAnime();
				chara->animeControl();
				bullet->stepSimulation(world);
				chara->update();
				obj.update();
				network->multiplayerUpdate();
				sky.setPos(chara->getPos());
				// ライティングの計算をしないように設定を変更
				SetUseLighting(FALSE);
				sky.draw();
				SetUseLighting(TRUE);
				chara->draw();
				network->multiplayerDraw();
				stage.draw();
				obj.draw();
				//chara->debug();
				scene.drawName();
				scene.tutorial();

				//デバッグドロー  
#ifdef B2D_DEBUG_DRAW  
				bullet->getWorld(world)->debugDrawWorld();
#endif  

				if (!scene.chat())
				{
					chara->playerMovementKeyboard();
				}
				else
				{
					chara->noMovement();
				}

				network->updateYotude();

				model_manager->update();
				network->displayPing(1280 - 50, 720 - 15, -1);
				fps.displayFps(1280 - 20, 0);
				fps.controlClearDrawScreenFps();
			}
		}
	}

	DxLib_End();
	return 0;
}