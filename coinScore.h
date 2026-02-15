#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include "debug_text.h"
#include "sprite_anim.h"

using namespace DirectX;

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

    void SetCoinCount(int count);
    void Update(double elapsed);
    void Draw();
    int GetCoinPattern() const { return m_CoinAnimPattern; }

private:
    ID3D11Device* m_Device;
    ID3D11DeviceContext* m_Context;

    hal::DebugText* m_DebugText;

    int m_CoinCount = 0;

    // --- coin icon animation ---
    int m_CoinAnimPattern = -1;
    int m_CoinAnimPlay = -1;
};
