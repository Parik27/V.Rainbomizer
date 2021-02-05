#pragma once

#include "server.hh"
#include "CTheScripts.hh"
#include <vector>
#include <memory>
#include <map>
#include <optional>

class ScriptDebugInterface
{
    inline static struct
    {
        uint64_t *        Stack;
        uint64_t *        Globals;
        scrProgram *      Program;
        scrThreadContext *Ctx;
    } ThreadInfo;

    struct CapturedThread
    {
        const inline static int BREAKPOINT_OPCODE = 127;

        enum
        {
            STOPPED,
            RUNNING
        } m_eState
            = STOPPED;

        scrProgram *m_Program;
        scrThread * m_Thread;

        std::map<uint64_t, uint8_t> m_Breakpoints;
        std::unique_ptr<uint64_t[]> m_Stack;
        uint32_t                    m_nStackSize;

        uint8_t m_OverrideNextOpcode = 0xFF;
        int32_t m_NumOpsUntilBreak   = -1;

        std::optional<eScriptState> m_NextScriptState;
        std::vector<std::pair<RainbomizerDebugServer::WebsocketUserData,
                              nlohmann::json>>
            m_NativeRequests;

        void UpdateBreakpoints (scrThreadContext *ctx);

        inline void
        AddBreakpoint (uint64_t offset)
        {
            m_Breakpoints[offset] = BREAKPOINT_OPCODE;
        }

        void RemoveBreakpoint (uint64_t offset);
        void BreakNow (scrThread *);

        void
        Break ()
        {
            m_OverrideNextOpcode = BREAKPOINT_OPCODE;
        }

        inline void
        Continue ()
        {
            m_eState = RUNNING;
        }
        void
        NextInstruction ()
        {
            m_eState           = RUNNING;
            m_NumOpsUntilBreak = 1;
        }

        inline uint8_t
        GetOverriddenOpcode (uint8_t defaultOpcode)
        {
            if (m_NumOpsUntilBreak >= 0 && m_NumOpsUntilBreak-- == 0)
                return BREAKPOINT_OPCODE;

            if (m_OverrideNextOpcode != 0xFF)
                return std::exchange (m_OverrideNextOpcode, 0xFF);

            return defaultOpcode;
        }

        CapturedThread (scrThread *thread);
        CapturedThread (const CapturedThread &) = delete;
    };

    inline static std::map<uint32_t, CapturedThread> m_Threads;
    inline static std::vector<uint32_t>              m_CaptureRequests;

    inline static bool m_bHookDisabled = false;

    inline static CapturedThread *m_CapturedThread = nullptr;

    static void SendThreadsList (uWS::HttpResponse<false> *res,
                                 uWS::HttpRequest *        req);

    static void SendThread (uWS::HttpResponse<false> *res,
                            uWS::HttpRequest *        req);

    static void SendThreadStack (uWS::HttpResponse<false> *res,
                                 uWS::HttpRequest *        req);

    static bool HandleWebSocketRequests (RainbomizerDebugServer::WebSocket *,
                                         nlohmann::json &req);

    static void CallNativeNow (nlohmann::json &                          req,
                               RainbomizerDebugServer::WebsocketUserData data);

    static uint32_t PerOpcodeHook (uint8_t *ip, uint64_t *SP, uint64_t *FSP);
    static void     InitialisePerOpcodeHook ();
    static void     HandleCaptureRequests ();

    template <auto &O>
    static eScriptState RunThreadHook (uint64_t *stack, uint64_t *globals,
                                       scrProgram *      program,
                                       scrThreadContext *ctx);

    static void HitBreakPoint (uint32_t, scrThread *thread);

public:
    ScriptDebugInterface ();

    static void Initialise (uWS::App &app);

    static void
    SetOpcodeHookDisabled (bool disabled)
    {
        m_bHookDisabled = disabled;
    }
};
