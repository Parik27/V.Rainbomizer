#include "config.hh"
#include <string>
#include <variant>
#include <charconv>

/*******************************************************/
void
ConfigDebugInterface::Initialise (uWS::App &app)
{
    app.get ("/get/configs", SendConfigOptions);
    app.post ("/set/configs", HandleUpdateConfig);
}

/*******************************************************/
template <typename T>
void
ConvertFromChars (std::string a, T &out)
{
    if constexpr (std::is_same_v<T, bool>)
        out = a == "true";
    else if constexpr (std::is_same_v<T, int>)
        out = std::stoi (a);
    else
        out = T (a);
}

/*******************************************************/
std::string
ConfigDebugInterface::UpdateOption (const nlohmann::json &j)
{
    try
        {
            std::visit (
                [&j] (auto &&arg) { ConvertFromChars (j.at ("data"), *arg); },
                sm_ConfigOptions.at (j.at ("key")));
        }
    catch (std::exception &e)
        {
            return e.what ();
        }

    return ":)";
}

/*******************************************************/
void
ConfigDebugInterface::HandleUpdateConfig (uWS::HttpResponse<false> *res,
                                          uWS::HttpRequest *        req)
{
    res->onData ([res, buffer = std::string ("")] (std::string_view data,
                                                   bool last) mutable {
        buffer.append (data.data (), data.length ());

        if (last)
            {
                res->writeHeader ("Access-Control-Allow-Origin", "*");
                res->end (
                    UpdateOption (nlohmann::json::parse (buffer)).c_str ());
            }
    });
    res->onAborted ([] () {});
}

/*******************************************************/
void
ConfigDebugInterface::SendConfigOptions (uWS::HttpResponse<false> *res,
                                         uWS::HttpRequest *        req)
{
    res->writeHeader ("Content-Type", "application/json");
    res->writeHeader ("Access-Control-Allow-Origin", "*");

    nlohmann::json j;
    for (auto &[key, data] : sm_ConfigOptions)
        std::visit ([&, key = key] (auto &&arg) { j[key] = *arg; }, data);

    res->end (j.dump ());
}
