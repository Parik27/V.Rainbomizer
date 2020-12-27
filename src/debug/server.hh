#pragma once

#include <thread>
#include <string_view>
#include <queue>
#include <utility>
#include <vector>
#include <memory>

class RainbomizerDebugServer
{
public:
    using WebSocket = uWS::WebSocket<false, true>;
    using HandlerFunction
        = std::function<bool (WebSocket *, nlohmann::json &req)>;

    struct WebsocketUserData
    {
        std::shared_ptr<bool> bValid;
        WebSocket* pWebSocket;

        void
        Send (const nlohmann::json &j)
        {
            if (bValid && *bValid)
                {
                    pWebSocket->send (j.dump (), uWS::OpCode::TEXT);
                }
        }
    };

private:
    std::thread m_ServerThread;

    std::vector<HandlerFunction>                 m_Handlers;
    std::queue<std::function<void (uWS::App &)>> m_DeferredFunctions;

    void InitialiseServer ();
    void InitialiseWebSockets (uWS::App &app);

    template <typename... Interfaces> void InitialiseInterfaces (uWS::App &app);

    void        Process (uWS::App &app);
    static void HandleMessage (WebSocket *ws, std::string_view msg,
                               uWS::OpCode opcode);

    RainbomizerDebugServer ();
    RainbomizerDebugServer (RainbomizerDebugServer &other) = delete;

public:
    static RainbomizerDebugServer &
    Get ()
    {
        static RainbomizerDebugServer server;
        return server;
    }

    inline void
    Broadcast (std::string path, const nlohmann::json &j)
    {
        DeferFunction ([path, j] (uWS::App &app) {
            std::cout << "Broadcasting: " << path << ": " << j.dump() << std::endl;
            app.publish (path, j.dump (), uWS::OpCode::TEXT, true);
        });
    }

    inline void
    DeferFunction (std::function<void (uWS::App &)> function)
    {
        m_DeferredFunctions.push (function);
    }

    inline void
    RegisterHandler (HandlerFunction handler)
    {
        m_Handlers.push_back (handler);
    }
};

