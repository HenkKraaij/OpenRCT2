/*****************************************************************************
 * Copyright (c) 2014-2020 OpenRCT2 developers
 *
 * For a complete list of all authors, please refer to contributors.md
 * Interested in contributing? Visit https://github.com/OpenRCT2/OpenRCT2
 *
 * OpenRCT2 is licensed under the GNU General Public License version 3.
 *****************************************************************************/

#include "FootpathAdditionRemoveAction.h"

#include "../Cheats.h"
#include "../OpenRCT2.h"
#include "../core/MemoryStream.h"
#include "../interface/Window.h"
#include "../localisation/StringIds.h"
#include "../management/Finance.h"
#include "../world/Footpath.h"
#include "../world/Location.hpp"
#include "../world/Park.h"
#include "../world/Wall.h"

void FootpathAdditionRemoveAction::AcceptParameters(GameActionParameterVisitor& visitor)
{
    visitor.Visit(_loc);
}

void FootpathAdditionRemoveAction::Serialise(DataSerialiser& stream)
{
    GameAction::Serialise(stream);

    stream << DS_TAG(_loc);
}

GameActions::Result::Ptr FootpathAdditionRemoveAction::Query() const
{
    if (!LocationValid(_loc))
    {
        return MakeResult(GameActions::Status::InvalidParameters, STR_CANT_REMOVE_THIS, STR_OFF_EDGE_OF_MAP);
    }

    if (!((gScreenFlags & SCREEN_FLAGS_SCENARIO_EDITOR) || gCheatsSandboxMode) && !map_is_location_owned(_loc))
    {
        return MakeResult(GameActions::Status::Disallowed, STR_CANT_REMOVE_THIS, STR_LAND_NOT_OWNED_BY_PARK);
    }

    if (_loc.z < FootpathMinHeight)
    {
        return MakeResult(GameActions::Status::InvalidParameters, STR_CANT_REMOVE_THIS, STR_TOO_LOW);
    }

    if (_loc.z > FootpathMaxHeight)
    {
        return MakeResult(GameActions::Status::InvalidParameters, STR_CANT_REMOVE_THIS, STR_TOO_HIGH);
    }

    auto tileElement = map_get_footpath_element(_loc);
    if (tileElement == nullptr)
    {
        log_warning("Could not find path element.");
        return MakeResult(GameActions::Status::InvalidParameters, STR_CANT_REMOVE_THIS);
    }

    auto pathElement = tileElement->AsPath();
    if (pathElement == nullptr)
    {
        log_warning("Could not find path element.");
        return MakeResult(GameActions::Status::InvalidParameters, STR_CANT_REMOVE_THIS);
    }

    if (!pathElement->AdditionIsGhost() && (GetFlags() & GAME_COMMAND_FLAG_GHOST))
    {
        log_warning("Tried to remove non ghost during ghost removal.");
        return MakeResult(GameActions::Status::Disallowed, STR_CANT_REMOVE_THIS);
    }
    auto res = MakeResult();
    res->Position = _loc;
    res->Cost = MONEY(0, 0);
    return res;
}

GameActions::Result::Ptr FootpathAdditionRemoveAction::Execute() const
{
    auto tileElement = map_get_footpath_element(_loc);
    auto pathElement = tileElement->AsPath();

    if (!(GetFlags() & GAME_COMMAND_FLAG_GHOST))
    {
        footpath_interrupt_peeps(_loc);
    }

    if (pathElement == nullptr)
    {
        log_error("Could not find path element.");
        return MakeResult(GameActions::Status::InvalidParameters, STR_CANT_REMOVE_THIS);
    }

    pathElement->SetAddition(0);
    map_invalidate_tile_full(_loc);

    auto res = MakeResult();
    res->Position = _loc;
    res->Cost = MONEY(0, 0);
    return res;
}
