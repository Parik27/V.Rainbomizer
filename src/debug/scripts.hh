#pragma once

#include "server.hh"

class ScriptDebugInterface
{
private:
    static void SendThreadsList (uWS::HttpResponse<false> *res,
                                 uWS::HttpRequest *        req);
    
    static void SendThread (uWS::HttpResponse<false> *res,
                            uWS::HttpRequest *        req);

    static bool HandleNativeCallReq (RainbomizerDebugServer::WebSocket *, nlohmann::json &req);

public:
    static void Initialise (uWS::App &app);
};
