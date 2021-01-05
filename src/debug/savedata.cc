/* A file to generate C source code for the GTA 5 save data */

#include <Utils.hh>
#include <CTheScripts.hh>
#include <variant>
#include <fstream>
#include <fmt/core.h>
#include <scrThread.hh>

class SaveDataStructGenerator
{
    enum eSaveType
    {
        BOOL,
        INT,
        FLOAT,
        ENUM,
        TEXT_LABEL,
        STRUCT
    };

    struct SaveDataStruct;

    struct StructListReference
    {
        bool   bTemporary = true;
        size_t nIndex     = 0;

        bool
        operator== (const StructListReference &other) const
        {
            return bTemporary == other.bTemporary && other.nIndex == nIndex;
        }
    };

    struct SaveDataField
    {
        std::string         sFieldName   = "";
        uint64_t            nFieldOffset = 0;
        eSaveType           eSaveType    = BOOL;
        StructListReference pStruct;

        bool
        operator== (const SaveDataField &other) const
        {
            return other.sFieldName == sFieldName
                   && other.nFieldOffset == nFieldOffset
                   && other.eSaveType == eSaveType && other.pStruct == pStruct;
        }
    };

    struct SaveDataStruct
    {
        uint64_t nSize  = 0;
        bool     bArray = false;

        std::vector<SaveDataField> aFields;

        bool
        operator== (const SaveDataStruct &other) const
        {
            return other.nSize == nSize && other.bArray == bArray
                   && other.aFields == aFields;
        }
    };

    struct StructStackEntry
    {
        SaveDataStruct sdStruct;
        uint64_t       nOffset;
    };

    inline static std::vector<SaveDataStruct>                      mStructList;
    inline static std::vector<StructStackEntry>                    mStructStack;

    static uint64_t
    GetFieldSize (const SaveDataField &field)
    {
        if (field.eSaveType == STRUCT) {
            return mStructList[field.pStruct.nIndex].nSize;
        }

        return 1;
    }

    static std::string
    GetFieldType (const SaveDataField& field)
    {
        static const std::map<eSaveType, std::string> fieldNames = {
            {BOOL, "bool"},
            {INT, "int"},
            {FLOAT, "float"},
            {ENUM, "enum"},
            {TEXT_LABEL, "string"},
            {STRUCT, "struct_"}
        };

        std::string typeStr = fieldNames.at (field.eSaveType);
        if (field.eSaveType == STRUCT)
            typeStr += std::to_string (field.pStruct.nIndex);

        return typeStr;
    }

    static void
    PrintStruct (SaveDataStruct &st, std::ofstream &out, std::string name = "")
    {        
        if (st.bArray)
            out << "/*Array*/\n";
        out << "/*Size: " << st.nSize << " */\n";

        if (name == "")
            out << "struct struct_" << (&st - mStructList.data ());
        else
            out << "struct " << name;

        out << "\n{\n";

        std::sort (std::begin (st.aFields), std::end (st.aFields),
                   [] (const SaveDataField &a, const SaveDataField &b) {
                       return a.nFieldOffset < b.nFieldOffset;
                   });

        

        #define PRINT_FIELDS
        
        #ifdef PRINT_FIELDS
        for (uint64_t i = 0, field = 0; i < st.nSize;)
            {
                if (field < st.aFields.size ()
                    && st.aFields[field].nFieldOffset == i * 8)
                    {
                        const auto &f = st.aFields[field];
                        out << "\t" << GetFieldType (f) << " " << f.sFieldName
                            << ";\n";

                        field++;
                        i += GetFieldSize (f);
                    }
                else
                    {
                        out << "\t"
                            << "int field_" << i << ";\n";
                        i++;
                    }
            }
#else
        for (const auto &field : st.aFields)
            {
                out << "\t" << GetFieldType (field) << " " << field.sFieldName << ";";
                out << " // Size: " << GetFieldSize (field)
                    << ", Offset: " << field.nFieldOffset << "\n";
            }
#endif

        out << "};\n";
    }

    template <bool stop>
    static void
    SaveDataHook (scrThread::Info *info)
    {
        if constexpr (!stop)
            {
                mStructList.clear ();
                mStructStack.clear ();

                uint64_t offset = info->GetArg (0);
                uint64_t size   = info->GetArg<uint32_t> (1);

                mStructStack.push_back ({{size, false}, offset});
            }
        else
            {
                std::ofstream f ("save_data.txt", std::ofstream::app);

                while (mStructStack.size () > 1)
                    StopSaveStructHook ();

                for (auto &str : mStructList)
                    PrintStruct (str, f);

                PrintStruct (
                    mStructStack[0].sdStruct, f,
                    fmt::format ("{}_{:x}",
                                 scrThread::GetActiveThread ()->m_szScriptName,
                                 mStructStack[0].nOffset));
            }
    }

    static StructListReference
    AddNewStruct (SaveDataStruct &src)
    {
        for (size_t i = 0; i < mStructList.size (); ++i)
            {
                if (src == mStructList[i])
                    return {false, i};
            }

        mStructList.push_back (src);
        return {false, mStructList.size () - 1};
    }

    static StructStackEntry &
    GetParentStructAndOffset (uint64_t offset)
    {
        for (;;)
            {
                auto &[parentStruct, parentOffset] = mStructStack.back ();

                if (offset >= (parentStruct.nSize * 8) + parentOffset
                    || offset < parentOffset)
                    StopSaveStructHook ();
                else
                    break;
            }

        return mStructStack.back ();
    }

    template <bool array>
    static void
    StartSaveStructHook (scrThread::Info *info)
    {
        uint64_t offset = info->GetArg (0);
        uint64_t size   = info->GetArg<uint32_t> (1);
        char *   name   = info->GetArg<char *> (2);

        auto &[parentStruct, parentOffset] = GetParentStructAndOffset (offset);

        mStructStack.push_back (
            StructStackEntry{SaveDataStruct{size, array}, offset});
        parentStruct.aFields.push_back (
            SaveDataField{name, offset - parentOffset, STRUCT,
                          StructListReference{true, mStructStack.size () - 1}});
    }

    static void
    StopSaveStructHook ()
    {
        SaveDataStruct &theStruct = mStructStack.back ().sdStruct;
        SaveDataStruct &parentStruct
            = mStructStack[mStructStack.size () - 2].sdStruct;
        StructListReference dstStruct = AddNewStruct (theStruct);

        for (SaveDataField &field : parentStruct.aFields)
            {
                if (field.pStruct.bTemporary
                    && field.pStruct.nIndex == mStructStack.size () - 1)
                    field.pStruct = dstStruct;
            }

        mStructStack.pop_back ();
    }

    template <eSaveType type>
    static void
    RegisterTypeToSaveHook (scrThread::Info *info)
    {
        uint64_t offset = info->GetArg (0);
        char *   name   = info->GetArg<char *> (1);
        
        auto &[parentStruct, parentOffset] = GetParentStructAndOffset (offset);
        
        parentStruct.aFields.push_back (
            SaveDataField{name, offset - parentOffset, type});
    }

public:
    SaveDataStructGenerator ()
    {
#define HOOK(native, func)                                                     \
    NativeCallbackMgr::InitCallback<native##_joaat, func, true> ()
        
        // Save Data
        HOOK ("START_SAVE_DATA", SaveDataHook<false>);
        HOOK ("STOP_SAVE_DATA", SaveDataHook<true>);

        // Struct
        HOOK ("_START_SAVE_STRUCT", StartSaveStructHook<false>);
        //HOOK ("STOP_SAVE_STRUCT", StopSaveStructHook);
        HOOK ("_START_SAVE_ARRAY", StartSaveStructHook<true>);
        //HOOK ("STOP_SAVE_ARRAY", StopSaveStructHook);

        // Fields
        HOOK ("REGISTER_INT_TO_SAVE", RegisterTypeToSaveHook<INT>);
        HOOK ("REGISTER_ENUM_TO_SAVE", RegisterTypeToSaveHook<ENUM>);
        HOOK ("REGISTER_FLOAT_TO_SAVE", RegisterTypeToSaveHook<FLOAT>);
        HOOK ("REGISTER_BOOL_TO_SAVE", RegisterTypeToSaveHook<BOOL>);
        HOOK ("REGISTER_TEXT_LABEL_TO_SAVE",
              RegisterTypeToSaveHook<TEXT_LABEL>);

#undef HOOK
    }
}
#ifdef ENABLE_SAVE_DATA_PRINTING
_savedata
#endif
    ;
