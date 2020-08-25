#include <cstdio>
#include <Utils.hh>
#include <cctype>
#include <common/logger.hh>
#include <exceptions/exceptions_Mgr.hh>
#include <common/config.hh>

struct pgRequestInitParams
{
    uint64_t field_0x0;
    void *   pData;
    uint64_t nSizeInMem;
};

using pgRequestCallback = void (*) (void *);
uint64_t (*pgStreamer_Request) (uint32_t, pgRequestInitParams *, uint32_t,
                                uint32_t, pgRequestCallback, void *, uint32_t,
                                uint32_t, uint64_t, uint32_t);

pgRequestCallback CB_sysIpcSignalSema;

/*******************************************************/
/* Text Case Randomizer - randomizes the case of each letter for all texts
   in-game. It's possible that it randomizes some important non-displayed data
   (such as audio related subtitles that are used to generate speech, but
   hopefully the game treats them as case-sensitive so we don't have to worry
   about them :P
*/
/*******************************************************/
class TextCaseRandomizer
{    
    // Used to store pointer to where the file is read in addition to the
    // original data, which is stored in pOriginalData.
    struct CallbackData
    {
        void *pOutData;
        void *pOriginalData;
        pgRequestCallback pOriginalCb;

        CallbackData (void *data, void *origData, pgRequestCallback cb)
            : pOutData (data), pOriginalData (origData), pOriginalCb (cb)
        {
        }
    };

    static inline struct
    {
        int Odds;
    } m_Config;
    
    /*******************************************************/
    static void
    RandomizeTextCase (void* data)
    {
        CallbackData* cbData = static_cast<CallbackData*> (data);
        
        // Check .gxt2 structure here - https://gtamods.com/wiki/.gxt2
        struct CTextFile_Data
        {
            char Signature[4];
            uint32_t NumKeys;
            struct KeyEntry
            {
                uint32_t KeyHash;
                uint32_t DataOffset;
            } Keys[1];
        };
        
        CTextFile_Data *file = static_cast<CTextFile_Data *> (cbData->pOutData);
        if (strncmp ("2TXG", file->Signature, 4) == 0)
            {
                for (int i = 0; i < file->NumKeys; i++)
                    {
                        char *Str = reinterpret_cast<char *> (file)
                                    + file->Keys[i].DataOffset;

                        for (int j = 0; Str[j] != 0; j++)
                            {
                                char &c = Str[j];
                                if ((c >= 'a' && c <= 'z')
                                    || (c >= 'A' && c <= 'Z'))
                                    {
                                        bool upperCase = RandomInt (1);
                                        c = (upperCase) ? std::toupper (c)
                                                        : std::tolower (c);
                                    }
                            }
                    }
            }

        cbData->pOriginalCb (cbData->pOriginalData); // call original cb
        delete cbData;
    }
    
    /*******************************************************/
    static uint64_t
    TextOnRequestAsyncHook (uint32_t collectionId, pgRequestInitParams *request,
                            uint32_t numBuffers, uint32_t p4,
                            pgRequestCallback Cb, void *CbData, uint32_t p7,
                            uint32_t p8, uint64_t p9, uint32_t p10)
    {
        Rainbomizer::ExceptionHandlerMgr::GetInstance().Init();
        
        // Called by the game to request the gxt2 file to be read.
        // Cb (CbData) is called after the request is finished. If the streamer
        // has too many requests, it returns 0. So we make sure we don't leak
        // memory by freeing the temporary callback data pointer.

        CallbackData* data = new CallbackData(request->pData, CbData, Cb);

        uint64_t id = pgStreamer_Request (collectionId, request, numBuffers, p4,
                                          RandomizeTextCase, data, p7, p8, p9, p10);
        
        // Free data in case the call failed. If it didn't, it'll be freed in the callback.
        if (id == 0)
            delete data;

        return id;
    }

    /*******************************************************/
    static uint64_t
    TextOnRequestSyncHook (uint32_t collectionId, pgRequestInitParams *request,
                           uint32_t numBuffers, uint32_t p4, void *Sema,
                           uint32_t p8, uint32_t p10)
    {
        return TextOnRequestAsyncHook (collectionId, request, numBuffers, p4,
                                       CB_sysIpcSignalSema, Sema, 1, p8, 0,
                                       p10);
    }

public:
    TextCaseRandomizer ()
    {
        m_Config.Odds = 100;

        if (!ConfigManager::ReadConfig ("TextCaseRandomizer",
                                        std::pair ("Odds", &m_Config.Odds)))
            return;

        InitialiseAllComponents ();

        if (RandomInt (m_Config.Odds) != 0)
            return;
        
        CB_sysIpcSignalSema = (pgRequestCallback) GetRelativeReference (
            "? 89 44 ? ? ? 8d 05 ? ? ? ? ? 89 ? ? ? e8 ? ? ? ? ? 83 c4 58", 8,
            12);

        RegisterHook ("? 89 5c ? ? ? 89 ? ? ? e8 ? ? ? ? ? 85 c0 74 ?", 10,
                      pgStreamer_Request, TextOnRequestAsyncHook);

        auto pattern
            = hook::pattern ("? 8b c6 8b cf e8 ? ? ? ? ? 85 c0 74 ?").count (2);

        pattern.for_each_result ([] (hook::pattern_match m) {
            RegisterHook (m.get<void> (5), TextOnRequestSyncHook);
        });
    }
} _texts;
