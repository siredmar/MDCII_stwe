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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

#include "BshFile.h"
#include "Log.h"
#include "MdciiException.h"
#include "chunk/Chunk.h"
#include "ogl/OpenGL.h"
#include "ogl/resource/TextureUtils.h"

//-------------------------------------------------
// Ctors. / Dtor.
//-------------------------------------------------

mdcii::file::BshFile::BshFile(std::string t_filePath, std::vector<PaletteFile::Color32Bit> t_palette)
    : BinaryFile(std::move(t_filePath))
    , m_palette{ std::move(t_palette) }
{
    Log::MDCII_LOG_DEBUG("[BshFile::BshFile()] Create BshFile.");

    if (chunks.at(0)->id.c_str() != CHUNK_ID)
    {
        throw MDCII_EXCEPTION("[BshFile::BshFile()] Invalid Chunk Id.");
    }
}

mdcii::file::BshFile::~BshFile() noexcept
{
    Log::MDCII_LOG_DEBUG("[BshFile::~BshFile()] Destruct BshFile.");

    CleanUp();
}

//-------------------------------------------------
// Override
//-------------------------------------------------

void mdcii::file::BshFile::ReadDataFromChunks()
{
    Log::MDCII_LOG_DEBUG("[BshFile::ReadDataFromChunks()] Start reading BSH pixel data from Chunks...");

    // get pointer to the first element
    const auto* dataPtr{ reinterpret_cast<const uint32_t*>(chunks.at(0)->data.data()) };

    // read and store the first offset
    const auto firstOffset{ *dataPtr };
    m_offsets.push_back(firstOffset);

    // calc number of textures
    const auto count{ firstOffset / 4u };

    // store other offsets
    for (auto i{ 1u }; i < count; ++i)
    {
        m_offsets.push_back(dataPtr[i]);
    }

    Log::MDCII_LOG_DEBUG("[BshFile::ReadDataFromChunks()] Detected {} offsets.", m_offsets.size());

    // create BshTexture objects
    for (const auto offset : m_offsets)
    {
        DecodePixelData(offset);
    }

    // create OpenGL textures
    CreateGLTextures();

    // clear Cpu pixel data
    ClearTempData();

    Log::MDCII_LOG_DEBUG("[BshFile::ReadDataFromChunks()] BSH pixel data read successfully.");
}

//-------------------------------------------------
// Helper
//-------------------------------------------------

void mdcii::file::BshFile::DecodePixelData(const uint32_t t_offset)
{
    const auto* offset{ &chunks.at(0)->data[t_offset] };

    const auto width{ *reinterpret_cast<const uint32_t*>(offset) };
    const auto height{ *reinterpret_cast<const uint32_t*>((offset += sizeof(uint32_t))) };
    const auto type{ *reinterpret_cast<const uint32_t*>((offset += sizeof(uint32_t))) };
    const auto len{ *reinterpret_cast<const uint32_t*>((offset += sizeof(uint32_t))) };

    if (width <= 0 || height <= 0)
    {
        throw MDCII_EXCEPTION("[BshFile::DecodePixelData()] Invalid width or height.");
    }

    auto bshTexture{ std::make_unique<BshTexture>() };
    bshTexture->width = width;
    bshTexture->height = height;
    bshTexture->pixel.resize(static_cast<size_t>(width) * height);

    auto x{ 0 };
    auto y{ 0 };

    offset += 3;

    while (true)
    {
        const auto numAlpha{ static_cast<uint8_t>(*(offset += sizeof(uint8_t))) };

        if (numAlpha == END_MARKER)
        {
            break;
        }

        if (numAlpha == END_OF_ROW)
        {
            x = 0;
            y++;
            continue;
        }

        for (auto i{ 0 }; i < numAlpha; ++i)
        {
            bshTexture->pixel[static_cast<size_t>(y) * width + x] = 0;
            x++;
        }

        const auto numPixels{ static_cast<uint8_t>(*(offset += sizeof(uint8_t))) };
        for (auto i{ 0 }; i < numPixels; ++i)
        {
            const auto colorIndex{ static_cast<uint8_t>(*(offset += sizeof(uint8_t))) };
            bshTexture->pixel[static_cast<size_t>(y) * width + x] = m_palette[colorIndex];
            x++;
        }
    }

    bshTextures.push_back(std::move(bshTexture));
}

void mdcii::file::BshFile::CreateGLTextures() const
{
    for (const auto& texture : bshTextures)
    {
        const auto textureId{ ogl::resource::TextureUtils::GenerateNewTextureId() };

        ogl::resource::TextureUtils::Bind(textureId);
        ogl::resource::TextureUtils::UseNoFilter();

        texture->textureId = textureId;

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA8,
            static_cast<int32_t>(texture->width),
            static_cast<int32_t>(texture->height),
            0,
            GL_BGRA,
            GL_UNSIGNED_INT_8_8_8_8_REV,
            texture->pixel.data()
        );

        ogl::resource::TextureUtils::Unbind();
    }
}

//-------------------------------------------------
// CleanUp
//-------------------------------------------------

void mdcii::file::BshFile::ClearTempData()
{
    Log::MDCII_LOG_DEBUG("[BshFile::ClearTempData()] Deletes data that is no longer needed.");

    for (const auto& texture : bshTextures)
    {
        std::vector<PaletteFile::Color32Bit>().swap(texture->pixel);
    }

    std::vector<PaletteFile::Color32Bit>().swap(m_palette);
    std::vector<uint32_t>().swap(m_offsets);
}

void mdcii::file::BshFile::CleanUp() const
{
    Log::MDCII_LOG_DEBUG("[BshFile::CleanUp()] Clean up OpenGL textures.");

    const auto i{ bshTextures.size() };
    for (const auto& texture : bshTextures)
    {
        ogl::resource::TextureUtils::DeleteTexture(texture->textureId);
    }

    Log::MDCII_LOG_DEBUG("[BshFile::CleanUp()] {} OpenGL textures was deleted for file {}.", i, filePath);
}
