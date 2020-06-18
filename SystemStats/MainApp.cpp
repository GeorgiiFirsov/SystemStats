// Project headers
#include "stdafx.h"
#include "MainApp.h"
#include "ProcessInfo.h"

// STL headers
#include <vector>
#include <mutex> // std::call_once

namespace system_stats
{

    /* static */ CMainApp& CMainApp::Instance()
    {
        static CMainApp App;
        return App;
    }

    /* static */ LRESULT WINAPI CMainApp::WndProc(
        _In_ HWND   hWnd,
        _In_ UINT   Msg,
        _In_ WPARAM wParam,
        _In_ LPARAM lParam
    )
    {
        switch (Msg)
        {
        case WM_TIMEDUPDATE:
            return Application.OnTimedUpdate(hWnd, wParam, lParam);
            break;

        case WM_SIZE:
            return Application.OnSize(hWnd, wParam, lParam);
            break;

        case WM_SIZING:
            return Application.OnSizing(hWnd, wParam, lParam);
            break;

        case WM_COMMAND:
            return Application.OnCommand(hWnd, wParam, lParam);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, Msg, wParam, lParam);
            break;
        }
        
        return static_cast<LRESULT>(0);
    }

    void CMainApp::InitializeInstance(_In_ HINSTANCE hInstance, _In_ LPCTSTR pszClassName)
    {
        namespace exc = system_stats::exception;

        //
        // Initialize window class and register it
        // 

        static std::once_flag flag;
        std::call_once(flag, [this, hInstance, pszClassName]() {
            m_hInstance = hInstance;

            m_WndClass.cbSize        = sizeof(WNDCLASSEX);
            m_WndClass.style         = CS_HREDRAW | CS_VREDRAW;
            m_WndClass.lpfnWndProc   = &CMainApp::WndProc;
            m_WndClass.cbClsExtra    = 0;
            m_WndClass.cbWndExtra    = 0;
            m_WndClass.hInstance     = hInstance;
            m_WndClass.hIcon         = LoadIcon(hInstance, IDI_APPLICATION);
            m_WndClass.hCursor       = LoadCursor(nullptr, IDC_ARROW);
            m_WndClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
            m_WndClass.lpszMenuName  = nullptr;
            m_WndClass.lpszClassName = pszClassName;
            m_WndClass.hIconSm       = LoadIcon(m_WndClass.hInstance, IDI_APPLICATION);

            auto Result = ::RegisterClassEx(&m_WndClass);
            if (!Result) {
                ERROR_THROW_LAST();
            }

            m_bInitialized.store(true, std::memory_order_release);
        });
    }

    void CMainApp::CreateAndShowWindow(_In_ const CRect& Rect, _In_ int nShowCmd)
    {
        namespace exc = system_stats::exception;

        //
        // If current instance is not initialized yet,
        // assume it is an error
        // 

        if (!m_bInitialized.load(std::memory_order_acquire)) {
            ERROR_THROW_CODE(ERROR_INVALID_PARAMETER);
        }

        static std::once_flag flag;
        std::call_once(flag, [this, &Rect, nShowCmd]() {

            //
            // Create main window and show it
            // 

            int iWidth  = Rect.Width() < iWndMinWidth ? iWndMinWidth : Rect.Width();
            int iHeight = Rect.Height() < iWndMinHeight ? iWndMinHeight : Rect.Height();

            m_hWnd = ::CreateWindow(
                szApplicationName,
                szApplicationName,
                WS_OVERLAPPEDWINDOW,
                Rect.left, Rect.top,
                iWidth, iHeight,
                nullptr,
                nullptr,
                m_hInstance,
                nullptr
            );

            if (!m_hWnd) {
                ERROR_THROW_LAST();
            }

            ::ShowWindow(m_hWnd, nShowCmd);
            ::UpdateWindow(m_hWnd);

            //
            // Initialize processes view and show it
            // 

            CPoint TLPoint(iLVOffsetLeft, iLVOffsetTop);
            CPoint BRPoint(
                Rect.Width() - iLVOffsetLeft - 18, // Magic number
                Rect.Height() - iLVOffsetTop - 10  // Magic number
            );

            m_View.Create(
                WS_CHILD | LVS_REPORT | LVS_EDITLABELS,
                CRect(TLPoint, BRPoint),
                CWnd::FromHandle(m_hWnd),
                UNIQUE_ID
            );

            m_View.InsertColumns(g_columns);
            m_View.ShowWindow(nShowCmd);

            //
            // Create snapshot button
            // 

            CPoint SnapBtnPos(iSnapBtnOffsetLeft, iSnapBtnOffsetTop);

            m_hSnapBtn.Create(
                szSnapBtnText, 
                WS_CHILD | BS_CENTER | BS_VCENTER | BS_PUSHBUTTON, 
                CRect(SnapBtnPos, BUTTON_SIZE),
                CWnd::FromHandle(m_hWnd),
                nSnapBtnID
            );

            m_hSnapBtn.ShowWindow(nShowCmd);
            m_hSnapBtn.UpdateWindow();

            //
            // Create save button
            // 
            
            CPoint SaveBtnPos(iSaveBtnOffsetLeft, iSaveBtnOffsetTop);

            m_hSaveBtn.Create(
                szSaveBtnText,
                WS_CHILD | BS_CENTER | BS_VCENTER | BS_PUSHBUTTON,
                CRect(SaveBtnPos, BUTTON_SIZE),
                CWnd::FromHandle(m_hWnd),
                nSaveBtnID
            );

            m_hSaveBtn.ShowWindow(nShowCmd);
            m_hSaveBtn.UpdateWindow();

            _RunScheduler();
        });
    }

    DWORD CMainApp::_UpdateInfo()
    {
        //
        // Freeze current state to retreive information
        // 

        HANDLE hProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

        if (hProcessSnapshot == INVALID_HANDLE_VALUE) 
        {
            ::OutputDebugString(__FUNCTIONW__ L": CreateToolhelp32Snapshot failed");
            return GetLastError();
        }

        //
        // Walk through all captured processes
        // 

        PROCESSENTRY32 Info { sizeof(PROCESSENTRY32) };
        if (!::Process32First(hProcessSnapshot, &Info)) 
        {
            ::OutputDebugString(__FUNCTIONW__ L": Process32First failed");
            return GetLastError();
        }

        m_View.DeleteAllItems();

        do 
        {
            m_View.InsertItem(
                CProcessInfo(Info.szExeFile, Info.th32ProcessID, Info.th32ParentProcessID, Info.cntThreads)
            );
        } while (::Process32Next(hProcessSnapshot, &Info));

        return ERROR_SUCCESS;
    }

    void CMainApp::_RunScheduler()
    {
        namespace exc = system_stats::exception;

        //
        // Reset cancellation event and start background
        // thread with scheduler
        // 

        m_hSchedulerWantStop.Reset();

        m_hSchedulerThread = reinterpret_cast<HANDLE>(::_beginthreadex(
            nullptr /* default security */,
            0       /* inherit stack size */,
            &CMainApp::_SchedulerProcedure,
            this    /* pass current instance as parameter */,
            0       /* no flags */,
            nullptr /* thread id is not in demand */
        ));

        if (!m_hSchedulerThread) {
            ERROR_THROW_CODE(ERROR_FUNCTION_NOT_CALLED);
        }
    }

    void CMainApp::_StopScheduler()
    {
        if (m_hSchedulerThread) 
        {
            //
            // Set stop event to signaling state to 
            // inform scheduler about cancellation.
            // Wait untill it stops for some time
            // 

            m_hSchedulerWantStop.Set();
            ::WaitForSingleObject(m_hSchedulerThread, ullSchedulerWaitTimeout);
            ::CloseHandle(m_hSchedulerThread), m_hSchedulerThread = nullptr;
        }
    }

    bool CMainApp::_IsSchedulerRunning()
    {
        if (!m_hSchedulerThread) {
            return false;
        }

        DWORD dwExitCode = ERROR_SUCCESS;
        ::GetExitCodeThread(m_hSchedulerThread, &dwExitCode);

        if (dwExitCode != STILL_ACTIVE) {
            return false;
        }

        DWORD dwWaitResult = ::WaitForSingleObject(m_hSchedulerThread, 0);
        return dwWaitResult == WAIT_TIMEOUT;
    }

    /* static */ unsigned int __stdcall CMainApp::_SchedulerProcedure(_In_ void* pThis) noexcept
    {
        auto self = reinterpret_cast<CMainApp*>(pThis);

        //
        // If no cancellation requested - send command and sleep
        // for some time given in seconds
        // 

        while(::WaitForSingleObject(self->m_hSchedulerWantStop, 0) == WAIT_TIMEOUT)
        {
            ::SendMessage(self->m_hWnd, WM_TIMEDUPDATE, 0, 0);
            ::Sleep(ullTimeout * 1000);
        }

        ::ExitThread(ERROR_SUCCESS);
    }


    /* Message handlers implemetation */

    LRESULT MSG_HANDLER CMainApp::OnTimedUpdate(_In_ HWND hWnd, _In_ WPARAM wParam, _In_ LPARAM lParam)
    {
        //
        // Handler of WM_TIMEDUPDATE message
        // 

        UNREFERENCED_PARAMETER(hWnd);
        UNREFERENCED_PARAMETER(wParam);
        UNREFERENCED_PARAMETER(lParam);

        //
        // Refresh processes info and redraw window
        // 

        _UpdateInfo();

        RECT Rect;
        ::GetClientRect(m_hWnd, &Rect);
        ::InvalidateRect(m_hWnd, &Rect, TRUE);

        return static_cast<LRESULT>(0);
    }

    LRESULT MSG_HANDLER CMainApp::OnSize(_In_ HWND hWnd, _In_ WPARAM wParam, _In_ LPARAM lParam)
    {
        //
        // Handler of WM_SIZE message
        // 

        UNREFERENCED_PARAMETER(hWnd);
        UNREFERENCED_PARAMETER(wParam);
        UNREFERENCED_PARAMETER(lParam);

        CRect Rect;
        ::GetWindowRect(m_hWnd, &Rect);

        Rect.right  -= Rect.left - iLVOffsetLeft;
        Rect.bottom -= Rect.top - iLVOffsetTop;
        Rect.left = iLVOffsetLeft;
        Rect.top  = iLVOffsetTop;

        m_View.MoveWindow(Rect);

        return static_cast<LRESULT>(0);
    }

    LRESULT MSG_HANDLER CMainApp::OnSizing(_In_ HWND hWnd, _In_ WPARAM wParam, _In_ LPARAM lParam)
    {
        //
        // Handler of WM_SIZING message
        // 

        UNREFERENCED_PARAMETER(hWnd);

        //
        // Check minimal size and apply minimal values if necessary
        // 

        auto pRect = reinterpret_cast<LPRECT>(lParam);

        int iWidth = pRect->right - pRect->left;
        int iHeight = pRect->bottom - pRect->top;

        if (iWidth < iWndMinWidth)
        {
            if (wParam == WMSZ_LEFT || wParam == WMSZ_TOPLEFT || wParam == WMSZ_BOTTOMLEFT) {
                pRect->left = pRect->right - iWndMinWidth;
            }
            else if (wParam == WMSZ_RIGHT || wParam == WMSZ_TOPRIGHT || wParam == WMSZ_BOTTOMRIGHT) {
                pRect->right = pRect->left + iWndMinWidth;
            }
        }

        if (iHeight < iWndMinHeight)
        {
            if (wParam == WMSZ_TOP || wParam == WMSZ_TOPLEFT || wParam == WMSZ_TOPRIGHT) {
                pRect->top = pRect->bottom - iWndMinHeight;
            }
            else if (wParam == WMSZ_BOTTOM || wParam == WMSZ_BOTTOMLEFT || wParam == WMSZ_BOTTOMRIGHT) {
                pRect->bottom = pRect->top + iWndMinHeight;
            }
        }

        return static_cast<LRESULT>(0);
    }

    LRESULT MSG_HANDLER CMainApp::OnCommand(_In_ HWND hWnd, _In_ WPARAM wParam, _In_ LPARAM lParam)
    {
        //
        // Handler of WM_COMMAND message
        // 
        
        switch (LOWORD(wParam))
        {
        case nSnapBtnID:
            return OnSnapBtnPressed();
            break;

        case nSaveBtnID:
            return OnSaveBtnPressed();
            break;

        default:
            return DefWindowProc(hWnd, WM_COMMAND, wParam, lParam);
            break;
        }
    }

    LRESULT MSG_HANDLER CMainApp::OnSnapBtnPressed()
    {
        namespace exc = system_stats::exception;

        static short counter = 0;

        try
        {
            if (counter % 2) _RunScheduler();
            else _StopScheduler();

            counter = counter % 2 + 1;
        }
        catch(const std::runtime_error& error)
        {
            exc::DisplayErrorMessage(error);
            return ERROR_FUNCTION_FAILED;
        }

        return static_cast<LRESULT>(0);
    }

    LRESULT MSG_HANDLER CMainApp::OnSaveBtnPressed()
    {
        namespace exc = system_stats::exception;
        try
        {
            bool bIsSchedulerRunning = _IsSchedulerRunning();

            if (bIsSchedulerRunning) {
                _StopScheduler();
            }

            CDlgSave Dlg(CWnd::FromHandle(m_hWnd));

            if (Dlg.DoModal() == IDOK) {
                ::MessageBox(m_hWnd, L"Saving is not supported yet", szApplicationName, MB_ICONWARNING);
            }

            if (bIsSchedulerRunning) {
                _RunScheduler();
            }
        }
        catch(const std::runtime_error& error)
        {
            OutputDebugStringA(error.what());
            exc::DisplayErrorMessage(error);

            return ERROR_FUNCTION_FAILED;
        }

        return static_cast<LRESULT>(0);
    }

} // namespace system_stats