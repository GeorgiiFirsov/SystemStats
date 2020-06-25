// Project headers
#include "stdafx.h"
#include "MainApp.h"
#include "resource.h"
#include "i18n.h"
#include "Utils.h"

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

        case WM_VIEWITEMDBLCLICK:
            return Application.OnViewItemDblClicked(hWnd, wParam, lParam);
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

            int iWidth  = Rect.Width() < g_iWndMinWidth ? g_iWndMinWidth : Rect.Width();
            int iHeight = Rect.Height() < g_iWndMinHeight ? g_iWndMinHeight : Rect.Height();

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

            CPoint TLPoint(g_iLVOffsetLeft, g_iLVOffsetTop);
            CPoint BRPoint(
                Rect.Width() - g_iLVOffsetLeft - 18, // Magic number
                Rect.Height() - g_iLVOffsetTop - 10  // Magic number
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

            CPoint SnapBtnPos(g_iSnapBtnOffsetLeft, g_iSnapBtnOffsetTop);

            m_hSnapBtn.Create(
                i18n::LoadUIString(IDS_SNAPSHOT), 
                WS_CHILD | BS_CENTER | BS_VCENTER | BS_PUSHBUTTON, 
                CRect(SnapBtnPos, BUTTON_SIZE),
                CWnd::FromHandle(m_hWnd),
                g_nSnapBtnID
            );

            m_hSnapBtn.ShowWindow(nShowCmd);
            m_hSnapBtn.UpdateWindow();

            //
            // Create save button
            // 
            
            CPoint SaveBtnPos(g_iSaveBtnOffsetLeft, g_iSaveBtnOffsetTop);

            m_hSaveBtn.Create(
                i18n::LoadUIString(IDS_DUMP),
                WS_CHILD | BS_CENTER | BS_VCENTER | BS_PUSHBUTTON,
                CRect(SaveBtnPos, BUTTON_SIZE),
                CWnd::FromHandle(m_hWnd),
                g_nSaveBtnID
            );

            m_hSaveBtn.ShowWindow(nShowCmd);
            m_hSaveBtn.UpdateWindow();

            // TODO: timeout combobox

            _RunScheduler();
        });
    }

    DWORD CMainApp::_UpdateInfo()
	{
		utils::CWaitCursor wc;

        //
        // Freeze current state to retrieve information
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

        //
        // Clear view and fill it with new info
        // 

        m_View.DeleteAllItems();

        do 
        {
            m_View.InsertItem(Info);
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
            ::WaitForSingleObject(m_hSchedulerThread, g_dwSchedulerWaitTimeout);
            ::CloseHandle(m_hSchedulerThread), m_hSchedulerThread = nullptr;
        }
    }

    bool CMainApp::_IsSchedulerRunning()
    {
        if (!m_hSchedulerThread) {
            return false;
        }

        //
        // Double check for thread state
        // 

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
            ::Sleep(self->m_dwTimeOut * 1000);
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

        Rect.right  -= Rect.left - g_iLVOffsetLeft;
        Rect.bottom -= Rect.top - g_iLVOffsetTop;
        Rect.left = g_iLVOffsetLeft;
        Rect.top  = g_iLVOffsetTop;

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

        if (iWidth < g_iWndMinWidth)
        {
            if (wParam == WMSZ_LEFT || wParam == WMSZ_TOPLEFT || wParam == WMSZ_BOTTOMLEFT) {
                pRect->left = pRect->right - g_iWndMinWidth;
            }
            else if (wParam == WMSZ_RIGHT || wParam == WMSZ_TOPRIGHT || wParam == WMSZ_BOTTOMRIGHT) {
                pRect->right = pRect->left + g_iWndMinWidth;
            }
        }

        if (iHeight < g_iWndMinHeight)
        {
            if (wParam == WMSZ_TOP || wParam == WMSZ_TOPLEFT || wParam == WMSZ_TOPRIGHT) {
                pRect->top = pRect->bottom - g_iWndMinHeight;
            }
            else if (wParam == WMSZ_BOTTOM || wParam == WMSZ_BOTTOMLEFT || wParam == WMSZ_BOTTOMRIGHT) {
                pRect->bottom = pRect->top + g_iWndMinHeight;
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
        case g_nSnapBtnID:
            return OnSnapBtnPressed();
            break;

        case g_nSaveBtnID:
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
        catch(const exc::CWin32Error& error)
        {
            exc::DisplayErrorMessage(error);
            return error.Code();
        }

        return static_cast<LRESULT>(0);
    }

    LRESULT MSG_HANDLER CMainApp::OnSaveBtnPressed()
    {
        namespace exc = system_stats::exception;

        DWORD dwResult = 0;
        bool bIsSchedulerRunning = _IsSchedulerRunning();

        if (bIsSchedulerRunning) {
            _StopScheduler();
        }

        try
        {
            CDlgSave Dlg(CWnd::FromHandle(m_hWnd));

            if (Dlg.DoModal() == IDOK) 
            {
                ::MessageBox(m_hWnd, L"Saving is not supported yet", szApplicationName, MB_ICONWARNING);
            }
        }
        catch(const exc::CWin32Error& error)
        {
            exc::DisplayErrorMessage(error);
            dwResult = error.Code();
        }

        if (bIsSchedulerRunning) {
            _RunScheduler();
        }

        return static_cast<LRESULT>(dwResult);
    }

    LRESULT MSG_HANDLER CMainApp::OnViewItemDblClicked(_In_ HWND hWnd, _In_ WPARAM wParam, _In_ LPARAM lParam)
    {
        UNREFERENCED_PARAMETER(hWnd);
        UNREFERENCED_PARAMETER(lParam);

        namespace exc = system_stats::exception;

        DWORD dwResult = 0;
        bool bIsSchedulerRunning = _IsSchedulerRunning();

        if (bIsSchedulerRunning) {
            _StopScheduler();
        }

        try
        {
            auto nItem = static_cast<size_t>(wParam);

            //
            // Query selected item from view and
            // start dialog
            // 

            PROCESSENTRY32 Info;
            dwResult = m_View.GetNthProcess(nItem, Info);

            if (dwResult != ERROR_SUCCESS) {
                ERROR_THROW_CODE(ERROR_INVALID_INDEX, L"Invalid item selected");
            }

            CDlgProcessInfo Dlg(Info, CWnd::FromHandle(m_hWnd));
            Dlg.DoModal();
        }
        catch(const exc::CWin32Error& error)
        {
            exc::DisplayErrorMessage(error);
            dwResult = error.Code();
        }

        if (bIsSchedulerRunning) {
            _RunScheduler();
        }

        return static_cast<LRESULT>(dwResult);
    }

} // namespace system_stats