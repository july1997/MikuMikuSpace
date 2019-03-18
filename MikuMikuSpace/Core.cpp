#include "Core.h"


Core::Core()
{
}


Core::~Core()
{
}


BOOLEAN BlockAPI(HANDLE hProcess, CHAR *libName, CHAR *apiName)
{
    CHAR pRet[] = { 0x31, 0xC0, // XOR eax, eax
                    0xC3
                  };    // RET
    HINSTANCE hLib = NULL;
    VOID *pAddr = NULL;
    BOOL bRet = FALSE;
    SIZE_T dwRet = 0;
    hLib = LoadLibrary(libName);

    if (hLib)
    {
        pAddr = (VOID*)GetProcAddress(hLib, apiName);

        if (pAddr)
        {
            if (WriteProcessMemory(hProcess,
                                   (LPVOID)pAddr,
                                   (LPCVOID)pRet,
                                   sizeof(pRet),
                                   &dwRet))
            {
                if (dwRet)
                {
                    bRet = TRUE;
                }
            }
        }

        FreeLibrary(hLib);
    }

    return bRet;
}


void AntiInject()
{
    HANDLE hProc = GetCurrentProcess();

    while (TRUE)
    {
        BlockAPI(hProc, (CHAR*)"NTDLL.DLL", (CHAR*)"LdrLoadDll");
        BlockAPI(hProc, (CHAR*)"kERNEL32.DLL", (CHAR*)"BaseThreadInitThunk");
        Sleep(100);
    }
}

size_t Core::setup()
{
    //外部デバッカ対策　公開するときにコメントを外す
    //if(IsDebuggerPresent()!=0)return 0;
    //画面モード
    SetGraphMode(1280, 720, 32);
    ChangeWindowMode(1);
    SetMainWindowText(MMS_version);//ウィンドウモード
    //　ウインドウモードの時にウインドウのサイズを自由に変更出来るように設定する
    SetWindowSizeChangeEnableFlag(TRUE);
    SetMultiThreadFlag(TRUE);// マルチスレッドに適したモードで起動する
    //ログを出力しない
    //SetOutApplicationLogValidFlag(FALSE);
    //ログファイル
    Logger::Initialize("MikuMikuOpenWorldLog.txt");
    // DirectX11を使用する。
    SetUseDirect3DVersion(DX_DIRECT3D_11);
    //Zバッファの深度
    SetZBufferBitDepth(24);
    //TCPで送信するデータの先頭に４バイト付けない
    SetUseDXNetWorkProtocol(FALSE);
    SetAlwaysRunFlag(TRUE);//ウインドウがアクティブではない状態でも処理を続行する
    SetWaitVSyncFlag(FALSE);//垂直同期を待たない
    //多重起動
    SetDoubleStartValidFlag(TRUE);
    //UTF8
    SetUseCharCodeFormat(DX_CHARCODEFORMAT_UTF8);

    if (DxLib_Init() == -1) { return -1; }

    //anti DLL Injection
    //CreateThread(0, 0, (LPTHREAD_START_ROUTINE)AntiInject, 0, 0, 0);
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