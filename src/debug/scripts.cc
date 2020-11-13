#include "scripts.hh"
#include <CTheScripts.hh>
#include "server.hh"

/*******************************************************/
void
to_json (nlohmann::json &j, const scrThread &thread)
{
    j["Name"]  = std::string (thread.m_szScriptName);
    j["Hash"]  = thread.m_Context.m_nScriptHash;
    j["Id"]    = thread.m_Context.m_nThreadId;
    j["State"] = int (thread.m_Context.m_nState);
    j["nIP"]   = thread.m_Context.m_nIp;
}

/*******************************************************/
void
to_json (nlohmann::json &j, const scrProgram &program)
{
    scrProgram *sProgram = const_cast<scrProgram *> (&program);

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
bool
ScriptDebugInterface::HandleNativeCallReq (
    RainbomizerDebugServer::WebSocket *socket, nlohmann::json &req)
{
    std::cout << req.dump (4) << std::endl;
    if (req["Type"].get<std::string> () != "Request"
        || req["Topic"].get<std::string> () != "Scripts"
        || req["Data"].get<std::string> () != "InvokeNative")
        return false;

    std::string native = req["Native"];
    std::string response = "";
    
    uint32_t    nativeHash
        = rage::atLiteralStringHash (native.c_str (), native.size ());

    if (!NativeManager::DoesNativeExist(nativeHash))
        response = "Native not found - " + native;

    else
        {
            scrThread::Info info;
            for (const auto &i : req["NativeArgs"])
                {
                    switch (i["Type"].get<int> ())
                        {
                            // INT32
                        case 0:
                            info.PushArg<int> (i["Data"]);
                            break;

                            // FLOAT
                        case 1: info.PushArg<float> (i["Data"]);
                        }
                }

            NativeManager::InvokeNative (nativeHash, &info);

            if (req["ReturnType"].get<int> () == 0)
                response = std::to_string (info.GetReturn ());
            else
                response = std::to_string (info.GetReturn<float> ());
        }

    nlohmann::json res;
    res["Type"]     = "Response";
    res["Topic"]    = "Scripts";
    res["Data"]     = "InvokeNative";
    res["Response"] = response;

    socket->send(res.dump(), uWS::OpCode::TEXT);
    return true;
}

/*******************************************************/
void
ScriptDebugInterface::Initialise (uWS::App &app)
{
    app.get ("/get/threads", SendThreadsList);
    app.get ("/get/threads/:hash", SendThread);

    RainbomizerDebugServer::Get ().RegisterHandler (HandleNativeCallReq);
}
