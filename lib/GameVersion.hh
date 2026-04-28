#pragma once

#include "common/logger.hh"
#include <array>
#include <windows.h>
#include <string>
#include <stdexcept>

#pragma pack(push, 1)
struct RSDSHeader
{
	DWORD Signature;
	GUID Guid;
	DWORD Age;
	char PdbFileName[1];
};
#pragma pack(pop)

class GameVersion
{
public:
    enum GameVersionEnum
    {
        GTA5_LEGACY,
        GTA5_ENHANCED
    };

private:
    static std::string
    GetPdbPath ()
    {
        HMODULE hModule = GetModuleHandle (nullptr);
        if (!hModule)
            throw std::runtime_error ("Failed to get main module handle.");

        const BYTE             *base = reinterpret_cast<BYTE *> (hModule);
        const IMAGE_DOS_HEADER *dos
            = reinterpret_cast<const IMAGE_DOS_HEADER *> (base);
        if (dos->e_magic != IMAGE_DOS_SIGNATURE)
            throw std::runtime_error ("Invalid DOS signature");

        const IMAGE_NT_HEADERS *nt
            = reinterpret_cast<const IMAGE_NT_HEADERS *> (base + dos->e_lfanew);
        if (nt->Signature != IMAGE_NT_SIGNATURE)
            throw std::runtime_error ("Invalid PE signature.");

        const IMAGE_DATA_DIRECTORY &debugData
            = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
        if (debugData.VirtualAddress == 0)
            throw std::runtime_error ("No debug directory found.");

        const IMAGE_DEBUG_DIRECTORY *debugDir
            = reinterpret_cast<const IMAGE_DEBUG_DIRECTORY *> (
                base + debugData.VirtualAddress);
        if (debugDir->Type != IMAGE_DEBUG_TYPE_CODEVIEW)
            throw std::runtime_error ("Not a CodeView debug directory.");

        const BYTE       *debugInfo = base + debugDir->AddressOfRawData;
        const RSDSHeader *rsds
            = reinterpret_cast<const RSDSHeader *> (debugInfo);
        if (rsds->Signature != 'SDSR')
            throw std::runtime_error ("Invalid RSDS signature.");

        return std::string (rsds->PdbFileName);
    }

    static bool
    CheckPdbPathForVersion (const std::string &pdbPath)
    {
        if (pdbPath.find ("dev_gen9") != std::string::npos)
            {
                Rainbomizer::Logger::LogMessage (
                    "Enhanced Version Detected with PDB Path: %s",
                    pdbPath.c_str ());
                return true;
            }
        else if (pdbPath.find ("dev_ng") != std::string::npos)
            {
                Rainbomizer::Logger::LogMessage (
                    "Legacy Version Detected with PDB Path: %s",
                    pdbPath.c_str ());
                return false;
            }

        throw std::runtime_error (std::string ("Unknown PDB Path: " + pdbPath));
    }

    static bool
    DetectByFilename ()
    {
        char moduleName[MAX_PATH];
        GetModuleFileNameA ((HMODULE) nullptr, moduleName, MAX_PATH);
        std::string_view ws (moduleName);

        if (ws.ends_with ("_Enhanced.exe"))
            {
                Rainbomizer::Logger::LogMessage (
                    "Enhanced Version Detected With Filename Check");
                return true;
            }

        Rainbomizer::Logger::LogMessage ("Legacy Version detected");
        return false;
    }

public:
    static GameVersionEnum
    GetGame ()
    {
        static const auto cachedVersion = [] () -> auto {
            try
                {
                    const std::string pdbPath = GetPdbPath ();
                    bool isEnhanced = CheckPdbPathForVersion (pdbPath);
                    return isEnhanced ? GameVersion::GTA5_ENHANCED
                                      : GameVersion::GTA5_LEGACY;
                }
            catch (const std::runtime_error &e)
                {
                    Rainbomizer::Logger::LogMessage (
                        "Failed to extract PDB path! Falling back to Filename "
                        "Check: %s",
                        e.what ());

                    bool isEnhanced = DetectByFilename ();
                    return isEnhanced ? GameVersion::GTA5_ENHANCED
                                      : GameVersion::GTA5_LEGACY;
                }
        }();

        return cachedVersion;
    }

    static bool
    IsEnhanced ()
    {
        return GetGame () == GameVersion::GTA5_ENHANCED;
    }

    static bool
    IsLegacy ()
    {
        return GetGame () == GameVersion::GTA5_LEGACY;
    }
};
