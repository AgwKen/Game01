#include "CoinScore.h"
#include <cstdio>  // for sprintf_s
#include "texture.h"

CoinScoreUI::CoinScoreUI(
    ID3D11Device* device,
    ID3D11DeviceContext* context,
    int screenWidth,
    int screenHeight
)
    : m_Device(device), m_Context(context)
{
    // number text only
    m_DebugText = new hal::DebugText(
        m_Device,
        m_Context,
        L"consolab_ascii_512.png",
        screenWidth,
        screenHeight,
        68.0f, 20.0f,   // number position (right of coin)
        1, 8,
        20.0f, 16.0f
    );

    // --- register coin sprite animation ---
    int coinTex = Texture_Load(L"Texture/coin.png"); // sprite sheet

    m_CoinAnimPattern = SpriteAnim_RegisterPattern(
        coinTex,
        8,                  // total frames
        8,                  // horizontal frames
        0.5,               // speed
        { 16, 16 },         // frame size
        { 0, 0 },
        true
    );

    m_CoinAnimPlay = SpriteAnim_CreatePlayer(m_CoinAnimPattern);
}

CoinScoreUI::~CoinScoreUI()
{
    SpriteAnim_DestroyPlayer(m_CoinAnimPlay);
    delete m_DebugText;
}

void CoinScoreUI::Update(double elapsed)
{
    SpriteAnim_UpdatePlayer(m_CoinAnimPlay, elapsed);
}


void CoinScoreUI::SetCoinCount(int count)
{
    m_CoinCount = count;

    char buf[16];
    sprintf_s(buf, "%d", m_CoinCount);

    m_DebugText->Clear();
    m_DebugText->SetText(buf, XMFLOAT4(1, 1, 1, 1));
}

void CoinScoreUI::Draw()
{
    // draw animated coin icon
    SpriteAnim_Draw(
        m_CoinAnimPlay,
        12.0f, 12.0f,   // screen position
        48.0f, 48.0f    // size
    );

    // draw number
    m_DebugText->Draw();
}
