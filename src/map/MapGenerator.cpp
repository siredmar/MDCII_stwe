// This file is part of the MDCII project.
//
// Copyright (c) 2022. stwe <https://github.com/stwe/MDCII>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include <filesystem>
#include "MapGenerator.h"
#include "Game.h"
#include "Log.h"
#include "map/MapContent.h"

//-------------------------------------------------
// Ctors. / Dtor.
//-------------------------------------------------

mdcii::map::MapGenerator::MapGenerator(const int t_width, const int t_height, const std::string& t_fileName)
{
    Log::MDCII_LOG_DEBUG("[MapGenerator::MapGenerator()] Create MapGenerator.");

    Init(t_width, t_height, t_fileName);
}

mdcii::map::MapGenerator::~MapGenerator() noexcept
{
    Log::MDCII_LOG_DEBUG("[MapGenerator::~MapGenerator()] Destruct MapGenerator.");
}

//-------------------------------------------------
// Init
//-------------------------------------------------

void mdcii::map::MapGenerator::Init(const int t_width, const int t_height, const std::string& t_fileName)
{
    const auto fileName{ Game::RESOURCES_REL_PATH + t_fileName };
    if (std::filesystem::exists(fileName))
    {
        Log::MDCII_LOG_WARN("[MapGenerator::Init()] The {} file already exists.", fileName);

        return;
    }

    nlohmann::json j;
    nlohmann::json t = nlohmann::json::object();
    nlohmann::json b = nlohmann::json::object();

    std::ofstream file{ fileName };
    if (!file.is_open())
    {
        throw MDCII_EXCEPTION("[MapGenerator::Init()] Error while opening file " + fileName + ".");
    }

    Log::MDCII_LOG_DEBUG("[MapGenerator::Init()] Starts creating a new map file...");

    std::vector<MapTile> terrainTiles;
    std::vector<MapTile> buildingsTiles;

    // first pass: grass, no buildings
    for (auto y{ 0 }; y < t_height; ++y)
    {
        for (auto x{ 0 }; x < t_width; ++x)
        {
            MapTile terrainTile;
            terrainTile.buildingId = GRASS;
            terrainTile.rotation = Rotation::DEG0;
            terrainTile.x = 0;
            terrainTile.y = 0;
            //mapTile.mapX = x;
            //mapTile.mapY = y;

            // store grass as default
            terrainTiles.push_back(terrainTile);

            // no building
            buildingsTiles.emplace_back();
        }
    }

    auto mapIndex = [&](const int t_x, const int t_y) {
        return t_y * t_width + t_x;
    };

    // second pass: corners
    terrainTiles.at(mapIndex(0, 0)).buildingId = COAST_CORNER; // top left
    terrainTiles.at(mapIndex(0, 0)).rotation = Rotation::DEG90;

    terrainTiles.at(mapIndex(t_width - 1, 0)).buildingId = COAST_CORNER; // top right
    terrainTiles.at(mapIndex(t_width - 1, 0)).rotation = Rotation::DEG180;

    terrainTiles.at(mapIndex(t_width - 1, t_height - 1)).buildingId = COAST_CORNER; // bottom right
    terrainTiles.at(mapIndex(t_width - 1, t_height - 1)).rotation = Rotation::DEG270;

    terrainTiles.at(mapIndex(0, t_height - 1)).buildingId = COAST_CORNER; // bottom left
    terrainTiles.at(mapIndex(0, t_height - 1)).rotation = Rotation::DEG0;

    // third pass: borders

    // left to right
    for (auto x{ 1 }; x < t_width - 1; ++x)
    {
        terrainTiles.at(mapIndex(x, 0)).buildingId = COAST;
        terrainTiles.at(mapIndex(x, 0)).rotation = Rotation::DEG180;

        terrainTiles.at(mapIndex(x, t_height - 1)).buildingId = COAST;
        terrainTiles.at(mapIndex(x, t_height - 1)).rotation = Rotation::DEG0;
    }

    // top to bottom
    for (auto y{ 1 }; y < t_height - 1; ++y)
    {
        terrainTiles.at(mapIndex(0, y)).buildingId = COAST;
        terrainTiles.at(mapIndex(0, y)).rotation = Rotation::DEG90;

        terrainTiles.at(mapIndex(t_width - 1, y)).buildingId = COAST;
        terrainTiles.at(mapIndex(t_width - 1, y)).rotation = Rotation::DEG270;
    }

    // write
    j["width"] = t_width;
    j["height"] = t_height;
    j["layers"] = nlohmann::json::array();
    t["terrain"] = terrainTiles;
    b["buildings"] = buildingsTiles;
    j["layers"].push_back(t);
    j["layers"].push_back(b);

    file << j;

    Log::MDCII_LOG_DEBUG("[MapGenerator::Init()] The map file was created successfully and saved in {}.", fileName);
}

//-------------------------------------------------
// Create map
//-------------------------------------------------

std::vector<mdcii::map::MapTile> mdcii::map::MapGenerator::CreateTerrainLayer()
{
    return {};
}

std::vector<mdcii::map::MapTile> mdcii::map::MapGenerator::CreateBuildingsLayer()
{
    return {};
}
