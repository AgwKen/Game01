#include "CoinScore.h"
#include "texture.h"
#include "sprite_anim.h"
#include <cstdio>

CoinScoreUI::CoinScoreUI(
    ID3D11Device* device,
    ID3D11DeviceContext* context,
    int screenWidth,
    int screenHeight
)
    : m_Device(device), m_Context(context)
{
    // =============================
    // Initialize Font
    // =============================
    m_Font = new UIFont();
    m_Font->Initialize(
        L"Texture/WhitePeaberry.png",
        "Texture/WhitePeaberry.fnt"
    );

    // =============================
    // Register Coin Sprite Animation
    // =============================
    int coinTex = Texture_Load(L"Texture/coin.png");

    m_CoinAnimPattern = SpriteAnim_RegisterPattern(
        coinTex,
        8,              // total frames
        8,              // horizontal frames
        0.5,            // animation speed
        { 16,16 },      // frame size
        { 0,0 },
        true
    );

    m_CoinAnimPlay = SpriteAnim_CreatePlayer(m_CoinAnimPattern);
}

CoinScoreUI::~CoinScoreUI()
{
    SpriteAnim_DestroyPlayer(m_CoinAnimPlay);

    delete m_Font;
    m_Font = nullptr;
}

void CoinScoreUI::Update(double elapsed)
{
    SpriteAnim_UpdatePlayer(m_CoinAnimPlay, elapsed);

    // Combo lifetime reset
    if (m_Combo > 0)
    {
        m_ComboTimer += (float)elapsed;

        if (m_ComboTimer > m_ComboWindow)
        {
            m_Combo = 0;
        }
    }

    // ===============================
    // COMBO POP ANIMATION
    // ===============================
    if (m_ComboPopTimer < m_ComboPopDuration)
    {
        m_ComboPopTimer += (float)elapsed;

        float t = m_ComboPopTimer / m_ComboPopDuration;

        // Ease-out scaling
        m_ComboScale = 1.8f - (0.8f * t);

        if (m_ComboScale < 1.0f)
            m_ComboScale = 1.0f;
    }
}

void CoinScoreUI::SetCoinCount(int count)
{
    // Check if combo window expired
    if (m_ComboTimer > m_ComboWindow)
    {
        m_Combo = 1;  // reset to 1
    }
    else
    {
        m_Combo++;    // continue combo
    }

    m_CoinCount = count;
    m_ComboTimer = 0.0f;

    // Trigger pop animation
    m_ComboPopTimer = 0.0f;
    m_ComboScale = 1.8f;
}
void CoinScoreUI::SetInitialScore(int count)
{
    m_CoinCount = count;
    m_Combo = 0;
    m_ComboTimer = 0.0f;
}
void CoinScoreUI::Draw()
{
    // =============================
    // Draw Animated Coin Icon
    // =============================
    SpriteAnim_Draw(
        m_CoinAnimPlay,
        50.0f, 150.0f,   // position
        48.0f, 48.0f     // size
    );

    // =============================
    // Draw Coin Count
    // =============================
    char scoreBuf[32];
    sprintf_s(scoreBuf, "%d", m_CoinCount);

    m_Font->DrawString(
        scoreBuf,
        120.0f,
        155.0f,
        2.0f,
        DirectX::XMFLOAT4(1, 1, 1, 1)
    );

    // =============================
    // Draw Combo Multiplier
    // =============================
    if (m_Combo > 1)
    {
        char comboBuf[32];
        sprintf_s(comboBuf, "x%d", m_Combo);

        DirectX::XMFLOAT4 comboColor =
            (m_Combo > 5)
            ? DirectX::XMFLOAT4(1, 0.2f, 0.2f, 1)
            : DirectX::XMFLOAT4(1, 0.8f, 0.2f, 1);

        // Add bounce effect
        float bounce = sinf(m_ComboTimer * 10.0f) * 5.0f;

        m_Font->DrawString(
            comboBuf,
            110.0f,
            180.0f + bounce,
            2.0f * m_ComboScale,
            comboColor
        );
    }
}