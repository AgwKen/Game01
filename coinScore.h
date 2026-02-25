#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "UIFont.h"

class CoinScoreUI
{
public:
    CoinScoreUI(
        ID3D11Device* device,
        ID3D11DeviceContext* context,
        int screenWidth,
        int screenHeight
    );

    ~CoinScoreUI();

    void Update(double elapsed);
    void Draw();

    void SetCoinCount(int count);
    void SetInitialScore(int count);

    int GetCoinPattern() const { return m_CoinAnimPattern; }

private:
    ID3D11Device* m_Device = nullptr;
    ID3D11DeviceContext* m_Context = nullptr;

    // Font
    UIFont* m_Font = nullptr;

    // Coin animation
    int m_CoinAnimPattern = -1;
    int m_CoinAnimPlay = -1;

    // Score
    int m_CoinCount = 0;

    // Combo system
    int   m_Combo = 0;
    float m_ComboTimer = 0.0f;
    float m_ComboWindow = 2.0f; // seconds to maintain combo

    float m_ComboScale = 1.0f;
    float m_ComboPopTimer = 0.0f;
    float m_ComboPopDuration = 0.25f; // pop animation length
};