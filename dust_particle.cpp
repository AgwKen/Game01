#include "dust_particle.h"
#include "billboard.h"
#include <cmath>
#include <algorithm>

using namespace DirectX;

// ============================================================
// DustParticle
// ============================================================

DustParticle::DustParticle(
    int texture_id,
    const DirectX::XMVECTOR& position,
    const DirectX::XMVECTOR& velocity,
    double life_time,
    const DirectX::XMFLOAT4& color
)
    : m_texId(texture_id),
    Particle(position, velocity, life_time)
{
    SetColor(color);
}

void DustParticle::Update(double elapsed_time)
{
    // ----- movement -----
    AddPosition(GetVelocity() * elapsed_time);

    // get current position
    XMFLOAT3 pos;
    XMStoreFloat3(&pos, GetPosition());

    // check terrain height
    float ground = Mesh_GetHeightAt(pos.x, pos.z);

    // ===== GROUND COLLISION =====
    if (!m_landed && pos.y <= ground)
    {
        pos.y = ground;
        SetPosition(XMLoadFloat3(&pos));

        m_landed = true;
        m_groundTime = 0.0f;

        // tiny bounce + energy loss
        XMFLOAT3 vel;
        XMStoreFloat3(&vel, GetVelocity());

        vel.y = fabs(vel.y) * 0.25f;
        vel.x *= 0.6f;
        vel.z *= 0.6f;

        SetVelocity(XMLoadFloat3(&vel));
    }

    // ===== PHYSICS =====
    if (!m_landed)
    {
        // airborne: gentle gravity
        AddVelocity(XMVECTOR{ 0, -0.25f * (float)elapsed_time, 0 });

        // air resistance
        AddVelocity(GetVelocity() * -0.25f * (float)elapsed_time);
    }
    else
    {
        // on ground: sliding friction
        m_groundTime += (float)elapsed_time;

        AddVelocity(GetVelocity() * -1.8f * (float)elapsed_time);

        // keep stuck to ground
        pos.y = ground;
        SetPosition(XMLoadFloat3(&pos));
    }

    // ===== FADING =====
    float lifeRatio =
        (float)(1.0 - (GetAccumulatedTime() / GetLifeTime()));

    XMFLOAT4 c = GetColor();

    if (!m_landed)
    {
        c.w = 0.25f + lifeRatio * 0.45f;

    }
    else
    {
        // slower fade after landing
        float fadeAfter = std::max(0.0f, lifeRatio - 0.25f);
        c.w = fadeAfter * 0.9f;
    }

    SetColor(c);

    Particle::Update(elapsed_time);
}

void DustParticle::Render() const
{
    XMFLOAT3 position;
    XMStoreFloat3(&position, GetPosition());

    unsigned int width = Texture_Width(m_texId);
    unsigned int height = Texture_Height(m_texId);

    // slightly larger and more visible dust
    // very tiny particles with small variation
    // ultra fine sand-like particles
    float size = 0.0012f + (rand() % 100) / 100.0f * 0.0010f;



    Billboard_Draw(
        m_texId,
        position,
        DirectX::XMFLOAT2(0.5f, 0.5f),
        DirectX::XMUINT4(0, 0, width, height),
        DirectX::XMFLOAT2(size, size),
        GetColor()
    );
}

// ============================================================
// DustEmitter
// ============================================================

DustEmitter::DustEmitter(
    size_t capacity,
    const DirectX::XMVECTOR& position,
    double particles_per_second,
    bool is_emmit
)
    : Emitter(capacity, position, particles_per_second, is_emmit)
    , m_texture_id(Texture_Load(L"Texture/dust.png"))
{
    std::random_device rd;
    m_mt.seed(rd());
}

Particle* DustEmitter::createParticle()
{
    // ===== RADIAL SPREAD =====
    float angle = m_color_dist(m_mt) * DirectX::XM_2PI;

    // MUCH wider outward spread
  // tighter and more controlled spread
    float speed = 0.32f + m_color_dist(m_mt) * 0.45f;


    float vx = cosf(angle) * speed;
    float vz = sinf(angle) * speed;

    // tiny upward kick
    float vy = 0.08f + m_color_dist(m_mt) * 0.12f;



    XMVECTOR velocity{ vx, vy, vz };

    // ===== DARK BROWN DUST COLOR =====
    float base = 0.16f + m_color_dist(m_mt) * 0.10f;

    XMFLOAT4 color{
     base * 2.2f,
     base * 1.6f,
     base * 1.0f,
     0.60f
    };

    return new DustParticle(
        m_texture_id,
        GetPosition(),
        velocity,
        m_lifetime_dist(m_mt),
        color
    );
}
