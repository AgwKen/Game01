#pragma once
#ifndef DUST_PARTICLE_H
#define DUST_PARTICLE_H

#include "particle.h"
#include "texture.h"
#include "terrain.h"        // needed for ground height
#include <random>

class DustParticle : public Particle
{
private:
    int m_texId{ -1 };

    bool m_landed = false;
    float m_groundTime = 0.0f;

public:
    DustParticle(
        int texture_id,
        const DirectX::XMVECTOR& position,
        const DirectX::XMVECTOR& velocity,
        double life_time,
        const DirectX::XMFLOAT4& color
    );

    void Update(double elapsed_time) override;
    void Render() const override;
};

// ============================================================

class DustEmitter : public Emitter
{
private:
    int m_texture_id{ -1 };

    std::mt19937 m_mt;

    std::uniform_real_distribution<float> m_lifetime_dist{ 0.9f, 1.4f };
    std::uniform_real_distribution<float> m_color_dist{ 0.0f, 1.0f };

public:
    DustEmitter(
        size_t capacity,
        const DirectX::XMVECTOR& position,
        double particles_per_second,
        bool is_emmit = false
    );

protected:
    Particle* createParticle() override;
};

#endif
