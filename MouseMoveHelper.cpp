// MouseMoveHelper.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "framework.h"
#include "MouseMoveHelper.h"

#define MAX_LOADSTRING 100
#define WM_TASKTRAY (WM_APP + 1)
#define EDGE_CHECK_OFFSET  100

// グローバル変数:
HINSTANCE hInst;                                // 現在のインターフェイス
WCHAR szTitle[MAX_LOADSTRING];                  // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING];            // メイン ウィンドウ クラス名
HICON hIcon;                                    // アイコン
HMENU hTaskTrayMenu;                            // メニュー
HHOOK hHook;                                    // マウスのフック用
HWND  hWnd;                                     // Wndのハンドル

std::vector<std::shared_ptr<EDGEDATA>> RightEdges;
std::vector<std::shared_ptr<EDGEDATA>> LeftEdges;
std::vector<std::shared_ptr<EDGEDATA>> TopEdges;
std::vector<std::shared_ptr<EDGEDATA>> BottomEdges;

BOOL RelativeMode = FALSE;
BOOL AvoidMode = TRUE;


#ifdef _DEBUG
HANDLE stdoutHandle;                            // DEBUG用コンソールの標準出力
int dbg_n;                                      // DEBUG用コンソールから出力した文字数
#endif

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM                MyRegisterClass(HINSTANCE hInstance) noexcept;
BOOL                InitInstance(HINSTANCE, int) noexcept;
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM) noexcept;


LRESULT CALLBACK LowLevelMouseProc(_In_ int nCode, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
    MSLLHOOKSTRUCT* pMsLLStr = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
    POINT p = {};
    static long x = 0;
    static long y = 0;

#ifdef _DEBUG
    static char direction_old = 0;
    static int z = 0;
#endif

    char direction_current = 0;
    std::vector<std::shared_ptr<EDGEDATA>>::const_iterator result;

    if (nCode < HC_ACTION)
    {
        return CallNextHookEx(hHook, nCode, wParam, lParam);
    }

    if (wParam != WM_MOUSEMOVE) return nCode;

    auto end = gsl::finally([&pMsLLStr, &direction_current, nCode, wParam, lParam]()
#ifndef _DEBUG        
        noexcept
#endif
        {

#ifdef _DEBUG
        std::wstring str_x_c = std::to_wstring(pMsLLStr->pt.x);
        std::wstring str_x_o = std::to_wstring(x);
        std::wstring str_y_c = std::to_wstring(pMsLLStr->pt.y);
        std::wstring str_y_o = std::to_wstring(y);
        std::wstring str_d_c = std::to_wstring(direction_current);
        std::wstring str_d_o = std::to_wstring(direction_old);
        std::wstring str_z   = std::to_wstring(z);
        std::wstring str_c(_T(","));
        std::wstring str_n(_T("\n"));

        WriteConsole(stdoutHandle, str_x_c.c_str(), gsl::narrow_cast<DWORD>(str_x_o.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
        WriteConsole(stdoutHandle, str_c.c_str(), gsl::narrow_cast<DWORD>(str_c.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
        WriteConsole(stdoutHandle, str_x_o.c_str(), gsl::narrow_cast<DWORD>(str_x_o.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
        WriteConsole(stdoutHandle, str_c.c_str(), gsl::narrow_cast<DWORD>(str_c.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
        WriteConsole(stdoutHandle, str_y_c.c_str(), gsl::narrow_cast<DWORD>(str_y_c.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
        WriteConsole(stdoutHandle, str_c.c_str(), gsl::narrow_cast<DWORD>(str_c.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
        WriteConsole(stdoutHandle, str_y_o.c_str(), gsl::narrow_cast<DWORD>(str_y_o.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
        WriteConsole(stdoutHandle, str_c.c_str(), gsl::narrow_cast<DWORD>(str_c.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
        WriteConsole(stdoutHandle, str_d_c.c_str(), gsl::narrow_cast<DWORD>(str_d_c.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
        WriteConsole(stdoutHandle, str_c.c_str(), gsl::narrow_cast<DWORD>(str_c.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
        WriteConsole(stdoutHandle, str_d_o.c_str(), gsl::narrow_cast<DWORD>(str_d_o.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
        WriteConsole(stdoutHandle, str_c.c_str(), gsl::narrow_cast<DWORD>(str_c.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
        WriteConsole(stdoutHandle, str_z.c_str(), gsl::narrow_cast<DWORD>(str_z.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
        WriteConsole(stdoutHandle, str_n.c_str(), gsl::narrow_cast<DWORD>(str_n.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);

        direction_old = direction_current;
        z++;
#endif

        x = pMsLLStr->pt.x;
        y = pMsLLStr->pt.y;


        return CallNextHookEx(hHook, nCode, wParam, lParam);
        });


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

    // 左に移動していた場合
    if ((direction_current & MOVE_LEFT) != 0)
    {
        result = std::find_if(LeftEdges.begin(), LeftEdges.end(), [pMsLLStr](std::shared_ptr<EDGEDATA> pEdge) noexcept
        {
            return ((pMsLLStr->pt.x <= pEdge->edge_near)
                && (pMsLLStr->pt.x > pEdge->edge_far - EDGE_CHECK_OFFSET)
                && ((pMsLLStr->pt.y < pEdge->translate_range_high)
                    || (pMsLLStr->pt.y > pEdge->translate_range_low))
                );
        });

        if (result != LeftEdges.end())
        {

            if (pMsLLStr->pt.y < (std::shared_ptr<EDGEDATA>(*result))->translate_range_high)
            {
                PostMessage(hWnd, WM_MOUSEMOVE, 0, static_cast<LPARAM>((std::shared_ptr<EDGEDATA>(*result))->translate_range_high << 16) | gsl::narrow_cast<unsigned short>((std::shared_ptr<EDGEDATA>(*result))->edge_far - 1));
            }
            else
            {
                PostMessage(hWnd, WM_MOUSEMOVE, 0, static_cast<LPARAM>(((std::shared_ptr<EDGEDATA>(*result))->translate_range_low - 1) << 16) | gsl::narrow_cast<unsigned short>((std::shared_ptr<EDGEDATA>(*result))->edge_far - 1));
            }

#ifdef _DEBUG
            std::wstring str_sc(_T("SetCursorPos : "));
            std::wstring str_c(_T(","));
            std::wstring str_n(_T("\n"));
            std::wstring str_x = std::to_wstring((std::shared_ptr<EDGEDATA>(*result))->edge_far - 1);
            std::wstring str_y_h = std::to_wstring((std::shared_ptr<EDGEDATA>(*result))->translate_range_high);
            std::wstring str_y_l = std::to_wstring((std::shared_ptr<EDGEDATA>(*result))->translate_range_low - 1);

            WriteConsole(stdoutHandle, str_sc.c_str(), gsl::narrow_cast<DWORD>(str_sc.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
            WriteConsole(stdoutHandle, str_x.c_str(), gsl::narrow_cast<DWORD>(str_x.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
            WriteConsole(stdoutHandle, str_c.c_str(), gsl::narrow_cast<DWORD>(str_c.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
            WriteConsole(stdoutHandle, str_y_h.c_str(), gsl::narrow_cast<DWORD>(str_y_h.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
            WriteConsole(stdoutHandle, str_c.c_str(), gsl::narrow_cast<DWORD>(str_c.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
            WriteConsole(stdoutHandle, str_y_l.c_str(), gsl::narrow_cast<DWORD>(str_y_l.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
            WriteConsole(stdoutHandle, str_n.c_str(), gsl::narrow_cast<DWORD>(str_n.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
#endif
            return nCode;
        }
    }

    // 右に移動していた場合
    if ((direction_current & MOVE_RIGHT) != 0)
    {
        result = std::find_if(RightEdges.begin(), RightEdges.end(), [pMsLLStr](std::shared_ptr<EDGEDATA> pEdge) noexcept
        {
            return ((pMsLLStr->pt.x >= pEdge->edge_near)
                && (pMsLLStr->pt.x < pEdge->edge_far + EDGE_CHECK_OFFSET)
                && ((pMsLLStr->pt.y < pEdge->translate_range_high)
                    || (pMsLLStr->pt.y > pEdge->translate_range_low))
                );
        });

        if (result != RightEdges.end())
        {
            if (pMsLLStr->pt.y < (std::shared_ptr<EDGEDATA>(*result))->translate_range_high)
                PostMessage(hWnd, WM_MOUSEMOVE, 0, static_cast<LPARAM>((std::shared_ptr<EDGEDATA>(*result))->translate_range_high << 16) | gsl::narrow_cast<unsigned short>((std::shared_ptr<EDGEDATA>(*result))->edge_far + 1));
            else
                PostMessage(hWnd, WM_MOUSEMOVE, 0, static_cast<LPARAM>(((std::shared_ptr<EDGEDATA>(*result))->translate_range_low - 1) << 16) | gsl::narrow_cast<unsigned short>((std::shared_ptr<EDGEDATA>(*result))->edge_far + 1));

#ifdef _DEBUG
            std::wstring str_sc(_T("SetCursorPos : "));
            std::wstring str_c(_T(","));
            std::wstring str_n(_T("\n"));
            std::wstring str_x = std::to_wstring((std::shared_ptr<EDGEDATA>(*result))->edge_far + 1);
            std::wstring str_y_h = std::to_wstring((std::shared_ptr<EDGEDATA>(*result))->translate_range_high);
            std::wstring str_y_l = std::to_wstring((std::shared_ptr<EDGEDATA>(*result))->translate_range_low - 1);

            WriteConsole(stdoutHandle, str_sc.c_str(), gsl::narrow_cast<DWORD>(str_sc.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
                WriteConsole(stdoutHandle, str_x.c_str(), gsl::narrow_cast<DWORD>(str_x.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
                WriteConsole(stdoutHandle, str_c.c_str(), gsl::narrow_cast<DWORD>(str_c.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
                WriteConsole(stdoutHandle, str_y_h.c_str(), gsl::narrow_cast<DWORD>(str_y_h.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
                WriteConsole(stdoutHandle, str_c.c_str(), gsl::narrow_cast<DWORD>(str_c.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
                WriteConsole(stdoutHandle, str_y_l.c_str(), gsl::narrow_cast<DWORD>(str_y_l.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
                WriteConsole(stdoutHandle, str_n.c_str(), gsl::narrow_cast<DWORD>(str_n.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
#endif
            return nCode;
        }
    }

    // 上に移動していた場合
    if ((direction_current & MOVE_UP) != 0)
    {
        result = std::find_if(TopEdges.begin(), TopEdges.end(), [pMsLLStr](std::shared_ptr<EDGEDATA> pEdge) noexcept
        {
            return ((pMsLLStr->pt.y <= pEdge->edge_near)
                && (pMsLLStr->pt.y > pEdge->edge_far - EDGE_CHECK_OFFSET)
                && ((pMsLLStr->pt.x < pEdge->translate_range_high)
                    || (pMsLLStr->pt.x > pEdge->translate_range_low))
                );
        });

        if (result != TopEdges.end())
        {
            if (pMsLLStr->pt.x > (std::shared_ptr<EDGEDATA>(*result))->translate_range_high)
                PostMessage(hWnd, WM_MOUSEMOVE, 0, static_cast<LPARAM>(((std::shared_ptr<EDGEDATA>(*result))->edge_far - 1) << 16) | gsl::narrow_cast<unsigned short>((std::shared_ptr<EDGEDATA>(*result)->translate_range_high - 1)));
            else
                PostMessage(hWnd, WM_MOUSEMOVE, 0, static_cast<LPARAM>(((std::shared_ptr<EDGEDATA>(*result))->edge_far - 1) << 16) | gsl::narrow_cast<unsigned short>(std::shared_ptr<EDGEDATA>(*result)->translate_range_low));
 
#ifdef _DEBUG
            std::wstring str_sc(_T("SetCursorPos : "));
            std::wstring str_c(_T(","));
            std::wstring str_n(_T("\n"));
            std::wstring str_x_h = std::to_wstring((std::shared_ptr<EDGEDATA>(*result))->translate_range_high - 1);
            std::wstring str_x_l = std::to_wstring((std::shared_ptr<EDGEDATA>(*result))->translate_range_low);
            std::wstring str_y = std::to_wstring((std::shared_ptr<EDGEDATA>(*result))->edge_far - 1);

            WriteConsole(stdoutHandle, str_sc.c_str(), gsl::narrow_cast<DWORD>(str_sc.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
                WriteConsole(stdoutHandle, str_x_h.c_str(), gsl::narrow_cast<DWORD>(str_x_h.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
                WriteConsole(stdoutHandle, str_n.c_str(), gsl::narrow_cast<DWORD>(str_n.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
                WriteConsole(stdoutHandle, str_x_l.c_str(), gsl::narrow_cast<DWORD>(str_x_l.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
                WriteConsole(stdoutHandle, str_c.c_str(), gsl::narrow_cast<DWORD>(str_c.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
                WriteConsole(stdoutHandle, str_y.c_str(), gsl::narrow_cast<DWORD>(str_y.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
                WriteConsole(stdoutHandle, str_n.c_str(), gsl::narrow_cast<DWORD>(str_n.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
#endif
            return nCode;
        }
    }


    // 下に移動していた場合
    if ((direction_current & MOVE_DOWN) != 0)
    {
        result = std::find_if(BottomEdges.begin(), BottomEdges.end(), [pMsLLStr](std::shared_ptr<EDGEDATA> pEdge) noexcept
        {
            return ((pMsLLStr->pt.y >= pEdge->edge_near)
                && (pMsLLStr->pt.y < pEdge->edge_far + EDGE_CHECK_OFFSET)
                && ((pMsLLStr->pt.x < pEdge->translate_range_high)
                    || (pMsLLStr->pt.x > pEdge->translate_range_low))
                );
        });

        if (result != BottomEdges.end())
        {
            if (pMsLLStr->pt.x > (std::shared_ptr<EDGEDATA>(*result))->translate_range_high)
                PostMessage(hWnd, WM_MOUSEMOVE, 0, static_cast<LPARAM>(((std::shared_ptr<EDGEDATA>(*result))->edge_far + 1) << 16) | gsl::narrow_cast<unsigned short>((std::shared_ptr<EDGEDATA>(*result)->translate_range_high - 1)));
            else
                PostMessage(hWnd, WM_MOUSEMOVE, 0, static_cast<LPARAM>(((std::shared_ptr<EDGEDATA>(*result))->edge_far + 1) << 16) | gsl::narrow_cast<unsigned short>(std::shared_ptr<EDGEDATA>(*result)->translate_range_low));

#ifdef _DEBUG
            std::wstring str_sc(_T("SetCursorPos : "));
            std::wstring str_c(_T(","));
            std::wstring str_n(_T("\n"));
            std::wstring str_x_h = std::to_wstring((std::shared_ptr<EDGEDATA>(*result))->translate_range_high - 1);
            std::wstring str_x_l = std::to_wstring((std::shared_ptr<EDGEDATA>(*result))->translate_range_low);
            std::wstring str_y = std::to_wstring((std::shared_ptr<EDGEDATA>(*result))->edge_far + 1);

            WriteConsole(stdoutHandle, str_sc.c_str(), gsl::narrow_cast<DWORD>(str_sc.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
                WriteConsole(stdoutHandle, str_x_h.c_str(), gsl::narrow_cast<DWORD>(str_x_h.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
                WriteConsole(stdoutHandle, str_c.c_str(), gsl::narrow_cast<DWORD>(str_c.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
                WriteConsole(stdoutHandle, str_x_l.c_str(), gsl::narrow_cast<DWORD>(str_x_l.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
                WriteConsole(stdoutHandle, str_c.c_str(), gsl::narrow_cast<DWORD>(str_c.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
                WriteConsole(stdoutHandle, str_y.c_str(), gsl::narrow_cast<DWORD>(str_y.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
                WriteConsole(stdoutHandle, str_n.c_str(), gsl::narrow_cast<DWORD>(str_n.size()), reinterpret_cast<LPDWORD>(&dbg_n), NULL);
#endif
            return nCode;
        }
    }

    return nCode;
}

BOOL CALLBACK InfoEnumProc(HMONITOR hMon, HDC hdcMon, gsl::not_null<LPRECT> lpMon, LPARAM dwData) {
    BOOL ret = FALSE;
    MONITORINFO mi = {};

    mi.cbSize = sizeof(mi);
    ret = GetMonitorInfoW(hMon, &mi);

    (reinterpret_cast<std::vector<std::shared_ptr<MONITORINFO>>*>(dwData))->push_back(move(std::make_shared<MONITORINFO>(mi)));


    return TRUE;
}

void GetMonitorAreaInfo()
{
    std::vector<std::shared_ptr<MONITORINFO>> mon;                         // モニタ情報

    EnumDisplayMonitors(NULL, NULL, (MONITORENUMPROC)InfoEnumProc, reinterpret_cast<LPARAM>(& mon));

    LeftEdges.clear();
    RightEdges.clear();
    TopEdges.clear();
    BottomEdges.clear();

    std::for_each(mon.begin(), mon.end(), [mon](std::shared_ptr<MONITORINFO> mi) {
        std::vector<std::shared_ptr<MONITORINFO>>::const_iterator result;

        std::shared_ptr<EDGEDATA> pEdge_left = std::make_shared<EDGEDATA>();
        std::shared_ptr<EDGEDATA> pEdge_right = std::make_shared<EDGEDATA>();
        std::shared_ptr<EDGEDATA> pEdge_top = std::make_shared<EDGEDATA>();
        std::shared_ptr<EDGEDATA> pEdge_bottom = std::make_shared<EDGEDATA>();

        pEdge_left->edge_near = mi->rcMonitor.left;
        pEdge_right->edge_near = mi->rcMonitor.right;
        pEdge_top->edge_near = mi->rcMonitor.top;
        pEdge_bottom->edge_near = mi->rcMonitor.bottom;

        // 左端検索
        result = std::find_if(mon.begin(), mon.end(), [pEdge_left](std::shared_ptr<MONITORINFO> mi) noexcept
            {
                return pEdge_left->edge_near == mi->rcMonitor.right;
            });

        if (result != mon.end())
        {
            if (AvoidMode)
            {
                pEdge_left->edge_far = ((*result))->rcWork.right;
                pEdge_left->translate_range_high = ((*result))->rcWork.top;
                pEdge_left->translate_range_low = ((*result))->rcWork.bottom;
            }
            else
            {
                pEdge_left->edge_far = ((*result))->rcMonitor.right - 100;
                pEdge_left->translate_range_high = ((*result))->rcMonitor.top;
                pEdge_left->translate_range_low = ((*result))->rcMonitor.bottom;
            }

            LeftEdges.push_back(std::move(pEdge_left));
        }


        // 右端検索
        result = std::find_if(mon.begin(), mon.end(), [pEdge_right](std::shared_ptr<MONITORINFO> mi) noexcept
            {
                return pEdge_right->edge_near == mi->rcMonitor.left;
            });

        if (result != mon.end())
        {
            if (AvoidMode)
            {
                pEdge_right->edge_far = ((*result))->rcWork.left + 100;
                pEdge_right->translate_range_high = ((*result))->rcWork.top;
                pEdge_right->translate_range_low = ((*result))->rcWork.bottom;
            }
            else
            {
                pEdge_right->edge_far = ((*result))->rcMonitor.left + 100;
                pEdge_right->translate_range_high = ((*result))->rcMonitor.top;
                pEdge_right->translate_range_low = ((*result))->rcMonitor.bottom;
            }

            RightEdges.push_back(std::move(pEdge_right));
        }

        // 上端検索
        result = std::find_if(mon.begin(), mon.end(), [pEdge_top](std::shared_ptr<MONITORINFO> mi) noexcept
            {
                return pEdge_top->edge_near == mi->rcMonitor.bottom;
            });

        if (result != mon.end())
        {
            if (AvoidMode)
            {
                pEdge_top->edge_far = ((*result))->rcWork.bottom - 100;
                pEdge_top->translate_range_high = ((*result))->rcWork.right;
                pEdge_top->translate_range_low = ((*result))->rcWork.left;
            }
            else
            {
                pEdge_top->edge_far = ((*result))->rcMonitor.bottom - 100;
                pEdge_top->translate_range_high = ((*result))->rcMonitor.right;
                pEdge_top->translate_range_low = ((*result))->rcMonitor.left;
            }

            TopEdges.push_back(std::move(pEdge_top));
        }

        // 下端検索
        result = std::find_if(mon.begin(), mon.end(), [pEdge_bottom](std::shared_ptr<MONITORINFO> mi) noexcept
            {
                return pEdge_bottom->edge_near == mi->rcMonitor.top;
            });

        if (result != mon.end())
        {
            if (AvoidMode)
            {
                pEdge_bottom->edge_far = ((*result))->rcWork.top + 100;
                pEdge_bottom->translate_range_high = ((*result))->rcWork.right;
                pEdge_bottom->translate_range_low = ((*result))->rcWork.left;
            }
            else
            {
                pEdge_bottom->edge_far = ((*result))->rcMonitor.top + 100;
                pEdge_bottom->translate_range_high = ((*result))->rcMonitor.right;
                pEdge_bottom->translate_range_low = ((*result))->rcMonitor.left;
            }

            BottomEdges.push_back(std::move(pEdge_bottom));
        }
    });

}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: ここにコードを挿入してください。
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
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
    GetMonitorAreaInfo();
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
ATOM MyRegisterClass(HINSTANCE hInstance) noexcept
{
    WNDCLASSEXW wcex{};

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = hIcon;
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = reinterpret_cast<HBRUSH>(COLOR_WINDOW+1);
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
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) noexcept
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

void addTaskTrayIcon(HWND hWnd) noexcept
{
    NOTIFYICONDATA nid = { 0 };
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.uFlags = (NIF_ICON | NIF_MESSAGE | NIF_TIP);
    nid.hWnd = hWnd;
    nid.hIcon = hIcon;
    nid.uCallbackMessage = WM_TASKTRAY;

    Shell_NotifyIcon(NIM_ADD, &nid);
}

void delTaskTrayIcon(HWND hWnd) noexcept
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
        const int param = LOWORD(wParam);
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

        default:
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

        default:
            break;
        }
    }

    case WM_COMMAND:
        {
            const int wmId = LOWORD(wParam);
            // 選択されたメニューの解析:
            switch (wmId)
            {
            case ID_MENU_RELATIVE_MODE:
                if (hTaskTrayMenu)
                {
                    RelativeMode = !RelativeMode;
                    if(RelativeMode)
                        CheckMenuItem(hTaskTrayMenu, ID_MENU_RELATIVE_MODE, MF_BYCOMMAND | MFS_CHECKED);
                    else
                        CheckMenuItem(hTaskTrayMenu, ID_MENU_RELATIVE_MODE, MF_BYCOMMAND | MFS_UNCHECKED);
                }

                GetMonitorAreaInfo();

                break;

            case ID_MENU_TASKBAR_AVOID_MODE:
                if (hTaskTrayMenu)
                {
                    AvoidMode = !AvoidMode;
                    if (AvoidMode)
                        CheckMenuItem(hTaskTrayMenu, ID_MENU_TASKBAR_AVOID_MODE, MF_BYCOMMAND | MFS_CHECKED);
                    else
                        CheckMenuItem(hTaskTrayMenu, ID_MENU_TASKBAR_AVOID_MODE, MF_BYCOMMAND | MFS_UNCHECKED);
                }

                GetMonitorAreaInfo();
                break;

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

    case WM_MOUSEMOVE:
    {   
        UnhookWindowsHookEx(hHook);
        Sleep(100);
        SetCursorPos(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        Sleep(100);
        hHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0);

        break;
    }

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            //HDC hdc = BeginPaint(hWnd, &ps);
            const HDC__* const hdc = BeginPaint(hWnd, &ps);
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
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) noexcept
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return static_cast<INT_PTR>(TRUE);

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return static_cast<INT_PTR>(TRUE);
        }
        break;

    default: break;
    }
    return static_cast<INT_PTR>(FALSE);
}
