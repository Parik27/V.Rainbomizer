#pragma once

#include "common/logger.hh"
#include <functional>
#include <type_traits>
#include <vector>

struct scrProgram;
class scrThreadContext;

namespace Rainbomizer {

template <typename Ret = void, typename... Args> class Event
{
    using FunctionType = std::function<Ret (Args...)>;
    std::vector<FunctionType> m_Callbacks;

    Ret
    operator() (Args... args)
    {
        if constexpr (std::is_same_v<Ret, void>)
            {
                for (auto i : m_Callbacks)
                    i (args...);
            }
        else
            {
                Ret ret;
                for (auto i : m_Callbacks)
                    ret = i (args...);

                return ret;
            }
    }

public:
    Event () = default;

    Event (const Event &other) = delete;

    void
    operator+= (FunctionType f)
    {
        m_Callbacks.push_back (f);
    }

    friend class EventTriggerer;
};

struct EventsList
{
    // Init -> bool session
    // Mission Start -> int missionHash, bool replaying
    // Mission End
    // Fade

    Event<void, bool>           OnInit;
    Event<void, uint32_t, bool> OnMissionStart;
    Event<>                     OnMissionEnd;
    Event<>                     OnFade;
    Event<void, uint32_t>       OnScriptStart;

    Event<void, uint64_t *, uint64_t *, scrProgram *, scrThreadContext *>
        OnRunThread;
};

inline EventsList &
Events ()
{
    static EventsList list;
    return list;
}
}; // namespace Rainbomizer
