#include "particle_test.h"
#include "billboard.h"

using namespace DirectX;

void NormalParticle::Update(double elapsed_time)
{
    AddPosition(GetVelocity() * elapsed_time);

    // simulate cooling / slowing down
    AddVelocity(DirectX::XMVECTOR{ 0, -2.0f * (float)elapsed_time, 0 });

    // FADE OUT ALPHA OVER TIME
    float lifeRatio = (float)(1.0 - (GetAccumulatedTime() / GetLifeTime()));

    XMFLOAT4 c = GetColor();
    c.w = lifeRatio;
    SetColor(c);

    Particle::Update(elapsed_time);
}

void NormalParticle::Render() const
{
    XMFLOAT3 position;
    XMStoreFloat3(&position, GetPosition());

    unsigned int width = Texture_Width(m_texId);
    unsigned int height = Texture_Height(m_texId);

    Billboard_Draw(
        m_texId,
        position,
        DirectX::XMFLOAT2(0.5f, 0.5f),
        DirectX::XMUINT4(0, 0, width, height),
        DirectX::XMFLOAT2(0.01f, 0.01f),
        GetColor()      // NOW USE PARTICLE COLOR
    );
}

Particle* NormalEmitter::createParticle()
{
    float vx = (m_color_dist(m_mt) - 0.5f) * 1.0f;   // small left/right
    float vy = m_speed_dist(m_mt) + 1.0f;// strong upward force
    float vz = (m_color_dist(m_mt) - 0.5f) * 1.0f;   // small forward/back

    XMVECTOR velocity{ vx, vy, vz };

    // FIRE COLOR
    float r = 1.0f;
    float g = m_color_dist(m_mt) * 0.5f + 0.2f;   // 0.2 – 0.7
    float b = 0.0f;

    XMFLOAT4 color{ r, g, b, 1.0f };

    return new NormalParticle(
        m_texture_id,
        GetPosition(),
        velocity,
        m_lifetime_dist(m_mt),
        color
    );
}
