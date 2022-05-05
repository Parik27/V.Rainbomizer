#include "common/logger.hh"
#include "debug/backend.hh"
#include "injector/calling.hpp"
#include <cstdio>
#include <mutex>
#include <utility>
#include <windows.h>

#include <imgui.h>
#include <implot.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include <d3d11.h>
#include "base.hh"

BOOL
IsMainWindow (HWND handle)
{
    return GetWindow (handle, GW_OWNER) == (HWND) 0 && IsWindowVisible (handle);
}

struct handle_data
{
    unsigned long processID;
    HWND          windowHandle;
};

BOOL CALLBACK
EnumWindowsCallback (HWND handle, LPARAM lParam)
{
    handle_data & data       = *(handle_data *) lParam;
    unsigned long process_id = 0;
    GetWindowThreadProcessId (handle, &process_id);
    if (data.processID != process_id || !IsMainWindow (handle))
        return TRUE;
    data.windowHandle = handle;
    return FALSE;
}

// https://stackoverflow.com/questions/1888863/how-to-get-main-window-handle-from-process-id
HWND
FindMainWindow (unsigned long process_id)
{
    handle_data data;
    data.processID    = process_id;
    data.windowHandle = 0;
    EnumWindows (EnumWindowsCallback, (LPARAM) &data);
    return data.windowHandle;
}

/* So much Windows specific code here I don't wanna look at it :no_eyes: */
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler (HWND hWnd, UINT msg,
                                                       WPARAM wParam,
                                                       LPARAM lParam);
class DebugServerBackend
{
    typedef void (*PresentCallback) (IDXGISwapChain *);

    inline static bool bIsOpen = false;

    /*******************************************************/
    // Stolen from
    // https://stackoverflow.com/questions/1888863/how-to-get-main-window-handle-from-process-id
    // Kappa
    /*******************************************************/
    static HWND
    FindTopWindow (DWORD pid)
    {
        std::pair<HWND, DWORD> params = {0, pid};

        // Enumerate the windows using a lambda to process each window
        BOOL bResult = EnumWindows (
            [] (HWND hwnd, LPARAM lParam) -> BOOL {
                auto pParams = (std::pair<HWND, DWORD> *) (lParam);

                DWORD processId;
                if (GetWindowThreadProcessId (hwnd, &processId)
                    && processId == pParams->second)
                    {
                        // Stop enumerating
                        SetLastError (-1);
                        pParams->first = hwnd;
                        return FALSE;
                    }

                // Continue enumerating
                return TRUE;
            },
            (LPARAM) &params);

        if (!bResult && GetLastError () == -1 && params.first)
            {
                return params.first;
            }

        return 0;
    }

    /*******************************************************/
    template <auto &OrigWndProc>
    static LRESULT __stdcall WndProc (HWND hand, UINT msg, WPARAM wParam,
                                      LPARAM lParam)
    {
        if (msg == WM_KEYUP)
            {
                if (wParam == VK_F9)
                    {
                        bIsOpen                         = !bIsOpen;
                        ImGui::GetIO ().MouseDrawCursor = bIsOpen;
                        DebugInterfaceManager::SetControlsBlocked (bIsOpen);
                    }
            }

        if (bIsOpen)
            ImGui_ImplWin32_WndProcHandler (hand, msg, wParam, lParam);

        return CallWindowProcW ((WNDPROC) OrigWndProc, hand, msg, wParam,
                                lParam);
    }

    /*******************************************************/
    static void
    UpdateInterfaces ()
    {
        ImGui_ImplDX11_NewFrame ();
        ImGui_ImplWin32_NewFrame ();
        ImGui::NewFrame ();

        DebugInterfaceManager::DrawAll ();

        ImGui::Render ();
        ImGui_ImplDX11_RenderDrawData (ImGui::GetDrawData ());
    }

    /*******************************************************/
    static void
    OnPresent (IDXGISwapChain *swap)
    {
        thread_local static bool bInitialisedImgui = false;

        if (!bInitialisedImgui)
            {
                ID3D11Device *       device;
                ID3D11DeviceContext *context;

                swap->GetDevice (IID_ID3D11Device, (void **) &device);
                device->GetImmediateContext (&context);

                ImGui::CreateContext ();
                ImPlot::CreateContext ();
                ImGui::StyleColorsDark ();

                HWND window = FindMainWindow (GetCurrentProcessId ());
                if (ImGui_ImplWin32_Init (window)
                    && ImGui_ImplDX11_Init (device, context))
                    {
                        static LONG_PTR oldProc;
                        oldProc
                            = SetWindowLongPtr (window, GWLP_WNDPROC,
                                                (LONG_PTR) WndProc<oldProc>);

                        Rainbomizer::Logger::LogMessage (
                            "Initialised imgui, windae: %x, dev1ce: %x, sw4p: "
                            "%x, ctx: "
                            "%x",
                            window, device, swap, context);

                        bInitialisedImgui = true;
                    }
            }

        for (const auto &i : BackendWalkieTalkie::sm_Functions)
            i ();

        BackendWalkieTalkie::sm_Functions.clear ();

        if (bIsOpen)
            UpdateInterfaces ();

        DebugInterfaceManager::UpdateAll ();
    }

public:
    /*******************************************************/
    DebugServerBackend ()
    {
        HMODULE scriptHook = LoadLibrary (TEXT ("ScriptHookV.dll"));
        if (!scriptHook)
            {
                Rainbomizer::Logger::LogMessage ("No ScriptHook");
                return;
            }

        typedef void (*presentCallbackRegister_t) (PresentCallback);
        auto *presentCallbackRegister
            = (presentCallbackRegister_t) (GetProcAddress (
                scriptHook, "?presentCallbackRegister@@YAXP6AXPEAX@Z@Z"));

        if (presentCallbackRegister)
            presentCallbackRegister (OnPresent);
        else
            Rainbomizer::Logger::LogMessage ("No presentCallbackRegister");
        ;
    }
} debug;
