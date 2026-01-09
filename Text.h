#pragma once
#include <DirectXMath.h>
#include <string>

class Text
{
public:
    Text();
    void Load(const wchar_t* texturePath);

    void SetText(const char* text,
        float x,
        float y,
        const DirectX::XMFLOAT4& color,
        float scale = 1.0f);

    void Draw();

private:
    struct Glyph
    {
        int col;
        int row;
    };

    Glyph GetGlyph(char c);

private:
    int m_TexID = -1;
    std::string m_Text;

    float m_X = 0.0f;
    float m_Y = 0.0f;
    float m_Scale = 1.0f;

    DirectX::XMFLOAT4 m_Color;

    static constexpr int CHAR_W = 8;
    static constexpr int CHAR_H = 7;
    static constexpr int COLS = 16;
};
