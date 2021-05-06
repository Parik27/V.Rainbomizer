#pragma once

#include <string>
#include <variant>
#include <string_view>
#include <map>

class ConfigDebugInterface
{
public:
    using Type = std::variant<int *, std::string *, bool *>;

private:
    inline static std::map<std::string, Type> sm_ConfigOptions;

    static std::string UpdateOption (const nlohmann::json &request);

    static void HandleUpdateConfig (uWS::HttpResponse<false> *res,
                                    uWS::HttpRequest *        req);

    static void SendConfigOptions (uWS::HttpResponse<false> *res,
                                   uWS::HttpRequest *        req);

public:
    static void Initialise (uWS::App &app);

    template <typename T>
    static void
    AddConfigOption (std::string_view path, T *data)
    {
        sm_ConfigOptions[std::string (path)] = data;
    }
};
