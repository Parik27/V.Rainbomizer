#pragma once

#include "rage.hh"
#include "server.hh"
#include <deque>

class LoggerDebugInterface
{
    static constexpr int      MAX_LOG_HISTORY = 20;
    inline static std::string m_History;

public:
    /*******************************************************/
    static void
    PublishLogMessage (std::string_view msg)
    {
        nlohmann::json j;
        j["Type"]  = "Update";
        j["Topic"] = "Log";
        j["Data"]  = std::string (msg) + '\n';

        RainbomizerDebugServer::Get ().Broadcast ("logs", j);
        m_History += j["Data"];

        while (m_History.size () > 20'000)
            m_History = m_History.substr (m_History.find ('\n') + 1);
    }

    /*******************************************************/
    static void
    SendLogHistory (uWS::HttpResponse<false> *res)
    {
        res->writeHeader ("Content-Type", "text/plain");
        res->writeHeader ("Access-Control-Allow-Origin", "*");
        res->end (m_History);
    }

    /*******************************************************/
    static void
    Initialise (uWS::App &app)
    {
        app.get ("/get/logs/",
                 [] (uWS::HttpResponse<false> *res, uWS::HttpRequest *req) {
                     SendLogHistory (res);
                 });
    }
};
