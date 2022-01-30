// MouseMoveHelper.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "framework.h"
#include "MouseMoveHelper.h"

#define MAX_LOADSTRING 100
#define WM_TASKTRAY (WM_APP + 1)

// グローバル変数:
HINSTANCE hInst;                                // 現在のインターフェイス
WCHAR szTitle[MAX_LOADSTRING];                  // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING];            // メイン ウィンドウ クラス名
HICON hIcon;                                    // アイコン
HMENU hTaskTrayMenu;                            // メニュー
HHOOK hHook;                                    // マウスのフック用
HWND  hWnd;                                     // Wndのハンドル

std::vector<EDGEDATA*> RightEdges;
std::vector<EDGEDATA*> LeftEdges;
std::vector<EDGEDATA*> TopEdges;
std::vector<EDGEDATA*> BottomEdges;


#ifdef _DEBUG
HANDLE stdoutHandle;                            // DEBUG用コンソールの標準出力
int dbg_n;                                      // DEBUG用コンソールから出力した文字数
#endif

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


LRESULT CALLBACK LowLevelMouseProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    MSLLHOOKSTRUCT* pMsLLStr = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
    static long x = 0;
    static long y = 0;
    static char direction_old = 0;

    char direction_current = 0;
    BOOL ret = FALSE;

    if (nCode < HC_ACTION)
    {
        return CallNextHookEx(hHook, nCode, wParam, lParam);
    }

    if (x != pMsLLStr->pt.x)
    {
        if (x > pMsLLStr->pt.x) direction_current |= MOVE_LEFT; // 左に移動中
        else direction_current |= MOVE_RIGHT;                    // 右に移動中
    }

    if (y != pMsLLStr->pt.y)
    {
        if (y > pMsLLStr->pt.y) direction_current |= MOVE_UP; // 上に移動中
        else direction_current |= MOVE_DOWN;                    // 下に移動中
    }

    // 左に移動していたのが停止した場合
    if (((direction_old & MOVE_LEFT) != 0) && ((direction_current & MOVE_LEFT) == 0))
    {
        std::vector<EDGEDATA*>::const_iterator result = std::find_if(LeftEdges.begin(), LeftEdges.end(), [pMsLLStr](EDGEDATA* pEdge)
        {
            return ((pMsLLStr->pt.x <= pEdge->edge_near)
                && (pMsLLStr->pt.x > pEdge->edge_far)
                && ((pMsLLStr->pt.y < pEdge->translate_range_high)
                    || (pMsLLStr->pt.y > pEdge->translate_range_low))
                );
        });

        if (result != LeftEdges.end())
        {
            SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);

            if (pMsLLStr->pt.y < ((EDGEDATA*)(*result))->translate_range_high)
                ret = SetCursorPos(pMsLLStr->pt.x, ((EDGEDATA*)(*result))->translate_range_high);
            else
                ret = SetCursorPos(pMsLLStr->pt.x, ((EDGEDATA*)(*result))->translate_range_low - 1);

#ifdef _DEBUG
            std::wstring str_sc(_T("SetCursorPos :"));
            std::wstring str_t(_T("TRUE\n"));
            std::wstring str_f(_T("FALSE\n"));

            WriteConsole(stdoutHandle, str_sc.c_str(), (DWORD)str_sc.size(), (LPDWORD)&dbg_n, NULL);
            if (ret)
                WriteConsole(stdoutHandle, str_t.c_str(), (DWORD)str_t.size(), (LPDWORD)&dbg_n, NULL);
            else
                WriteConsole(stdoutHandle, str_f.c_str(), (DWORD)str_f.size(), (LPDWORD)&dbg_n, NULL);
#endif

        }

        goto finish;
    }

    // 右に移動していたのが停止した場合
    if (((direction_old & MOVE_RIGHT) != 0) && ((direction_current & MOVE_RIGHT) == 0))
    {
        std::vector<EDGEDATA*>::const_iterator result = std::find_if(RightEdges.begin(), RightEdges.end(), [pMsLLStr](EDGEDATA* pEdge)
        {
            return ((pMsLLStr->pt.x >= pEdge->edge_near)
                && (pMsLLStr->pt.x < pEdge->edge_far)
                && ((pMsLLStr->pt.y < pEdge->translate_range_high)
                    || (pMsLLStr->pt.y > pEdge->translate_range_low))
                );
        });

        if (result != RightEdges.end())
        {
            SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);

            if (pMsLLStr->pt.y < ((EDGEDATA*)(*result))->translate_range_high)
                ret = SetCursorPos(pMsLLStr->pt.x, ((EDGEDATA*)(*result))->translate_range_high);
            else
                ret = SetCursorPos(pMsLLStr->pt.x, ((EDGEDATA*)(*result))->translate_range_low - 1);

#ifdef _DEBUG
            std::wstring str_sc(_T("SetCursorPos :"));
            std::wstring str_t(_T("TRUE\n"));
            std::wstring str_f(_T("FALSE\n"));

            WriteConsole(stdoutHandle, str_sc.c_str(), (DWORD)str_sc.size(), (LPDWORD)&dbg_n, NULL);
            if (ret)
                WriteConsole(stdoutHandle, str_t.c_str(), (DWORD)str_t.size(), (LPDWORD)&dbg_n, NULL);
            else
                WriteConsole(stdoutHandle, str_f.c_str(), (DWORD)str_f.size(), (LPDWORD)&dbg_n, NULL);
#endif

        }

        goto finish;
    }

    // 上に移動していたのが停止した場合
    if (((direction_old & MOVE_UP) != 0) && ((direction_current & MOVE_UP) == 0))
    {
        std::vector<EDGEDATA*>::const_iterator result = std::find_if(TopEdges.begin(), TopEdges.end(), [pMsLLStr](EDGEDATA* pEdge)
        {
            return ((pMsLLStr->pt.y <= pEdge->edge_near)
                && (pMsLLStr->pt.y > pEdge->edge_far)
                && ((pMsLLStr->pt.x < pEdge->translate_range_high)
                    || (pMsLLStr->pt.x > pEdge->translate_range_low))
                );
        });

        if (result != TopEdges.end())
        {
            SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);

            if (pMsLLStr->pt.x > ((EDGEDATA*)(*result))->translate_range_high)
                ret = SetCursorPos(((EDGEDATA*)(*result))->translate_range_high-1, pMsLLStr->pt.y);
            else
                ret = SetCursorPos(((EDGEDATA*)(*result))->translate_range_low, pMsLLStr->pt.y);

#ifdef _DEBUG
            std::wstring str_sc(_T("SetCursorPos :"));
            std::wstring str_t(_T("TRUE\n"));
            std::wstring str_f(_T("FALSE\n"));

            WriteConsole(stdoutHandle, str_sc.c_str(), (DWORD)str_sc.size(), (LPDWORD)&dbg_n, NULL);
            if (ret)
                WriteConsole(stdoutHandle, str_t.c_str(), (DWORD)str_t.size(), (LPDWORD)&dbg_n, NULL);
            else
                WriteConsole(stdoutHandle, str_f.c_str(), (DWORD)str_f.size(), (LPDWORD)&dbg_n, NULL);
#endif

        }

        goto finish;
    }


    // 下に移動していたのが停止した場合
    if (((direction_old & MOVE_DOWN) != 0) && ((direction_current & MOVE_DOWN) == 0))
    {
        std::vector<EDGEDATA*>::const_iterator result = std::find_if(BottomEdges.begin(), BottomEdges.end(), [pMsLLStr](EDGEDATA* pEdge)
        {
            return ((pMsLLStr->pt.y >= pEdge->edge_near)
                && (pMsLLStr->pt.y < pEdge->edge_far)
                && ((pMsLLStr->pt.x < pEdge->translate_range_high)
                    || (pMsLLStr->pt.x > pEdge->translate_range_low))
                );
        });

        if (result != BottomEdges.end())
        {
            SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);

            if (pMsLLStr->pt.x > ((EDGEDATA*)(*result))->translate_range_high)
                ret = SetCursorPos(((EDGEDATA*)(*result))->translate_range_high - 1, pMsLLStr->pt.y);
            else
                ret = SetCursorPos(((EDGEDATA*)(*result))->translate_range_low, pMsLLStr->pt.y);

#ifdef _DEBUG
            std::wstring str_sc(_T("SetCursorPos :"));
            std::wstring str_t(_T("TRUE\n"));
            std::wstring str_f(_T("FALSE\n"));

            WriteConsole(stdoutHandle, str_sc.c_str(), (DWORD)str_sc.size(), (LPDWORD)&dbg_n, NULL);
            if (ret)
                WriteConsole(stdoutHandle, str_t.c_str(), (DWORD)str_t.size(), (LPDWORD)&dbg_n, NULL);
            else
                WriteConsole(stdoutHandle, str_f.c_str(), (DWORD)str_f.size(), (LPDWORD)&dbg_n, NULL);
#endif

        }

        goto finish;
    }

finish:

#ifdef _DEBUG
    std::wstring str_x_c = std::to_wstring(pMsLLStr->pt.x);
    std::wstring str_x_o = std::to_wstring(x);
    std::wstring str_y_c = std::to_wstring(pMsLLStr->pt.y);
    std::wstring str_y_o = std::to_wstring(y);
    std::wstring str_d_c = std::to_wstring(direction_current);
    std::wstring str_d_o = std::to_wstring(direction_old);
    std::wstring str_c(_T(","));
    std::wstring str_n(_T("\n"));
    
    WriteConsole(stdoutHandle, str_x_c.c_str(), (DWORD)str_x_o.size(), (LPDWORD)&dbg_n, NULL);
    WriteConsole(stdoutHandle, str_c.c_str(), (DWORD)str_c.size(), (LPDWORD)&dbg_n, NULL);
    WriteConsole(stdoutHandle, str_x_o.c_str(), (DWORD)str_x_o.size(), (LPDWORD)&dbg_n, NULL);
    WriteConsole(stdoutHandle, str_c.c_str(), (DWORD)str_c.size(), (LPDWORD)&dbg_n, NULL);
    WriteConsole(stdoutHandle, str_y_c.c_str(), (DWORD)str_y_c.size(), (LPDWORD)&dbg_n, NULL);
    WriteConsole(stdoutHandle, str_c.c_str(), (DWORD)str_c.size(), (LPDWORD)&dbg_n, NULL);
    WriteConsole(stdoutHandle, str_y_o.c_str(), (DWORD)str_y_o.size(), (LPDWORD)&dbg_n, NULL);
    WriteConsole(stdoutHandle, str_c.c_str(), (DWORD)str_c.size(), (LPDWORD)&dbg_n, NULL);
    WriteConsole(stdoutHandle, str_d_c.c_str(), (DWORD)str_d_c.size(), (LPDWORD)&dbg_n, NULL);
    WriteConsole(stdoutHandle, str_c.c_str(), (DWORD)str_c.size(), (LPDWORD)&dbg_n, NULL);
    WriteConsole(stdoutHandle, str_d_o.c_str(), (DWORD)str_d_o.size(), (LPDWORD)&dbg_n, NULL);
    WriteConsole(stdoutHandle, str_n.c_str(), (DWORD)str_n.size(), (LPDWORD)&dbg_n, NULL);
#endif

    x = pMsLLStr->pt.x;
    y = pMsLLStr->pt.y;

    direction_old = direction_current;

    return nCode;
}

BOOL CALLBACK InfoEnumProc(HMONITOR hMon, HDC hdcMon, LPRECT lpMon, LPARAM dwData) {
    RECT* rect = new RECT(*(RECT*)lpMon);
    ((std::vector<RECT *>*)dwData)->push_back(rect);

    return TRUE;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: ここにコードを挿入してください。
    std::vector<RECT*> mon;                         // モニタ情報

#ifdef _DEBUG
    AllocConsole();
    stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
#endif

    nCmdShow = SW_MINIMIZE;
    if(lstrcpynW(szTitle, _T("MouseMoveHelper"), MAX_LOADSTRING) == NULL) exit(-1);
    hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MOUSEMOVEHELPER));
    hTaskTrayMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDC_MOUSEMOVEHELPER));

    // グローバル文字列を初期化する
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MOUSEMOVEHELPER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // アプリケーション初期化の実行:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
    EnumDisplayMonitors(NULL, NULL, (MONITORENUMPROC)InfoEnumProc, (LPARAM)&mon);

    std::for_each(mon.begin(), mon.end(), [mon](RECT* rect) {
        std::vector<RECT*>::const_iterator result;

        EDGEDATA* pEdge_left = new EDGEDATA();
        EDGEDATA* pEdge_right = new EDGEDATA();
        EDGEDATA* pEdge_top = new EDGEDATA();
        EDGEDATA* pEdge_bottom = new EDGEDATA();

        pEdge_left->edge_near = rect->left;
        pEdge_right->edge_near = rect->right;
        pEdge_top->edge_near = rect->top;
        pEdge_bottom->edge_near = rect->bottom;

        // 左端検索
        result = std::find_if(mon.begin(), mon.end(), [pEdge_left](RECT* rect)
        {
            return pEdge_left->edge_near == rect->right;
        });

        if (result != mon.end())
        {
            pEdge_left->edge_far = ((RECT*)(*result))->right - 100;
            pEdge_left->translate_range_high = ((RECT*)(*result))->top;
            pEdge_left->translate_range_low = ((RECT*)(*result))->bottom;

            LeftEdges.push_back(pEdge_left);
        }

        // 右端検索
        result = std::find_if(mon.begin(), mon.end(), [pEdge_right](RECT* rect)
        {
            return pEdge_right->edge_near == rect->left;
        });

        if (result != mon.end())
        {
            pEdge_right->edge_far = ((RECT*)(*result))->left + 100;
            pEdge_right->translate_range_high = ((RECT*)(*result))->top;
            pEdge_right->translate_range_low = ((RECT*)(*result))->bottom;

            RightEdges.push_back(pEdge_right);
        }

        // 上端検索
        result = std::find_if(mon.begin(), mon.end(), [pEdge_top](RECT* rect)
        {
            return pEdge_top->edge_near == rect->bottom;
        });

        if (result != mon.end())
        {
            pEdge_top->edge_far = ((RECT*)(*result))->bottom - 100;
            pEdge_top->translate_range_high = ((RECT*)(*result))->right;
            pEdge_top->translate_range_low = ((RECT*)(*result))->left;

            TopEdges.push_back(pEdge_top);
        }

        // 下端検索
        result = std::find_if(mon.begin(), mon.end(), [pEdge_bottom](RECT* rect)
        {
            return pEdge_bottom->edge_near == rect->top;
        });

        if (result != mon.end())
        {
            pEdge_bottom->edge_far = ((RECT*)(*result))->top + 100;
            pEdge_bottom->translate_range_high = ((RECT*)(*result))->right;
            pEdge_bottom->translate_range_low = ((RECT*)(*result))->left;

            BottomEdges.push_back(pEdge_bottom);
        }
    });

    hHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0);

    MSG msg;

    // メイン メッセージ ループ:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}



//
//  関数: MyRegisterClass()
//
//  目的: ウィンドウ クラスを登録します。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = hIcon;
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MOUSEMOVEHELPER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   関数: InitInstance(HINSTANCE, int)
//
//   目的: インスタンス ハンドルを保存して、メイン ウィンドウを作成します
//
//   コメント:
//
//        この関数で、グローバル変数でインスタンス ハンドルを保存し、
//        メイン プログラム ウィンドウを作成および表示します。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // グローバル変数にインスタンス ハンドルを格納する

   hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

void addTaskTrayIcon(HWND hWnd)
{
    NOTIFYICONDATA nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.uFlags = (NIF_ICON | NIF_MESSAGE | NIF_TIP);
    nid.hWnd = hWnd;
    nid.hIcon = hIcon;
    nid.uCallbackMessage = WM_TASKTRAY;

    Shell_NotifyIcon(NIM_ADD, &nid);
}

void delTaskTrayIcon(HWND hWnd)
{
    NOTIFYICONDATA nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

//
//  関数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND  - アプリケーション メニューの処理
//  WM_PAINT    - メイン ウィンドウを描画する
//  WM_DESTROY  - 中止メッセージを表示して戻る
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SIZE:
    {
        int param = LOWORD(wParam);
        switch (param)
        {
            // 最小化した時
        case SIZE_MINIMIZED:
        {
            // タスクトレイにアイコンを追加
            addTaskTrayIcon(hWnd);

            // タスクバー内のアプリアイコンを非表示
            ShowWindow(FindWindow(TEXT("MouseMoveHelper"), NULL), SW_HIDE);
        }
        break;
        }
    }
    break;

    case WM_TASKTRAY:
    {
        switch (lParam)
        {
        case WM_RBUTTONDOWN:
        {
            // hTaskTrayMenuは初期処理でセットしている
            if (hTaskTrayMenu)
            {
                // GetSubMenuは0でメニューの一番左のドロップダウンリストを取得する
                HMENU hTrayIconSubMenu = GetSubMenu(hTaskTrayMenu, 0);
                if (hTrayIconSubMenu)
                {
                    // タスクトレイのアイコンを右クリックした時の座標を取得
                    POINT po;
                    GetCursorPos(&po);

                    // ウィンドウをフォアグラウンドに持ってくる
                    SetForegroundWindow(hWnd);

                    // これをしないと、メニュー外をクリックした時に、ポップアップメニューが消えない
                    SetFocus(hWnd);

                    // メニューの表示
                    TrackPopupMenu(hTrayIconSubMenu, TPM_LEFTALIGN | TPM_BOTTOMALIGN,
                        po.x, po.y, 0, hWnd, NULL
                    );
                }
            }
        }
        break;
        }
    }

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 選択されたメニューの解析:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: HDC を使用する描画コードをここに追加してください...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        //終了時タスクトレイからIconを削除
        delTaskTrayIcon(hWnd);
        UnhookWindowsHookEx(hHook);

#ifdef _DEBUG
        FreeConsole();
#endif

        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// バージョン情報ボックスのメッセージ ハンドラーです。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
