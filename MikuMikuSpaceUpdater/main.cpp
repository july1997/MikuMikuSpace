#include "DxLib.h"

#include "Core.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    Core core;
    if (core.setup() == -1) { return -1; }

	core.load();

	// アップロード開始
	if (core.donwloadUpdater()) 
	{
		if (core.update())
		{
			if(!core.lunchExe())
			{
				//失敗した場合のエラー処理
				MessageBox(GetMainWindowHandle(), "ShellExecuteEx Failed.", "error", MB_OK);//エラー表示
			}
		}
		else 
		{
			MessageBox(GetMainWindowHandle(), "ファイルを開けません", "error", MB_OK);
		}
	}
	else
	{
		MessageBox(GetMainWindowHandle(), "エラーが発生しました", "error", MB_OK);
	}

    DxLib_End();
    return 0;
}