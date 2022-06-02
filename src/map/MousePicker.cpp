#include <imgui.h>
#include "MousePicker.h"
#include "Map.h"
#include "Log.h"
#include "renderer/TileRenderer.h"
#include "renderer/Utils.h"
#include "ogl/resource/ResourceManager.h"

//-------------------------------------------------
// Ctors. / Dtor.
//-------------------------------------------------

mdcii::map::MousePicker::MousePicker(std::shared_ptr<Map> t_map)
    : m_map{ std::move(t_map) }
{
    Log::MDCII_LOG_DEBUG("[MousePicker::MousePicker()] Create MousePicker.");

    Init();
}

mdcii::map::MousePicker::~MousePicker() noexcept
{
    Log::MDCII_LOG_DEBUG("[MousePicker::~MousePicker()] Destruct MousePicker.");
}

//-------------------------------------------------
// Logic
//-------------------------------------------------

void mdcii::map::MousePicker::Render(const ogl::Window& t_window, const camera::Camera& t_camera)
{
    m_mouse = glm::ivec2(t_window.GetMouseX(), t_window.GetMouseY());
    m_cell = glm::ivec2(m_mouse.x / 64, m_mouse.y / 32);
    m_offsetIntoCell = glm::ivec2( m_mouse.x % 64, m_mouse.y % 32 );

    const glm::ivec2 origin{
        static_cast<int>(t_camera.position.x) / 64,
    static_cast<int>(t_camera.position.y) / 32
    };

    if (m_map->rotation == Rotation::DEG0)
    {
        m_selected = glm::ivec2(
            (m_cell.y + origin.y) + (m_cell.x + origin.x),
            (m_cell.y + origin.y) - (m_cell.x + origin.x)
        );
    }

    if (m_map->rotation == Rotation::DEG90)
    {
        m_selected = glm::ivec2(
            (m_cell.y + origin.y) - (m_cell.x + origin.x),
            m_map->width - 1 - ((m_cell.x + origin.x) + (m_cell.y + origin.y))
        );
    }

    if (m_map->rotation == Rotation::DEG180)
    {
        m_selected = glm::ivec2(
            m_map->width - 1 - ((m_cell.y + origin.y) + (m_cell.x + origin.x)),
            m_map->height - 1 - ((m_cell.y + origin.y) - (m_cell.x + origin.x))
        );
    }

    if (m_map->rotation == Rotation::DEG270)
    {
        m_selected = glm::ivec2(
            m_map->height - 1 - ((m_cell.y + origin.y) - (m_cell.x + origin.x)),
            (m_cell.y + origin.y) + (m_cell.x + origin.x)
        );
    }

    m_renderer->RenderTile(
        renderer::Utils::GetModelMatrix(
            m_map->MapToIso(m_selected.x, m_selected.y, m_map->rotation), // todo: rotation remove arg.
            glm::vec2(64.0f, 32.0f)
        ),
        ogl::resource::ResourceManager::LoadTexture("resources/textures/frame.png").id,
        t_window, t_camera
    );

    /*
    m_renderer->RenderTile(
        renderer::Utils::GetModelMatrix(
            glm::vec2(m_cell.x * 64, m_cell.y * 32),
            glm::vec2(64.0f, 32.0f)
        ),
        ogl::resource::ResourceManager::LoadTexture("resources/textures/frame.png").id,
        t_window, t_camera
    );
    */
}

void mdcii::map::MousePicker::RenderImGui()
{
    ImGui::Begin("MousePicker", nullptr, 0);

    ImGui::Text("Mouse x: %d, y: %d", m_mouse.x, m_mouse.y);
    ImGui::Text("Cell x: %d, y: %d", m_cell.x, m_cell.y);
    ImGui::Text("Selected x: %d, y: %d", m_selected.x, m_selected.y);

    ImGui::End();
}

//-------------------------------------------------
// Init
//-------------------------------------------------

void mdcii::map::MousePicker::Init()
{
    Log::MDCII_LOG_DEBUG("[MousePicker::Init()] Initializing mouse picker.");

    m_renderer = std::make_unique<renderer::TileRenderer>();

    Log::MDCII_LOG_DEBUG("[MousePicker::Init()] The mouse picker was successfully initialized.");
}
