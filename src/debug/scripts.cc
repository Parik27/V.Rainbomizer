#include "scripts.hh"
#include <CTheScripts.hh>
#include "server.hh"
#include "Utils.hh"
#include <fmt/core.h>
#include "mission/missions_YscUtils.hh"
#include "common/logger.hh"

/*******************************************************/
void
to_json (nlohmann::json &j, const scrThread &thread)
{
    j["Name"]  = std::string (thread.m_szScriptName);
    j["Hash"]  = thread.m_Context.m_nScriptHash;
    j["Id"]    = thread.m_Context.m_nThreadId;
    j["State"] = int (thread.m_Context.m_nState);
    j["IP"]    = thread.m_Context.m_nIp;
    j["SP"]    = thread.m_Context.m_nSP;
}

/*******************************************************/
void
to_json (nlohmann::json &j, const scrProgram &program)
{
    j["ParameterCount"] = program.m_nParameterCount;
    j["CodeSize"]       = program.m_nCodeSize;
    j["StaticCount"]    = program.m_nStaticCount;
}

/*******************************************************/
void
ScriptDebugInterface::SendThreadsList (uWS::HttpResponse<false> *res,
                                       uWS::HttpRequest *        req)
{
    res->writeHeader ("Content-Type", "application/json");
    nlohmann::json threads = {};

    if (CTheScripts::aThreads)
        {
            for (auto i : *CTheScripts::aThreads)
                {
                    if (i->m_Context.m_nThreadId == 0
                        || i->m_Context.m_nState == eScriptState::KILLED)
                        continue;

                    threads.push_back (*i);
                }
        }

    res->end (threads.dump ());
}

/*******************************************************/
void
ScriptDebugInterface::SendThread (uWS::HttpResponse<false> *res,
                                  uWS::HttpRequest *        req)
{
    res->writeHeader ("Content-Type", "application/json");

    int        threadId = std::stoi (std::string (req->getParameter (0)));
    scrThread *thread   = nullptr;

    for (auto i : *CTheScripts::aThreads)
        {
            if (i->m_Context.m_nThreadId == 0
                || i->m_Context.m_nState == eScriptState::KILLED)
                continue;

            if (i->m_Context.m_nThreadId == threadId)
                {
                    thread = i;
                    break;
                }
        }

    if (!thread)
        {
            return res->writeStatus ("404 Not Found")->end ();
        }

    nlohmann::json j       = *thread;
    scrProgram *   program = thread->GetProgram ();

    j["Stack"]
        = std::vector<uint64_t> (thread->m_pStack,
                                 &thread->m_pStack[thread->m_Context.m_nSP]);
    j["Program"] = *program;
    res->end (j.dump ());
}

/*******************************************************/
void
ScriptDebugInterface::SendThreadStack (uWS::HttpResponse<false> *res,
                                       uWS::HttpRequest *        req)
{
    int threadId = std::stoi (std::string (req->getParameter (0)));

    if (auto captThread = LookupMap (m_Threads, threadId))
        {
            res->write (std::string_view ((char *) captThread->m_Stack.get (),
                                          captThread->m_nStackSize));
        }

    return res->writeStatus ("404 Not Found")->end ();
}

/*******************************************************/
bool
ScriptDebugInterface::HandleWebSocketRequests (
    RainbomizerDebugServer::WebSocket *socket, nlohmann::json &req)
{
    if (req.at ("Type").get<std::string> () != "Request"
        || req.at ("Topic").get<std::string> () != "Scripts")
        return false;

    uint32_t DataHash
        = rage::atStringHash (req.at ("Data").get<std::string> ());

    uint32_t threadId = 0;
    if (req.count ("CapturedThread"))
        threadId = req.at ("CapturedThread");

    auto userData = static_cast<RainbomizerDebugServer::WebsocketUserData *> (
        socket->getUserData ());

    switch (DataHash)
        {
            case "invokenative"_joaat: {
                if (threadId == 0)
                    CallNativeNow (req, *userData);

                else if (auto captThread = LookupMap (m_Threads, threadId))
                    captThread->m_NativeRequests.push_back ({*userData, req});

                break;
            }

            case "set_global"_joaat: {
                uint32_t globalIndex = req.at ("Index");
                if (req.at ("Value").at ("Type") == "float")
                    scrThread::GetGlobal<float> (globalIndex)
                        = req.at ("Value").at ("Data").get<float> ();
                else
                    scrThread::GetGlobal (globalIndex)
                        = req.at ("Value").at ("Data");

                [[fallthrough]];
            }
            case "global"_joaat: {

                uint32_t globalIndex = req.at ("Index");

                std::string globalStr
                    = fmt::format ("Global {} = {}/{}", globalIndex,
                                   scrThread::GetGlobal (globalIndex),
                                   scrThread::GetGlobal<float> (globalIndex));

                userData->Send ({{"Type", "Response"},
                                 {"Topic", "Scripts"},
                                 {"Data", "Global"},
                                 {"Response", globalStr}});
                break;
            }

            case "set_static"_joaat: {
                uint32_t index = req.at ("Index");
                if (auto captThread = LookupMap (m_Threads, threadId))
                    {
                        auto thread = captThread->m_Thread;
                        if (req.at ("Value").at ("Type") == "float")
                            thread->GetStaticVariable<float> (index)
                                = req.at ("Value").at ("Data").get<float> ();
                        else
                            thread->GetStaticVariable (index)
                                = req.at ("Value").at ("Data");
                    }
                [[fallthrough]];
            }
            case "static"_joaat: {
                uint32_t localIndex = req.at ("Index");

                if (auto captThread = LookupMap (m_Threads, threadId))
                    {
                        auto thread = captThread->m_Thread;

                        std::string str = fmt::format (
                            "Static {} = {}/{}", localIndex,
                            thread->GetStaticVariable (localIndex),
                            thread->GetStaticVariable<float> (localIndex));

                        userData->Send ({{"Type", "Response"},
                                         {"Topic", "Scripts"},
                                         {"Data", "Static"},
                                         {"Response", str}});
                    }

                break;
            }

            case "set_local"_joaat: {
                uint32_t index = req.at ("Index");
                if (auto captThread = LookupMap (m_Threads, threadId))
                    {
                        auto thread = captThread->m_Thread;
                        if (req.at ("Value").at ("Type") == "float")
                            thread->GetLocalVariable<float> (index)
                                = req.at ("Value").at ("Data").get<float> ();
                        else
                            thread->GetLocalVariable (index)
                                = req.at ("Value").at ("Data");
                    }
                [[fallthrough]];
            }
            case "local"_joaat: {
                uint32_t localIndex = req.at ("Index");

                if (auto captThread = LookupMap (m_Threads, threadId))
                    {
                        auto thread = captThread->m_Thread;

                        std::string str = fmt::format (
                            "Local {} = {}/{}", localIndex,
                            thread->GetLocalVariable (localIndex),
                            thread->GetLocalVariable<float> (localIndex));

                        userData->Send ({{"Type", "Response"},
                                         {"Topic", "Scripts"},
                                         {"Data", "Local"},
                                         {"Response", str}});
                    }
                break;
            }

            case "capturethread"_joaat: {
                uint32_t hash = rage::atStringHash (
                    req.at ("ThreadName").get<std::string> ());

                m_CaptureRequests.push_back (hash);
                socket->subscribe (fmt::format ("scripts/capture/{:x}", hash));
                break;
            }
            case "setbreakpoint"_joaat: {
                if (auto captThread = LookupMap (m_Threads, threadId))
                    captThread->AddBreakpoint (req.at ("Offset"));
                break;
            }
            case "removebreakpoint"_joaat: {
                if (auto captThread = LookupMap (m_Threads, threadId))
                    captThread->RemoveBreakpoint (req.at ("Offset"));
                break;
            }
            case "nextinstruction"_joaat: {
                if (auto captThread = LookupMap (m_Threads, threadId))
                    captThread->NextInstruction ();
                break;
            }
            case "continue"_joaat: {
                if (auto captThread = LookupMap (m_Threads, threadId))
                    captThread->Continue ();
                break;
            }
            case "break"_joaat: {
                if (auto captThread = LookupMap (m_Threads, threadId))
                    captThread->Break ();
                break;
            }
        }

    return true;
}

/*******************************************************/
void
ScriptDebugInterface::CallNativeNow (
    nlohmann::json &req, RainbomizerDebugServer::WebsocketUserData data)
{
    std::string native   = req.at ("Native");
    std::string response = "";

    uint32_t nativeHash = rage::atLiteralStringHash (native);

    if (!NativeManager::DoesNativeExist (nativeHash))
        response = "Native not found - " + native;

    else
        {
            scrThread::Info info{};
            for (const auto &i : req.at ("NativeArgs"))
                {
                    switch (i.at ("Type").get<int> ())
                        {
                            // INT32
                        case 0:
                            info.PushArg<int> (i.at ("Data"));
                            break;

                            // FLOAT
                        case 1: info.PushArg<float> (i.at ("Data"));
                        }
                }

            NativeManager::InvokeNative (nativeHash, &info);

            if (req.at ("ReturnType").get<int> () == 0)
                response = std::to_string (info.GetReturn ());
            else
                response = std::to_string (info.GetReturn<float> ());
        }

    data.Send ({{"Type", "Response"},
                {"Topic", "Scripts"},
                {"Data", "InvokeNative"},
                {"Response", response}});
}

/*******************************************************/
void
ScriptDebugInterface::Initialise (uWS::App &app)
{
    app.get ("/get/threads", SendThreadsList);
    app.get ("/get/threads/:hash", SendThread);
    app.get ("/get/threads/:hash/stack", SendThreadStack);

    RainbomizerDebugServer::Get ().RegisterHandler (HandleWebSocketRequests);
}

/*******************************************************/
void
ScriptDebugInterface::CapturedThread::UpdateBreakpoints (scrThreadContext *ctx)
{
    if (!m_Program)
        return;

    for (auto &[offset, value] : m_Breakpoints)
        {
            if (value == CapturedThread::BREAKPOINT_OPCODE)
                {
                    uint8_t &code = m_Program->GetCodeByte<uint8_t> (offset);
                    value         = std::exchange (code, BREAKPOINT_OPCODE);
                }
        }
}

/*******************************************************/
void
ScriptDebugInterface::CapturedThread::RemoveBreakpoint (uint64_t offset)
{
    if (auto breakPoint = LookupMap (m_Breakpoints, offset))
        {
            if (!m_Program)
                return;

            m_Program->GetCodeByte<uint8_t> (offset) = *breakPoint;
        }
}

/*******************************************************/
void
ScriptDebugInterface::CapturedThread::BreakNow (scrThread *thread)
{
    nlohmann::json j;
    j["Type"]   = "Update";
    j["Topic"]  = "Scripts";
    j["Data"]   = "ThreadBreak";
    j["Thread"] = *thread;

    for (uint32_t i = 0; i < thread->m_Context.m_nStackSize / 8; i++)
        {
            if (m_Stack[i] != thread->m_pStack[i])
                {
                    j["Stack"].push_back ({i, thread->m_pStack[i]});
                    m_Stack[i] = thread->m_pStack[i];
                }
        }

    RainbomizerDebugServer::Get ().Broadcast (
        fmt::format ("scripts/threads/{}", thread->m_Context.m_nThreadId), j);

    if (auto opcode = LookupMap (m_Breakpoints, thread->m_Context.m_nIp))
        m_OverrideNextOpcode = *opcode;

    m_eState          = CapturedThread::STOPPED;
    m_NextScriptState = thread->m_Context.m_nState;
}

/*******************************************************/
ScriptDebugInterface::CapturedThread::CapturedThread (scrThread *thread)
{
    m_Thread = thread;

    m_Stack = std::make_unique<uint64_t[]> (thread->m_Context.m_nStackSize / 8);
    memcpy (m_Stack.get (), thread->m_pStack, thread->m_Context.m_nStackSize);
    m_nStackSize = thread->m_Context.m_nStackSize;

    nlohmann::json j;
    j["Type"]   = "Update";
    j["Topic"]  = "Scripts";
    j["Data"]   = "ScriptCaptured";
    j["Thread"] = *thread;

    RainbomizerDebugServer::Get ().Broadcast (
        fmt::format ("scripts/capture/{:x}", thread->m_Context.m_nScriptHash),
        j);
}

/*******************************************************/
void
ScriptDebugInterface::HandleCaptureRequests ()
{
    scrThread *thread = scrThread::GetActiveThread ();
    for (auto it = m_CaptureRequests.begin (); it != m_CaptureRequests.end ();)
        {
            if (thread->m_Context.m_nScriptHash == *it)
                {
                    it = m_CaptureRequests.erase (it);
                    m_Threads.emplace (thread->m_Context.m_nThreadId, thread);
                    continue;
                }

            ++it;
        }
}

/*******************************************************/
template <auto &O>
eScriptState
ScriptDebugInterface::RunThreadHook (uint64_t *stack, uint64_t *globals,
                                     scrProgram *program, scrThreadContext *ctx)
{
    eScriptState state = eScriptState::WAITING;

    HandleCaptureRequests ();

    if ((m_CapturedThread = LookupMap (m_Threads, ctx->m_nThreadId)))
        {
            m_CapturedThread->m_Program = program;
            m_CapturedThread->m_Thread  = scrThread::GetActiveThread ();

            m_CapturedThread->UpdateBreakpoints (ctx);
            for (auto &[wsData, js] : m_CapturedThread->m_NativeRequests)
                CallNativeNow (js, wsData);
            m_CapturedThread->m_NativeRequests.clear ();

            if (m_CapturedThread->m_eState != CapturedThread::STOPPED)
                state = O (stack, globals, program, ctx);

            if (m_CapturedThread->m_NextScriptState)
                {
                    ctx->m_nState = *m_CapturedThread->m_NextScriptState;
                    m_CapturedThread->m_NextScriptState = std::nullopt;

                    state = ctx->m_nState;
                }
        }
    else
        state = O (stack, globals, program, ctx);

    return state;
}

/*******************************************************/
void
ScriptDebugInterface::HitBreakPoint (uint32_t, scrThread *thread)
{
    thread->m_Context.m_nIp--;
    m_Threads.at (thread->m_Context.m_nThreadId).BreakNow (thread);
}

/*******************************************************/
uint32_t
ScriptDebugInterface::PerOpcodeHook (uint8_t *ip, uint64_t *SP, uint64_t *FSP)
{
    if (m_CapturedThread)
        {
            auto &context      = m_CapturedThread->m_Thread->m_Context;
            auto  stack        = m_CapturedThread->m_Thread->m_pStack;
            context.m_nSP      = uint64_t (SP - stack) + 1;
            context.m_nFrameSP = uint64_t (FSP - stack);

            return m_CapturedThread->GetOverriddenOpcode (*ip);
        }
    return *ip;
}

/*******************************************************/
void
ScriptDebugInterface::InitialisePerOpcodeHook ()
{
    const int CALL_OFFSET       = 15;
    const int JMP_OFFSET        = 24;
    const int ORIG_INSTRUCTIONS = 6;

    unsigned char Instructions[] = {
        0x48, 0xff, 0xc7,             // INC     RDI
        0x0f, 0xb6, 0x07,             // MOVZX   EAX,byte ptr [RDI]
        0x48, 0x89, 0xf9,             // MOV     RCX,RDI
        0x48, 0x89, 0xda,             // MOV     RDX,RBX
        0x4d, 0x89, 0xd8,             // MOV     R8, R11
        0xe8, 0x00, 0x00, 0x00, 0x00, // CALL    0x0
        0x4c, 0x8b, 0x45, 0x6f,       // MOV     R8, qword ptr [RBP + 0x10]
        0x4c, 0x8b, 0x4d, 0x67, 0x4c, 0x8b, 0x5d,
        0x7f, 0xe9, 0x00, 0x00, 0x00, 0x00 // JMP     0x0
    };

    void *addr       = hook::get_pattern ("48 ff c7 0f b6 07 83 f8 7e 0f 87");
    auto  trampoline = Trampoline::MakeTrampoline (GetModuleHandle (nullptr))
                          ->Pointer<decltype (Instructions)> ();

    memcpy (trampoline, Instructions, sizeof (Instructions));
    memcpy (trampoline, addr, ORIG_INSTRUCTIONS);

    injector::MakeNOP (addr, ORIG_INSTRUCTIONS);
    injector::MakeJMP (addr, trampoline);
    injector::MakeJMP (&trampoline[0][JMP_OFFSET], (uint8_t *) addr + 5);

    RegisterHook (&trampoline[0][CALL_OFFSET], PerOpcodeHook);

    printf ("Address: %p\nTrampoline: %p", addr, trampoline);
}

/*******************************************************/
ScriptDebugInterface::ScriptDebugInterface ()
{

    RegisterHook ("2b fe 89 7a 14 33 c9 e8", 7, HitBreakPoint);

    InitialiseAllComponents ();

    REGISTER_HOOK ("8d 15 ? ? ? ? ? 8b c0 e8 ? ? ? ? ? 85 ff ? 89 1d", 9,
                   RunThreadHook, eScriptState, uint64_t *, uint64_t *,
                   scrProgram *, scrThreadContext *);

    InitialisePerOpcodeHook ();
}

ScriptDebugInterface scrDebIntf;
