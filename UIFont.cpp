#include "UIFont.h"
#include "texture.h"
#include "sprite.h"

#include <fstream>
#include <sstream>

bool UIFont::Initialize(const wchar_t* textureFile, const char* fntFile)
{
    m_TextureID = Texture_Load(textureFile);
    if (m_TextureID < 0)
        return false;

    std::ifstream file(fntFile);
    if (!file.is_open())
        return false;

    std::string line;

    while (std::getline(file, line))
    {
        if (line.find("char id=") != std::string::npos)
        {
            std::stringstream ss(line);

            std::string tmp;
            FontGlyph g;
            int id;

            ss >> tmp >> tmp;
            id = std::stoi(tmp.substr(tmp.find("=") + 1));

            ss >> tmp;
            g.px = std::stoi(tmp.substr(tmp.find("=") + 1));

            ss >> tmp;
            g.py = std::stoi(tmp.substr(tmp.find("=") + 1));

            ss >> tmp;
            g.pw = std::stoi(tmp.substr(tmp.find("=") + 1));

            ss >> tmp;
            g.ph = std::stoi(tmp.substr(tmp.find("=") + 1));

            ss >> tmp;
            g.xoffset = std::stoi(tmp.substr(tmp.find("=") + 1));

            ss >> tmp;
            g.yoffset = std::stoi(tmp.substr(tmp.find("=") + 1));

            ss >> tmp;
            g.xadvance = std::stoi(tmp.substr(tmp.find("=") + 1));

            m_Glyphs[id] = g;
        }
    }

    return true;
}

void UIFont::DrawString(
    const std::string& text,
    float startX,
    float startY,
    float scale,
    const DirectX::XMFLOAT4& color)
{
    if (m_TextureID < 0) return;

    float cursorX = startX;

    for (char c : text)
    {
        int id = static_cast<int>(c);

        auto it = m_Glyphs.find(id);
        if (it == m_Glyphs.end())
            continue;

        const FontGlyph& g = it->second;

        float drawX = cursorX + g.xoffset * scale;
        float drawY = startY + g.yoffset * scale;
        float drawW = g.pw * scale;
        float drawH = g.ph * scale;

        Sprite_Draw(
            m_TextureID,
            drawX,
            drawY,
            drawW,
            drawH,
            g.px,
            g.py,
            g.pw,
            g.ph,
            color
        );

        cursorX += g.xadvance * scale;
    }
}