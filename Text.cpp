#include "text.h"
#include "sprite.h"
#include "texture.h"

using namespace DirectX;

Text::Text()
{
}

void Text::Load(const wchar_t* texturePath)
{
    m_TexID = Texture_Load(texturePath);
}

void Text::SetText(const char* text,
    float x,
    float y,
    const XMFLOAT4& color,
    float scale)
{
    m_Text = text;
    m_X = x;
    m_Y = y;
    m_Color = color;
    m_Scale = scale;
}

Text::Glyph Text::GetGlyph(char c)
{
    // -------- ROW 0 --------
    const char* row0 = "ABCDEFGHIOKLMNOP";
    // -------- ROW 1 --------
    const char* row1 = "ORSTUVWXYZ123456";
    // -------- ROW 2 --------
    const char* row2 = "7890#, .!?:*%()+-";

    const char* rows[] = { row0, row1, row2 };

    for (int r = 0; r < 3; r++)
    {
        const char* row = rows[r];
        for (int i = 0; row[i]; i++)
        {
            if (row[i] == c)
                return { i, r };
        }
    }

    // fallback ¨ space
    return { 0, 0 };
}

void Text::Draw()
{
    if (m_TexID < 0) return;

    float x = m_X;
    float y = m_Y;

    for (char c : m_Text)
    {
        if (c == ' ')
        {
            x += CHAR_W * m_Scale;
            continue;
        }

        Glyph g = GetGlyph(c);

        Sprite_Draw(
            m_TexID,
            x,
            y,
            CHAR_W * m_Scale,
            CHAR_H * m_Scale,
            g.col * CHAR_W,
            g.row * CHAR_H,
            CHAR_W,
            CHAR_H,
            m_Color
        );

        x += CHAR_W * m_Scale;
    }
}
