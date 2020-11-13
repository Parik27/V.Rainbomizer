#include "server.hh"
#include "html/dashboard.html"
#include "common/logger.hh"

// Interfaces
#include "debug/logger.hh"
#include "debug/scripts.hh"

struct UserData
{
};

/*******************************************************/
void
RainbomizerDebugServer::Process (uWS::App &app)
{
    constexpr int MAX_MESSAGES = 8;

    int i = MAX_MESSAGES;
    while (i-- && m_DeferredFunctions.size ())
        {
            auto &func = m_DeferredFunctions.front ();
            func (app);

            m_DeferredFunctions.pop ();
        }
}

/*******************************************************/
void
RainbomizerDebugServer::HandleMessage (WebSocket *ws, std::string_view msg,
                                       uWS::OpCode opcode)
{
    nlohmann::json j;
    try
        {
            j = nlohmann::json::parse(msg);
        }
    catch (...)
        {
            return;
        }

    for (auto &i : Get ().m_Handlers)
        i (ws, j);
}

/*******************************************************/
void
RainbomizerDebugServer::InitialiseWebSockets (uWS::App &app)
{
    uWS::App::WebSocketBehavior settings = {};
    settings.open    = [] (auto *ws) { ws->subscribe ("logs"); };
    settings.message = HandleMessage;

    app.ws<UserData> ("/ws/*", std::move (settings));
}

/*******************************************************/
template <typename... Interface>
void
RainbomizerDebugServer::InitialiseInterfaces (uWS::App &app)
{
    (..., Interface::Initialise (app));
}

/*******************************************************/
void
RainbomizerDebugServer::InitialiseServer ()
{
    auto app = uWS::App ().get ("/", [] (uWS::HttpResponse<false> *res,
                                         uWS::HttpRequest *        req) {
        res->writeHeader ("Content-Type", "text/html; charset=utf-8");
        res->write (RainbomizerDebugServerHtml::Dashboard);
        res->end ();
    });

    InitialiseWebSockets (app);

    app.listen (12327, [] (auto *listenSocket) {
        if (listenSocket)
            {
                Rainbomizer::Logger::LogMessage (
                    "RainbomizerDebugServer: listening on port "
                    "12327: http://localhost:12327");
            }
    });

    InitialiseInterfaces<LoggerDebugInterface, ScriptDebugInterface> (app);

    uWS::Loop::get ()->addPreHandler (
        (void *) 10,
        std::bind (&RainbomizerDebugServer::Process, this, std::ref (app)));

    app.run ();
}

/*******************************************************/
RainbomizerDebugServer::RainbomizerDebugServer ()
{
    m_ServerThread = std::thread (
        std::bind (&RainbomizerDebugServer::InitialiseServer, this));
}
