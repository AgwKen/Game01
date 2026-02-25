#pragma once

#include <unordered_map>
#include <string>
#include <DirectXMath.h>

struct FontGlyph
{
    int px;
    int py;
    int pw;
    int ph;
    int xoffset;
    int yoffset;
    int xadvance;
};

class UIFont
{
public:
    bool Initialize(const wchar_t* textureFile, const char* fntFile);

    void DrawString(
        const std::string& text,
        float startX,
        float startY,
        float scale,
        const DirectX::XMFLOAT4& color);

private:
    std::unordered_map<int, FontGlyph> m_Glyphs;
    int m_TextureID = -1;
};