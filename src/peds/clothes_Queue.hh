#pragma once

#include <vector>
#include <mutex>
#include <cstdint>

#include <CTheScripts.hh>
#include <CPed.hh>

class ClothesRandomizer_Queue
{
    inline static std::vector<uint32_t> mData;
    inline static std::mutex            mMutex;

public:
    /*******************************************************/
    static void
    Add (CPed *ped)
    {
        Add (fwScriptGuid::CreateGuid (ped));
    }

    /*******************************************************/
    static void
    Add (uint32_t idx)
    {
        std::lock_guard guard (mMutex);
        for (auto i : mData)
            {
                if (i == idx)
                    return;
            }
        
        mData.push_back (idx);
    }

    friend class ClothesRandomizer;
};
