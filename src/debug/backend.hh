#pragma once

#include <functional>
#include <vector>

class BackendWalkieTalkie
{
public:
    inline static std::vector<std::function<void ()>> sm_Functions;
};
