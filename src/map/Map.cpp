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

#include <imgui.h>
#include "Map.h"
#include "MousePicker.h"
#include "MapContent.h"
#include "Log.h"
#include "MdciiAssert.h"
#include "renderer/ImGuiTileRenderer.h"
#include "renderer/RenderUtils.h"
#include "renderer/TileRenderer.h"
#include "renderer/TextRenderer.h"
#include "ecs/EcsUtils.h"

//-------------------------------------------------
// Ctors. / Dtor.
//-------------------------------------------------

mdcii::map::Map::Map(
    const std::string& t_filePath,
    std::shared_ptr<data::Buildings> t_buildings,
    const ogl::Window& t_window,
    const camera::Camera& t_camera
)
    : m_buildings{ std::move(t_buildings) }
{
    Log::MDCII_LOG_DEBUG("[Map::Map()] Create Map.");

    Init(t_filePath, t_window, t_camera);
}

mdcii::map::Map::~Map() noexcept
{
    Log::MDCII_LOG_DEBUG("[Map::~Map()] Destruct Map.");
}

//-------------------------------------------------
// Render
//-------------------------------------------------

void mdcii::map::Map::Render(const ogl::Window& t_window, const camera::Camera& t_camera) const
{
    RenderGridEntities(t_window, t_camera);
    RenderEntities(t_window, t_camera);

    mousePicker->Render(t_window, t_camera);
}

void mdcii::map::Map::RenderImGui()
{
    // Map stuff
    ImGui::Begin("Map");

    ImGui::Checkbox("Render grid", &renderGrid);
    ImGui::Checkbox("Render grid coords", &renderText);
    ImGui::Separator();
    ImGui::Checkbox("Render terrain layer", &renderTerrainLayer);
    ImGui::SameLine();
    ImGui::Checkbox("Skip buildings layer tiles", &skipBuildingsLayerTiles);
    ImGui::Separator();
    ImGui::Checkbox("Render buildings layer", &renderBuildingsLayer);

    ImGui::End();

    // MapContent entities
    mapContent->RenderImGui();

    // MousePicker Gui
    mousePicker->RenderImGui();
}

void mdcii::map::Map::RenderBuilding(
    const ogl::Window& t_window,
    const camera::Camera& t_camera,
    const int t_gfx,
    glm::vec2 t_screenPosition,
    const float t_elevation,
    const bool t_selected
) const
{
    const auto w{ static_cast<float>(stdBshFile->bshTextures[t_gfx]->width) };
    const auto h{ static_cast<float>(stdBshFile->bshTextures[t_gfx]->height) };
    const auto textureId{ stdBshFile->bshTextures[t_gfx]->textureId };

    t_screenPosition.y -= h - MapTile::TILE_HEIGHT;
    t_screenPosition.y -= t_elevation;

    renderer->RenderTile(
        renderer::RenderUtils::GetModelMatrix(t_screenPosition, glm::vec2(w, h)),
        textureId,
        t_window, t_camera,
        t_selected
    );
}

//-------------------------------------------------
// Rotate
//-------------------------------------------------

void mdcii::map::Map::Rotate(const ChangeRotation t_changeRotation) const
{
    switch (t_changeRotation)
    {
    case ChangeRotation::LEFT:
        mapContent->RotateLeft();
        break;
    case ChangeRotation::RIGHT:
        mapContent->RotateRight();
        break;
    }
}

const char* mdcii::map::Map::ShowCurrentRotation() const
{
    return magic_enum::enum_name(mapContent->rotation).data();
}

//-------------------------------------------------
// Init
//-------------------------------------------------

void mdcii::map::Map::Init(const std::string& t_filePath, const ogl::Window& t_window, const camera::Camera& t_camera)
{
    Log::MDCII_LOG_DEBUG("[Map::Init()] Initializing map.");

    // create the MousePicker to select tiles
    mousePicker = std::make_unique<MousePicker>(this, t_window, t_camera);

    // load map content
    mapContent = std::make_unique<MapContent>(t_filePath, m_buildings);

    // load palette from stadtfld.col
    m_paletteFile = std::make_unique<file::PaletteFile>(Game::RESOURCES_PATH + "STADTFLD.COL");
    m_paletteFile->ReadDataFromChunks();

    // load bsh graphics from stadtfld.bsh
    stdBshFile = std::make_unique<file::BshFile>(Game::RESOURCES_PATH + "STADTFLD.BSH", m_paletteFile->palette);
    stdBshFile->ReadDataFromChunks();

    // load bsh graphics from bauhaus.bsh
    bauhausBshFile = std::make_unique<file::BshFile>(Game::RESOURCES_PATH + "BAUHAUS.BSH", m_paletteFile->palette);
    bauhausBshFile->ReadDataFromChunks();

    // create tile renderer
    renderer = std::make_unique<renderer::TileRenderer>();

    // create text renderer
    textRenderer = std::make_unique<renderer::TextRenderer>(Game::RESOURCES_PATH + "bitter/Bitter-Regular.otf");

    Log::MDCII_LOG_DEBUG("[Map::Init()] The map was successfully initialized.");
}

//-------------------------------------------------
// Render Entities
//-------------------------------------------------

void mdcii::map::Map::RenderGridEntities(const ogl::Window& t_window, const camera::Camera& t_camera) const
{
    if (!renderGrid && !renderText)
    {
        return;
    }

    const auto view{ Game::ecs.view<const ecs::GridComponent>() };
    for (const auto entity : view)
    {
        const auto& gc{ view.get<const ecs::GridComponent>(entity) };
        const auto screenPosition{ gc.mapTile.screenPositions[magic_enum::enum_integer(mapContent->rotation)] };

        if (renderGrid)
        {
            renderer->RenderTile(
                renderer::RenderUtils::GetModelMatrix(
                    screenPosition,
                    glm::vec2(MapTile::TILE_WIDTH, MapTile::TILE_HEIGHT)
                ),
                gc.textureId,
                t_window, t_camera
            );
        }

        if (renderText)
        {
            textRenderer->RenderText(
                std::to_string(gc.mapTile.mapX).append(", ").append(std::to_string(gc.mapTile.mapY)),
                screenPosition.x + static_cast<float>(MapTile::TILE_WIDTH) / 4.0f,
                screenPosition.y + static_cast<float>(MapTile::TILE_HEIGHT) / 4.0f,
                0.25f,
                glm::vec3(1.0f, 0.0f, 0.0f),
                t_window, t_camera
            );
        }
    }
}

void mdcii::map::Map::RenderEntities(const ogl::Window& t_window, const camera::Camera& t_camera) const
{
    // render nothing
    if (!renderTerrainLayer && !renderBuildingsLayer)
    {
        return;
    }

    // render terrain only
    if (renderTerrainLayer && !renderBuildingsLayer)
    {
        RenderTerrainLayerEntities(t_window, t_camera);
        return;
    }

    // render buildings only
    if (!renderTerrainLayer && renderBuildingsLayer)
    {
        RenderBuildingsLayerEntities(t_window, t_camera);
        return;
    }

    // render terrain or buildings
    RenderTerrainOrBuildingsEntities(t_window, t_camera);
}

void mdcii::map::Map::RenderEntity(
    const ogl::Window& t_window,
    const camera::Camera& t_camera,
    const MapTile& t_mapTile,
    const data::Building& t_building,
    const bool t_selected
) const
{
    // get gfx in the right direction for the current map rotation
    const auto mapRotation{ magic_enum::enum_integer(mapContent->rotation) };
    auto buildingRotation{ t_mapTile.orientation };
    if (t_building.rotate > 0)
    {
        buildingRotation = (buildingRotation + mapRotation) % 4;
    }

    auto gfx{ t_mapTile.gfxs[buildingRotation] };

    if (t_building.size.w > 1)
    {
        // default: orientation 0
        auto rp{ glm::ivec2(t_mapTile.x, t_mapTile.y) };

        if (t_mapTile.orientation == 3)
        {
            rp = mapContent->RotatePosition(
                t_mapTile.x, t_mapTile.y,
                t_building.size.w, t_building.size.h,
                Rotation::DEG90
            );
        }

        if (t_mapTile.orientation == 2)
        {
            rp = mapContent->RotatePosition(
                t_mapTile.x, t_mapTile.y,
                t_building.size.w, t_building.size.h,
                Rotation::DEG180
            );
        }

        if (t_mapTile.orientation == 1)
        {
            rp = mapContent->RotatePosition(
                t_mapTile.x, t_mapTile.y,
                t_building.size.w, t_building.size.h,
                Rotation::DEG270
            );
        }

        const auto offset{ rp.y * t_building.size.w + rp.x };
        gfx += offset;
    }

    // Example 503: Bakery

    // DEG0   = 0, 1, 2, 3 -> orient 0
    // DEG90  = 1, 3, 0, 2 -> orient 3
    // DEG180 = 3, 2, 1, 0 -> orient 2
    // DEG270 = 2, 0, 3, 1 -> orient 1

    //0
    /*
    if (gfx == 3760 && t_mapTile.x == 0 && t_mapTile.y == 0)
        gfx = 3760;
    if (gfx == 3760 && t_mapTile.x == 1 && t_mapTile.y == 0)
        gfx = 3761;
    if (gfx == 3760 && t_mapTile.x == 0 && t_mapTile.y == 1)
        gfx = 3762;
    if (gfx == 3760 && t_mapTile.x == 1 && t_mapTile.y == 1)
        gfx = 3763;
    */

    // 1
    /*
    if (gfx == 3764 && t_mapTile.x == 0 && t_mapTile.y == 0)
        gfx = 3766;                                           // + 2
    if (gfx == 3764 && t_mapTile.x == 1 && t_mapTile.y == 0)
        gfx = 3764;                                           // + 0
    if (gfx == 3764 && t_mapTile.x == 0 && t_mapTile.y == 1)
        gfx = 3767;                                           // + 3
    if (gfx == 3764 && t_mapTile.x == 1 && t_mapTile.y == 1)
        gfx = 3765;                                           // + 1
    */

    // 2
    /*
    if (gfx == 3768 && t_mapTile.x == 0 && t_mapTile.y == 0)
        gfx = 3771;                                           // + 3
    if (gfx == 3768 && t_mapTile.x == 1 && t_mapTile.y == 0)
        gfx = 3770;                                           // + 2
    if (gfx == 3768 && t_mapTile.x == 0 && t_mapTile.y == 1)
        gfx = 3769;                                           // + 1
    if (gfx == 3768 && t_mapTile.x == 1 && t_mapTile.y == 1)
        gfx = 3768;                                           // + 0
    */

    // 3
    /*
    if (gfx == 3772 && t_mapTile.x == 0 && t_mapTile.y == 0)
        gfx = 3773;                                           // + 1
    if (gfx == 3772 && t_mapTile.x == 1 && t_mapTile.y == 0)
        gfx = 3775;                                           // + 3
    if (gfx == 3772 && t_mapTile.x == 0 && t_mapTile.y == 1)
        gfx = 3772;                                           // + 0
    if (gfx == 3772 && t_mapTile.x == 1 && t_mapTile.y == 1)
        gfx = 3774;                                           // + 2
    */

    RenderBuilding(
        t_window,
        t_camera,
        gfx,
        t_mapTile.screenPositions[mapRotation],
        static_cast<float>(t_building.posoffs),
        t_selected
    );
}

void mdcii::map::Map::RenderE(
    const ogl::Window& t_window,
    const camera::Camera& t_camera,
    const MapTile& t_mapTile,
    const data::Building& t_building
) const
{
    const auto rot{ magic_enum::enum_integer(mapContent->rotation) };
    auto gfx{ t_mapTile.gfxs[t_mapTile.orientation] };

    if (t_building.size.w > 1)
    {
        const auto offset{ t_mapTile.y * t_building.size.w + t_mapTile.x };
        gfx += offset;
    }

    RenderBuilding(
        t_window,
        t_camera,
        gfx,
        t_mapTile.screenPositions[rot],
        static_cast<float>(t_building.posoffs),
        false
    );
}

void mdcii::map::Map::RenderTerrainLayerEntities(const ogl::Window& t_window, const camera::Camera& t_camera) const
{
#ifdef MDCII_DEBUG_BUILD
    auto t{ 0 }; // terrain tiles expected result = width * height
#endif

    const auto view{ Game::ecs.view<const ecs::TerrainLayerTileComponent>() };
    for (const auto entity : view)
    {
        if (skipBuildingsLayerTiles)
        {
            if (Game::ecs.all_of<const ecs::BuildingsLayerTileComponent>(entity))
            {
#ifdef MDCII_DEBUG_BUILD
                t++;
#endif
                continue;
            }
        }

        const auto& [terrainLayerTileComponent] { view.get(entity) };
        RenderEntity(t_window, t_camera, terrainLayerTileComponent.mapTile, terrainLayerTileComponent.building, false);
#ifdef MDCII_DEBUG_BUILD
        t++;
#endif
    }

    MDCII_ASSERT(t == mapContent->width * mapContent->height, "[Map::RenderTerrainLayerEntities()] Invalid number of entities.")
}

void mdcii::map::Map::RenderBuildingsLayerEntities(const ogl::Window& t_window, const camera::Camera& t_camera) const
{
    const auto view{ Game::ecs.view<ecs::BuildingsLayerTileComponent>(entt::exclude<ecs::BuildingUpdatedComponent>) };
    for (const auto entity : view)
    {
        const auto& [buildingsComponent] { view.get(entity) };
        RenderEntity(t_window, t_camera, buildingsComponent.mapTile, buildingsComponent.building, false);
    }
}

void mdcii::map::Map::RenderTerrainOrBuildingsEntities(const ogl::Window& t_window, const camera::Camera& t_camera) const
{
    const auto view{ Game::ecs.view<ecs::TerrainLayerTileComponent>(/*entt::exclude<ecs::BuildingUpdatedComponent>*/)};
    for (const auto entity : view)
    {
        const auto& terrainLayerTileComponent { view.get<ecs::TerrainLayerTileComponent>(entity) };

        if (Game::ecs.all_of<const ecs::BuildingsLayerTileComponent>(entity))
        {
            const auto& buildingsLayerTileComponent{ Game::ecs.get<const ecs::BuildingsLayerTileComponent>(entity) };

            RenderEntity(t_window, t_camera, buildingsLayerTileComponent.mapTile, buildingsLayerTileComponent.building, false);
        }
        else
        {
            RenderEntity(t_window, t_camera, terrainLayerTileComponent.mapTile, terrainLayerTileComponent.building, false);
        }
    }
}
