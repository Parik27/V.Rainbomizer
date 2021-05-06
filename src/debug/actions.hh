#pragma once

#include "server.hh"
#include <map>
#include <string>
#include <utility>

class ActionsDebugInterface
{
    struct ActionData
    {
        std::string Name;
        std::string Description;
        void (*Function) ();
    };

    /*******************************************************/
    static void
    ReloadPatterns ()
    {
        sm_ReloadPatternsRequested = true;
    }

    static void
    ReloadMissions ()
    {
        sm_ReloadMissionsRequested = true;
    }

    inline static std::map<std::string, ActionData> m_Funcs
        = {{"reload_patterns",
            {"Reload Vehicle Patterns", "Reload the VehiclePatterns.txt file",
             ReloadPatterns}},

           {"refresh_mission_order",
            {"Reload Missions",
             "Refreshes the in-game mission order after a "
             "change to the seed or forced mission",
             ReloadMissions}}};

public:
    inline static bool sm_ReloadPatternsRequested = false;
    inline static bool sm_ReloadMissionsRequested = false;

    /*******************************************************/
    static void
    SendActionsList (uWS::HttpResponse<false> *res, uWS::HttpRequest *req)
    {

        res->writeHeader ("Content-Type", "application/json");
        res->writeHeader ("Access-Control-Allow-Origin", "*");

        nlohmann::json j;
        for (auto &[key, data] : m_Funcs)
            j[key] = {{"name", data.Name}, {"desc", data.Description}};

        res->end (j.dump ());
    }

    /*******************************************************/
    static std::string
    CallAction (std::string_view actionStr)
    {
        try
            {
                m_Funcs.at (std::string (actionStr)).Function ();
            }
        catch (...)
            {
                return ":(";
            }
        return ":)";
    }

    /*******************************************************/
    static void
    HandleUpdateConfig (uWS::HttpResponse<false> *res, uWS::HttpRequest *req)
    {
        res->writeHeader ("Access-Control-Allow-Origin", "*");
        res->end (CallAction (req->getParameter (0)));
    }

    /*******************************************************/
    static void
    Initialise (uWS::App &app)
    {
        app.get ("/get/actions", SendActionsList);
        app.post ("/set/actions/:id", HandleUpdateConfig);
    }
};
