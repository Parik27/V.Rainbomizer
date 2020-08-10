#include "vehicle_patterns.hh"
#include "common/common.hh"
#include <rage.hh>
#include <CModelInfo.hh>
#include <Natives.hh>
#include <CStreaming.hh>
#include <sstream>
#include <Utils.hh>
#include "vehicle_cacheMgr.hh"

bool
ScriptVehiclePattern::VehicleTypes::GetValue (uint32_t type)
{
    switch (type)
        {
        case "VEHICLE_TYPE_CAR"_joaat: return Cars;
        case "VEHICLE_TYPE_BICYCLE"_joaat: return Bicycles;
        case "VEHICLE_TYPE_BIKE"_joaat: return Bikes;
        case "VEHILE_TYPE_BLIMP"_joaat: return Blimps;
        case "VEHICLE_TYPE_BOAT"_joaat: return Boats;
        case "VEHICLE_TYPE_HELI"_joaat: return Helicopters;
        case "VEHICLE_TYPE_PLANE"_joaat: return Planes;
        case "VEHICLE_TYPE_SUBMARINE"_joaat: return Submarines;
        case "VEHICLE_TYPE_SUBMARINECAR"_joaat: return SubmarineCars;
        case "VEHICLE_TYPE_TRAILER"_joaat: return Trailers;
        case "VEHICLE_TYPE_TRAIN"_joaat: return Trains;
        case "VEHICLE_TYPE_QUADBIKE"_joaat: return Quadbikes;
        case "VEHICLE_TYPE_AMPHIBIOUS_AUTOMOBILE"_joaat: return Cars || Boats;
        case "VEHICLE_TYPE_AMPHIBIOUS_QUADBIKE"_joaat:
            return Quadbikes || Boats;
        default: return Cars;
        }
}

/*******************************************************/
bool
ScriptVehiclePattern::DoesVehicleMatchPattern (uint32_t hash)
{
    uint32_t           index = -1;
    CVehicleModelInfo *model
        = CStreaming::GetModelAndIndexByHash<CVehicleModelInfo> (hash, index);

    if (!model)
        return false;

    uint32_t numSeats = GET_VEHICLE_MODEL_NUMBER_OF_SEATS (hash);

    // Seat check
    if (numSeats < m_nSeatCheck)
        return false;

    // Bounds check (check so that cache manager is only initialised if it's
    // needed
    if (m_vecBounds.x > 0.1 || m_vecBounds.y > 0.1 || m_vecBounds.z > 0.1)
        {
            const Vector3 &bounds
                = VehicleModelInfoCacheMgr::GetInstance ()->GetVehicleBounds (
                    hash);

            if (bounds.x > m_vecBounds.x || bounds.y > m_vecBounds.y
                || bounds.z > m_vecBounds.z)
                return false;
        }

    if (mFlags.CanTow && hash != "towtruck"_joaat && hash != "towtruck2"_joaat)
        return false;

    if (mFlags.AttachVehicle != 0)
        return false;

    // Type check (it has to be both not moved and allowed)
    if (!mAllowedTypes.GetValue (model->GetVehicleType ())
        && !mMovedTypes.GetValue (model->GetVehicleType ()))
        return false;

    return true;
}

/*******************************************************/
void
ScriptVehiclePattern::Cache ()
{
    auto &hashes = Rainbomizer::Common::GetVehicleHashes ();
    m_aCache.clear ();

    for (uint32_t i : hashes)
        {
            if (i == m_nOriginalVehicle || DoesVehicleMatchPattern (i))
                m_aCache.push_back (i);
        }

    m_bCached = true;
}

/*******************************************************/
uint32_t
ScriptVehiclePattern::GetRandomLoaded (Vector3 &pos)
{
    if (!m_bCached)
        Cache ();

    std::vector<uint32_t> LoadedMatches;
    
    for (auto i : m_aCache)
        if (CStreaming::HasModelLoaded(CStreaming::GetModelIndex(i)))
            LoadedMatches.push_back (i);

    // If no matching vehicles are loaded, return the original vehicle if that's
    // valid, otherwise return a truly random vehicle
    if (LoadedMatches.size () < 1)
        {
            auto &indices = Rainbomizer::Common::GetVehicleHashes ();
            if (DoesElementExist (indices, GetOriginalVehicle ()))
                return GetOriginalVehicle ();

            return GetRandom (pos);
        }

    uint32_t modelHash = GetRandomElement (LoadedMatches);
    auto modelInfo = CStreaming::GetModelByHash<CVehicleModelInfo> (modelHash);

    if (mMovedTypes.GetValue (modelInfo->GetVehicleType ()))
        pos += GetMovedCoordinates ();

    return modelHash;
}

/*******************************************************/
uint32_t
ScriptVehiclePattern::GetRandom (Vector3 &pos)
{
    if (!m_bCached)
        Cache ();

    uint32_t modelHash = GetRandomElement (m_aCache);
    auto modelInfo = CStreaming::GetModelByHash<CVehicleModelInfo> (modelHash);

    if (mMovedTypes.GetValue (modelInfo->GetVehicleType ()))
        pos += GetMovedCoordinates ();

    return modelHash;
}

/*******************************************************/
bool
ScriptVehiclePattern::MatchVehicle (uint32_t hash, const Vector3& coords)
{
    if (hash != GetOriginalVehicle ()
        && GetOriginalVehicle () != "allvehicles"_joaat)
        return false;

    // Coordinates check
    if (m_vecCoordsCheck.x && m_vecCoordsCheck.x != int(coords.x))
        return false;
    if (m_vecCoordsCheck.y && m_vecCoordsCheck.y != int(coords.y))
        return false;
    if (m_vecCoordsCheck.z && m_vecCoordsCheck.z != int(coords.z))
        return false;

    return true;
}

/*******************************************************/
void
ScriptVehiclePattern::ReadFlag (const std::string &flag)
{
    m_bCached = false;

    /*
      xyz - bound checks
      flying - can vehicle fly (to try and include flying cars / bikes)
      can_tow - vehicles that can tow cars with hook
      can_attach - trucks that can attach a trailer
    */
    if (flag == "flying")
        mFlags.Flying = true;

    else if (flag == "can_tow")
        mFlags.CanTow = true;

    else if (flag == "no_rotors")
        mFlags.NoRotors = true;

    else if (flag.find ("can_attach") == 0)
        mFlags.AttachVehicle = rage::atStringHash (
            flag.substr (sizeof ("can_attach")).c_str ());

    // Bounds
    else if (flag.find ("w=") == 0)
        m_vecBounds.x = std::stof (flag.substr (2));
    else if (flag.find ("l=") == 0)
        m_vecBounds.y = std::stof (flag.substr (2));
    else if (flag.find ("h=") == 0)
        m_vecBounds.z = std::stof (flag.substr (2));

    // Coordinates
    else if (flag.find ("x=") == 0)
        m_vecCoordsCheck.x = std::stoi (flag.substr (2));
    else if (flag.find ("y=") == 0)
        m_vecCoordsCheck.y = std::stoi (flag.substr (2));
    else if (flag.find ("z=") == 0)
        m_vecCoordsCheck.z = std::stoi (flag.substr (2));
}

/*******************************************************/
void
ScriptVehiclePattern::ParseFlags (const std::string &flags)
{
    std::istringstream flagStream (flags);
    std::string        flag = "";

    while (std::getline (flagStream, flag, '+'))
        ReadFlag (flag);
}
