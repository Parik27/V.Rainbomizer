#include "server.hh"
#include "html/dashboard.html"
#include "common/logger.hh"
#include "common/common.hh"

// Interfaces
#include "debug/logger.hh"
#include "debug/scripts.hh"

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

    if (j["Type"] == "Subscribe")
        ws->subscribe (j["Data"].get<std::string> ());
    else if (j["Type"] == "Unsubscribe")
        ws->unsubscribe (j["Data"].get<std::string> ());

    for (auto &i : Get ().m_Handlers)
        {
            try
                {
                    i (ws, j);
                }
            catch (std::out_of_range &e)
                {
                    ws->send (nlohmann::json ({{"Type", "Error"},
                                               {"Topic", "*"},
                                               {"Data", e.what ()}})
                                  .dump ());
                }
        }
}

/*******************************************************/
void
RainbomizerDebugServer::InitialiseWebSockets (uWS::App &app)
{
    uWS::App::WebSocketBehavior settings = {};
    settings.open                        = [] (auto *ws) {
        auto userData = static_cast<WebsocketUserData *> (ws->getUserData ());
        userData->bValid     = std::make_shared<bool> (true);
        userData->pWebSocket = ws;
    };
    settings.close = [] (auto *ws, int code, std::string_view message) {
        auto userData = static_cast<WebsocketUserData *> (ws->getUserData ());
        *userData->bValid = false;
    };
    settings.message = HandleMessage;

    app.ws<WebsocketUserData> ("/ws/*", std::move (settings));
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
    Rainbomizer::Logger::LogMessage("Failed to listen");
}

/*******************************************************/
RainbomizerDebugServer::RainbomizerDebugServer ()
{
    Rainbomizer::Common::AddInitCallback ([this] (bool test) {
        static bool initialised = false;
        if (!std::exchange (initialised, true))
            {
                m_ServerThread = std::thread (
                    std::bind (&RainbomizerDebugServer::InitialiseServer,
                               this));
            }
    });
}
